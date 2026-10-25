// Moemon Music Studio: Standard MIDI File reader (format 0 and 1).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midi.h"

static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static uint32_t be16(const uint8_t *p) { return (uint32_t)p[0] << 8 | p[1]; }

void midi_free(Midi *m)
{
    if (m->tracks)
        for (int i = 0; i < m->ntracks; i++)
            free(m->tracks[i].ev);
    free(m->tracks);
    free(m->raw);
    memset(m, 0, sizeof(*m));
}

int midi_parse(Midi *m, const uint8_t *buf, size_t len, char *err, int errlen)
{
    memset(m, 0, sizeof(*m));
    if (len < 14 || memcmp(buf, "MThd", 4) || be32(buf + 4) < 6) {
        snprintf(err, errlen, "not a MIDI file");
        return 0;
    }
    m->raw = malloc(len);
    memcpy(m->raw, buf, len);
    m->rawlen = len;
    const uint8_t *p = m->raw;
    m->format = (int)be16(p + 8);
    m->ntracks = (int)be16(p + 10);
    int div = (int)be16(p + 12);
    if (m->format > 1) { snprintf(err, errlen, "MIDI format %d isn't supported (only 0 and 1)", m->format); midi_free(m); return 0; }
    if (div & 0x8000) { snprintf(err, errlen, "SMPTE-timed MIDI files aren't supported"); midi_free(m); return 0; }
    if (div == 0) { snprintf(err, errlen, "MIDI file has no time base"); midi_free(m); return 0; }
    m->division = div;
    m->tracks = calloc((size_t)(m->ntracks ? m->ntracks : 1), sizeof(MTrack));
    size_t pos = 8 + be32(p + 4);
    int found = 0;
    while (found < m->ntracks && pos + 8 <= len) {
        uint32_t clen = be32(p + pos + 4);
        if (memcmp(p + pos, "MTrk", 4)) { pos += 8 + clen; continue; }
        size_t q = pos + 8, end = pos + 8 + clen;
        if (end > len) end = len;
        MTrack *t = &m->tracks[found++];
        int cap = 256;
        t->ev = malloc(sizeof(MEvent) * cap);
        uint32_t time = 0;
        uint8_t running = 0;
        while (q < end) {
            uint32_t delta = 0;
            uint8_t c;
            do { c = p[q++]; delta = (delta << 7) | (c & 0x7F); } while ((c & 0x80) && q < end);
            time += delta;
            if (q >= end) break;
            MEvent ev = {0};
            ev.time = time;
            uint8_t st = p[q];
            if (st < 0x80) {
                if (!running) break;   // data without status: give up on this track
                st = running;
            } else {
                q++;
            }
            ev.status = st;
            if (st == 0xFF) {
                running = 0;
                if (q >= end) break;
                ev.meta = p[q++];
                uint32_t l = 0;
                do { c = p[q++]; l = (l << 7) | (c & 0x7F); } while ((c & 0x80) && q < end);
                if (q + l > end) l = (uint32_t)(end - q);
                ev.len = l;
                ev.data = p + q;
                q += l;
                if (ev.meta == 0x03 && !t->name[0]) {
                    size_t n = l < sizeof(t->name) - 1 ? l : sizeof(t->name) - 1;
                    memcpy(t->name, ev.data, n);
                    t->name[n] = 0;
                    for (size_t i = 0; i < n; i++) if ((unsigned char)t->name[i] < 32) t->name[i] = ' ';
                }
            } else if (st == 0xF0 || st == 0xF7) {
                running = 0;
                uint32_t l = 0;
                do { c = p[q++]; l = (l << 7) | (c & 0x7F); } while ((c & 0x80) && q < end);
                if (q + l > end) l = (uint32_t)(end - q);
                ev.len = l;
                ev.data = p + q;
                q += l;
            } else if (st >= 0xF0) {
                running = 0;    // system common/realtime: no data we care about
                if (st == 0xF2) q += 2;
                else if (st == 0xF1 || st == 0xF3) q += 1;
            } else {
                running = st;
                int n = ((st & 0xF0) == 0xC0 || (st & 0xF0) == 0xD0) ? 1 : 2;
                if (q + n > end) break;
                ev.a = p[q++];
                if (n == 2) ev.b = p[q++];
                if ((st & 0xF0) == 0x90 && ev.b == 0)
                    ev.status = 0x80 | (st & 0x0F);    // note on with velocity 0 = note off
            }
            if (t->nev == cap) { cap *= 2; t->ev = realloc(t->ev, sizeof(MEvent) * cap); }
            t->ev[t->nev++] = ev;
            if (ev.status == 0xFF && ev.meta == 0x2F) break;
        }
        t->end = time;
        pos = end;
    }
    m->ntracks = found;
    if (!found) { snprintf(err, errlen, "MIDI file has no tracks"); midi_free(m); return 0; }
    return 1;
}
