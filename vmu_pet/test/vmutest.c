/* Moemon VMU pet: two VMUs in libevmu, joined by their top connectors, run by a script (headless tests).
 *
 * usage: vmutest IMAGE_A IMAGE_B SCRIPT OUTDIR
 *   IMAGE_A / IMAGE_B: 128 KB VMU images (vmufs.py make); each unit starts its GAME.
 * Script lines (# comments):
 *   run SECS                    both units run SECS seconds of VMU time
 *   press UNIT BUTTONS SECS     hold buttons (letters: U D L R A B M S) on unit a or b for SECS, then let go
 *   cable on|off                the units joined or not (P7 and the serial lines)
 *   shot UNIT NAME              OUTDIR/NAME.pbm (48x32) and NAME.txt (# for a lit pixel)
 *   save UNIT PATH              the unit's flash as a VMU image
 *   expect UNIT LINE TEXT...    fail unless the screen's text line LINE (0-4, read back through the 3x5 font) is TEXT
 *   date UNIT Y M D H MI S      set the unit's clock
 *   vm2 UNIT                    the unit receives like a VM2 (bytes clocked slower than 1 ms a bit arrive garbled)
 *   rules on|off                real-hardware rules (default on): XRAM's missing bytes, VCCR bit 6, drawing on RC
 *
 * The link: libevmu keeps SCON/SBUF as plain registers (it emulates no serial port), so this does it. When a unit
 * starts a send (SCON0 bit 3), the byte takes 8 bit times, a bit time (256 - SBR) * 2 cycles; then its SCON0 says
 * done (bit 3 clear, bit 1 set) and, when the cable is on and the other unit is listening (SCON1 bit 3), the byte
 * lands in that unit's SBUF1 with its SCON1 done. A unit not listening loses the byte, as a real one would.
 */
#include <evmu/hw/evmu_device.h>
#include <evmu/hw/evmu_ram.h>
#include <evmu/hw/evmu_lcd.h>
#include <evmu/hw/evmu_gamepad.h>
#include <evmu/hw/evmu_cpu.h>
#include <evmu/hw/evmu_clock.h>
#include <evmu/hw/evmu_rom.h>
#include <evmu/hw/evmu_flash.h>
#include "hw/evmu_ram_.h"  /* the private RAM: XRAM's missing bytes are checked directly */
#include <evmu/hw/evmu_address_space.h>
#include <evmu/fs/evmu_file_manager.h>
#include <evmu/types/evmu_ibehavior.h>
#include <gimbal/utils/gimbal_date_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    EvmuDevice* dev;
    double      sendLeft;   /* seconds until the byte in flight is out, < 0 when none */
    double      bitTime;    /* the byte in flight's bit time */
    char        name;
    int         vm2;        /* "vm2 UNIT": receives like a VM2 (bytes clocked too slowly arrive garbled) */
    double      clock;      /* seconds run */
    unsigned char xramWas[2][128];
    int         ruleHoles, ruleCrystal, ruleDrawXt;
    int         crystalOpen;  /* checks in a row on the crystal with VCCR bit 6 clear */
} Unit;

static Unit  units[2];
static int   cable = 0;
static int   failures = 0;
static const char* outdir;

static EvmuWord sfr(Unit* u, EvmuAddress a) { return EvmuRam_readData(u->dev->pRam, a); }
static void setSfr(Unit* u, EvmuAddress a, EvmuWord v) { EvmuRam_writeData(u->dev->pRam, a, v); }

static void setCable(int on) {
    cable = on;
    for(int i = 0; i < 2; ++i) {
        /* P70 = 0 (no Dreamcast), P71 = 1 (battery fine), P72 = 0 and P73 = 1 when another VMU is there */
        EvmuWord p7 = 0x02 | (on ? 0x08 : 0x04);
        setSfr(&units[i], EVMU_ADDRESS_SFR_P7, p7);
    }
}

static double cycleSecs(Unit* u) {
    return EvmuClock_systemSecsPerCycle(u->dev->pClock);
}

