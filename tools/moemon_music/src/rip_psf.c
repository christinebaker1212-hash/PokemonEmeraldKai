// Moemon Music Studio: PSF (PlayStation) and PSF2 (PlayStation 2) rips, played by Highly
// Experimental (third_party/he) with a BIOS made from the player's own PlayStation 2 BIOS
// (mkhebios). The capture hooks in spucore.c say when a voice starts; its registers say the rest.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include "rip.h"
#include "psflib.h"
#include "psf2fs.h"
#include "bios.h"
#include "psx.h"
#include "iop.h"
#include "r3000.h"
#include "spucore.h"
#include "mkhebios.h"

// ------------------------------------------------------------------ files for psflib
static void *pf_open(void *ctx, const char *path) { (void)ctx; return fopen(path, "rb"); }
static size_t pf_read(void *buf, size_t size, size_t n, void *h) { return fread(buf, size, n, (FILE *)h); }
static int pf_seek(void *h, int64_t ofs, int whence) { return fseek((FILE *)h, (long)ofs, whence); }
static int pf_close(void *h) { return fclose((FILE *)h); }
static long pf_tell(void *h) { return ftell((FILE *)h); }
const psf_file_callbacks rip_psf_files = {"\\/:", NULL, pf_open, pf_read, pf_seek, pf_close, pf_tell};

static int pf_info(void *ctx, const char *name, const char *value) { rip_psf_tag((Rip *)ctx, name, value); return 0; }

// ------------------------------------------------------------------ the BIOS
static void *g_hebios;
static int g_hebiosSize;

static int try_bios(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    if (n != 0x400000) { fclose(f); return 0; }
    fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc(n);
    if (!b || fread(b, 1, n, f) != (size_t)n) { free(b); fclose(f); return 0; }
    fclose(f);
    int size = (int)n;
    void *he = mkhebios_create(b, &size);
    free(b);
    if (!he) return 0;
    g_hebios = he;
    g_hebiosSize = size;
    return 1;
}

// Looks through biosDir (and the folder of the rip) for a 4 MB PlayStation 2 BIOS mkhebios can use.
static int find_bios(const char *biosDir, const char *ripPath) {
    if (g_hebios) return 1;
    char dirs[2][1024];
    snprintf(dirs[0], sizeof dirs[0], "%s", biosDir ? biosDir : "");
    snprintf(dirs[1], sizeof dirs[1], "%s", ripPath);
    char *slash = strrchr(dirs[1], '/');
    char *bslash = strrchr(dirs[1], '\\');
    if (bslash > slash) slash = bslash;
    if (slash) *slash = 0; else strcpy(dirs[1], ".");
    for (int d = 0; d < 2; d++) {
        if (!dirs[d][0]) continue;
        DIR *dir = opendir(dirs[d]);
        if (!dir) continue;
        struct dirent *de;
        while ((de = readdir(dir))) {
            char p[2048];
            snprintf(p, sizeof p, "%s/%s", dirs[d], de->d_name);
            struct stat st;
            if (stat(p, &st) || !S_ISREG(st.st_mode) || st.st_size != 0x400000) continue;
            if (try_bios(p)) { closedir(dir); return 1; }
        }
        closedir(dir);
    }
    return 0;
}

// ------------------------------------------------------------------ capture
typedef struct {
    int sample;              // RipSample of the note playing (-1 none)
    int keyon;               // key-ons since the last snapshot
    int pending;             // key-on not looked at yet
    uint32_t ssa, lsa;
} PsxVoice;

typedef struct {
    Rip *rip;
    int version;
    void *state;
    void *fs;                // PSF2 virtual file system
    void *core[2];           // spucore states, in the order they first render (core 0 renders first)
    uint16_t *ram[2];
    uint32_t memsize;
    int baseRate;            // 44100 (PS1) / 48000 (PS2): the rate at pitch 0x1000
    PsxVoice v[48];
    // PSF1 entry point (minipsf headers often don't have one)
    int first;
    uint32_t pc, sp;
    int refresh;
} Psx;

