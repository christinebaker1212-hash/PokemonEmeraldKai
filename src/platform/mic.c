// Moemon Kai: the microphone, for CHATOT's CHATTER (include/mic.h). SDL's recording devices, opened only while the
// game listens: the pick (every device at once, the first spoken into wins) and a recording (the chosen device, or the
// PC's default, for one second). Devices are heard at 8000 samples a second and each four averaged into one, so a
// recording is Platinum's own kind of sound: 2000 signed 8-bit samples (sound_chatot.c's CHATOT_CRY_SAMPLING_RATE).
// Tests: MOEMON_MIC_WAV=file.wav stands in for a microphone ("TEST MICROPHONE"), fed in as fast as a real one would be.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "mic.h"

void MPNet_Log(const char *fmt, ...);

#define IN_RATE      8000
#define IN_SAMPLES   (MIC_RECORD_SAMPLES * (IN_RATE / MIC_RECORD_RATE))   // one second heard
#define MAX_DEVICES  16
#define PICK_LOUD    6000                                                // a peak this loud picks the device
#define PICK_FRAMES  (60 * 15)                                           // the pick gives up after 15 seconds

static char sSelected[256];   // "" = the default device

struct Listener
{
    SDL_AudioDeviceID dev;
    char name[256];
    SDL_atomic_t peak;        // the loudest sample since the game last looked
};

static struct
{
    // the pick
    bool8 picking;
    u16 pickFrames;
    struct Listener pick[MAX_DEVICES];
    u8 pickCount;
    // a recording
    bool8 recording;
    SDL_AudioDeviceID dev;
    s16 in[IN_SAMPLES];
    SDL_atomic_t have;        // samples heard so far
    SDL_atomic_t peak;
    u8 level;
    u32 startedMs;            // (SDL_GetTicks when the recording began)
    // the test file: 8000 samples a second
    bool8 testLoaded;
    s16 *test;
    u32 testLen, testPos;
} M;

// ---------------------------------------------------------------------------------------------------------------
// The test microphone (MOEMON_MIC_WAV)

static const char *TestPath(void)
{
    const char *p = getenv("MOEMON_MIC_WAV");
    return p != NULL && p[0] ? p : NULL;
}

static u32 LE32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }
static u16 LE16(const u8 *p) { return p[0] | p[1] << 8; }

// The WAV as 8000 samples a second, one channel (8- or 16-bit PCM)
static void LoadTest(void)
{
    FILE *f;
    long n;
    u8 *d;
    u32 i = 12, rate = 0, bits = 0, ch = 0, dataOff = 0, dataLen = 0, frames;
    if (M.testLoaded || TestPath() == NULL)
        return;
    M.testLoaded = TRUE;
    f = fopen(TestPath(), "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    d = n > 44 ? malloc(n) : NULL;
    if (d == NULL || fread(d, 1, n, f) != (size_t)n || memcmp(d, "RIFF", 4) != 0 || memcmp(d + 8, "WAVE", 4) != 0)
    {
        free(d);
        fclose(f);
        return;
    }
    fclose(f);
    while (i + 8 <= (u32)n)
    {
        u32 len = LE32(d + i + 4);
        if (memcmp(d + i, "fmt ", 4) == 0 && len >= 16)
            ch = LE16(d + i + 10), rate = LE32(d + i + 12), bits = LE16(d + i + 22);
        else if (memcmp(d + i, "data", 4) == 0)
            dataOff = i + 8, dataLen = len;
        i += 8 + len + (len & 1);
    }
    if (rate == 0 || ch == 0 || (bits != 8 && bits != 16) || dataOff == 0 || dataOff + dataLen > (u32)n)
    {
        free(d);
        return;
    }
    frames = dataLen / (ch * bits / 8);
    M.testLen = (u32)((u64)frames * IN_RATE / rate);
    M.test = malloc(sizeof(s16) * (M.testLen + 1));
    for (i = 0; M.test != NULL && i < M.testLen; i++)
    {
        u32 src = (u32)((u64)i * rate / IN_RATE);
        const u8 *s = d + dataOff + src * ch * (bits / 8);
        M.test[i] = bits == 16 ? (s16)LE16(s) : (s16)((s[0] - 128) << 8);
    }
    free(d);
}

static bool8 TestMode(void)
{
    LoadTest();
    return M.test != NULL;
}

// ---------------------------------------------------------------------------------------------------------------
// Devices

static bool8 AudioReady(void)
{
    static bool8 sTried;
    if (!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) && !sTried)
    {
        // (the tests start the game without sound: the recording devices still want SDL's audio)
        sTried = TRUE;
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
            MPNet_Log("microphone: no audio (%s)", SDL_GetError());
    }
    return (SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) != 0;
}

