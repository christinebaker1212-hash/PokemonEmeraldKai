// Moemon Music Studio: MIDI -> song bytecode. The pipeline is tools/mid2agb's (midi.cpp, agb.cpp)
// step for step, so a game MIDI converted here gives the bytes the game build makes from it
// (with -E and -N); the GM fixes are extra and can be turned off.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "convert.h"
#include "sf.h"

enum {
    EV_EndOfTie = 0x01, EV_Label = 0x11, EV_LoopEnd = 0x12, EV_LoopEndBegin = 0x13, EV_LoopBegin = 0x14,
    EV_OriginalTimeSignature = 0x15, EV_WholeNoteMark = 0x16, EV_Pattern = 0x17, EV_TimeSignature = 0x18,
    EV_Tempo = 0x19, EV_InstrumentChange = 0x21, EV_Controller = 0x22, EV_PitchBend = 0x23,
    EV_KeyShift = 0x31, EV_Note = 0x40, EV_TimeSplit = 0xFE, EV_EndOfTrack = 0xFF,
};

typedef struct {
    int64_t time;
    int type;
    uint8_t note;
    uint8_t param1;
    int32_t param2;
} Ev;

typedef struct { Ev *v; int n, cap; } EvList;

static void push(EvList *l, Ev e)
{
    if (l->n == l->cap) { l->cap = l->cap ? l->cap * 2 : 256; l->v = realloc(l->v, sizeof(Ev) * l->cap); }
    l->v[l->n++] = e;
}

static int ev_less(const Ev *a, const Ev *b)   // mid2agb EventCompare
{
    if (a->time < b->time) return 1;
    if (a->time > b->time) return 0;
    unsigned ta = (unsigned)a->type, tb = (unsigned)b->type;
    if (a->type == EV_Note) ta += a->note;
    if (b->type == EV_Note) tb += b->note;
    if (ta < tb) return 1;
    if (ta > tb) return 0;
    if (a->type == EV_EndOfTie) {
        if (a->note < b->note) return 1;
        if (a->note > b->note) return 0;
    }
    return 0;
}

static void stable_sort(EvList *l)
{
    if (l->n < 2) return;
    Ev *tmp = malloc(sizeof(Ev) * l->n);
    for (int w = 1; w < l->n; w *= 2) {
        for (int lo = 0; lo < l->n; lo += 2 * w) {
            int mid = lo + w < l->n ? lo + w : l->n, hi = lo + 2 * w < l->n ? lo + 2 * w : l->n;
            int i = lo, j = mid, k = lo;
            while (i < mid && j < hi) tmp[k++] = ev_less(&l->v[j], &l->v[i]) ? l->v[j++] : l->v[i++];
            while (i < mid) tmp[k++] = l->v[i++];
            while (j < hi) tmp[k++] = l->v[j++];
        }
        memcpy(l->v, tmp, sizeof(Ev) * l->n);
    }
    free(tmp);
}


// ------------------------------------------------------------------ reading

// Duration in ticks of the note starting at index i (mid2agb FindNoteEnd).
static uint32_t note_len(const MTrack *t, int i)
{
    const MEvent *on = &t->ev[i];
    int ch = on->status & 0x0F;
    for (int j = i + 1; j < t->nev; j++) {
        const MEvent *e = &t->ev[j];
        if ((e->status & 0xF0) == 0x80 && (e->status & 0x0F) == ch && e->a == on->a)
            return e->time - on->time;
    }
    return t->end - on->time;   // mid2agb stops with "note doesn't end"; hold it to the track's end instead
}

static int text_is(const MEvent *e, const char *s)
{
    size_t n = strlen(s);
    return e->len == n && !memcmp(e->data, s, n);
}

static int text_has(const MEvent *e, const char *s)
{
    size_t n = strlen(s);
    for (size_t i = 0; i + n <= e->len; i++) {
        size_t k = 0;
        while (k < n && tolower(e->data[i + k]) == s[k]) k++;
        if (k == n) return 1;
    }
    return 0;
}