static Psx *g_psx;           // the hooks have no context pointer

static int core_index(void *state) {
    for (int i = 0; i < 2; i++) if (g_psx->core[i] == state) return i;
    for (int i = 0; i < 2; i++) if (!g_psx->core[i]) { g_psx->core[i] = state; return i; }
    return 0;
}

static void tap_render(void *state, uint16 *ram) {
    if (!g_psx) return;
    g_psx->ram[core_index(state)] = ram;
}

static void tap_keyon(void *state, int voice) {
    if (!g_psx) return;
    int c = core_index(state);
    PsxVoice *v = &g_psx->v[c * 24 + voice];
    v->keyon++;
    v->pending = 1;
    v->ssa = spucore_getreg_voice(state, voice, SPUREG_VOICE_SSA);
    v->lsa = spucore_getreg_voice(state, voice, SPUREG_VOICE_LSAX);
}

static const int k_f0[8] = {0, 60, 115, 98, 122, 0, 0, 0};
static const int k_f1[8] = {0, 0, -52, -55, -60, 0, 0, 0};

// The sample a voice starts at `ssa`: its ADPCM blocks up to the end flag, decoded.
static int psx_sample(Psx *p, int core, uint32_t ssa, uint32_t lsa) {
    const uint8_t *ram = (const uint8_t *)p->ram[core];
    if (!ram) ram = (const uint8_t *)p->ram[0];
    if (!ram) return -1;
    uint32_t mem = p->memsize;
    // the blocks and a hash of them
    uint32_t a = ssa & (mem - 1), end = a;
    int nblocks = 0, loopBlock = -1;
    for (;;) {
        if (end + 16 > mem) break;
        uint8_t fl = ram[end + 1];
        if ((fl & 4) && loopBlock < 0) loopBlock = nblocks;
        nblocks++;
        if (fl & 1) break;
        end += 16;
        if (nblocks > 65536) break;
    }
    uint32_t h = rip_hash(ram + a, nblocks * 16);
    int found = rip_find_sample(p->rip, ssa, h);
    if (found >= 0) return found;
    uint8_t lastFlags = ram[a + (nblocks - 1) * 16 + 1];
    int loops = (lastFlags & 3) == 3;
    if (loops && loopBlock < 0) {
        // no loop-start flag: the loop register says where (if it's inside the sample)
        uint32_t la = lsa & (mem - 1);
        if (la >= a && la < a + nblocks * 16u) loopBlock = (la - a) / 16;
        else loops = 0;
    }
    RipSample s;
    memset(&s, 0, sizeof s);
    s.kind = RIPS_PCM;
    s.len = nblocks * 28;
    s.pcm = malloc(sizeof(int16_t) * s.len);
    if (!s.pcm) return -1;
    int s1 = 0, s2 = 0;
    for (int b = 0; b < nblocks; b++) {
        const uint8_t *blk = ram + a + b * 16;
        int shift = blk[0] & 15, filt = (blk[0] >> 4) & 7;
        if (shift > 12) shift = 9;
        for (int i = 0; i < 28; i++) {
            int nib = (blk[2 + i / 2] >> ((i & 1) * 4)) & 15;
            int v = (int16_t)(nib << 12) >> shift;
            v += (s1 * k_f0[filt] + s2 * k_f1[filt] + 32) >> 6;
            if (v > 32767) v = 32767;
            if (v < -32768) v = -32768;
            s.pcm[b * 28 + i] = (int16_t)v;
            s2 = s1;
            s1 = v;
        }
    }
    s.rate = p->baseRate;
    s.loops = loops;
    s.loopStart = loops ? loopBlock * 28 : 0;
    s.id = ssa;
    s.hash = h;
    int idx = rip_add_sample(p->rip, &s);
    free(s.pcm);
    return idx;
}

