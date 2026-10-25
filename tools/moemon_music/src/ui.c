// Moemon Music Studio: the window. Immediate-mode: every frame draws everything and reads the
// mouse / keys as it goes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL.h>
#include "studio.h"
#include "inst.h"
#include "export.h"
#include "dialog.h"
#include "font.h"

#define W 1280
#define H 800

int studio_open(Studio *s, const char *path);

// ------------------------------------------------------------------ state

static SDL_Window *sWin;
static SDL_Renderer *sRen;
// fonts: [0] the pixel font, [1] Smooth, [2] Bold; Smooth and Bold come at 1x, 2x and 3x
enum { FONT_PIXEL, FONT_SMOOTH, FONT_BOLD, NUM_FONTS };
static const char *FONT_NAMES[NUM_FONTS] = {"Pixel", "Smooth", "Bold"};
static SDL_Texture *sFonts[NUM_FONTS][3];
static int sFontChoice = FONT_PIXEL;
// window size: 0 = fit the screen, else a percentage of 1280x800
static const int WIN_SIZES[] = {0, 100, 125, 150, 175, 200};
#define NUM_WIN_SIZES 6
static int sWinSize = 0;
extern const unsigned char ui_fonts[];
static SDL_AudioDeviceID sDev;

static Studio S;
static Engine E;
static Engine sSeek;              // scratch engine for seeking
static int sPlaying;              // the song is running (not paused / stopped)
static long sFrames;              // frames played since the start
static float sPending[ENG_FRAME_SAMPLES * 2];
static int sPendingPos = ENG_FRAME_SAMPLES;
static int sPreviewFrames;        // auditions stop by themselves after this many frames
static int sHolding;              // a hold-to-hear button or piano key is down

static char sStatus[512] = "Open a MIDI file (or drop one on the window) to start.";
static int sSelSlot = -1;
static int sTrackScroll, sSlotScroll;

enum { MODAL_NONE, MODAL_INSTRUMENT, MODAL_SONG_TARGET, MODAL_SONG_VOICES, MODAL_PATH, MODAL_CONFIRM, MODAL_HELP, MODAL_DISPLAY };
static int sModal;
static int sInModal;              // drawing the modal (its widgets take input)
static char sSearch[64];
static int sPickCat = -1;         // -1 = all
static int sPickSel = -1, sPickScroll, sPickKey = 60, sPickJump;
static int sPathAction;           // what the path prompt is for
static char sPathBuf[1024];
static int sConfirmAction;        // what "Discard" continues with
static char sConfirmText[256];

enum { ACT_OPEN_MIDI = 1, ACT_OPEN_PROJECT, ACT_SAVE_AS, ACT_EXPORT_KAISONG, ACT_EXPORT_DECOMP, ACT_QUIT, ACT_OPEN_DROPPED };
static char sDropped[1024];
static int sQuit;

// input for this frame
static int mx, my, mDown, mPressed, mReleased, pressX, pressY, wheel, dblClick;
static Uint32 lastClickTime;
static char sTyped[64];
static int kBackspace, kEnter, kEsc, kUp, kDown, kPgUp, kPgDn, kSpace;

// colours
typedef struct { Uint8 r, g, b; } Col;
static const Col C_BG = {0x1d, 0x1b, 0x26}, C_PANEL = {0x27, 0x24, 0x33}, C_ROW = {0x30, 0x2c, 0x3f},
                 C_HOT = {0x3c, 0x36, 0x52}, C_SEL = {0x5a, 0x3a, 0x6a}, C_TEXT = {0xec, 0xea, 0xf4},
                 C_DIM = {0x9d, 0x98, 0xb0}, C_ACCENT = {0xff, 0x7a, 0xb6}, C_OK = {0x7a, 0xd9, 0x9a},
                 C_WARN = {0xff, 0xc4, 0x6b}, C_BTN = {0x3a, 0x35, 0x4c}, C_BTNHOT = {0x4d, 0x46, 0x66},
                 C_LINE = {0x45, 0x40, 0x5a};

// ------------------------------------------------------------------ audio

static void audio_cb(void *u, Uint8 *stream, int len)
{
    (void)u;
    float *out = (float *)stream;
    int n = len / (int)(sizeof(float) * 2);
    for (int i = 0; i < n; i++) {
        if (sPendingPos >= ENG_FRAME_SAMPLES) {
            eng_frame(&E, sPending);
            if (sPlaying) {
                if (E.playing) sFrames++;
                else sPlaying = 0;
            }
            if (sPreviewFrames > 0 && --sPreviewFrames == 0 && !sHolding) eng_preview_off(&E);
            sPendingPos = 0;
        }
        float l = sPending[sPendingPos * 2], r = sPending[sPendingPos * 2 + 1];
        out[i * 2] = l > 1 ? 1 : l < -1 ? -1 : l;
        out[i * 2 + 1] = r > 1 ? 1 : r < -1 ? -1 : r;
        sPendingPos++;
    }
}

static void lock(void) { if (sDev) SDL_LockAudioDevice(sDev); }
static void unlock(void) { if (sDev) SDL_UnlockAudioDevice(sDev); }

static int have_song(void) { return S.haveConv && S.song.ntracks > 0; }

// Starts the song at a frame (seeking runs a scratch engine, then swaps it in).
static void play_from(long frame)
{
    if (!have_song()) return;
    lock();
    eng_stop(&E);
    unlock();
    eng_init(&sSeek);
    eng_start(&sSeek, &S.song);
    sSeek.muteMask = E.muteMask;
    long done = eng_skip(&sSeek, (int)frame);
    lock();
    // keep the preview track's sound going across the swap
    eng_copy(&E, &sSeek);
    sFrames = done;
    sPlaying = E.playing;
    sPendingPos = ENG_FRAME_SAMPLES;
    unlock();
}

static void stop_song(void)
{
    lock();
    eng_stop(&E);
    sPlaying = 0;
    unlock();
}

static void pause_song(void)
{
    lock();
    eng_stop(&E);
    sPlaying = 0;
    unlock();
}

// position in the song for the bar (a looping song wraps back into its loop)
static long song_pos(void)
{
    long f = sFrames;
    if (S.loopStartFrame >= 0 && S.totalFrames > S.loopStartFrame && f >= S.totalFrames)
        f = S.loopStartFrame + (f - S.loopStartFrame) % (S.totalFrames - S.loopStartFrame);
    return f;
}

static void update_mute_mask(void)
{
    uint32_t m = 0;
    if (S.p.solo) {
        for (int t = 0; t < S.c.ntracks; t++)
            if (!((S.p.solo >> S.c.candOf[t]) & 1)) m |= 1u << t;
    }
    lock();
    E.muteMask = m;
    unlock();
}

// After settings or mutes change: convert again and carry on from the same place.
static void rebuild(void)
{
    int was = sPlaying;
    long pos = song_pos();
    stop_song();
    if (!studio_rebuild(&S))
        snprintf(sStatus, sizeof(sStatus), "%s", S.msg);
    update_mute_mask();
    if (was) play_from(pos);
    else sFrames = pos < S.totalFrames ? pos : 0;
}

static void rebuild(void);

static void voices_changed(void)
{
    // a different instrument changes its volume factor, and a different kit moves drum notes: convert again
    int again = S.p.gmFixes && S.p.balance;
    for (int i = 0; i < 128; i++)
        if (S.c.slotDrum[i] && S.p.gmFixes) again = 1;
    if (again) {
        S.p.dirty = 1;
        rebuild();
        return;
    }
    lock();
    studio_refresh_voices(&S);
    eng_refresh_instruments(&E);
    unlock();
    S.p.dirty = 1;
}

static void audition(const SfVoice *v, int key, int hold)
{
    static EInst inst;
    lock();
    inst_make(v, &inst);
    eng_preview_on(&E, &inst, key, 112);
    sPreviewFrames = hold ? 0 : 45;
    sHolding = hold;
    unlock();
}

static void audition_off(void)
{
    lock();
    sHolding = 0;
    eng_preview_off(&E);
    unlock();
}

// ------------------------------------------------------------------ drawing

static void color(Col c) { SDL_SetRenderDrawColor(sRen, c.r, c.g, c.b, 255); }

static void fill(int x, int y, int w, int h, Col c)
{
    SDL_Rect r = {x, y, w, h};
    color(c);
    SDL_RenderFillRect(sRen, &r);
}

static void frame_rect(int x, int y, int w, int h, Col c)
{
    SDL_Rect r = {x, y, w, h};
    color(c);
    SDL_RenderDrawRect(sRen, &r);
}

// how many screen pixels one layout pixel is: picks the font size that stays sharp
static int font_scale(void)
{
    int ow = W, oh = H;
    SDL_GetRendererOutputSize(sRen, &ow, &oh);
    float k = (float)ow / W < (float)oh / H ? (float)ow / W : (float)oh / H;
    return k <= 1.3f ? 1 : k <= 2.3f ? 2 : 3;
}

