// Moemon Music Studio: loading a rip and recording what its sound chip does, frame by frame.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <fenv.h>
#include "rip.h"
#include "engine.h"

static const char *const k_kindNames[RIP_NUM_KINDS] = {"PSF", "PSF2", "USF", "GBS", "2SF", "GSF"};

const char *rip_kind_name(int kind) { return kind >= 0 && kind < RIP_NUM_KINDS ? k_kindNames[kind] : "?"; }
int rip_needs_bios(int kind) { return kind == RIP_PSF1 || kind == RIP_PSF2; }

static int ext_is(const char *path, const char *ext) {
    const char *dot = strrchr(path, '.');
    if (!dot) return 0;
    dot++;
    while (*dot && *ext) {
        if (tolower((unsigned char)*dot) != *ext) return 0;
        dot++;
        ext++;
    }
    return !*dot && !*ext;
}

int rip_kind_of(const char *path) {
    if (ext_is(path, "psf") || ext_is(path, "minipsf") || ext_is(path, "psflib")) return RIP_PSF1;
    if (ext_is(path, "psf2") || ext_is(path, "minipsf2") || ext_is(path, "psf2lib")) return RIP_PSF2;
    if (ext_is(path, "usf") || ext_is(path, "miniusf") || ext_is(path, "usflib")) return RIP_USF;
    if (ext_is(path, "gbs")) return RIP_GBS;
    if (ext_is(path, "m3u")) {
        // a Game Boy playlist line names its .gbs ("file.gbs::GBS,..."); other playlists aren't rips
        FILE *f = fopen(path, "rb");
        char buf[4096];
        size_t n = f ? fread(buf, 1, sizeof buf - 1, f) : 0;
        if (f) fclose(f);
        buf[n] = 0;
        for (char *p = buf; *p; p++) *p = (char)toupper((unsigned char)*p);
        return strstr(buf, "::GBS") ? RIP_GBS : -1;
    }
    if (ext_is(path, "2sf") || ext_is(path, "mini2sf") || ext_is(path, "2sflib")) return RIP_2SF;
    if (ext_is(path, "gsf") || ext_is(path, "minigsf") || ext_is(path, "gsflib")) return RIP_GSF;
    return -1;
}

// "3:25.5", "205", "1:02:03"
double rip_parse_time(const char *s) {
    double t = 0, part = 0, frac = 0, div = 1;
    int inFrac = 0, any = 0;
    for (; *s; s++) {
        if (*s >= '0' && *s <= '9') {
            any = 1;
            if (inFrac) { div *= 10; frac += (*s - '0') / div; }
            else part = part * 10 + (*s - '0');
        } else if (*s == ':') {
            t = (t + part) * 60;
            part = 0;
        } else if (*s == '.' || *s == ',') {
            inFrac = 1;
        } else if (*s == ' ') {
            continue;
        } else break;
    }
    return any ? t + part + frac : 0;
}

static void copy_tag(char *dst, size_t n, const char *v) {
    snprintf(dst, n, "%s", v);
}

int rip_psf_tag(Rip *r, const char *name, const char *value) {
    if (!strcmp(name, "title")) copy_tag(r->title, sizeof r->title, value);
    else if (!strcmp(name, "artist")) copy_tag(r->artist, sizeof r->artist, value);
    else if (!strcmp(name, "game")) copy_tag(r->game, sizeof r->game, value);
    else if (!strcmp(name, "year")) copy_tag(r->year, sizeof r->year, value);
    else if (!strcmp(name, "copyright")) copy_tag(r->copyright, sizeof r->copyright, value);
    else if (!strcmp(name, "psfby") || !strcmp(name, "usfby") || !strcmp(name, "2sfby") || !strcmp(name, "gsfby"))
        copy_tag(r->ripper, sizeof r->ripper, value);
    else if (!strcmp(name, "length")) r->length = rip_parse_time(value);
    else if (!strcmp(name, "fade")) r->fade = rip_parse_time(value);
    return 0;
}