static float vol_level(void *core, int voice, int reg) {
    int v = (int16_t)spucore_getreg_voice(core, voice, reg);
    return fabsf((float)v) / 32768.0f;
}

static void psx_snapshot(RipEmu *e, RipVF *out) {
    Psx *p = e->ctx;
    int ncores = p->version == 2 ? 2 : 1;
    for (int c = 0; c < ncores; c++) {
        void *core = p->core[c];
        float mvl = 1, mvr = 1, ext = 1;
        if (core) {
            mvl = fabsf((float)(int16_t)spucore_getreg(core, SPUREG_MVOLXL)) / 32768.0f;
            mvr = fabsf((float)(int16_t)spucore_getreg(core, SPUREG_MVOLXR)) / 32768.0f;
        }
        float extl = 1, extr = 1;
        if (p->version == 2 && c == 0 && p->core[1]) {
            // core 0 is heard through core 1's sound input and master volume
            void *c1 = p->core[1];
            extl = fabsf((float)(int16_t)spucore_getreg(c1, SPUREG_AVOLL)) / 32768.0f *
                   fabsf((float)(int16_t)spucore_getreg(c1, SPUREG_MVOLXL)) / 32768.0f;
            extr = fabsf((float)(int16_t)spucore_getreg(c1, SPUREG_AVOLR)) / 32768.0f *
                   fabsf((float)(int16_t)spucore_getreg(c1, SPUREG_MVOLXR)) / 32768.0f;
        }
        (void)ext;
        uint32_t vmixl = core ? spucore_getreg(core, SPUREG_VMIXL) : 0;
        uint32_t vmixr = core ? spucore_getreg(core, SPUREG_VMIXR) : 0;
        uint32_t vmixe = core ? spucore_getreg(core, SPUREG_VMIXE) : 0;
        uint32_t noise = core ? spucore_getreg(core, SPUREG_NOISE) : 0;
        for (int i = 0; i < 24; i++) {
            PsxVoice *v = &p->v[c * 24 + i];
            RipVF *o = &out[c * 24 + i];
            memset(o, 0, sizeof *o);
            o->sample = -1;
            if (!core) continue;
            if (v->pending) {
                v->pending = 0;
                if (noise & (1u << i)) {
                    RipSample s;
                    memset(&s, 0, sizeof s);
                    s.kind = RIPS_NOISE;
                    s.id = 0xFFFFFFFF;
                    s.rate = p->baseRate;
                    v->sample = rip_add_sample(p->rip, &s);
                } else {
                    v->sample = psx_sample(p, c, v->ssa, v->lsa);
                }
            }
            o->keyon = v->keyon > 0;
            v->keyon = 0;
            int env = (int)spucore_getreg_voice(core, i, SPUREG_VOICE_ENVX);
            if (env <= 0 || v->sample < 0) { if (!o->keyon) continue; }
            float ev = env / 32768.0f;
            int pitch = (int)spucore_getreg_voice(core, i, SPUREG_VOICE_PITCH);
            if (pitch > 0x3FFF) pitch = 0x3FFF;
            o->sample = (int16_t)v->sample;
            o->rate = pitch / 4096.0f * p->baseRate;
            float l = ev * vol_level(core, i, SPUREG_VOICE_VOLXL) * mvl * 0.5f;
            float r = ev * vol_level(core, i, SPUREG_VOICE_VOLXR) * mvr * 0.5f;
            if (p->version == 2 && c == 0) { l *= extl; r *= extr; }
            if (!(vmixl & (1u << i))) l = 0;
            if (!(vmixr & (1u << i))) r = 0;
            o->l = l;
            o->r = r;
            if (vmixe & (1u << i)) o->flags |= RIPV_REVERB;
        }
    }
}