// The "sequence" events (tempo, time signatures, loop markers). mid2agb reads them from the
// first MIDI track only; with GM fixes every track counts.
static void read_seq(const Midi *m, int gm, EvList *seq)
{
    int ntr = gm ? m->ntracks : 1;
    uint32_t end = 0;
    for (int ti = 0; ti < ntr; ti++) {
        const MTrack *t = &m->tracks[ti];
        if (t->end > end) end = t->end;
        for (int i = 0; i < t->nev; i++) {
            const MEvent *e = &t->ev[i];
            Ev ev = {0};
            ev.time = e->time;
            if (e->status == 0xFF && e->meta >= 1 && e->meta <= 7) {
                if (e->len <= 2) {
                    if (text_is(e, "[")) ev.type = EV_LoopBegin;
                    else if (text_is(e, "][")) ev.type = EV_LoopEndBegin;
                    else if (text_is(e, "]")) ev.type = EV_LoopEnd;
                    else if (text_is(e, ":")) ev.type = EV_Label;
                }
                if (!ev.type && gm) {
                    if (text_has(e, "loopstart") || text_has(e, "loop start")) ev.type = EV_LoopBegin;
                    else if (text_has(e, "loopend") || text_has(e, "loop end")) ev.type = EV_LoopEnd;
                }
                if (!ev.type) continue;
            } else if (e->status == 0xFF && e->meta == 0x51 && e->len == 3) {
                ev.type = EV_Tempo;
                ev.param2 = (int32_t)(e->data[0] << 16 | e->data[1] << 8 | e->data[2]);
            } else if (e->status == 0xFF && e->meta == 0x58 && e->len == 4) {
                int num = e->data[0], den = e->data[1];
                if (den >= 16) continue;
                int ts = 96 * num / (1 << den);
                if (ts <= 0 || ts >= 0x10000) continue;
                ev.type = EV_TimeSignature;
                ev.param2 = ts;
            } else if (gm && (e->status & 0xF0) == 0xB0 && e->a == 111) {
                ev.type = EV_LoopBegin;    // RPG Maker's loop point
            } else {
                continue;
            }
            push(seq, ev);
        }
    }
    if (gm && ntr > 1) {
        // merge the tracks' lists by time, keeping each time's events in track order
        for (int i = 1; i < seq->n; i++) {
            Ev x = seq->v[i];
            int j = i - 1;
            while (j >= 0 && seq->v[j].time > x.time) { seq->v[j + 1] = seq->v[j]; j--; }
            seq->v[j + 1] = x;
        }
    }
    Ev eot = {0};
    eot.time = gm ? end : m->tracks[0].end;
    eot.type = EV_EndOfTrack;
    push(seq, eot);
}

static int is_block(int type) { return type == EV_LoopBegin || type == EV_LoopEnd || type == EV_LoopEndBegin || type == EV_Label; }

static void set_loop(EvList *seq, int mode, uint32_t start, uint32_t end)
{
    int w = 0;
    for (int i = 0; i < seq->n; i++)
        if (!is_block(seq->v[i].type) || mode == LOOP_MARKERS)
            seq->v[w++] = seq->v[i];
    seq->n = w;
    if (mode == LOOP_WHOLE || mode == LOOP_CUSTOM) {
        if (end <= start) return;
        Ev b = {0}, e = {0};
        b.time = start; b.type = EV_LoopBegin;
        e.time = end; e.type = EV_LoopEnd;
        // keep the list in time order: insert before the first event that is later
        for (int k = 0; k < 2; k++) {
            Ev x = k ? e : b;
            int at = seq->n;
            for (int i = 0; i < seq->n; i++) if (seq->v[i].time > x.time) { at = i; break; }
            push(seq, x);
            memmove(&seq->v[at + 1], &seq->v[at], sizeof(Ev) * (seq->n - 1 - at));
            seq->v[at] = x;
        }
    }
    int block = 0;
    for (int i = 0; i < seq->n; i++)
        if (is_block(seq->v[i].type)) seq->v[i].param1 = (uint8_t)block++;
}