static int sFontNow = 1;   // the scale picked for this frame

static int text_font(int x, int y, const char *s, Col c, int font)
{
    int sc = font == FONT_PIXEL ? 1 : sFontNow;
    SDL_Texture *t = sFonts[font][sc - 1];
    SDL_SetTextureColorMod(t, c.r, c.g, c.b);
    int x0 = x;
    for (; *s; s++) {
        unsigned ch = (unsigned char)*s;
        if (ch < 32 || ch > 126) ch = '?';
        SDL_Rect src = {(int)(ch - 32) * 8 * sc, 0, 8 * sc, 16 * sc}, dst = {x, y, 8, 16};
        SDL_RenderCopy(sRen, t, &src, &dst);
        x += 8;
    }
    return x - x0;
}

static int text(int x, int y, const char *s, Col c)
{
    return text_font(x, y, s, c, sFontChoice);
}

static int textw(const char *s) { return (int)strlen(s) * 8; }

// text cut to fit a width
static void textfit(int x, int y, int w, const char *s, Col c)
{
    int maxc = w / 8;
    if ((int)strlen(s) <= maxc) { text(x, y, s, c); return; }
    char buf[256];
    if (maxc > (int)sizeof(buf) - 1) maxc = sizeof(buf) - 1;
    if (maxc < 3) return;
    memcpy(buf, s, (size_t)maxc - 2);
    buf[maxc - 2] = '.';
    buf[maxc - 1] = '.';
    buf[maxc] = 0;
    text(x, y, buf, c);
}

static int inside(int x, int y, int w, int h, int px, int py) { return px >= x && py >= y && px < x + w && py < y + h; }
static int active(void) { return sModal == MODAL_NONE || sInModal; }
static int hot(int x, int y, int w, int h) { return active() && inside(x, y, w, h, mx, my); }
static int clicked(int x, int y, int w, int h) { return active() && mReleased && inside(x, y, w, h, mx, my) && inside(x, y, w, h, pressX, pressY); }
static int pressed(int x, int y, int w, int h) { return active() && mPressed && inside(x, y, w, h, mx, my); }

static int button(int x, int y, int w, int h, const char *label, int enabled)
{
    int ho = enabled && hot(x, y, w, h);
    fill(x, y, w, h, ho ? C_BTNHOT : C_BTN);
    frame_rect(x, y, w, h, ho ? C_ACCENT : C_LINE);
    text(x + (w - textw(label)) / 2, y + (h - 16) / 2, label, enabled ? C_TEXT : C_DIM);
    return enabled && clicked(x, y, w, h);
}

static int checkbox(int x, int y, const char *label, int *v)
{
    int w = 22 + textw(label);
    int ho = hot(x, y, w, 18);
    frame_rect(x, y + 1, 16, 16, ho ? C_ACCENT : C_LINE);
    if (*v) fill(x + 4, y + 5, 8, 8, C_ACCENT);
    text(x + 22, y + 1, label, C_TEXT);
    if (clicked(x, y, w, 18)) { *v = !*v; return 1; }
    return 0;
}

// [-] value [+], the wheel works over it too
static int stepper(int x, int y, const char *label, int *v, int lo, int hi, int step, const char *valueText)
{
    int lw = label ? textw(label) + 8 : 0;
    if (label) text(x, y + 2, label, C_DIM);
    int bx = x + lw;
    int changed = 0, nv = *v;
    if (button(bx, y, 20, 20, "-", *v > lo)) nv -= step;
    char buf[32];
    if (!valueText) { snprintf(buf, sizeof(buf), "%d", *v); valueText = buf; }
    fill(bx + 22, y, 44, 20, C_ROW);
    text(bx + 22 + (44 - textw(valueText)) / 2, y + 2, valueText, C_TEXT);
    if (button(bx + 68, y, 20, 20, "+", *v < hi)) nv += step;
    if (wheel && hot(bx, y, 88, 20)) nv += wheel > 0 ? step : -step;
    if (nv < lo) nv = lo;
    if (nv > hi) nv = hi;
    if (nv != *v) { *v = nv; changed = 1; }
    return changed;
}

static void panel(int x, int y, int w, int h, const char *title)
{
    fill(x, y, w, h, C_PANEL);
    if (title) text(x + 10, y + 6, title, C_ACCENT);
}

static void scrollbar(int x, int y, int h, int total, int visible, int *scroll)
{
    if (total <= visible) { *scroll = 0; return; }
    fill(x, y, 8, h, C_ROW);
    int th = h * visible / total;
    if (th < 16) th = 16;
    int ty = y + (h - th) * *scroll / (total - visible);
    fill(x, ty, 8, th, C_LINE);
    if (active() && mDown && inside(x - 4, y, 16, h, pressX, pressY)) {
        int s = (my - y - th / 2) * (total - visible) / (h - th > 0 ? h - th : 1);
        *scroll = s < 0 ? 0 : s > total - visible ? total - visible : s;
    }
}

static const char *NOTE_NAMES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static void note_name(int k, char *buf, int n) { snprintf(buf, n, "%s%d", NOTE_NAMES[k % 12], k / 12 - 1); }

static void fmt_time(long frames, char *buf, int n)
{
    long s = frames / 60;
    snprintf(buf, n, "%ld:%02ld", s / 60, s % 60);
}

// ------------------------------------------------------------------ actions

static void set_status(const char *fmt, const char *a)
{
    snprintf(sStatus, sizeof(sStatus), fmt, a);
}

static void after_open(const char *path)
{
    sSelSlot = -1;
    sTrackScroll = sSlotScroll = 0;
    S.p.solo = 0;
    for (int i = 0; i < 128; i++) if (S.c.slotUsed[i]) { sSelSlot = i; break; }
    update_mute_mask();
    sFrames = 0;
    char m[700];
    if (S.msg[0]) snprintf(m, sizeof(m), "Opened %s. Note: %s", path_base(path), S.msg);
    else if (S.p.target[0] && !S.p.gmFixes) snprintf(m, sizeof(m), "Opened %s: a game song, with the game's own instruments and settings.", path_base(path));
    else snprintf(m, sizeof(m), "Opened %s. Each instrument starts as the closest match from the game; click one to change it.", path_base(path));
    snprintf(sStatus, sizeof(sStatus), "%s", m);
}

static int ends_with(const char *s, const char *e)
{
    size_t a = strlen(s), b = strlen(e);
    if (a < b) return 0;
    for (size_t i = 0; i < b; i++) if (tolower((unsigned char)s[a - b + i]) != e[i]) return 0;
    return 1;
}

static void open_path(const char *path)
{
    stop_song();
    static Studio tmp;
    studio_init(&tmp);
    if (!studio_open(&tmp, path)) {
        set_status("Couldn't open it: %s", tmp.msg);
        project_free(&tmp.p);
        if (tmp.haveConv) conv_free(&tmp.c);
        return;
    }
    if (S.haveConv) conv_free(&S.c);
    project_free(&S.p);
    S = tmp;
    // the engine song points into S: rebuild the pointers for the moved copy
    S.song.voices = S.voices;
    for (int i = 0; i < S.c.ntracks; i++) S.song.data[i] = S.c.data[i];
    after_open(path);
}

static void ask_path(int action, const char *def)
{
    sModal = MODAL_PATH;
    sPathAction = action;
    snprintf(sPathBuf, sizeof(sPathBuf), "%s", def ? def : "");
}

static void do_save(const char *path)
{
    char err[512];
    char p[1100];
    snprintf(p, sizeof(p), "%s%s", path, ends_with(path, ".mmsproj") ? "" : ".mmsproj");
    if (project_save(&S.p, p, err, sizeof(err))) set_status("Saved %s", path_base(p));
    else set_status("Couldn't save: %s", err);
}

static void default_name(char *buf, int n, const char *ext)
{
    const char *stem = S.p.target[0] ? S.p.target : S.p.title[0] ? S.p.title : "song";
    snprintf(buf, n, "%s%s", stem, ext);
}

static int export_blocked(void)
{
    if (!have_song()) { set_status("%s", "Open a MIDI first."); return 1; }
    if (!S.p.target[0]) {
        set_status("%s", "Pick which game song this replaces first (SONG > Replaces).");
        sModal = MODAL_SONG_TARGET;
        sSearch[0] = 0; sPickSel = -1; sPickScroll = 0;
        return 1;
    }
    return 0;
}