static int psx_render(RipEmu *e, int16_t *buf, int n) {
    Psx *p = e->ctx;
    while (n > 0) {
        uint32_t got = (uint32_t)n;
        sint32 r = psx_execute(p->state, 0x7FFFFFFF, buf, &got, 0);
        if (r < 0) {
            // the program stopped: the rest is silence
            memset(buf, 0, (size_t)n * 4);
            return 0;
        }
        buf += got * 2;
        n -= (int)got;
    }
    return 0;
}

static void psx_close(RipEmu *e) {
    Psx *p = e->ctx;
    if (!p) return;
    spucore_tap_keyon = NULL;
    spucore_tap_render = NULL;
    if (g_psx == p) g_psx = NULL;
    if (p->fs) psf2fs_delete(p->fs);
    free(p->state);
    free(p);
    e->ctx = NULL;
}

static int load1(void *ctx, const uint8_t *exe, size_t n, const uint8_t *res, size_t rn) {
    Psx *p = ctx;
    (void)res; (void)rn;
    if (n < 0x800) return -1;
    if (p->first) {
        memcpy(&p->pc, exe + 0x10, 4);
        memcpy(&p->sp, exe + 0x30, 4);
        p->first = 0;
    }
    return psx_upload_psxexe(p->state, (void *)exe, (uint32_t)n) < 0 ? -1 : 0;
}

static int g_heInit;

int rip_open_psf(RipEmu *e, Rip *r, const char *biosDir, char *err, int errlen) {
    if (!find_bios(biosDir, r->path)) {
        snprintf(err, errlen, "PlayStation music needs a PlayStation 2 BIOS (a 4 MB file such as scph39001.bin). "
                 "Choose the folder that has it under Display > BIOS folder, or put it next to the music.");
        return 0;
    }
    if (!g_heInit) {
        bios_set_image(g_hebios, (uint32)g_hebiosSize);
        if (psx_init()) { snprintf(err, errlen, "the PlayStation emulator didn't start"); return 0; }
        g_heInit = 1;
    }
    Psx *p = calloc(1, sizeof *p);
    if (!p) return 0;
    p->rip = r;
    p->version = r->kind == RIP_PSF2 ? 2 : 1;
    p->state = malloc(psx_get_state_size(p->version));
    if (!p->state) { free(p); return 0; }
    psx_clear_state(p->state, p->version);
    p->first = 1;
    for (int i = 0; i < 48; i++) p->v[i].sample = -1;
    if (p->version == 1) {
        if (psf_load(r->path, &rip_psf_files, 1, load1, p, pf_info, r, 0, NULL, NULL) < 0) {
            snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
            free(p->state); free(p);
            return 0;
        }
        void *cpu = iop_get_r3000_state(psx_get_iop_state(p->state));
        r3000_setreg(cpu, R3000_REG_PC, p->pc);
        r3000_setreg(cpu, R3000_REG_GEN + 29, p->sp);
        p->memsize = 0x80000;
        p->baseRate = 44100;
        e->rate = 44100;
        e->nvoices = 24;
    } else {
        p->fs = psf2fs_create();
        if (!p->fs || psf_load(r->path, &rip_psf_files, 2, psf2fs_load_callback, p->fs, pf_info, r, 0, NULL, NULL) < 0) {
            snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
            if (p->fs) psf2fs_delete(p->fs);
            free(p->state); free(p);
            return 0;
        }
        psx_set_readfile(p->state, (psx_readfile_t)psf2fs_virtual_readfile, p->fs);
        p->memsize = 0x200000;
        p->baseRate = 48000;
        e->rate = 48000;
        e->nvoices = 48;
    }
    g_psx = p;
    spucore_tap_keyon = tap_keyon;
    spucore_tap_render = tap_render;
    snprintf(r->console, sizeof r->console, p->version == 1 ? "PlayStation" : "PlayStation 2");
    e->ctx = p;
    e->rip = r;
    e->render = psx_render;
    e->snapshot = psx_snapshot;
    e->close = psx_close;
    return 1;
}