// ------------------------------------------------------------------ one track

typedef struct {
    uint8_t *b;
    int n, cap;
    int labelAt[256];          // block number -> offset (-1 unknown)
    int memLabelAt[128];       // MEMACC labels (CC 17) -> offset
    int fix[512], fixBlock[512], nfix;   // fixBlock >= 1000: MEMACC label
    int *reloc; int nreloc;
} Out;

static void put(Out *o, int byte)
{
    if (o->n == o->cap) { o->cap = o->cap ? o->cap * 2 : 1024; o->b = realloc(o->b, o->cap); }
    o->b[o->n++] = (uint8_t)byte;
}

static int clock_index(int clocks)
{
    for (int i = 0; i < 49; i++) if (sf_gClockTable[i] == clocks) return i;
    return -1;
}

static void put_wait(Out *o, int64_t wait)
{
    if (wait <= 0) return;
    int idx = clock_index((int)wait);
    if (idx < 0) idx = clock_index(sf_noteDurationLUT[wait > 96 ? 96 : wait]);   // can't happen: SplitTime keeps waits valid
    put(o, 0x80 + idx);
}

static void put_target(Out *o, int block)
{
    if (o->nfix < 512) { o->fix[o->nfix] = o->n; o->fixBlock[o->nfix++] = block; }
    o->reloc = realloc(o->reloc, sizeof(int) * (o->nreloc + 1));
    o->reloc[o->nreloc++] = o->n;
    for (int i = 0; i < 4; i++) put(o, 0);
}

static void put_goto(Out *o, int block)
{
    put(o, 0xB2);
    put_target(o, block);
}

// mid2agb keeps these between tracks (they're file statics there)
typedef struct {
    int extendedCommand;
    int memaccOp, memaccParam1, memaccParam2;
} TrackVars;

