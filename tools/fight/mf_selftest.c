/*
 * Moemon Fight: runs the fight engine without the game (Linux), for testing the port.
 *   mf_selftest SCRIPT OUTDIR
 * SCRIPT lines: "begin cpu|versus SEED", "step N [P1HEX [P2HEX]]" (buttons held for N frames),
 * "shot NAME" (OUTDIR/NAME.ppm of the current frame), "wait_end MAX" (steps until the session ends),
 * "result" (prints MfEngine_Result), "audio NAME" (from now on the sound of every step goes to
 * OUTDIR/NAME.wav, 42048 Hz stereo like the game's), "shots PREFIX N" (a shot every frame for N steps).
 * Rollback: "begin versus SEED rollback" keeps the fight's frames; "wait_kept MAX" steps until they're kept;
 * "fight N BACK [SEED]" plays N frames with made-up buttons for both sides (the same for the same frame and
 * SEED), printing MfEngine_Hash every 60 frames; with BACK > 0, every 7 frames it goes BACK frames back, runs
 * them again with other buttons, goes back again and runs them with the right ones (a misprediction and its
 * correction): the hashes must be those of BACK 0.
 * Run from the repository root (mugen/ is read from there).
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef __GLIBC__
#include <malloc.h>
#endif

#include "mf_engine.h"
#include "../../fight/host/mf_rollback.h"

static FILE *sWav;
static long sWavFrames;
static double sAudioDue;

static void Wav_Header(FILE *fp, long frames)
{
    unsigned int data = (unsigned int)frames * 4, rate = 42048, byterate = 42048 * 4;
    unsigned char h[44];
    memcpy(h, "RIFF", 4);
    h[4] = (unsigned char)(data + 36); h[5] = (unsigned char)((data + 36) >> 8);
    h[6] = (unsigned char)((data + 36) >> 16); h[7] = (unsigned char)((data + 36) >> 24);
    memcpy(h + 8, "WAVEfmt ", 8);
    h[16] = 16; h[17] = h[18] = h[19] = 0;
    h[20] = 1; h[21] = 0; h[22] = 2; h[23] = 0;
    h[24] = (unsigned char)rate; h[25] = (unsigned char)(rate >> 8); h[26] = (unsigned char)(rate >> 16); h[27] = 0;
    h[28] = (unsigned char)byterate; h[29] = (unsigned char)(byterate >> 8); h[30] = (unsigned char)(byterate >> 16); h[31] = 0;
    h[32] = 4; h[33] = 0; h[34] = 16; h[35] = 0;
    memcpy(h + 36, "data", 4);
    h[40] = (unsigned char)data; h[41] = (unsigned char)(data >> 8); h[42] = (unsigned char)(data >> 16);
    h[43] = (unsigned char)(data >> 24);
    fseek(fp, 0, SEEK_SET);
    fwrite(h, 1, 44, fp);
    fseek(fp, 0, SEEK_END);
}

/* one game frame of sound (42048 / 60 frames) */
static void Audio_Step(void)
{
    static short buf[2048 * 2];
    int n;

    if (sWav == NULL) {
        return;
    }
    sAudioDue += 42048.0 / 60.0;
    n = (int)sAudioDue;
    sAudioDue -= n;
    memset(buf, 0, sizeof(buf));
    MfEngine_MixAudio(buf, n, 42048);
    fwrite(buf, 4, (size_t)n, sWav);
    sWavFrames += n;
}

/* Made-up buttons for a frame: directions and attacks, never start */
static void Fight_Buttons(unsigned int frame, unsigned int seed, unsigned short out[2])
{
    int p;

    for (p = 0; p < 2; p++) {
        unsigned int x = (frame / 4) * 2654435761u ^ (seed + (unsigned int)p) * 40503u;
        x ^= x >> 13;
        x *= 0x5bd1e995u;
        x ^= x >> 15;
        out[p] = (unsigned short)(x & 0x3FF);
        if ((out[p] & 0x3) == 0x3) {
            out[p] &= ~0x1;     /* not up and down together */
        }
        if ((out[p] & 0xC) == 0xC) {
            out[p] &= ~0x4;
        }
    }
}