uint32_t rip_hash(const void *data, int len) {
    const uint8_t *p = data;
    uint32_t h = 2166136261u;
    for (int i = 0; i < len; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

int rip_find_sample(const Rip *r, uint32_t id, uint32_t hash) {
    for (int i = 0; i < r->nsamples; i++)
        if (r->samples[i].id == id && r->samples[i].hash == hash) return i;
    return -1;
}

int rip_add_sample(Rip *r, const RipSample *s) {
    uint32_t hash = s->hash;
    if (!hash) {
        hash = s->pcm ? rip_hash(s->pcm, s->len * 2) : rip_hash(s->wave, 32);
        hash ^= (uint32_t)s->kind * 0x9E3779B9u ^ (uint32_t)s->param;
    }
    int f = rip_find_sample(r, s->id, hash);
    if (f >= 0) return f;
    if (r->nsamples == r->capSamples) {
        int cap = r->capSamples ? r->capSamples * 2 : 64;
        RipSample *ns = realloc(r->samples, sizeof *ns * cap);
        if (!ns) return -1;
        r->samples = ns;
        r->capSamples = cap;
    }
    RipSample *d = &r->samples[r->nsamples];
    *d = *s;
    d->hash = hash;
    d->rootKey = 60;
    if (s->pcm && s->len > 0) {
        d->pcm = malloc(sizeof(int16_t) * s->len);
        if (!d->pcm) return -1;
        memcpy(d->pcm, s->pcm, sizeof(int16_t) * s->len);
    } else {
        d->pcm = NULL;
    }
    snprintf(d->name, sizeof d->name, "s%02d", r->nsamples);
    return r->nsamples++;
}

void rip_free(Rip *r) {
    for (int i = 0; i < r->nsamples; i++) free(r->samples[i].pcm);
    free(r->samples);
    free(r->vf);
    free(r->pcm);
    free(r->rom);
    memset(r, 0, sizeof *r);
}

static int rip_load_inner(Rip *r, const char *path, const char *biosDir, int gbsTrack, int seconds, char *err, int errlen);

// The emulators change the CPU's floating-point rounding (the N64 core sets the N64's own); the game
// engine's mixing must not inherit it, so the environment is put back afterwards.
int rip_load(Rip *r, const char *path, const char *biosDir, int gbsTrack, int seconds, char *err, int errlen) {
    fenv_t env;
    fegetenv(&env);
    int ok = rip_load_inner(r, path, biosDir, gbsTrack, seconds, err, errlen);
    fesetenv(&env);
    return ok;
}

static int rip_load_inner(Rip *r, const char *path, const char *biosDir, int gbsTrack, int seconds, char *err, int errlen) {
    memset(r, 0, sizeof *r);
    r->kind = rip_kind_of(path);
    snprintf(r->path, sizeof r->path, "%s", path);
    if (r->kind < 0) { snprintf(err, errlen, "%s isn't a music rip this studio knows", path); return 0; }
    RipEmu e;
    memset(&e, 0, sizeof e);
    int ok = 0;
    switch (r->kind) {
    case RIP_PSF1: case RIP_PSF2: ok = rip_open_psf(&e, r, biosDir, err, errlen); break;
    case RIP_USF: ok = rip_open_usf(&e, r, err, errlen); break;
    case RIP_GBS: ok = rip_open_gbs(&e, r, gbsTrack, err, errlen); break;
    case RIP_2SF: ok = rip_open_2sf(&e, r, err, errlen); break;
    case RIP_GSF: ok = rip_open_gsf(&e, r, err, errlen); break;
    }
    if (!ok) {
        if (!err[0]) snprintf(err, errlen, "couldn't play %s", path);
        rip_free(r);
        return 0;
    }
    double secs = r->length > 0 ? r->length + r->fade : (seconds > 0 ? seconds : 180);
    if (secs > RIP_MAX_SECONDS) secs = RIP_MAX_SECONDS;
    if (secs < 1) secs = 1;
    // one row per game frame
    double fps = (double)ENG_RATE / ENG_FRAME_SAMPLES;
    int nframes = (int)ceil(secs * fps);
    r->rate = e.rate;
    r->nvoices = e.nvoices;
    r->nframes = nframes;
    long total = (long)ceil((double)nframes * e.rate / fps);
    r->pcm = malloc(sizeof(int16_t) * 2 * (size_t)(total + 16));
    r->vf = calloc((size_t)nframes * e.nvoices, sizeof(RipVF));
    if (!r->pcm || !r->vf) {
        snprintf(err, errlen, "out of memory");
        e.close(&e);
        rip_free(r);
        return 0;
    }
    long pos = 0;
    for (int f = 0; f < nframes; f++) {
        long upto = (long)llround((double)(f + 1) * e.rate / fps);
        if (upto > total) upto = total;
        if (upto > pos) {
            if (e.render(&e, r->pcm + pos * 2, (int)(upto - pos))) {
                snprintf(err, errlen, "the emulator stopped at %.1f s", pos / (double)e.rate);
                e.close(&e);
                rip_free(r);
                return 0;
            }
            pos = upto;
        }
        e.snapshot(&e, r->vf + (size_t)f * e.nvoices);
    }
    r->pcmFrames = (int)pos;
    if (e.finalize) e.finalize(&e);
    e.close(&e);
    // the tags' fade-out, for the original-instruments player
    if (r->length > 0 && r->fade > 0) {
        long fs = (long)(r->length * e.rate), fe = (long)((r->length + r->fade) * e.rate);
        for (long i = fs; i < r->pcmFrames; i++) {
            float g = i >= fe ? 0.0f : 1.0f - (float)(i - fs) / (float)(fe - fs);
            r->pcm[i * 2] = (int16_t)(r->pcm[i * 2] * g);
            r->pcm[i * 2 + 1] = (int16_t)(r->pcm[i * 2 + 1] * g);
        }
    }
    if (!r->title[0]) {
        const char *b = strrchr(path, '/');
        const char *b2 = strrchr(path, '\\');
        if (b2 > b) b = b2;
        snprintf(r->title, sizeof r->title, "%s", b ? b + 1 : path);
        char *dot = strrchr(r->title, '.');
        if (dot) *dot = 0;
    }
    return 1;
}