static void emit_track(Out *o, EvList *events, int64_t initialWait, int mvl, int exact, int gm, TrackVars *tvp)
{
    TrackVars tv = *tvp;
    int loopEndBlockNum = 0;
    int blockNum = 0;
    Ev *ev = events->v;

    int foundVol = 0;
    for (int i = 0; i < events->n; i++) {
        if (ev[i].type == EV_Note) break;
        if (ev[i].type == EV_Controller && ev[i].param1 == 0x07) { foundVol = 1; break; }
    }
    if (!foundVol) { put(o, 0xBE); put(o, 127 * mvl / 127); }
    put_wait(o, initialWait);
    put(o, 0xBC); put(o, 0);   // KEYSH , _key+0

    for (int i = 0; ev[i].type != EV_EndOfTrack; i++) {
        Ev *e = &ev[i];
        switch (e->type) {
        case EV_Note: {
            int velocity = sf_noteVelocityLUT[e->param1 & 0x7F];
            if (e->param2 == -1) {
                put(o, 0xCF); put(o, e->note); put(o, velocity);
            } else {
                int dur = sf_noteDurationLUT[e->param2];
                int gtp = exact ? e->param2 - dur : 0;
                put(o, 0xCF + clock_index(dur));
                put(o, e->note);
                put(o, velocity);
                if (gtp > 0) put(o, gtp);
            }
            put_wait(o, e->time);
            break;
        }
        case EV_EndOfTie:
            put(o, 0xCE); put(o, e->note);
            put_wait(o, e->time);
            break;
        case EV_Label:
        case EV_LoopBegin:
        case EV_LoopEnd:
        case EV_LoopEndBegin:
            if (e->type == EV_LoopEnd || e->type == EV_LoopEndBegin)
                put_goto(o, loopEndBlockNum);
            blockNum = e->param1 + 1;
            o->labelAt[blockNum & 255] = o->n;
            put_wait(o, e->time);
            if (e->type == EV_LoopBegin || e->type == EV_LoopEndBegin)
                loopEndBlockNum = blockNum;
            break;
        case EV_Tempo: {
            int bpm = (int)lround(60000000.0f / (float)e->param2);
            put(o, 0xBB); put(o, (bpm * 1 / 2) & 0xFF);
            put_wait(o, e->time);
            break;
        }
        case EV_InstrumentChange:
            put(o, 0xBD); put(o, e->param1);
            put_wait(o, e->time);
            break;
        case EV_PitchBend:
            put(o, 0xC0); put(o, 0x40 + (e->param2 - 64));
            put_wait(o, e->time);
            break;
        case EV_Controller:
            switch (e->param1) {
            case 0x01: put(o, 0xC4); put(o, e->param2); put_wait(o, e->time); break;
            case 0x07: put(o, 0xBE); put(o, e->param2 * mvl / 127); put_wait(o, e->time); break;
            case 0x0A: put(o, 0xBF); put(o, 0x40 + (e->param2 - 64)); put_wait(o, e->time); break;
            case 0x14: put(o, 0xC1); put(o, e->param2); put_wait(o, e->time); break;
            case 0x15: put(o, 0xC2); put(o, e->param2); put_wait(o, e->time); break;
            case 0x16: put(o, 0xC5); put(o, e->param2); put_wait(o, e->time); break;
            case 0x18: put(o, 0xC8); put(o, 0x40 + (e->param2 - 64)); put_wait(o, e->time); break;
            case 0x1A: put(o, 0xC3); put(o, e->param2); put_wait(o, e->time); break;
            case 0x1D:
            case 0x1F:
                if (tv.extendedCommand == 0x08 || tv.extendedCommand == 0x09) {
                    put(o, 0xCD); put(o, tv.extendedCommand); put(o, e->param2);
                }
                put_wait(o, e->time);
                break;
            case 0x1E:
                tv.extendedCommand = e->param2;   // mid2agb prints no wait here (sic)
                if (gm) put_wait(o, e->time);
                break;
            case 0x21:
            case 0x27:
                put(o, 0xBA); put(o, e->param2); put_wait(o, e->time); break;
            case 0x0C:
            case 0x10:   // MEMACC
                if (tv.memaccOp >= 0 && tv.memaccOp <= 0x11) {
                    put(o, 0xB9); put(o, tv.memaccOp); put(o, tv.memaccParam1); put(o, e->param2);
                    if (tv.memaccOp >= 6) put_target(o, 1000 + (tv.memaccParam2 & 127));
                }
                put_wait(o, e->time);
                break;
            case 0x0D: tv.memaccOp = e->param2; put_wait(o, e->time); break;
            case 0x0E: tv.memaccParam1 = e->param2; put_wait(o, e->time); break;
            case 0x0F: tv.memaccParam2 = e->param2; put_wait(o, e->time); break;
            case 0x11:
                o->memLabelAt[e->param2 & 127] = o->n;
                put_wait(o, e->time);
                break;
            default:
                put_wait(o, e->time);
                break;
            }
            break;
        default:
            put_wait(o, e->time);
            break;
        }
    }
    put(o, 0xB1);   // FINE
    *tvp = tv;
    for (int i = 0; i < o->nfix; i++) {
        int at = o->fixBlock[i] >= 1000 ? o->memLabelAt[o->fixBlock[i] - 1000] : o->labelAt[o->fixBlock[i] & 255];
        if (at < 0) at = 0;
        for (int k = 0; k < 4; k++) o->b[o->fix[i] + k] = (uint8_t)(at >> (8 * k));
    }
}