/* A VM2's receiver loses track when the clock's bits come too far apart (seen on the user's VM2 with Sega's
 * crystal-clock rate: VMU -> VM2 bytes garbled, extra bytes). This much and more garbles a byte sent to a "vm2" unit. */
static const double kVm2MaxBit = 1e-3;

static void serial(Unit* u, Unit* other, double dt) {
    EvmuWord scon0 = sfr(u, EVMU_ADDRESS_SFR_SCON0);
    if(!(scon0 & 0x08)) { u->sendLeft = -1; return; }
    if(u->sendLeft < 0) {
        EvmuWord sbr = sfr(u, EVMU_ADDRESS_SFR_SBR);
        u->bitTime = (256 - sbr) * 2.0 * cycleSecs(u);
        u->sendLeft = 8.0 * u->bitTime;
        return;
    }
    u->sendLeft -= dt;
    if(u->sendLeft > 0) return;
    u->sendLeft = -1;
    setSfr(u, EVMU_ADDRESS_SFR_SCON0, (scon0 & ~0x08) | 0x02);
    if(!cable) return;
    EvmuWord scon1 = sfr(other, EVMU_ADDRESS_SFR_SCON1);
    if(!(scon1 & 0x08)) return;
    EvmuWord byte = sfr(u, EVMU_ADDRESS_SFR_SBUF0);
    if(other->vm2 && u->bitTime > kVm2MaxBit) byte = (byte * 7 + 0x5A) & 0xFF;  /* garbled */
    setSfr(other, EVMU_ADDRESS_SFR_SBUF1, byte);
    setSfr(other, EVMU_ADDRESS_SFR_SCON1, (scon1 & ~0x08) | 0x02);
}

/* Real hardware rules the emulator doesn't enforce (each found on the user's VMU, Sega's manuals give them):
 *  - XRAM's bytes with low digit C-F don't exist: writing them breaks a real VMU's screen up;
 *  - on the crystal with the screen on, VCCR bit 6 must be 1 (the CPU shut out of XRAM);
 *  - XRAM is only read or written on the RC clock;
 *  - VCCR is write-only and its bits 5-0 are always 0 (set1 / clr1 on it write them: the screen broke). */
static int g_rules = 1;

static void armRules(Unit* u) {
    EvmuRam_* r = EVMU_RAM_(u->dev->pRam);
    for(int b = 0; b < 2; ++b) {
        for(int a = 0; a < 128; ++a) if((a & 15) >= 12) r->xram[b][a] = 0xA5;
        memcpy(u->xramWas[b], r->xram[b], 128);
    }
}

static void checkRules(Unit* u) {
    if(!g_rules || u->clock < 0.05) return;  /* the BIOS's own state at the start */
    EvmuRam_* r = EVMU_RAM_(u->dev->pRam);
    EvmuWord ocr = sfr(u, EVMU_ADDRESS_SFR_OCR);
    EvmuWord vccr = r->sfr[EVMU_SFR_OFFSET(EVMU_ADDRESS_SFR_VCCR)];  /* write-only: what was written, not a read */
    int crystal = (ocr & 0x30) == 0x20;
    for(int b = 0; b < 2; ++b) {
        for(int a = 0; a < 128; ++a) {
            if((a & 15) >= 12 && r->xram[b][a] != 0xA5 && !u->ruleHoles++)
                fprintf(stderr, "rule: unit %c wrote XRAM's missing byte %03X (bank %d)\n", u->name, 0x180 + a, b);
            if(crystal && r->xram[b][a] != u->xramWas[b][a] && (a & 15) < 12 && !u->ruleDrawXt++)
                fprintf(stderr, "rule: unit %c changed XRAM on the crystal clock (at %.2f s)\n", u->name, u->clock);
        }
        memcpy(u->xramWas[b], r->xram[b], 128);
    }
    /* Sega's order going back to RC is VCCR first, then OCR: one instruction between (longer if an interrupt lands there) */
    u->crystalOpen = (crystal && !(vccr & 0x40)) ? u->crystalOpen + 1 : 0;
    if(u->crystalOpen > 500 && !u->ruleCrystal++)  /* 50 ms: not the documented gap, even with an interrupt in it */
        fprintf(stderr, "rule: unit %c on the crystal with VCCR bit 6 clear (at %.2f s; OCR %02X VCCR %02X PC %04X)\n", u->name, u->clock, ocr, vccr, (unsigned)EvmuCpu_pc(u->dev->pCpu));
    if((vccr & 0x3F) && !u->ruleCrystal++)
        fprintf(stderr, "rule: unit %c wrote VCCR's bits 5-0 (%02X; Sega: always 0; bit instructions on a write-only register do this)\n", u->name, vccr);
}