int Mic_Count(void)
{
    int n;
    if (TestMode())
        return 1;
    if (!AudioReady())
        return 0;
    n = SDL_GetNumAudioDevices(1);
    return n < 0 ? 0 : n > MAX_DEVICES ? MAX_DEVICES : n;
}

const char *Mic_Name(int i)
{
    const char *name;
    if (TestMode())
        return i == 0 ? "TEST MICROPHONE" : "";
    name = AudioReady() ? SDL_GetAudioDeviceName(i, 1) : NULL;
    return name != NULL ? name : "";
}

const char *Mic_SelectedName(void)
{
    return sSelected;
}

void Mic_SetSelected(const char *name)
{
    snprintf(sSelected, sizeof(sSelected), "%s", name != NULL ? name : "");
}

static int SelectedIndex(void)
{
    int i, n = Mic_Count();
    if (sSelected[0] == 0)
        return -1;
    for (i = 0; i < n; i++)
        if (strcmp(Mic_Name(i), sSelected) == 0)
            return i;
    return -2;   // (chosen, but not plugged in now)
}

void Mic_CycleSelected(int dir)
{
    int n = Mic_Count() + 1, cur = SelectedIndex() + 1;   // 0 = DEFAULT
    if (cur < 0)
        cur = 0;
    cur = ((cur + dir) % n + n) % n;
    Mic_SetSelected(cur == 0 ? "" : Mic_Name(cur - 1));
    MPNet_Log("microphone: %s", sSelected[0] ? sSelected : "the default");
}

bool8 Mic_IsAvailable(void)
{
    return Mic_Count() > 0 && SelectedIndex() != -2;
}

static SDL_AudioDeviceID Open(const char *name, SDL_AudioCallback cb, void *user)
{
    SDL_AudioSpec want, got;
    SDL_zero(want);
    want.freq = IN_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 256;
    want.callback = cb;
    want.userdata = user;
    return SDL_OpenAudioDevice(name != NULL && name[0] ? name : NULL, 1, &want, &got, 0);
}

static int Peak(const s16 *s, int n)
{
    int i, p = 0;
    for (i = 0; i < n; i++)
        if (abs(s[i]) > p)
            p = abs(s[i]);
    return p;
}

// ---------------------------------------------------------------------------------------------------------------
// The pick: every device listened to at once

static void SDLCALL PickCallback(void *user, Uint8 *stream, int len)
{
    struct Listener *l = user;
    int p = Peak((const s16 *)stream, len / 2);
    if (p > SDL_AtomicGet(&l->peak))
        SDL_AtomicSet(&l->peak, p);
}

void Mic_StartPick(void)
{
    int i, n = Mic_Count();
    Mic_StopPick();
    M.picking = TRUE;
    M.pickFrames = 0;
    M.pickCount = 0;
    M.testPos = 0;
    if (TestMode())
    {
        MPNet_Log("microphone: listening to 1 device(s) for the one spoken into");
        return;
    }
    for (i = 0; i < n && M.pickCount < MAX_DEVICES; i++)
    {
        struct Listener *l = &M.pick[M.pickCount];
        snprintf(l->name, sizeof(l->name), "%s", Mic_Name(i));
        SDL_AtomicSet(&l->peak, 0);
        l->dev = Open(l->name, PickCallback, l);
        if (l->dev != 0)
        {
            SDL_PauseAudioDevice(l->dev, 0);
            M.pickCount++;
        }
    }
    MPNet_Log("microphone: listening to %d device(s) for the one spoken into", M.pickCount);
}

void Mic_StopPick(void)
{
    u8 i;
    for (i = 0; i < M.pickCount; i++)
        if (M.pick[i].dev != 0)
            SDL_CloseAudioDevice(M.pick[i].dev);
    M.pickCount = 0;
    M.picking = FALSE;
}

bool8 Mic_IsPicking(void)
{
    return M.picking;
}

static void PickUpdate(void)
{
    u8 i, best = 0xFF;
    int bestPeak = PICK_LOUD, loudest = 0;
    if (TestMode())
    {
        // the test file is the only device: its first loud stretch picks it
        u32 i2, end = M.testPos + IN_RATE / 60;
        for (i2 = M.testPos; i2 < end && i2 < M.testLen; i2++)
            if (abs(M.test[i2]) > loudest)
                loudest = abs(M.test[i2]);
        M.testPos = end;
        M.level = (u8)(loudest * 16 / 32768);
        if (loudest >= PICK_LOUD)
        {
            Mic_SetSelected("TEST MICROPHONE");
            MPNet_Log("microphone: picked %s", sSelected);
            Mic_StopPick();
        }
    }
    else
    {
        for (i = 0; i < M.pickCount; i++)
        {
            int p = SDL_AtomicSet(&M.pick[i].peak, 0);
            if (p > loudest)
                loudest = p;
            if (p >= bestPeak)
                bestPeak = p, best = i;
        }
        M.level = (u8)(loudest * 16 / 32768);
        if (best != 0xFF)
        {
            Mic_SetSelected(M.pick[best].name);
            MPNet_Log("microphone: picked %s", sSelected);
            Mic_StopPick();
            return;
        }
    }
    if (M.picking && ++M.pickFrames >= PICK_FRAMES)
    {
        MPNet_Log("microphone: nothing heard, the pick ends");
        Mic_StopPick();
    }
}