static void do_export_kaisong(const char *path)
{
    char err[512], p[1100], warn[400];
    snprintf(p, sizeof(p), "%s%s", path, ends_with(path, ".kaisong") ? "" : ".kaisong");
    export_warnings(&S.p, &S.c, warn, sizeof(warn));
    if (export_kaisong(&S.p, &S.c, p, err, sizeof(err)))
        snprintf(sStatus, sizeof(sStatus), "Exported %s. %s", path_base(p), warn);
    else set_status("Couldn't export: %s", err);
}

static void do_export_decomp(const char *dir)
{
    char err[512], warn[400];
    export_warnings(&S.p, &S.c, warn, sizeof(warn));
    if (export_decomp(&S.p, &S.c, dir, err, sizeof(err)))
        snprintf(sStatus, sizeof(sStatus), "Wrote %s.s and %s_kai.inc. %s", S.p.target, S.p.target, warn);
    else set_status("Couldn't export: %s", err);
}

static void run_action(int act)
{
    char path[1024];
    int r;
    switch (act) {
    case ACT_OPEN_MIDI:
        r = dlg_open(path, sizeof(path), "Open a MIDI file", "MIDI files", "*.mid;*.midi");
        if (r > 0) open_path(path);
        else if (r < 0) ask_path(act, "");
        break;
    case ACT_OPEN_PROJECT:
        r = dlg_open(path, sizeof(path), "Open a project", "Moemon Music Studio projects", "*.mmsproj");
        if (r > 0) open_path(path);
        else if (r < 0) ask_path(act, "");
        break;
    case ACT_OPEN_DROPPED:
        open_path(sDropped);
        break;
    case ACT_SAVE_AS: {
        char def[300];
        default_name(def, sizeof(def), ".mmsproj");
        r = dlg_save(path, sizeof(path), "Save the project", "Moemon Music Studio projects", "*.mmsproj", def);
        if (r > 0) do_save(path);
        else if (r < 0) ask_path(act, def);
        break;
    }
    case ACT_EXPORT_KAISONG: {
        if (export_blocked()) break;
        char def[300];
        default_name(def, sizeof(def), ".kaisong");
        r = dlg_save(path, sizeof(path), "Export the song for Moemon Emerald Kai", "Kai songs", "*.kaisong", def);
        if (r > 0) do_export_kaisong(path);
        else if (r < 0) ask_path(act, def);
        break;
    }
    case ACT_EXPORT_DECOMP:
        if (export_blocked()) break;
        r = dlg_folder(path, sizeof(path), "Folder for the song's .s and voicegroup .inc");
        if (r > 0) do_export_decomp(path);
        else if (r < 0) ask_path(act, ".");
        break;
    case ACT_QUIT:
        sQuit = 1;
        break;
    }
}

// asks first when there are unsaved changes
static void guarded(int act)
{
    if (S.p.dirty && have_song()) {
        sModal = MODAL_CONFIRM;
        sConfirmAction = act;
        snprintf(sConfirmText, sizeof(sConfirmText), "The project has changes that aren't saved.");
        return;
    }
    run_action(act);
}

// ------------------------------------------------------------------ main view

static void draw_top(void)
{
    fill(0, 0, W, 44, C_PANEL);
    text(14, 14, "Moemon Music Studio", C_ACCENT);
    int x = 190;
    if (button(x, 8, 104, 28, "Open MIDI", 1)) guarded(ACT_OPEN_MIDI);
    x += 110;
    if (button(x, 8, 120, 28, "Open Project", 1)) guarded(ACT_OPEN_PROJECT);
    x += 126;
    if (button(x, 8, 64, 28, "Save", have_song())) {
        if (S.p.path[0]) do_save(S.p.path);
        else run_action(ACT_SAVE_AS);
    }
    x += 70;
    if (button(x, 8, 88, 28, "Save As", have_song())) run_action(ACT_SAVE_AS);
    x += 94;
    if (button(x, 8, 128, 28, "Export .kaisong", have_song())) run_action(ACT_EXPORT_KAISONG);
    x += 134;
    if (button(x, 8, 140, 28, "Export decomp", have_song())) run_action(ACT_EXPORT_DECOMP);
    x += 146;
    if (button(W - 44, 8, 32, 28, "?", 1)) sModal = MODAL_HELP;
    if (button(W - 84, 8, 36, 28, "Aa", 1)) sModal = MODAL_DISPLAY;
    char name[400];
    if (have_song())
        snprintf(name, sizeof(name), "%s%s", S.p.path[0] ? path_base(S.p.path) : S.p.midiName, S.p.dirty ? " *" : "");
    else name[0] = 0;
    textfit(x + 4, 14, W - 92 - x - 8, name, C_DIM);
}

static void draw_transport(void)
{
    int y = 50;
    fill(0, y, W, 46, C_BG);
    int playable = have_song();
    if (button(10, y + 8, 90, 30, sPlaying ? "Pause" : "Play", playable)) {
        if (sPlaying) pause_song();
        else play_from(sFrames >= S.totalFrames && S.loopStartFrame < 0 ? 0 : sFrames);
    }
    if (button(106, y + 8, 70, 30, "Stop", playable)) { stop_song(); sFrames = 0; }
    char t[64], tt[32];
    fmt_time(song_pos(), t, sizeof(t));
    fmt_time(S.totalFrames, tt, sizeof(tt));
    char both[96];
    snprintf(both, sizeof(both), "%s / %s", t, tt);
    text(190, y + 15, both, C_TEXT);
    // seek bar
    int bx = 320, bw = 700, by = y + 18;
    fill(bx, by, bw, 10, C_ROW);
    if (S.totalFrames > 0) {
        if (S.loopStartFrame >= 0) {
            int lx = bx + (int)((long)bw * S.loopStartFrame / S.totalFrames);
            fill(lx, by - 4, 2, 18, C_OK);
            fill(bx + bw - 2, by - 4, 2, 18, C_OK);
        }
        int px = bx + (int)((long)bw * song_pos() / S.totalFrames);
        if (px > bx + bw) px = bx + bw;
        fill(bx, by, px - bx, 10, C_ACCENT);
        if (playable && pressed(bx, by - 8, bw, 26)) {
            long f = (long)(mx - bx) * S.totalFrames / bw;
            if (sPlaying) play_from(f);
            else sFrames = f;
        }
    }
    if (S.loopStartFrame >= 0) text(bx, y + 30, "loop", C_OK);
    // DirectSound voices in use (the game mixes 5 at once)
    int busy = eng_ds_busy(&E);
    char v[64];
    int nv = E.playing ? E.numChans : (have_song() ? S.p.voices : ENG_DS_CHANNELS);
    snprintf(v, sizeof(v), "Voices %d/%d", busy, nv);
    text(1036, y + 6, v, busy >= nv ? C_WARN : C_DIM);
    for (int i = 0; i < nv; i++) fill(1036 + i * 10, y + 26, 8, 8, i < busy ? (busy >= nv ? C_WARN : C_OK) : C_ROW);
    if (have_song() && S.peakVoices >= S.p.voices) text(1164, y + 6, "all busy:", C_WARN), text(1164, y + 24, "notes cut", C_WARN);
}

static const char *slot_label(int slot, char *buf, int n)
{
    if (slot < 0) { snprintf(buf, n, "-"); return buf; }
    voice_name(&S.p.voice[slot], buf, n);
    return buf;
}