static void run(double secs) {
    const double slice = 100e-6;  /* finer than one crystal-clock instruction */
    for(double t = 0; t < secs; t += slice) {
        for(int i = 0; i < 2; ++i) {
            if(cable) setCable(1);  /* keep P7 (a reset or the program can't change the plug) */
            EvmuIBehavior_update(EVMU_IBEHAVIOR(units[i].dev), (EvmuTicks)(slice * 1e9));
            units[i].clock += slice;
            checkRules(&units[i]);
        }
        serial(&units[0], &units[1], slice);
        serial(&units[1], &units[0], slice);
    }
}

static Unit* unit(const char* s) { return &units[s[0] == 'b' ? 1 : 0]; }

static void setButtons(Unit* u, const char* keys, int down) {
    EvmuGamepad* g = u->dev->pGamepad;
    for(const char* k = keys; *k; ++k) switch(*k) {
        case 'U': g->up = down; break;     case 'D': g->down = down; break;
        case 'L': g->left = down; break;   case 'R': g->right = down; break;
        case 'A': g->a = down; break;      case 'B': g->b = down; break;
        case 'M': g->mode = down; break;   case 'S': g->sleep = down; break;
    }
}

static int pixel(Unit* u, int row, int col) { return EvmuLcd_pixel(u->dev->pLcd, col, row) ? 1 : 0; }  /* libevmu takes (x, y) */

static void shot(Unit* u, const char* name) {
    char path[1024];
    snprintf(path, sizeof path, "%s/%s.pbm", outdir, name);
    FILE* f = fopen(path, "w");
    fprintf(f, "P1\n48 32\n");
    for(int r = 0; r < 32; ++r) { for(int c = 0; c < 48; ++c) fprintf(f, "%d ", pixel(u, r, c)); fprintf(f, "\n"); }
    fclose(f);
    snprintf(path, sizeof path, "%s/%s.txt", outdir, name);
    f = fopen(path, "w");
    for(int r = 0; r < 32; ++r) { for(int c = 0; c < 48; ++c) fputc(pixel(u, r, c) ? '#' : '.', f); fputc('\n', f); }
    fclose(f);
}

/* the 3x5 font, as gen_font.py writes it, for reading text back off the screen */
#include "font_table.h"

static void readLine(Unit* u, int line, char* out) {
    for(int cell = 0; cell < 12; ++cell) {
        char best = '?';
        for(int ch = 32; ch < 96; ++ch) {
            int ok = 1;
            for(int r = 0; r < 6 && ok; ++r) for(int x = 0; x < 4 && ok; ++x) {
                int want = (r < 5 && x < 3) ? ((kFont[ch - 32][r] >> (7 - x)) & 1) : 0;
                if(pixel(u, line * 6 + r, cell * 4 + x) != want) ok = 0;
            }
            if(ok) { best = (char)ch; break; }
        }
        out[cell] = best;
    }
    int n = 12;
    while(n > 0 && out[n - 1] == ' ') --n;
    out[n] = 0;
}

