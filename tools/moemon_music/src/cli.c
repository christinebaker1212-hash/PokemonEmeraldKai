// Moemon Music Studio: command-line jobs (also what the tests use).
//   moemon_music_studio --render IN OUT.wav [SECONDS]     IN = .mid or .mmsproj
//   moemon_music_studio --raw IN OUT.f32 FRAMES          float samples, like the game's audio buffer
//   moemon_music_studio --export IN OUT.kaisong
//   moemon_music_studio --decomp IN DIR
//   moemon_music_studio --dump IN                         the song bytecode, one track a line
//   moemon_music_studio --dump-exact IN VOLUME            the same, converted like mid2agb -E -V VOLUME
//   moemon_music_studio --save IN OUT.mmsproj [SONG]      a project (SONG: the game song it replaces)
//   moemon_music_studio --original IN OUT.wav             a rip's own sound (its console's emulator)
// IN can also be a rip (PSF, PSF2, USF, GBS / .m3u, 2SF, GSF); MMS_BIOS = the PlayStation BIOS folder.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "studio.h"
#include "export.h"
#include "inst.h"
#include "rip.h"

static int ends_with(const char *s, const char *e)
{
    size_t a = strlen(s), b = strlen(e);
    if (a < b) return 0;
    for (size_t i = 0; i < b; i++) {
        char x = s[a - b + i], y = e[i];
        if (x >= 'A' && x <= 'Z') x += 32;
        if (x != y) return 0;
    }
    return 1;
}

int studio_open_track(Studio *s, const char *path, int gbsTrack);
int studio_open(Studio *s, const char *path) { return studio_open_track(s, path, -1); }

int studio_open_track(Studio *s, const char *path, int gbsTrack)
{
    char err[512];
    int ok = ends_with(path, ".mmsproj") ? project_load(&s->p, path, err, sizeof(err))
           : rip_kind_of(path) >= 0      ? project_load_rip(&s->p, path, g_biosDir, gbsTrack, err, sizeof(err))
                                         : project_load_midi(&s->p, path, err, sizeof(err));
    if (!ok) { snprintf(s->msg, sizeof(s->msg), "%s", err); return 0; }
    if (!studio_rebuild(s)) return 0;
    if (rip_kind_of(path) >= 0) studio_match_rip(s);
    if (err[0] && ends_with(path, ".mmsproj")) snprintf(s->msg, sizeof(s->msg), "%s", err);
    return 1;
}

static void w16(FILE *f, int v) { fputc(v & 255, f); fputc((v >> 8) & 255, f); }
static void w32(FILE *f, uint32_t v) { w16(f, (int)(v & 0xFFFF)); w16(f, (int)(v >> 16)); }

int cli_main(int argc, char **argv)
{
    static Studio s;
    static Engine e;
    const char *mode = argv[1];
    if (argc < 3) { fprintf(stderr, "usage: see the top of src/cli.c\n"); return 2; }
    inst_init();
    studio_init(&s);
    if (!studio_open(&s, argv[2])) { fprintf(stderr, "%s\n", s.msg); return 1; }
    char err[512] = {0};

    if (!strcmp(mode, "--dump-exact")) {
        s.p.gmFixes = 0;
        s.p.exactGate = 1;
        s.p.loopMode = LOOP_MARKERS;
        s.p.masterVol = argc >= 4 ? atoi(argv[3]) : 127;
        if (!studio_rebuild(&s)) { fprintf(stderr, "%s\n", s.msg); return 1; }
        mode = "--dump";
    }
    if (!strcmp(mode, "--dump")) {
        for (int t = 0; t < s.c.ntracks; t++) {
            printf("T%d", t + 1);
            for (int i = 0; i < s.c.len[t]; i++) printf(" %02x", s.c.data[t][i]);
            printf("\n");
        }
        return 0;
    }
    if (!strcmp(mode, "--export") && argc >= 4) {
        if (!export_kaisong(&s.p, &s.c, argv[3], err, sizeof(err))) { fprintf(stderr, "%s\n", err); return 1; }
        return 0;
    }
    if (!strcmp(mode, "--save") && argc >= 4) {
        if (argc >= 5) snprintf(s.p.target, sizeof(s.p.target), "%s", argv[4]);
        if (!project_save(&s.p, argv[3], err, sizeof(err))) { fprintf(stderr, "%s\n", err); return 1; }
        return 0;
    }
    if (!strcmp(mode, "--decomp") && argc >= 4) {
        if (!export_decomp(&s.p, &s.c, argv[3], err, sizeof(err))) { fprintf(stderr, "%s\n", err); return 1; }
        return 0;
    }
    if (!strcmp(mode, "--original") && argc >= 4) {
        const Rip *r = s.p.rip;
        if (!r) { fprintf(stderr, "%s isn't a rip (or its file is gone)\n", argv[2]); return 1; }
        FILE *f = fopen(argv[3], "wb");
        if (!f) { fprintf(stderr, "can't write %s\n", argv[3]); return 1; }
        uint32_t n = (uint32_t)r->pcmFrames;
        fwrite("RIFF", 1, 4, f); w32(f, 36 + n * 4); fwrite("WAVEfmt ", 1, 8, f);
        w32(f, 16); w16(f, 1); w16(f, 2); w32(f, (uint32_t)r->rate); w32(f, (uint32_t)r->rate * 4); w16(f, 4); w16(f, 16);
        fwrite("data", 1, 4, f); w32(f, n * 4);
        fwrite(r->pcm, 4, n, f);
        fclose(f);
        return 0;
    }
    if ((!strcmp(mode, "--render") || !strcmp(mode, "--raw")) && argc >= 4) {
        int raw = !strcmp(mode, "--raw");
        int frames = argc >= 5 ? atoi(argv[4]) : 0;
        if (!raw) frames = frames > 0 ? frames * 60 : (s.totalFrames > 0 ? s.totalFrames : 60 * 60);
        FILE *f = fopen(argv[3], "wb");
        if (!f) { fprintf(stderr, "can't write %s\n", argv[3]); return 1; }
        uint32_t samples = (uint32_t)frames * ENG_FRAME_SAMPLES;
        if (!raw) {
            fwrite("RIFF", 1, 4, f); w32(f, 36 + samples * 4); fwrite("WAVEfmt ", 1, 8, f);
            w32(f, 16); w16(f, 1); w16(f, 2); w32(f, ENG_RATE); w32(f, ENG_RATE * 4); w16(f, 4); w16(f, 16);
            fwrite("data", 1, 4, f); w32(f, samples * 4);
        }
        eng_init(&e);
        eng_start(&e, &s.song);
        if (getenv("MMS_SOLO")) e.muteMask = ~strtoull(getenv("MMS_SOLO"), NULL, 0);   // tests: only these tracks
        float buf[ENG_FRAME_SAMPLES * 2];
        for (int i = 0; i < frames; i++) {
            eng_frame(&e, buf);
            if (raw) {
                fwrite(buf, sizeof(float), ENG_FRAME_SAMPLES * 2, f);
            } else {
                for (int k = 0; k < ENG_FRAME_SAMPLES * 2; k++) {
                    float v = buf[k];
                    if (v > 1) v = 1;
                    if (v < -1) v = -1;
                    w16(f, (int)(v * 32767));
                }
            }
        }
        fclose(f);
        return 0;
    }
    fprintf(stderr, "unknown command %s\n", mode);
    return 2;
}