static void draw_tracks(void)
{
    int x = 10, y = 104, w = 620, h = 456;
    panel(x, y, w, h, "TRACKS");
    text(x + 90, y + 6, "M = leave out of the song   S = listen alone", C_DIM);
    if (!have_song() && !S.c.ncand) { text(x + 14, y + 40, "No song yet.", C_DIM); return; }
    int rowH = 22, top = y + 30, rows = (h - 36) / rowH;
    if (wheel && hot(x, y, w, h)) { sTrackScroll -= wheel * 2; }
    int n = S.c.ncand;
    if (sTrackScroll > n - rows) sTrackScroll = n - rows;
    if (sTrackScroll < 0) sTrackScroll = 0;
    // output track index per candidate
    int outIdx[CONV_MAX_CAND];
    for (int k = 0; k < n; k++) outIdx[k] = -1;
    for (int t = 0; t < S.c.ntracks; t++) outIdx[S.c.candOf[t]] = t;
    for (int r = 0; r < rows && sTrackScroll + r < n; r++) {
        int k = sTrackScroll + r;
        const ConvCand *cd = &S.c.cand[k];
        int ry = top + r * rowH;
        int muted = (S.p.muted >> k) & 1, solo = (S.p.solo >> k) & 1;
        int ot = outIdx[k];
        int over = ot >= (S.p.moreTracks ? ENG_MAX_TRACKS : ENG_GAME_TRACKS);
        int slot = cd->slot;
        if (slot < 0 && cd->program >= 0) slot = cd->program;
        fill(x + 6, ry, w - 22, rowH - 2, (slot >= 0 && slot == sSelSlot) ? C_HOT : C_ROW);
        // M / S
        fill(x + 10, ry + 2, 18, 17, muted ? C_WARN : C_BTN);
        text(x + 15, ry + 3, "M", muted ? C_BG : C_DIM);
        if (clicked(x + 10, ry + 2, 18, 17)) {
            S.p.muted ^= 1ull << k;
            S.p.dirty = 1;
            rebuild();
        }
        fill(x + 32, ry + 2, 18, 17, solo ? C_OK : C_BTN);
        text(x + 37, ry + 3, "S", solo ? C_BG : C_DIM);
        if (clicked(x + 32, ry + 2, 18, 17)) {
            S.p.solo ^= 1ull << k;
            update_mute_mask();
        }
        char lab[160];
        if (ot >= 0) snprintf(lab, sizeof(lab), "%2d", ot + 1);
        else snprintf(lab, sizeof(lab), " -");
        text(x + 58, ry + 3, lab, muted ? C_DIM : C_TEXT);
        snprintf(lab, sizeof(lab), "%s%sch %d", cd->name, cd->name[0] ? "  " : "", cd->chan + 1);
        textfit(x + 84, ry + 3, 200, lab, muted ? C_DIM : C_TEXT);
        char nm[160];
        slot_label(slot, nm, sizeof(nm));
        if (over && !muted) snprintf(nm, sizeof(nm), S.p.moreTracks ? "over the 32-track limit" : "over 10 tracks: see SONG");
        textfit(x + 292, ry + 3, 260, nm, over && !muted ? C_WARN : muted ? C_DIM : C_TEXT);
        if (slot >= 0 && clicked(x + 292, ry, 260, rowH)) sSelSlot = slot;
        // activity
        int lvl = (ot >= 0 && ot < ENG_MAX_TRACKS) ? E.trackLevel[ot] : 0;
        if (lvl > 7) fill(x + w - 56, ry + 6, 36 * lvl / 255, 9, C_ACCENT);
    }
    scrollbar(x + w - 12, top, rows * rowH, n, rows, &sTrackScroll);
}

static void open_instrument_picker(void)
{
    sModal = MODAL_INSTRUMENT;
    sSearch[0] = 0;
    sPickScroll = 0;
    sPickSel = inst_find_preset(&S.p.voice[sSelSlot]);
    sPickCat = -1;
    sPickKey = S.c.slotDrum[sSelSlot] ? 38 : 60;
    sPickJump = 1;
}

static void draw_slots(void)
{
    int x = 640, y = 104, w = 630, h = 456;
    panel(x, y, w, h, "INSTRUMENTS");
    text(x + 130, y + 6, "voice slot / from the MIDI / game instrument", C_DIM);
    int used[128], n = 0;
    for (int i = 0; i < 128; i++) if (S.c.slotUsed[i]) used[n++] = i;
    if (!n) { text(x + 14, y + 40, "No song yet.", C_DIM); return; }
    int rowH = 22, top = y + 30, rows = 9;
    if (wheel && hot(x, top, w, rows * rowH)) sSlotScroll -= wheel * 2;
    if (sSlotScroll > n - rows) sSlotScroll = n - rows;
    if (sSlotScroll < 0) sSlotScroll = 0;
    for (int r = 0; r < rows && sSlotScroll + r < n; r++) {
        int s = used[sSlotScroll + r];
        int ry = top + r * rowH;
        int ho = hot(x + 6, ry, w - 22, rowH - 2);
        fill(x + 6, ry, w - 22, rowH - 2, s == sSelSlot ? C_SEL : ho ? C_HOT : C_ROW);
        char lab[160];
        snprintf(lab, sizeof(lab), "%3d", s);
        text(x + 12, ry + 3, lab, C_DIM);
        if (S.c.slotDrum[s]) snprintf(lab, sizeof(lab), "Drums (ch 10)");
        else if (!S.p.gmFixes) snprintf(lab, sizeof(lab), "voice %d", s);   // the game's own numbering, not GM
        else snprintf(lab, sizeof(lab), "%s", sf_gm_names[S.c.slotGm[s] >= 0 ? S.c.slotGm[s] : s]);
        textfit(x + 48, ry + 3, 210, lab, C_TEXT);
        char nm[160];
        voice_name(&S.p.voice[s], nm, sizeof(nm));
        textfit(x + 266, ry + 3, w - 290, nm, C_ACCENT);
        if (clicked(x + 6, ry, w - 22, rowH - 2)) {
            sSelSlot = s;
            if (dblClick) open_instrument_picker();
        }
    }
    scrollbar(x + w - 12, top, rows * rowH, n, rows, &sSlotScroll);

    // editor for the selected slot
    int ey = top + rows * rowH + 8;
    fill(x + 6, ey, w - 12, h - (ey - y) - 6, C_BG);
    if (sSelSlot < 0 || !S.c.slotUsed[sSelSlot]) { text(x + 16, ey + 10, "Pick an instrument above.", C_DIM); return; }
    SfVoice *v = &S.p.voice[sSelSlot];
    char nm[200], det[300];
    voice_name(v, nm, sizeof(nm));
    voice_detail(v, det, sizeof(det));
    char head[260];
    snprintf(head, sizeof(head), "Slot %d: %s", sSelSlot, nm);
    textfit(x + 16, ey + 8, w - 32, head, C_TEXT);
    textfit(x + 16, ey + 28, w - 32, det, C_DIM);
    int pi = inst_find_preset(v);
    if (pi >= 0 && sf_presets[pi].songs[0]) {
        char u[300];
        snprintf(u, sizeof(u), "In the game: %s", sf_presets[pi].songs);
        textfit(x + 16, ey + 46, w - 32, u, C_DIM);
    }
    if (S.p.gmFixes && S.p.balance && S.gain[sSelSlot] != 1.0f) {
        char g[64];
        snprintf(g, sizeof(g), "volume x%.2f", S.gain[sSelSlot]);
        text(x + w - 16 - textw(g), ey + 8, g, C_OK);
    }
    int by = ey + 70;
    if (button(x + 16, by, 160, 26, "Change...", 1)) open_instrument_picker();
    if (button(x + 182, by, 120, 26, "GM match", 1)) {
        int preset = S.c.slotDrum[sSelSlot] ? SF_GM_DRUM_PRESET : sf_gm_preset[S.c.slotGm[sSelSlot] >= 0 ? S.c.slotGm[sSelSlot] : sSelSlot];
        *v = sf_presets[preset].voice;
        voices_changed();
    }
    // hold to hear
    int hx = x + 308, hw = 130;
    int ho = hot(hx, by, hw, 26);
    fill(hx, by, hw, 26, sHolding ? C_SEL : ho ? C_BTNHOT : C_BTN);
    frame_rect(hx, by, hw, 26, ho ? C_ACCENT : C_LINE);
    text(hx + 12, by + 5, "Hold to hear", C_TEXT);
    if (pressed(hx, by, hw, 26)) audition(v, S.c.slotDrum[sSelSlot] ? 38 : 60, 1);
    if (sHolding && !mDown) audition_off();
    text(x + 446, by + 5, "or keys Z-M", C_DIM);

    // envelope
    int ry = by + 36;
    if (voice_is_group(v)) {
        text(x + 16, ry + 2, "Drum kits and key-split instruments keep each piece's own sound.", C_DIM);
        return;
    }
    int cgb = voice_is_cgb(v);
    int a = v->attack, d = v->decay, s = v->sustain, r = v->release;
    int ch = 0;
    ch |= stepper(x + 12, ry, "Attack", &a, 0, cgb ? 7 : 255, 1, NULL);
    ch |= stepper(x + 162, ry, "Decay", &d, 0, cgb ? 7 : 255, 1, NULL);
    ch |= stepper(x + 306, ry, "Sustain", &s, 0, cgb ? 15 : 255, 1, NULL);
    ch |= stepper(x + 466, ry, "Release", &r, 0, cgb ? 7 : 255, 1, NULL);
    if (ch) {
        v->attack = (uint8_t)a; v->decay = (uint8_t)d; v->sustain = (uint8_t)s; v->release = (uint8_t)r;
        voices_changed();
    }
    ry += 26;
    text(x + 16, ry, cgb ? "Game Boy channel: steps of 1/64 s, 0-7 (sustain 0-15)"
                         : "Each frame: attack adds; decay and release multiply by n/256", C_DIM);
    ry += 22;
    if ((v->type & 7) == 1 || (v->type & 7) == 2) {
        static const char *duty[] = {"12.5%", "25%", "50%", "75%"};
        int du = v->ref & 3;
        if (stepper(x + 12, ry, "Duty", &du, 0, 3, 1, duty[du])) { v->ref = (int16_t)du; voices_changed(); }
    } else if ((v->type & 7) == 4) {
        int pe = v->ref & 1;
        if (stepper(x + 12, ry, "Noise", &pe, 0, 1, 1, pe ? "short" : "white")) { v->ref = (int16_t)pe; voices_changed(); }
    }
}