// ---------------------------------------------------------------------------------------------------------------
// A recording: one second from the chosen device

static void SDLCALL RecordCallback(void *user, Uint8 *stream, int len)
{
    const s16 *s = (const s16 *)stream;
    int n = len / 2, have = SDL_AtomicGet(&M.have), take = IN_SAMPLES - have, p;
    (void)user;
    if (take <= 0)
        return;
    if (take > n)
        take = n;
    memcpy(M.in + have, s, take * sizeof(s16));
    p = Peak(s, take);
    if (p > SDL_AtomicGet(&M.peak))
        SDL_AtomicSet(&M.peak, p);
    SDL_AtomicAdd(&M.have, take);
}

bool8 Mic_StartRecording(void)
{
    Mic_StopPick();
    Mic_StopRecording();
    SDL_AtomicSet(&M.have, 0);
    SDL_AtomicSet(&M.peak, 0);
    M.testPos = 0;
    if (TestMode())
    {
        M.recording = TRUE;
        MPNet_Log("microphone: recording from %s", "TEST MICROPHONE");
        return TRUE;
    }
    if (!Mic_IsAvailable())
    {
        MPNet_Log("microphone: none to record from");
        return FALSE;
    }
    M.dev = Open(sSelected, RecordCallback, NULL);
    if (M.dev == 0 && sSelected[0])
        MPNet_Log("microphone: %s didn't open (%s)", sSelected, SDL_GetError());
    if (M.dev == 0)
        return FALSE;
    M.recording = TRUE;
    M.startedMs = SDL_GetTicks();
    SDL_PauseAudioDevice(M.dev, 0);
    MPNet_Log("microphone: recording from %s", sSelected[0] ? sSelected : "the default");
    return TRUE;
}

static void RecordUpdate(void)
{
    if (TestMode())
    {
        // as fast as a microphone hears it: a frame's worth of the file
        int have = SDL_AtomicGet(&M.have), take = IN_RATE / 60, i;
        if (take > IN_SAMPLES - have)
            take = IN_SAMPLES - have;
        for (i = 0; i < take; i++)
            M.in[have + i] = M.testPos + i < M.testLen ? M.test[M.testPos + i] : 0;
        SDL_AtomicSet(&M.peak, Peak(M.in + have, take));
        M.testPos += take;
        SDL_AtomicAdd(&M.have, take);
    }
    M.level = (u8)(SDL_AtomicSet(&M.peak, 0) * 16 / 32768);
}

bool8 Mic_RecordingDone(void)
{
    return M.recording && SDL_AtomicGet(&M.have) >= IN_SAMPLES;
}

// A device that stopped sending: three seconds and still not one heard (the test file is fed by frames: never)
bool8 Mic_RecordingStalled(void)
{
    return M.recording && !Mic_RecordingDone() && M.dev != 0 && SDL_GetTicks() - M.startedMs > 3000;
}

void Mic_TakeRecording(s8 *out)
{
    int i, k;
    if (M.dev != 0)
        SDL_PauseAudioDevice(M.dev, 1);
    for (i = 0; i < MIC_RECORD_SAMPLES; i++)
    {
        // four heard samples averaged into one (the high byte of their 16 bits: signed 8-bit, as Platinum's MIC)
        int sum = 0, v;
        for (k = 0; k < IN_RATE / MIC_RECORD_RATE; k++)
            sum += M.in[i * (IN_RATE / MIC_RECORD_RATE) + k];
        v = sum / (IN_RATE / MIC_RECORD_RATE) / 256;
        out[i] = (s8)(v < -128 ? -128 : v > 127 ? 127 : v);
    }
    Mic_StopRecording();
}

void Mic_StopRecording(void)
{
    if (M.dev != 0)
        SDL_CloseAudioDevice(M.dev);
    M.dev = 0;
    M.recording = FALSE;
    M.level = 0;
}

u8 Mic_Level(void)
{
    return M.level > 16 ? 16 : M.level;
}

void Mic_Update(void)
{
    if (M.picking)
        PickUpdate();
    else if (M.recording && !Mic_RecordingDone())
        RecordUpdate();
}