static void Shot(const char *dir, const char *name)
{
    char path[512];
    const unsigned short *f = MfEngine_Frame();
    FILE *fp;
    int i;

    snprintf(path, sizeof(path), "%s/%s.ppm", dir, name);
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return;
    }
    fprintf(fp, "P6\n%d %d\n255\n", MF_SCREEN_W, MF_SCREEN_H);
    for (i = 0; i < MF_SCREEN_W * MF_SCREEN_H; i++) {
        unsigned short p = f ? f[i] : 0;
        unsigned char rgb[3] = { (unsigned char)((p >> 11) << 3), (unsigned char)(((p >> 5) & 63) << 2),
                                 (unsigned char)((p & 31) << 3) };
        fwrite(rgb, 1, 3, fp);
    }
    fclose(fp);
    printf("shot %s (frame %s)\n", path, f ? "yes" : "none");
}

int main(int argc, char **argv)
{
    char line[256];
    FILE *script;
    long frame = 0;

    if (argc < 3 || (script = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "usage: mf_selftest SCRIPT OUTDIR\n");
        return 2;
    }
    while (fgets(line, sizeof(line), script) != NULL) {
        char cmd[32], arg[64];
        unsigned int a = 0, b = 0;
        long n = 0;
        if (sscanf(line, "%31s", cmd) != 1 || cmd[0] == '#') {
            continue;
        }
        if (strcmp(cmd, "begin") == 0) {
            struct MfSession s;
            memset(&s, 0, sizeof(s));
            arg[0] = '\0';
            {
                char opt[32] = "";
                sscanf(line, "%*s %63s %u %31s", arg, &a, opt);
                s.rollback = strcmp(opt, "rollback") == 0;
            }
            s.mode = strcmp(arg, "cpu") == 0 ? MF_MODE_CPU : MF_MODE_VERSUS;
            s.seed = a;
            snprintf(s.cabinet, sizeof(s.cabinet), "mauville_gc");
            printf("begin %s -> %d\n", arg, MfEngine_Begin(&s));
        } else if (strcmp(cmd, "step") == 0) {
            unsigned short buttons[2];
            sscanf(line, "%*s %ld %x %x", &n, &a, &b);
            buttons[0] = (unsigned short)a;
            buttons[1] = (unsigned short)b;
            while (n-- > 0) {
                while (!MfEngine_Step(buttons)) {
                    usleep(1000);   /* still loading: the buttons are for the next frame it runs */
                }
                Audio_Step();
                frame++;
            }
        } else if (strcmp(cmd, "shot") == 0) {
            sscanf(line, "%*s %63s", arg);
            Shot(argv[2], arg);
        } else if (strcmp(cmd, "wait_end") == 0) {
            unsigned short none[2] = { 0, 0 };
            sscanf(line, "%*s %ld", &n);
            while (n-- > 0 && MfEngine_IsActive()) {
                MfEngine_Step(none);
                Audio_Step();
                frame++;
            }
            printf("wait_end: active=%d at frame %ld\n", MfEngine_IsActive(), frame);
        } else if (strcmp(cmd, "audio") == 0) {
            char path[512];
            sscanf(line, "%*s %63s", arg);
            snprintf(path, sizeof(path), "%s/%s.wav", argv[2], arg);
            sWav = fopen(path, "wb");
            if (sWav != NULL) {
                Wav_Header(sWav, 0);
            }
        } else if (strcmp(cmd, "shots") == 0) {
            unsigned short none[2] = { 0, 0 };
            char name[96];
            long i;
            sscanf(line, "%*s %63s %ld", arg, &n);
            for (i = 0; i < n; i++) {
                MfEngine_Step(none);
                Audio_Step();
                frame++;
                snprintf(name, sizeof(name), "%s%03ld", arg, i);
                Shot(argv[2], name);
            }
        } else if (strcmp(cmd, "mem") == 0) {
#ifdef __GLIBC__
            struct mallinfo2 mi = mallinfo2();
            printf("mem: %zu KB in use, %zu KB mmapped\n", mi.uordblks / 1024, mi.hblkhd / 1024);
#endif
        } else if (strcmp(cmd, "wait_kept") == 0) {
            unsigned short none[2] = { 0, 0 };
            unsigned int oldest = 0;
            sscanf(line, "%*s %ld", &n);
            while (n-- > 0 && MfEngine_IsActive() && !MfEngine_Kept(&oldest)) {
                MfEngine_Step(none);
                frame++;
            }
            printf("wait_kept: kept=%d frame %u\n", MfEngine_Kept(&oldest), MfEngine_FrameCount());
        } else if (strcmp(cmd, "fight") == 0) {
            unsigned int back = 0, seed = 1, every = 60, i, k, start = MfEngine_FrameCount();
            unsigned short buttons[2], wrong[2];
            int failed = 0;
            sscanf(line, "%*s %ld %u %u %u", &n, &back, &seed, &every);
            for (i = 0; i < (unsigned int)n && MfEngine_IsActive(); i++) {
                unsigned int f = MfEngine_FrameCount();   /* frames done: this step runs frame f + 1 */
                Fight_Buttons(f + 1, seed, buttons);
                MfEngine_Step(buttons);
                frame++;
                if (back > 0 && i % 7 == 6 && MfEngine_Kept(NULL)) {
                    unsigned int now = MfEngine_FrameCount(), to = now - back;
                    if (!MfEngine_RollBack(to)) {
                        failed++;
                        continue;
                    }
                    for (k = to + 1; k <= now; k++) {
                        Fight_Buttons(k, seed + 99, wrong);
                        MfEngine_StepQuiet(wrong);
                    }
                    if (!MfEngine_RollBack(to)) {
                        failed++;
                        continue;
                    }
                    for (k = to + 1; k <= now; k++) {
                        Fight_Buttons(k, seed, buttons);
                        MfEngine_StepQuiet(buttons);
                    }
                }
                if (every > 0 && (MfEngine_FrameCount() - start) % every == 0) {
                    printf("hash %u %08x\n", MfEngine_FrameCount(), MfEngine_Hash());
                }
            }
            printf("fight: %ld frames, %d failed rollbacks, now frame %u\n", n, failed, MfEngine_FrameCount());
        } else if (strcmp(cmd, "hash") == 0) {
            printf("hash %u %08x\n", MfEngine_FrameCount(), MfEngine_Hash());
        } else if (strcmp(cmd, "hashes") == 0) {
            unsigned short none[2] = { 0, 0 };
            sscanf(line, "%*s %ld", &n);
            while (n-- > 0 && MfEngine_IsActive()) {
                MfEngine_Step(none);
                frame++;
                printf("hash %u %08x\n", MfEngine_FrameCount(), MfEngine_Hash());
            }
        } else if (strcmp(cmd, "rbstats") == 0) {
            MfRbStats st;
            MfRollback_Stats(&st);
            printf("rbstats: %lu frames kept, %.1f pages a frame (most %lu), %lu rewinds (%lu frames back), "
                   "globals %zu bytes, heap copy %zu KB\n", st.frames, st.frames ? (double)st.pages / st.frames : 0.0,
                   st.most_pages, st.rewinds, st.frames_back, st.globals, st.shadow / 1024);
        } else if (strcmp(cmd, "result") == 0) {
            printf("result %d error \"%s\"\n", MfEngine_Result(), MfEngine_Error());
        }
        fflush(stdout);
    }
    if (sWav != NULL) {
        Wav_Header(sWav, sWavFrames);
        fclose(sWav);
    }
    printf("frames %ld result %d active %d\n", frame, MfEngine_Result(), MfEngine_IsActive());
    return 0;
}