static const char *LOOP_LABELS[] = {"from the MIDI", "none", "whole song", "custom"};

static void draw_song(void)
{
    int x = 10, y = 568, w = 1260, h = 192;
    panel(x, y, w, h, "SONG");
    if (!have_song() && !S.c.ncand) return;
    Project *p = &S.p;
    int col1 = x + 16, row = y + 30;

    text(col1, row + 2, "Replaces", C_DIM);
    if (button(col1 + 80, row, 220, 22, p->target[0] ? p->target : "(pick a game song)", 1)) {
        sModal = MODAL_SONG_TARGET; sSearch[0] = 0; sPickSel = song_by_name(p->target); sPickScroll = 0;
    }
    text(col1 + 310, row + 2, "Title", C_DIM);
    textfit(col1 + 360, row + 2, 230, p->title, C_TEXT);
    row += 30;
    if (stepper(col1, row, "Volume", &p->masterVol, 0, 127, 1, NULL)) { p->dirty = 1; rebuild(); }
    int rev = p->reverb;
    char rb[16];
    snprintf(rb, sizeof(rb), rev < 0 ? "off" : "%d", rev);
    if (stepper(col1 + 150, row, "Reverb", &rev, -1, 127, rev < 0 ? 1 : 5, rb)) {
        if (rev > 127) rev = 127;
        p->reverb = rev;
        S.song.reverb = project_reverb_byte(p);
        lock(); E.reverb = rev < 0 ? 0 : (uint8_t)rev; unlock();
        p->dirty = 1;
    }
    if (stepper(col1 + 300, row, "Priority", &p->priority, 0, 255, 1, NULL)) {
        S.song.priority = (uint8_t)p->priority;
        p->dirty = 1;
    }
    if (stepper(col1 + 466, row, "Voices", &p->voices, 1, ENG_DS_MAX, 1, NULL)) {
        p->dirty = 1;
        rebuild();
        set_status("%s", p->voices > 5 ? "More sample voices mix at once. The game mixes 5 until it's set to more (m4aSoundMode)."
                                       : "Sample voices mixed at once (the game: 5).");
    }
    row += 30;
    if (checkbox(col1, row, "Exact note lengths (-E)", &p->exactGate)) { p->dirty = 1; rebuild(); }
    if (checkbox(col1 + 250, row, "GM MIDI fixes", &p->gmFixes)) { p->dirty = 1; rebuild(); }
    if (checkbox(col1 + 410, row, "Go over 10 tracks", &p->moreTracks)) {
        p->dirty = 1;
        rebuild();
        set_status("%s", p->moreTracks ? "Up to 32 tracks play and export. The game itself plays 10 until its music player gets more."
                                       : "Back to the game's 10 tracks.");
    }
    row += 26;
    if (p->gmFixes) {
        if (checkbox(col1, row, "Even out instrument volumes (like GM)", &p->balance)) { p->dirty = 1; rebuild(); }
        text(col1, row + 24, "GM fixes: expression, bend range, drums, loop clean-up, volumes.", C_DIM);
        text(col1, row + 42, "Off = converted exactly like the game's own songs.", C_DIM);
    } else {
        text(col1, row, "Converted exactly like the game's own songs (mid2agb).", C_DIM);
    }

    // loop
    int lx = x + 640, ly = y + 30;
    text(lx, ly + 2, "Loop", C_DIM);
    for (int i = 0; i < 4; i++) {
        int bx = lx + 50 + i * 128;
        if (p->loopMode == i) fill(bx - 2, ly - 2, 124, 26, C_ACCENT);
        if (button(bx, ly, 120, 22, LOOP_LABELS[i], 1) && p->loopMode != i) {
            if (i == LOOP_CUSTOM && !p->loopEnd) {
                p->loopStart = S.c.hasLoop ? S.c.loopStartTick : 0;
                p->loopEnd = S.c.hasLoop ? S.c.loopEndTick : S.c.endTick;
            }
            p->loopMode = i;
            p->dirty = 1;
            rebuild();
        }
    }
    ly += 34;
    int bar = S.p.midi.division * 4;   // a 4/4 bar in MIDI ticks
    if (bar <= 0) bar = 1920;
    if (p->loopMode == LOOP_CUSTOM) {
        int ls = (int)(p->loopStart / (uint32_t)bar) + 1, le = (int)((p->loopEnd + (uint32_t)bar - 1) / (uint32_t)bar) + 1;
        int maxBar = (int)(S.c.endTick / (uint32_t)bar) + 2;
        int c1 = stepper(lx + 50, ly, "Start bar", &ls, 1, maxBar, 1, NULL);
        int c2 = stepper(lx + 260, ly, "End bar", &le, 2, maxBar + 1, 1, NULL);
        if (c1 || c2) {
            if (le <= ls) le = ls + 1;
            p->loopStart = (uint32_t)(ls - 1) * (uint32_t)bar;
            p->loopEnd = (uint32_t)(le - 1) * (uint32_t)bar;
            p->dirty = 1;
            rebuild();
        }
        text(lx + 470, ly + 2, "(bars of 4 beats)", C_DIM);
    } else {
        char m[200];
        if (S.c.hasLoop)
            snprintf(m, sizeof(m), "Loops from beat %u back to beat %u.", S.c.loopEndTick / (uint32_t)S.p.midi.division + 1,
                     S.c.loopStartTick / (uint32_t)S.p.midi.division + 1);
        else snprintf(m, sizeof(m), p->loopMode == LOOP_NONE ? "Plays once and stops." : "The MIDI has no loop markers: it plays once.");
        text(lx + 50, ly + 2, m, C_DIM);
    }
    ly += 34;
    if (button(lx + 50, ly, 300, 24, "Use a game song's instruments...", 1)) {
        sModal = MODAL_SONG_VOICES; sSearch[0] = 0; sPickSel = -1; sPickScroll = 0;
    }
    if (button(lx + 360, ly, 200, 24, "All to GM matches", 1)) {
        project_default_voices(p, &S.c, 1);
        voices_changed();
    }
}

static void draw_status(void)
{
    fill(0, H - 34, W, 34, C_PANEL);
    char warn[400] = "";
    if (have_song()) export_warnings(&S.p, &S.c, warn, sizeof(warn));
    textfit(12, H - 25, W - 24, sStatus, C_TEXT);
    (void)warn;
}

// ------------------------------------------------------------------ modals

static int str_has(const char *hay, const char *needle)
{
    if (!needle[0]) return 1;
    size_t n = strlen(needle);
    for (; *hay; hay++) {
        size_t k = 0;
        while (k < n && hay[k] && tolower((unsigned char)hay[k]) == tolower((unsigned char)needle[k])) k++;
        if (k == n) return 1;
    }
    return 0;
}

static void search_field(int x, int y, int w)
{
    fill(x, y, w, 24, C_ROW);
    frame_rect(x, y, w, 24, C_ACCENT);
    size_t l = strlen(sSearch);
    for (const char *t = sTyped; *t; t++) if (l < sizeof(sSearch) - 1 && (unsigned char)*t >= 32) { sSearch[l++] = *t; sSearch[l] = 0; sPickScroll = 0; }
    if (kBackspace && l) { sSearch[--l] = 0; sPickScroll = 0; }
    text(x + 8, y + 4, sSearch[0] ? sSearch : "type to search", sSearch[0] ? C_TEXT : C_DIM);
    if ((SDL_GetTicks() / 500) & 1) fill(x + 8 + textw(sSearch), y + 4, 2, 16, C_TEXT);
}

static void modal_box(int *x, int *y, int w, int h, const char *title)
{
    SDL_SetRenderDrawBlendMode(sRen, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sRen, 0, 0, 0, 150);
    SDL_Rect all = {0, 0, W, H};
    SDL_RenderFillRect(sRen, &all);
    SDL_SetRenderDrawBlendMode(sRen, SDL_BLENDMODE_NONE);
    *x = (W - w) / 2;
    *y = (H - h) / 2;
    fill(*x, *y, w, h, C_PANEL);
    frame_rect(*x, *y, w, h, C_ACCENT);
    text(*x + 14, *y + 10, title, C_ACCENT);
}