static void saveFlash(Unit* u, const char* path) {
    static unsigned char buf[128 * 1024];
    size_t bytes = sizeof buf;
    EvmuFlash_readBytes(u->dev->pFlash, 0, buf, &bytes);
    FILE* f = fopen(path, "wb");
    fwrite(buf, 1, bytes, f);
    fclose(f);
}

static void setDate(Unit* u, int y, int mo, int d, int h, int mi, int s) {
    GblDateTime dt;
    GblDateTime_set(&dt, y, mo, d, h, mi, s, 0, 0);
    EvmuRom_setDateTime(u->dev->pRom, &dt);
}

int main(int argc, char** argv) {
    if(argc < 5) { fprintf(stderr, "usage: vmutest IMAGE_A IMAGE_B SCRIPT OUTDIR\n"); return 2; }
    outdir = argv[4];
    for(int i = 0; i < 2; ++i) {
        units[i].dev = EvmuDevice_create();
        units[i].name = 'a' + i;
        units[i].sendLeft = -1;
        if(!GBL_RESULT_SUCCESS(EvmuFileManager_load(units[i].dev->pFileMgr, argv[1 + i]))) {
            fprintf(stderr, "can't load %s\n", argv[1 + i]);
            return 2;
        }
        EvmuIBehavior_reset(EVMU_IBEHAVIOR(units[i].dev));
        armRules(&units[i]);
    }
    setCable(0);
    FILE* script = fopen(argv[3], "r");
    if(!script) { fprintf(stderr, "can't open %s\n", argv[3]); return 2; }
    char line[512];
    int lineNo = 0;
    while(fgets(line, sizeof line, script)) {
        ++lineNo;
        char* hash = strchr(line, '#');
        if(hash) *hash = 0;
        char cmd[32] = "", a1[256] = "", a2[256] = "", a3[256] = "";
        int n = sscanf(line, "%31s %255s %255s %255[^\n]", cmd, a1, a2, a3);
        if(n <= 0) continue;
        if(!strcmp(cmd, "run")) run(atof(a1));
        else if(!strcmp(cmd, "press")) {
            setButtons(unit(a1), a2, 1);
            run(atof(a3));
            setButtons(unit(a1), a2, 0);
        } else if(!strcmp(cmd, "cable")) setCable(!strcmp(a1, "on"));
        else if(!strcmp(cmd, "shot")) shot(unit(a1), a2);
        else if(!strcmp(cmd, "save")) saveFlash(unit(a1), a2);
        else if(!strcmp(cmd, "vm2")) unit(a1)->vm2 = 1;
        else if(!strcmp(cmd, "rules")) g_rules = !strcmp(a1, "on");
        else if(!strcmp(cmd, "expect")) {
            char got[16], want[300];
            snprintf(want, sizeof want, "%s", a3);
            int len = (int)strlen(want);
            while(len > 0 && (want[len - 1] == ' ' || want[len - 1] == '\r')) want[--len] = 0;
            readLine(unit(a1), atoi(a2), got);
            if(strcmp(got, want)) {
                fprintf(stderr, "line %d: unit %s line %s is \"%s\", expected \"%s\"\n", lineNo, a1, a2, got, want);
                ++failures;
            } else printf("ok: unit %s line %s \"%s\"\n", a1, a2, got);
        } else if(!strcmp(cmd, "date")) {
            int y, mo, d, h, mi, s;
            if(sscanf(line, "%*s %*s %d %d %d %d %d %d", &y, &mo, &d, &h, &mi, &s) == 6) setDate(unit(a1), y, mo, d, h, mi, s);
        } else { fprintf(stderr, "line %d: unknown command %s\n", lineNo, cmd); ++failures; }
    }
    fclose(script);
    for(int i = 0; i < 2; ++i)
        if(units[i].ruleHoles || units[i].ruleCrystal || units[i].ruleDrawXt) {
            fprintf(stderr, "unit %c broke real-hardware rules (see above)\n", units[i].name);
            ++failures;
        }
    printf(failures ? "FAILED (%d)\n" : "PASSED\n", failures);
    return failures ? 1 : 0;
}
