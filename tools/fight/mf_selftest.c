/*
 * Moemon Fight: runs the fight engine without the game (Linux), for testing the port.
 *   mf_selftest SCRIPT OUTDIR
 * SCRIPT lines: "begin cpu|versus SEED", "step N [P1HEX [P2HEX]]" (buttons held for N frames),
 * "shot NAME" (OUTDIR/NAME.ppm of the current frame), "wait_end MAX" (steps until the session ends),
 * "result" (prints MfEngine_Result). Run from the repository root (mugen/ is read from there).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mf_engine.h"

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
                frame++;
            }
            printf("wait_end: active=%d at frame %ld\n", MfEngine_IsActive(), frame);
        } else if (strcmp(cmd, "result") == 0) {
            printf("result %d error \"%s\"\n", MfEngine_Result(), MfEngine_Error());
        }
        fflush(stdout);
    }
    printf("frames %ld result %d active %d\n", frame, MfEngine_Result(), MfEngine_IsActive());
    return 0;
}