// mid2agb ConvertTimes / InsertTimingEvents / CreateTies / sort / SplitTime / CalculateWaits
static void pipeline(EvList *events, int division, int exact, int firstTrack, int64_t *initialWait)
{
    for (int i = 0; i < events->n; i++) {
        Ev *e = &events->v[i];
        e->time = (24 * e->time) / division;
        if (e->type == EV_Note) {
            e->param1 = sf_noteVelocityLUT[e->param1 & 0x7F];
            int64_t dur = (24 * (int64_t)e->param2) / division;
            if (dur == 0) dur = 1;
            if (!exact && dur < 96) dur = sf_noteDurationLUT[dur];
            e->param2 = (int32_t)dur;
        }
    }
    EvList t = {0};
    Ev timing = {0};
    timing.type = EV_TimeSignature;
    timing.param2 = 96;
    for (int i = 0; i < events->n; i++) {
        Ev *e = &events->v[i];
        while (ev_less(&timing, e)) {
            push(&t, timing);
            timing.time += timing.param2;
        }
        if (e->type == EV_TimeSignature) {
            if (firstTrack && e->param2 != timing.param2) {
                Ev orig = *e;
                orig.type = EV_OriginalTimeSignature;
                push(&t, orig);
            }
            timing.param2 = e->param2;
            timing.time = e->time + timing.param2;
        }
        push(&t, *e);
    }
    EvList ties = {0};
    for (int i = 0; i < t.n; i++) {
        Ev e = t.v[i];
        if (e.type == EV_Note && e.param2 > 96) {
            Ev tie = e;
            tie.param2 = -1;
            push(&ties, tie);
            Ev eot = {0};
            eot.time = e.time + e.param2;
            eot.type = EV_EndOfTie;
            eot.note = e.note;
            push(&ties, eot);
        } else {
            push(&ties, e);
        }
    }
    free(t.v);
    stable_sort(&ties);
    EvList s = {0};
    int64_t time = 0;
    for (int i = 0; i < ties.n; i++) {
        Ev e = ties.v[i];
        int64_t diff = e.time - time;
        if (diff > 96) {
            int64_t whole = (diff - 1) / 96;
            diff -= 96 * whole;
            for (int64_t k = 0; k < whole; k++) {
                time += 96;
                Ev sp = {0};
                sp.time = time;
                sp.type = EV_TimeSplit;
                push(&s, sp);
            }
        }
        if (diff >= 0 && diff <= 96) {
            int lut = sf_noteDurationLUT[diff];
            if (lut != diff) {
                Ev sp = {0};
                sp.time = time + lut;
                sp.type = EV_TimeSplit;
                push(&s, sp);
            }
        }
        time = e.time;
        push(&s, e);
    }
    free(ties.v);
    *initialWait = s.v[0].time;
    int whole = 0;
    for (int i = 0; i < s.n && s.v[i].type != EV_EndOfTrack; i++) {
        s.v[i].time = s.v[i + 1].time - s.v[i].time;
        if (s.v[i].type == EV_TimeSignature) {
            s.v[i].type = EV_WholeNoteMark;
            s.v[i].param2 = whole++;
        }
    }
    free(events->v);
    *events = s;
}

// ------------------------------------------------------------------ build

void conv_free(Conv *c)
{
    for (int i = 0; i < CONV_MAX_OUT; i++) { free(c->data[i]); free(c->reloc[i]); }
    memset(c, 0, sizeof(*c));
}

typedef struct { int vol, expr, pan, bend, mod, program, rpnM, rpnL, bendr; int havePan, haveMod, haveBendr; } ChanState;