static void draw_instrument_picker(void)
{
    int x, y, w = 1180, h = 700;
    modal_box(&x, &y, w, h, "CHOOSE AN INSTRUMENT FROM THE GAME");
    char sub[200];
    snprintf(sub, sizeof(sub), "for slot %d (%s). Click to hear it, double-click or Use to pick it.", sSelSlot,
             S.c.slotDrum[sSelSlot] ? "drums" : sf_gm_names[S.c.slotGm[sSelSlot] >= 0 ? S.c.slotGm[sSelSlot] : sSelSlot]);
    text(x + 320, y + 10, sub, C_DIM);
    // categories
    int cy = y + 40;
    for (int c = -1; c < SF_NUM_CATS; c++) {
        int ry = cy + (c + 1) * 26;
        int count = 0;
        for (int i = 0; i < SF_NUM_PRESETS; i++) if (c < 0 || sf_presets[i].cat == c) count++;
        char lab[64];
        snprintf(lab, sizeof(lab), "%s (%d)", c < 0 ? "Everything" : sf_cat_names[c], count);
        int ho = hot(x + 10, ry, 200, 24);
        fill(x + 10, ry, 200, 24, sPickCat == c ? C_SEL : ho ? C_HOT : C_ROW);
        text(x + 18, ry + 4, lab, C_TEXT);
        if (clicked(x + 10, ry, 200, 24)) { sPickCat = c; sPickScroll = 0; }
    }
    // search + list
    int lx = x + 222, lw = w - 236;
    search_field(lx, y + 40, 360);
    char kn[16];
    note_name(sPickKey, kn, sizeof(kn));
    stepper(lx + 380, y + 42, "Hear at", &sPickKey, 0, 127, 1, kn);
    int list[SF_NUM_PRESETS], n = 0;
    for (int i = 0; i < SF_NUM_PRESETS; i++) {
        const SfPreset *pr = &sf_presets[i];
        if (sPickCat >= 0 && pr->cat != sPickCat) continue;
        if (!str_has(pr->name, sSearch) && !str_has(pr->songs, sSearch) && !str_has(sf_cat_names[pr->cat], sSearch)) continue;
        list[n++] = i;
    }
    int top = y + 74, rowH = 22, rows = (h - 74 - 90) / rowH;
    if (wheel && hot(lx, top, lw, rows * rowH)) sPickScroll -= wheel * 3;
    if (kPgDn) sPickScroll += rows;
    if (kPgUp) sPickScroll -= rows;
    int selPos = -1;
    for (int i = 0; i < n; i++) if (list[i] == sPickSel) selPos = i;
    if (sPickJump) {   // show the current choice when the window opens
        if (selPos >= 0) sPickScroll = selPos - rows / 3;
        sPickJump = 0;
    }
    if (kDown || kUp) {
        selPos = selPos < 0 ? 0 : selPos + (kDown ? 1 : -1);
        if (selPos < 0) selPos = 0;
        if (selPos >= n) selPos = n - 1;
        if (n) {
            sPickSel = list[selPos];
            audition(&sf_presets[sPickSel].voice, sPickKey, 0);
            if (selPos < sPickScroll) sPickScroll = selPos;
            if (selPos >= sPickScroll + rows) sPickScroll = selPos - rows + 1;
        }
    }
    if (sPickScroll > n - rows) sPickScroll = n - rows;
    if (sPickScroll < 0) sPickScroll = 0;
    int use = 0;
    for (int r = 0; r < rows && sPickScroll + r < n; r++) {
        int i = list[sPickScroll + r];
        const SfPreset *pr = &sf_presets[i];
        int ry = top + r * rowH;
        int ho = hot(lx, ry, lw - 14, rowH - 2);
        fill(lx, ry, lw - 14, rowH - 2, i == sPickSel ? C_SEL : ho ? C_HOT : C_ROW);
        textfit(lx + 8, ry + 3, 330, pr->name, C_TEXT);
        char det[200];
        const SfVoice *v = &pr->voice;
        if (voice_is_group(v)) snprintf(det, sizeof(det), "%s", voice_kind(v));
        else snprintf(det, sizeof(det), "A%d D%d S%d R%d", v->attack, v->decay, v->sustain, v->release);
        text(lx + 344, ry + 3, det, C_DIM);
        textfit(lx + 520, ry + 3, lw - 540, pr->songs[0] ? pr->songs : "(not in a song)", C_DIM);
        if (clicked(lx, ry, lw - 14, rowH - 2)) {
            sPickSel = i;
            audition(&pr->voice, sPickKey, 0);
            if (dblClick) use = 1;
        }
    }
    scrollbar(lx + lw - 10, top, rows * rowH, n, rows, &sPickScroll);
    char cnt[64];
    snprintf(cnt, sizeof(cnt), "%d shown", n);
    text(lx + 640, y + 44, cnt, C_DIM);
    // footer
    int fy = y + h - 76;
    if (sPickSel >= 0) {
        char det[300];
        voice_detail(&sf_presets[sPickSel].voice, det, sizeof(det));
        textfit(x + 16, fy, w - 32, det, C_DIM);
        char u[300];
        snprintf(u, sizeof(u), "Used in %d slot(s) of the game's songs%s%s", sf_presets[sPickSel].uses,
                 sf_presets[sPickSel].songs[0] ? ": " : "", sf_presets[sPickSel].songs);
        textfit(x + 16, fy + 20, w - 32, u, C_DIM);
    }
    if (button(x + w - 360, y + h - 44, 110, 30, "Use", sPickSel >= 0) || (kEnter && sPickSel >= 0)) use = 1;
    if (button(x + w - 240, y + h - 44, 110, 30, "GM match", 1)) {
        sPickSel = S.c.slotDrum[sSelSlot] ? SF_GM_DRUM_PRESET : sf_gm_preset[S.c.slotGm[sSelSlot] >= 0 ? S.c.slotGm[sSelSlot] : sSelSlot];
        sPickCat = -1; sSearch[0] = 0;
        sPickJump = 1;
        audition(&sf_presets[sPickSel].voice, sPickKey, 0);
    }
    if (button(x + w - 120, y + h - 44, 104, 30, "Cancel", 1) || kEsc) sModal = MODAL_NONE;
    if (use) {
        S.p.voice[sSelSlot] = sf_presets[sPickSel].voice;
        S.p.voiceSet[sSelSlot] = 1;
        voices_changed();
        char nm[160];
        voice_name(&S.p.voice[sSelSlot], nm, sizeof(nm));
        snprintf(sStatus, sizeof(sStatus), "Slot %d is now %s.", sSelSlot, nm);
        sModal = MODAL_NONE;
    }
}

static void draw_song_picker(int forVoices)
{
    int x, y, w = 760, h = 640;
    modal_box(&x, &y, w, h, forVoices ? "USE THE INSTRUMENTS OF A GAME SONG" : "WHICH GAME SONG DOES THIS REPLACE?");
    text(x + 14, y + 30, forVoices ? "Every slot this MIDI uses gets what that song's voicegroup has in the same slot."
                                   : "The game plays this song in its place (the export is named after it).", C_DIM);
    search_field(x + 14, y + 54, 300);
    int list[SF_NUM_SONGS], n = 0;
    for (int i = 0; i < SF_NUM_SONGS; i++)
        if (str_has(sf_songs[i].name, sSearch)) list[n++] = i;
    int top = y + 88, rowH = 22, rows = (h - 88 - 60) / rowH;
    if (wheel && hot(x, top, w, rows * rowH)) sPickScroll -= wheel * 3;
    if (kPgDn) sPickScroll += rows;
    if (kPgUp) sPickScroll -= rows;
    if (sPickScroll > n - rows) sPickScroll = n - rows;
    if (sPickScroll < 0) sPickScroll = 0;
    int use = 0;
    for (int r = 0; r < rows && sPickScroll + r < n; r++) {
        int i = list[sPickScroll + r];
        int ry = top + r * rowH;
        int ho = hot(x + 14, ry, w - 40, rowH - 2);
        fill(x + 14, ry, w - 40, rowH - 2, i == sPickSel ? C_SEL : ho ? C_HOT : C_ROW);
        text(x + 22, ry + 3, sf_songs[i].name, C_TEXT);
        char d[120];
        snprintf(d, sizeof(d), "instruments: %s", sf_songs[i].group);
        text(x + 330, ry + 3, d, C_DIM);
        if (clicked(x + 14, ry, w - 40, rowH - 2)) { sPickSel = i; if (dblClick) use = 1; }
    }
    scrollbar(x + w - 22, top, rows * rowH, n, rows, &sPickScroll);
    if (button(x + w - 240, y + h - 44, 110, 30, "Use", sPickSel >= 0) || (kEnter && sPickSel >= 0)) use = 1;
    if (button(x + w - 120, y + h - 44, 104, 30, "Cancel", 1) || kEsc) sModal = MODAL_NONE;
    if (use && sPickSel >= 0) {
        if (forVoices) {
            int g = song_group_by_name(sf_songs[sPickSel].group);
            if (g >= 0) {
                project_use_song_group(&S.p, &S.c, g);
                voices_changed();
                set_status("Instruments now come from %s.", sf_songs[sPickSel].name);
            } else set_status("%s has no voicegroup of its own.", sf_songs[sPickSel].name);
        } else {
            snprintf(S.p.target, sizeof(S.p.target), "%s", sf_songs[sPickSel].name);
            S.p.dirty = 1;
            set_status("The export will replace %s.", S.p.target);
        }
        sModal = MODAL_NONE;
    }
}

