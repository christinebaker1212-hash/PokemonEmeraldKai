/*
 * Moemon Fight: runs the fight engine without the game (Linux), for testing the port.
 *   mf_selftest SCRIPT OUTDIR
 * SCRIPT lines: "begin cpu|versus SEED", "step N [P1HEX [P2HEX]]" (buttons held for N frames),
 * "shot NAME" (OUTDIR/NAME.ppm of the current frame), "wait_end MAX" (steps until the session ends),
 * "result" (prints MfEngine_Result), "audio NAME" (from now on the sound of every step goes to
 * OUTDIR/NAME.wav, 42048 Hz stereo like the game's), "shots PREFIX N" (a shot every frame for N steps).
 * Run from the repository root (mugen/ is read from there).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mf_engine.h"

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
            sscanf(line, "%*s %63s %u", arg, &a);
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
                MfEngine_Step(buttons);
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
