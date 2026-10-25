// Moemon Music Studio: USF (Nintendo 64) rips, played by lazyusf2 (third_party/lazyusf2).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "rip.h"
#include "psflib.h"
#include "usf/usf.h"

extern const psf_file_callbacks rip_psf_files;

typedef struct {
    Rip *rip;
    void *state;
    int compare, fifo;
} Usf;

static int usf_load(void *ctx, const uint8_t *exe, size_t en, const uint8_t *res, size_t rn) {
    Usf *u = ctx;
    if (exe && en) return -1;
    return usf_upload_section(u->state, res, rn);
}

static int usf_info(void *ctx, const char *name, const char *value) {
    Usf *u = ctx;
    if (!strcasecmp(name, "_enablecompare") && *value) u->compare = 1;
    else if (!strcasecmp(name, "_enablefifofull") && *value) u->fifo = 1;
    else rip_psf_tag(u->rip, name, value);
    return 0;
}

static int usf_render_cb(RipEmu *e, int16_t *buf, int n) {
    Usf *u = e->ctx;
    const char *err = usf_render_resampled(u->state, buf, (size_t)n, e->rate);
    if (err) memset(buf, 0, (size_t)n * 4);
    return 0;
}

static void usf_snapshot(RipEmu *e, RipVF *out) { (void)e; (void)out; }

static void usf_close(RipEmu *e) {
    Usf *u = e->ctx;
    if (!u) return;
    usf_shutdown(u->state);
    free(u->state);
    free(u);
    e->ctx = NULL;
}

int rip_open_usf(RipEmu *e, Rip *r, char *err, int errlen) {
    Usf *u = calloc(1, sizeof *u);
    if (!u) return 0;
    u->rip = r;
    u->state = malloc(usf_get_state_size());
    if (!u->state) { free(u); return 0; }
    usf_clear(u->state);
    if (psf_load(r->path, &rip_psf_files, 0x21, usf_load, u, usf_info, u, 0, NULL, NULL) < 0) {
        snprintf(err, errlen, "couldn't read %s (or its _lib file)", r->path);
        usf_shutdown(u->state);
        free(u->state);
        free(u);
        return 0;
    }
    usf_set_compare(u->state, u->compare);
    usf_set_fifo_full(u->state, u->fifo);
    usf_set_hle_audio(u->state, 1);
    snprintf(r->console, sizeof r->console, "Nintendo 64");
    e->ctx = u;
    e->rip = r;
    e->rate = 44100;
    e->nvoices = 0;
    e->render = usf_render_cb;
    e->snapshot = usf_snapshot;
    e->close = usf_close;
    return 1;
}