static void draw_path_prompt(void)
{
    int x, y, w = 820, h = 150;
    static const char *titles[] = {"", "OPEN A MIDI FILE", "OPEN A PROJECT", "SAVE THE PROJECT AS", "EXPORT .KAISONG TO", "EXPORT DECOMP FILES TO FOLDER", "", "OPEN"};
    modal_box(&x, &y, w, h, titles[sPathAction]);
    text(x + 14, y + 34, "No file dialog on this system: type the path (or drop a file on the window).", C_DIM);
    fill(x + 14, y + 60, w - 28, 26, C_ROW);
    frame_rect(x + 14, y + 60, w - 28, 26, C_ACCENT);
    size_t l = strlen(sPathBuf);
    for (const char *t = sTyped; *t; t++) if (l < sizeof(sPathBuf) - 1) { sPathBuf[l++] = *t; sPathBuf[l] = 0; }
    if (kBackspace && l) sPathBuf[--l] = 0;
    int maxc = (w - 44) / 8;
    const char *shown = (int)l > maxc ? sPathBuf + l - maxc : sPathBuf;
    text(x + 22, y + 65, shown, C_TEXT);
    if ((SDL_GetTicks() / 500) & 1) fill(x + 22 + textw(shown), y + 65, 2, 16, C_TEXT);
    int ok = button(x + w - 240, y + h - 44, 110, 30, "OK", l > 0) || (kEnter && l > 0);
    if (button(x + w - 120, y + h - 44, 104, 30, "Cancel", 1) || kEsc) { sModal = MODAL_NONE; return; }
    if (ok) {
        sModal = MODAL_NONE;
        switch (sPathAction) {
        case ACT_OPEN_MIDI: case ACT_OPEN_PROJECT: open_path(sPathBuf); break;
        case ACT_SAVE_AS: do_save(sPathBuf); break;
        case ACT_EXPORT_KAISONG: do_export_kaisong(sPathBuf); break;
        case ACT_EXPORT_DECOMP: do_export_decomp(sPathBuf); break;
        }
    }
}

static void draw_confirm(void)
{
    int x, y, w = 560, h = 150;
    modal_box(&x, &y, w, h, "UNSAVED CHANGES");
    text(x + 14, y + 40, sConfirmText, C_TEXT);
    if (button(x + 14, y + h - 44, 150, 30, "Save first", 1)) {
        sModal = MODAL_NONE;
        if (S.p.path[0]) { do_save(S.p.path); run_action(sConfirmAction); }
        else run_action(ACT_SAVE_AS);
    }
    if (button(x + w - 250, y + h - 44, 120, 30, "Discard", 1)) { sModal = MODAL_NONE; S.p.dirty = 0; run_action(sConfirmAction); }
    if (button(x + w - 120, y + h - 44, 104, 30, "Cancel", 1) || kEsc) sModal = MODAL_NONE;
}

static void draw_help(void)
{
    int x, y, w = 900, h = 520;
    modal_box(&x, &y, w, h, "MOEMON MUSIC STUDIO " STUDIO_VERSION);
    static const char *lines[] = {
        "Plays MIDI files with Moemon Emerald Kai's own instruments and sound engine, so a song",
        "sounds here as it will in the game (5 sample voices + 4 Game Boy channels, 10 tracks;",
        "SONG > Go over 10 tracks plays and exports up to 32 and Voices mixes up to 12 samples",
        "at once; the game can't play those yet).",
        "",
        "1. Open MIDI (or drop a .mid on the window). Each MIDI instrument starts as the game",
        "   instrument closest to its General MIDI sound; drums use a game drum kit.",
        "2. Click an instrument and Change... it, or tune its envelope. Hold to hear / keys Z-M.",
        "3. M leaves a track out of the song, S plays a track alone.",
        "4. SONG: pick the game song it replaces, its volume, reverb and loop.",
        "5. Save the project (.mmsproj keeps the MIDI inside), then Export:",
        "   .kaisong       one file with the song, its instruments and settings, for the game",
        "   Export decomp  <song>.s + voicegroup .inc in the game source's own format",
        "",
        "The game's own MIDIs (sound/songs/midi) open with their real instruments and settings",
        "and convert exactly like the game build converts them.",
        "",
        "Space: play / pause.  Esc: close a window.  Mouse wheel: scroll lists and change numbers.",
        "Aa (top right): the font (Pixel, Smooth, Bold) and the window size.",
    };
    for (int i = 0; i < (int)(sizeof(lines) / sizeof(lines[0])); i++) text(x + 20, y + 44 + i * 22, lines[i], C_TEXT);
    if (button(x + w - 120, y + h - 44, 104, 30, "Close", 1) || kEsc || kEnter) sModal = MODAL_NONE;
}

// ------------------------------------------------------------------ main loop

static const char PIANO_KEYS[] = "zsxdcvgbhnjm";

static void handle_piano(SDL_Keycode k, int down)
{
    if (sModal != MODAL_NONE || sSelSlot < 0 || !S.c.slotUsed[sSelSlot]) return;
    const char *p = strchr(PIANO_KEYS, (int)k);
    if (!p || !k) return;
    int key = 60 + (int)(p - PIANO_KEYS);
    if (down) audition(&S.p.voice[sSelSlot], key, 1);
    else audition_off();
}

static SDL_Texture *font_texture(int scale, int pixelFont, const unsigned char *alpha)
{
    int gw = 8 * scale, gh = 16 * scale;
    SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, 95 * gw, gh, 32, SDL_PIXELFORMAT_RGBA32);
    Uint32 *px = (Uint32 *)s->pixels;
    for (int c = 0; c < 95; c++)
        for (int y = 0; y < gh; y++)
            for (int x = 0; x < gw; x++) {
                unsigned a = pixelFont ? ((kFont[c][y] & (0x80 >> x)) ? 255 : 0) : alpha[(c * gh + y) * gw + x];
                px[y * (s->pitch / 4) + c * gw + x] = SDL_MapRGBA(s->format, 255, 255, 255, (Uint8)a);
            }
    SDL_Texture *t = SDL_CreateTextureFromSurface(sRen, s);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    // the pixel font keeps its pixels square; the others are smoothed when scaled
    SDL_SetTextureScaleMode(t, pixelFont ? SDL_ScaleModeNearest : SDL_ScaleModeLinear);
    SDL_FreeSurface(s);
    return t;
}

static void make_fonts(void)
{
    sFonts[FONT_PIXEL][0] = sFonts[FONT_PIXEL][1] = sFonts[FONT_PIXEL][2] = font_texture(1, 1, NULL);
    const unsigned char *p = ui_fonts;
    for (int f = FONT_SMOOTH; f <= FONT_BOLD; f++)
        for (int sc = 1; sc <= 3; sc++) {
            sFonts[f][sc - 1] = font_texture(sc, 0, p);
            p += 95 * (8 * sc) * (16 * sc);
        }
}

// Display settings live in the user's settings folder (SDL_GetPrefPath), not with projects.
static void settings_path(char *buf, int n)
{
    char *dir = SDL_GetPrefPath("Moemon", "MusicStudio");
    snprintf(buf, n, "%ssettings.ini", dir ? dir : "");
    SDL_free(dir);
}

static void settings_load(void)
{
    char path[1024], line[128];
    settings_path(path, sizeof(path));
    FILE *f = fopen(path, "r");
    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        int v;
        if (sscanf(line, "font=%d", &v) == 1 && v >= 0 && v < NUM_FONTS) sFontChoice = v;
        if (sscanf(line, "window=%d", &v) == 1) {
            for (int i = 0; i < NUM_WIN_SIZES; i++) if (WIN_SIZES[i] == v) sWinSize = i;
        }
    }
    fclose(f);
}

static void settings_save(void)
{
    char path[1024];
    settings_path(path, sizeof(path));
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "font=%d\nwindow=%d\n", sFontChoice, WIN_SIZES[sWinSize]);
    fclose(f);
}