int conv_build(Conv *c, const Midi *m, const ConvOpts *o, char *err, int errlen)
{
    memset(c, 0, sizeof(*c));
    c->division = m->division;
    int gm = o->gmFixes;

    // candidates: every (MIDI track, channel) with a sounding note
    for (int ti = 0; ti < m->ntracks; ti++) {
        const MTrack *t = &m->tracks[ti];
        for (int ch = 0; ch < 16; ch++) {
            int notes = 0, prog = -1;
            for (int i = 0; i < t->nev; i++) {
                const MEvent *e = &t->ev[i];
                if ((e->status & 0x0F) != ch || e->status >= 0xF0) continue;
                if ((e->status & 0xF0) == 0x90 && note_len(t, i) > 0) notes++;
                if ((e->status & 0xF0) == 0x90) {
                    uint32_t end = e->time + note_len(t, i);
                    if (end > c->endTick) c->endTick = end;
                }
                if ((e->status & 0xF0) == 0xC0 && prog < 0) prog = e->a;
            }
            if (!notes) continue;
            if (c->ncand == CONV_MAX_CAND) { c->dropped++; continue; }
            ConvCand *cd = &c->cand[c->ncand++];
            cd->midiTrack = ti;
            cd->chan = ch;
            cd->notes = notes;
            cd->drum = ch == 9;
            cd->program = prog;
            cd->slot = -1;
            snprintf(cd->name, sizeof(cd->name), "%s", t->name);
        }
    }
    if (!c->ncand) { snprintf(err, errlen, "the MIDI file has no notes"); return 0; }

    EvList seq = {0};
    read_seq(m, gm, &seq);
    uint32_t ls = o->loopStart, le = o->loopEnd;
    if (o->loopMode == LOOP_WHOLE) {
        // the end of the last note, rounded up to a whole bar of the first time signature
        int bar = m->division * 4;
        for (int i = 0; i < seq.n; i++)
            if (seq.v[i].type == EV_TimeSignature) { bar = (int)((int64_t)seq.v[i].param2 * m->division / 24); break; }
        if (bar <= 0) bar = m->division * 4;
        ls = 0;
        le = (c->endTick + bar - 1) / bar * bar;
    }
    set_loop(&seq, o->loopMode, ls, le);
    for (int i = 0; i < seq.n; i++) {
        if (seq.v[i].type == EV_LoopBegin && !c->hasLoop) { c->loopStartTick = (uint32_t)seq.v[i].time; c->hasLoop = 1; }
        else if ((seq.v[i].type == EV_LoopEnd || seq.v[i].type == EV_LoopEndBegin) && c->hasLoop == 1) {
            c->loopEndTick = (uint32_t)seq.v[i].time; c->hasLoop = 2;
        }
    }
    if (c->hasLoop != 2) { c->hasLoop = 0; c->loopStartTick = c->loopEndTick = 0; }
    c->loopStartClock = (uint32_t)(24 * (int64_t)c->loopStartTick / m->division);
    c->loopEndClock = (uint32_t)(24 * (int64_t)c->loopEndTick / m->division);
    c->endClock = (uint32_t)(24 * (int64_t)c->endTick / m->division);
    int clip = gm && c->hasLoop;

    // GM fixes: tempo in effect at the loop start, sent again there
    if (gm && c->hasLoop) {
        int32_t tempo = -1; int at = -1;
        for (int i = 0; i < seq.n; i++) {
            if (seq.v[i].type == EV_Tempo && seq.v[i].time <= c->loopStartTick) tempo = seq.v[i].param2;
            if (seq.v[i].type == EV_Tempo && seq.v[i].time == c->loopStartTick) at = i;
        }
        if (tempo > 0 && at < 0) {
            Ev x = {0};
            x.time = c->loopStartTick; x.type = EV_Tempo; x.param2 = tempo;
            int pos = seq.n - 1;
            for (int i = 0; i < seq.n; i++) if (seq.v[i].time > x.time) { pos = i; break; }
            push(&seq, x);
            memmove(&seq.v[pos + 1], &seq.v[pos], sizeof(Ev) * (seq.n - 1 - pos));
            seq.v[pos] = x;
        }
    }

    // voice slots: programs as they are; drum kits go to free slots from 127 down
    int drumSlot[128];
    for (int i = 0; i < 128; i++) { drumSlot[i] = -1; c->slotGm[i] = -1; }
    if (gm) {
        uint8_t melodic[128] = {0}, drums[128] = {0};
        for (int k = 0; k < c->ncand; k++) {
            if ((o->exclude >> k) & 1) continue;
            const MTrack *t = &m->tracks[c->cand[k].midiTrack];
            int any = 0;
            for (int i = 0; i < t->nev; i++) {
                const MEvent *e = &t->ev[i];
                if (e->status == (0xC0 | c->cand[k].chan)) { (c->cand[k].drum ? drums : melodic)[e->a] = 1; any = 1; }
            }
            if (!any || c->cand[k].program < 0) (c->cand[k].drum ? drums : melodic)[0] = 1;
        }
        int next = 127;
        for (int p = 0; p < 128; p++) {
            if (!drums[p]) continue;
            while (next >= 0 && (melodic[next] || c->slotUsed[next])) next--;
            if (next < 0) break;
            drumSlot[p] = next;
            c->slotGm[next] = (int8_t)p;
            c->slotUsed[next] = 1;
            next--;
        }
        memset(c->slotUsed, 0, sizeof(c->slotUsed));
    }

    int firstTrack = 1;
    TrackVars tv = {0};
    for (int k = 0; k < c->ncand; k++) {
        if ((o->exclude >> k) & 1) continue;
        if (c->ntracks == CONV_MAX_OUT) { c->dropped++; continue; }
        const ConvCand *cd = &c->cand[k];
        const MTrack *t = &m->tracks[cd->midiTrack];
        EvList tr = {0};
        ChanState cs = {127, 127, 64, 64, 0, -1, 127, 127, 2, 0, 0, 0};
        int sentState = !(gm && c->hasLoop);
        int firstNoteSeen = 0, progSeen = 0;

        for (int i = 0; i < t->nev; i++) {
            const MEvent *me = &t->ev[i];
            if (me->status >= 0xF0 || (me->status & 0x0F) != cd->chan) continue;
            if (!sentState && me->time >= c->loopStartTick) {
                // GM fixes: the loop start re-sends what the track had, since a jump back keeps the end's state
                Ev x = {0};
                x.time = c->loopStartTick;
                if (cs.program >= 0) {
                    x.type = EV_InstrumentChange;
                    x.param1 = (uint8_t)(cd->drum && drumSlot[cs.program] >= 0 ? drumSlot[cs.program] : cs.program);
                    push(&tr, x);
                }
                x.type = EV_Controller; x.param1 = 7; x.param2 = cs.vol * cs.expr / 127; push(&tr, x);
                if (cs.havePan) { x.param1 = 10; x.param2 = cs.pan; push(&tr, x); }
                if (cs.haveMod) { x.param1 = 1; x.param2 = cs.mod; push(&tr, x); }
                if (cs.haveBendr) { x.param1 = 0x14; x.param2 = cs.bendr; push(&tr, x); }
                x.type = EV_PitchBend; x.param1 = 0; x.param2 = cs.bend; push(&tr, x);
                sentState = 1;
            }
            Ev e = {0};
            e.time = me->time;
            switch (me->status & 0xF0) {
            case 0x90: {
                uint32_t dur = note_len(t, i);
                if (clip) {
                    if (me->time >= c->loopEndTick) continue;
                    if (me->time + dur > c->loopEndTick) dur = c->loopEndTick - me->time;
                }
                if (gm && !progSeen && !firstNoteSeen) {
                    Ev p = {0};
                    p.time = 0; p.type = EV_InstrumentChange;
                    p.param1 = (uint8_t)(cd->drum && drumSlot[0] >= 0 ? drumSlot[0] : 0);
                    push(&tr, p);
                    cs.program = 0;
                    progSeen = 1;
                }
                firstNoteSeen = 1;
                e.type = EV_Note; e.note = me->a; e.param1 = me->b; e.param2 = (int32_t)dur;
                break;
            }
            case 0xB0:
                e.type = EV_Controller; e.param1 = me->a; e.param2 = me->b;
                if (gm) {
                    int cc = me->a, v = me->b;
                    if (cc == 7 || cc == 11) {
                        if (cc == 7) cs.vol = v; else cs.expr = v;
                        e.param1 = 7; e.param2 = cs.vol * cs.expr / 127;
                    } else if (cc == 101) { cs.rpnM = v; continue; }
                    else if (cc == 100) { cs.rpnL = v; continue; }
                    else if (cc == 6) {
                        if (cs.rpnM != 0 || cs.rpnL != 0) continue;
                        e.param1 = 0x14; cs.bendr = v; cs.haveBendr = 1;
                    }
                    else if (cc == 10) { cs.pan = v; cs.havePan = 1; }
                    else if (cc == 1) { cs.mod = v; cs.haveMod = 1; }
                    else if (cc == 0x14) { cs.bendr = v; cs.haveBendr = 1; }
                    else if ((cc >= 12 && cc <= 17) || (cc >= 29 && cc <= 31) || cc == 33 || cc == 39 || cc == 38 ||
                             cc == 98 || cc == 99 || cc == 0 || cc == 32 || cc >= 64)
                        continue;   // not ones the game's songs mean
                }
                break;
            case 0xC0:
                e.type = EV_InstrumentChange;
                e.param1 = me->a;
                if (gm) {
                    cs.program = me->a;
                    if (cd->drum && drumSlot[me->a] >= 0) e.param1 = (uint8_t)drumSlot[me->a];
                    progSeen = 1;
                }
                break;
            case 0xE0:
                e.type = EV_PitchBend; e.param1 = me->a; e.param2 = me->b;
                cs.bend = me->b;
                break;
            default:
                continue;
            }
            if (clip && me->time > c->loopEndTick) continue;
            push(&tr, e);
        }
        Ev eot = {0};
        eot.time = t->end;
        eot.type = EV_EndOfTrack;
        push(&tr, eot);

        // which slots this track selects
        for (int i = 0; i < tr.n; i++) {
            if (tr.v[i].type == EV_InstrumentChange) {
                int s = tr.v[i].param1 & 0x7F;
                c->slotUsed[s] = 1;
                if (cd->drum) c->slotDrum[s] = 1;
                if (c->cand[k].slot < 0) c->cand[k].slot = s;
            }
        }

        // MergeEvents (seq ends at its own EndOfTrack)
        EvList merged = {0};
        int a = 0, b = 0;
        while (tr.v[a].type != EV_EndOfTrack && seq.v[b].type != EV_EndOfTrack) {
            if (ev_less(&tr.v[a], &seq.v[b])) push(&merged, tr.v[a++]);
            else push(&merged, seq.v[b++]);
        }
        while (tr.v[a].type != EV_EndOfTrack) push(&merged, tr.v[a++]);
        while (seq.v[b].type != EV_EndOfTrack) push(&merged, seq.v[b++]);
        push(&merged, ev_less(&tr.v[a], &seq.v[b]) ? seq.v[b] : tr.v[a]);
        free(tr.v);

        if (firstTrack) {   // TEMPO only goes in the first track
            int w = 0;
            for (int i = 0; i < seq.n; i++) if (seq.v[i].type != EV_Tempo) seq.v[w++] = seq.v[i];
            seq.n = w;
        }

        int64_t initialWait = 0;
        pipeline(&merged, m->division, o->exactGate, firstTrack, &initialWait);
        Out out = {0};
        for (int i = 0; i < 256; i++) out.labelAt[i] = -1;
        for (int i = 0; i < 128; i++) out.memLabelAt[i] = -1;
        emit_track(&out, &merged, initialWait, o->masterVol, o->exactGate, gm, &tv);
        free(merged.v);

        int n = c->ntracks++;
        c->candOf[n] = k;
        c->data[n] = out.b;
        c->len[n] = out.n;
        c->reloc[n] = out.reloc;
        c->nreloc[n] = out.nreloc;
        firstTrack = 0;
    }
    free(seq.v);
    // the GM program each slot stands for (a drum slot keeps the drum kit program it was given)
    for (int i = 0; i < 128; i++) {
        if (!c->slotUsed[i]) c->slotGm[i] = -1;
        else if (c->slotGm[i] < 0 || !c->slotDrum[i]) c->slotGm[i] = (int8_t)(c->slotDrum[i] ? 0 : i);
    }
    if (!c->ntracks) { snprintf(err, errlen, "every track is muted"); return 0; }
    return 1;
}