// Sizes the window: a share of 1280x800, or as big as fits, never bigger than the screen.
static void apply_window_size(void)
{
    SDL_Rect r;
    int d = SDL_GetWindowDisplayIndex(sWin);
    if (SDL_GetDisplayUsableBounds(d < 0 ? 0 : d, &r) != 0) { r.w = W; r.h = H; r.x = r.y = 0; }
    int maxw = r.w - 16, maxh = r.h - 48;   // room for the title bar
    float k = WIN_SIZES[sWinSize] ? WIN_SIZES[sWinSize] / 100.0f : 10.0f;
    if (W * k > maxw) k = (float)maxw / W;
    if (H * k > maxh) k = (float)maxh / H;
    if (!WIN_SIZES[sWinSize]) k *= 0.95f;
    SDL_SetWindowSize(sWin, (int)(W * k), (int)(H * k));
    SDL_SetWindowPosition(sWin, SDL_WINDOWPOS_CENTERED_DISPLAY(d < 0 ? 0 : d), SDL_WINDOWPOS_CENTERED_DISPLAY(d < 0 ? 0 : d));
}

static void draw_display(void)
{
    int x, y, w = 760, h = 330;
    modal_box(&x, &y, w, h, "DISPLAY");
    text(x + 20, y + 44, "Font", C_DIM);
    for (int i = 0; i < NUM_FONTS; i++) {
        int bx = x + 120 + i * 130;
        if (sFontChoice == i) fill(bx - 2, y + 38, 124, 32, C_ACCENT);
        if (button(bx, y + 40, 120, 28, FONT_NAMES[i], 1) && sFontChoice != i) { sFontChoice = i; settings_save(); }
    }
    fill(x + 20, y + 84, w - 40, 60, C_BG);
    text(x + 32, y + 94, "Moemon Music Studio  Route 101 (key-split)  A255 D0 S255 R165", C_TEXT);
    text(x + 32, y + 116, "0123456789  The quick brown Zigzagoon jumps over Wurmple.", C_DIM);
    text(x + 20, y + 170, "Window", C_DIM);
    for (int i = 0; i < NUM_WIN_SIZES; i++) {
        int bx = x + 120 + i * 102;
        char lab[16];
        if (WIN_SIZES[i]) snprintf(lab, sizeof(lab), "%d%%", WIN_SIZES[i]);
        else snprintf(lab, sizeof(lab), "Fit screen");
        if (sWinSize == i) fill(bx - 2, y + 164, 98, 32, C_ACCENT);
        if (button(bx, y + 166, 94, 28, lab, 1)) { sWinSize = i; apply_window_size(); settings_save(); }
    }
    text(x + 20, y + 214, "A bigger window makes everything bigger. Smooth and Bold stay sharp at any size;", C_DIM);
    text(x + 20, y + 234, "the window can also be resized by dragging its edges.", C_DIM);
    if (button(x + w - 120, y + h - 44, 104, 30, "Close", 1) || kEsc || kEnter) sModal = MODAL_NONE;
}

int ui_main(int argc, char **argv)
{
    // Windows: draw at the screen's real pixels instead of letting display scaling stretch (and blur) the window
    SDL_SetHint("SDL_WINDOWS_DPI_AWARENESS", "permonitorv2");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    settings_load();
    sWin = SDL_CreateWindow("Moemon Music Studio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H,
                            SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    sRen = SDL_CreateRenderer(sWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sRen) sRen = SDL_CreateRenderer(sWin, -1, 0);
    SDL_RenderSetLogicalSize(sRen, W, H);
    make_fonts();
    if (!getenv("MMS_SHOT")) apply_window_size();

    inst_init();
    studio_init(&S);
    eng_init(&E);
    SDL_AudioSpec want, got;
    SDL_zero(want);
    want.freq = ENG_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = audio_cb;
    sDev = SDL_OpenAudioDevice(NULL, 0, &want, &got, 0);
    if (!sDev) snprintf(sStatus, sizeof(sStatus), "No sound device (%s). Exports still work.", SDL_GetError());
    else SDL_PauseAudioDevice(sDev, 0);
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_StartTextInput();

    if (argc >= 2) open_path(argv[1]);
    if (getenv("MMS_TEST_FONT")) sFontChoice = atoi(getenv("MMS_TEST_FONT")) % NUM_FONTS;   // tests: a font
    if (getenv("MMS_TEST_WINDOW")) {                                                       // tests: "WxH"
        int ww = W, wh = H;
        sscanf(getenv("MMS_TEST_WINDOW"), "%dx%d", &ww, &wh);
        SDL_SetWindowSize(sWin, ww, wh);
    }
    if (getenv("MMS_TEST_SLOT")) sSelSlot = atoi(getenv("MMS_TEST_SLOT"));   // tests: select an instrument
    if (getenv("MMS_TEST_PLAY") && have_song()) play_from(0);                // tests: start playing
    if (getenv("MMS_TEST_MODAL")) {   // tests: open a window for a screenshot
        sModal = atoi(getenv("MMS_TEST_MODAL"));
        if (sModal == MODAL_INSTRUMENT && sSelSlot >= 0) open_instrument_picker();
    }

    while (!sQuit) {
        mPressed = mReleased = wheel = dblClick = 0;
        kBackspace = kEnter = kEsc = kUp = kDown = kPgUp = kPgDn = kSpace = 0;
        sTyped[0] = 0;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT: guarded(ACT_QUIT); break;
            case SDL_MOUSEMOTION: mx = ev.motion.x; my = ev.motion.y; break;
            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    mx = ev.button.x; my = ev.button.y;
                    mDown = 1; mPressed = 1; pressX = mx; pressY = my;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    mx = ev.button.x; my = ev.button.y;
                    mDown = 0; mReleased = 1;
                    Uint32 now = SDL_GetTicks();
                    dblClick = now - lastClickTime < 400;
                    lastClickTime = now;
                }
                break;
            case SDL_MOUSEWHEEL: wheel += ev.wheel.y; break;
            case SDL_TEXTINPUT:
                if (sModal == MODAL_INSTRUMENT || sModal == MODAL_SONG_TARGET || sModal == MODAL_SONG_VOICES || sModal == MODAL_PATH)
                    strncat(sTyped, ev.text.text, sizeof(sTyped) - strlen(sTyped) - 1);
                break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {
                case SDLK_BACKSPACE: kBackspace = 1; break;
                case SDLK_RETURN: case SDLK_KP_ENTER: kEnter = 1; break;
                case SDLK_ESCAPE: kEsc = 1; break;
                case SDLK_UP: kUp = 1; break;
                case SDLK_DOWN: kDown = 1; break;
                case SDLK_PAGEUP: kPgUp = 1; break;
                case SDLK_PAGEDOWN: kPgDn = 1; break;
                case SDLK_SPACE: kSpace = 1; break;
                default: break;
                }
                if (!ev.key.repeat) handle_piano(ev.key.keysym.sym, 1);
                break;
            case SDL_KEYUP:
                handle_piano(ev.key.keysym.sym, 0);
                break;
            case SDL_DROPFILE: {
                snprintf(sDropped, sizeof(sDropped), "%s", ev.drop.file);
                SDL_free(ev.drop.file);
                sModal = MODAL_NONE;
                guarded(ACT_OPEN_DROPPED);
                break;
            }
            }
        }
        if (kSpace && sModal == MODAL_NONE && have_song()) {
            if (sPlaying) pause_song();
            else play_from(sFrames >= S.totalFrames && S.loopStartFrame < 0 ? 0 : sFrames);
        }

        sFontNow = font_scale();
        color(C_BG);
        SDL_RenderClear(sRen);
        sInModal = 0;
        draw_top();
        draw_transport();
        draw_tracks();
        draw_slots();
        draw_song();
        draw_status();
        sInModal = 1;
        switch (sModal) {
        case MODAL_INSTRUMENT: draw_instrument_picker(); break;
        case MODAL_SONG_TARGET: draw_song_picker(0); break;
        case MODAL_SONG_VOICES: draw_song_picker(1); break;
        case MODAL_PATH: draw_path_prompt(); break;
        case MODAL_CONFIRM: draw_confirm(); break;
        case MODAL_HELP: draw_help(); break;
        case MODAL_DISPLAY: draw_display(); break;
        }
        SDL_RenderPresent(sRen);
        if (getenv("MMS_SHOT")) {   // tests: save a screenshot and quit
            static int frames;
            if (++frames == atoi(getenv("MMS_SHOT_FRAME") ? getenv("MMS_SHOT_FRAME") : "10")) {
                int ow = W, oh = H;
                SDL_GetRendererOutputSize(sRen, &ow, &oh);
                SDL_Surface *sh = SDL_CreateRGBSurfaceWithFormat(0, ow, oh, 32, SDL_PIXELFORMAT_ARGB8888);
                SDL_RenderReadPixels(sRen, NULL, SDL_PIXELFORMAT_ARGB8888, sh->pixels, sh->pitch);
                SDL_SaveBMP(sh, getenv("MMS_SHOT"));
                SDL_FreeSurface(sh);
                sQuit = 1;
            }
        }
    }
    if (sDev) SDL_CloseAudioDevice(sDev);
    SDL_Quit();
    return 0;
}
