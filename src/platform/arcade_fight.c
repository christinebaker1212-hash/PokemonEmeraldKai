// Moemon Fight: a CPS-2 arcade board (include/arcade.h) as the VS cabinet's fight engine (include/fight_engine.h),
// so the GAME CENTER's arcade cabinets get everything the MUGEN cabinet has: the CRT picture, the sound in place of
// the game's, spectating, 1P play and online VS with rollback and desync recovery (src/mp/mp_fight.c).
//
// The players' fight buttons are the board's: X Y Z its punches (LP MP HP), A B C its kicks (LK MK HK). The board has
// no START of its own for the player: a START press drops a coin (frames 0-4 after it) and then presses the board's
// START (frames 8-14, and as long as it's held), so START alone joins the game, as on a free-play cabinet. Holding
// START for 2 seconds leaves the cabinet (MF_RESULT_CANCELLED); online the leave counts only once that frame's
// buttons can't be corrected any more (LEAVE_SURE frames later), so a guessed button never ends a session.
//
// The 384x224 picture is shown as an arcade monitor shows it, 4:3: 427x320 in the middle of the 480x320 fight
// picture, black at the sides. The board's sound (24 kHz stereo) is stretched over each game frame's samples.
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arcade.h"
#include "fight_engine.h"
#include "platform/fight.h"   // (FIGHT_ARCADE_PIC_W)

unsigned char gFightKind;

#define BOOT_FRAMES   300     // online: the board's start-up runs on known buttons only (the input delay settles)
#define KEEP          12      // frames kept for rollback (mp_fight.c guesses up to 8 ahead)
#define LEAVE_FRAMES  120     // START held this long leaves
#define LEAVE_SURE    12      // frames after that before it counts (past any guess)
#define PIC_W         FIGHT_ARCADE_PIC_W   // the board's picture at 4:3 in the fight picture
#define PIC_X         ((MF_SCREEN_W - PIC_W) / 2)
#define STATE_CAP     (8u << 20)

// What the cabinet's own START handling needs to remember (kept with each frame for rollback)
struct ArcPads
{
    int16_t seq[2];      // frames since a START press began its coin-and-start (-1: none)
    int16_t held[2];     // frames START has been held
    uint32_t leaveAt;    // the frame START had been held long enough (0: not)
    uint8_t prev[2];     // START held on the last frame
};

static int sActive, sGame = -1, sResult, sRollback;
static unsigned int sFrames, sSerial;
static struct ArcPads sPads;
static char sError[160];
static unsigned short sPicture[MF_SCREEN_W * MF_SCREEN_H];
static int sHavePicture;

static struct
{
    unsigned int frame;
    size_t size;
    struct ArcPads pads;
    uint8_t *data;
} sKept[KEEP];
static unsigned int sKeptCount;   // valid entries, newest at (sKeptNext - 1)
static unsigned int sKeptNext;

static uint8_t *sStateBuf, *sPutBuf;
static size_t sStateSize, sPutSize;

static int16_t sAudio[8192 * 2];
static int sAudioPairs;

// ---------------------------------------------------------------------------------------------------------------
// Buttons

static uint16_t BoardButtons(int p, unsigned short b)
{
    uint16_t out = 0;
    int start = (b & MF_BTN_START) != 0;

    if (b & MF_BTN_UP)
        out |= ARCADE_UP;
    if (b & MF_BTN_DOWN)
        out |= ARCADE_DOWN;
    if (b & MF_BTN_LEFT)
        out |= ARCADE_LEFT;
    if (b & MF_BTN_RIGHT)
        out |= ARCADE_RIGHT;
    if (b & MF_BTN_X)
        out |= ARCADE_LP;
    if (b & MF_BTN_Y)
        out |= ARCADE_MP;
    if (b & MF_BTN_Z)
        out |= ARCADE_HP;
    if (b & MF_BTN_A)
        out |= ARCADE_LK;
    if (b & MF_BTN_B)
        out |= ARCADE_MK;
    if (b & MF_BTN_C)
        out |= ARCADE_HK;

    if (start && !sPads.prev[p] && sPads.seq[p] < 0)
        sPads.seq[p] = 0;
    if (sPads.seq[p] >= 0)
    {
        int s = sPads.seq[p];
        if (s <= 4)
            out |= ARCADE_COIN;
        else if (s >= 8)
            out |= ARCADE_START;
        if (++sPads.seq[p] > 14)
            sPads.seq[p] = -1;
    }
    else if (start && sPads.held[p] > 14)
    {
        out |= ARCADE_START;
    }
    sPads.held[p] = start ? sPads.held[p] + 1 : 0;
    sPads.prev[p] = (uint8_t)start;
    if (sPads.held[p] == LEAVE_FRAMES && sPads.leaveAt == 0)
        sPads.leaveAt = sFrames + 1;
    return out;
}

// ---------------------------------------------------------------------------------------------------------------
// The picture: 384x224 (0x00BBGGRR) to 427x320, each output pixel from its four nearest (bilinear), RGB565

static void Picture(void)
{
    static int sReady;
    static uint16_t xi[PIC_W], xw[PIC_W], yi[MF_SCREEN_H], yw[MF_SCREEN_H];
    const uint32_t *src = Arcade_Picture();
    int x, y;

    if (src == NULL)
        return;
    if (!sReady)
    {
        for (x = 0; x < PIC_W; x++)
        {
            int f = (int)(((x + 0.5) * ARCADE_WIDTH / PIC_W - 0.5) * 256);
            if (f < 0)
                f = 0;
            if (f > (ARCADE_WIDTH - 1) * 256)
                f = (ARCADE_WIDTH - 1) * 256;
            xi[x] = (uint16_t)(f >> 8);
            xw[x] = (uint16_t)(f & 255);
        }
        for (y = 0; y < MF_SCREEN_H; y++)
        {
            int f = (int)(((y + 0.5) * ARCADE_HEIGHT / MF_SCREEN_H - 0.5) * 256);
            if (f < 0)
                f = 0;
            if (f > (ARCADE_HEIGHT - 1) * 256)
                f = (ARCADE_HEIGHT - 1) * 256;
            yi[y] = (uint16_t)(f >> 8);
            yw[y] = (uint16_t)(f & 255);
        }
        sReady = 1;
    }
    for (y = 0; y < MF_SCREEN_H; y++)
    {
        const uint32_t *r0 = src + yi[y] * ARCADE_WIDTH;
        const uint32_t *r1 = yi[y] + 1 < ARCADE_HEIGHT ? r0 + ARCADE_WIDTH : r0;
        unsigned int wy = yw[y];
        unsigned short *out = sPicture + y * MF_SCREEN_W + PIC_X;
        for (x = 0; x < PIC_W; x++)
        {
            unsigned int i = xi[x], j = i + 1 < ARCADE_WIDTH ? i + 1 : i, wx = xw[x];
            uint32_t a = r0[i], b = r0[j], c = r1[i], d = r1[j];
            unsigned int w00 = (256 - wx) * (256 - wy), w01 = wx * (256 - wy), w10 = (256 - wx) * wy, w11 = wx * wy;
            // (the board's colors are 0x00BBGGRR)
            unsigned int R = ((a & 255) * w00 + (b & 255) * w01 + (c & 255) * w10 + (d & 255) * w11) >> 16;
            unsigned int G = ((a >> 8 & 255) * w00 + (b >> 8 & 255) * w01 + (c >> 8 & 255) * w10
                              + (d >> 8 & 255) * w11) >> 16;
            unsigned int B = ((a >> 16 & 255) * w00 + (b >> 16 & 255) * w01 + (c >> 16 & 255) * w10
                              + (d >> 16 & 255) * w11) >> 16;
            out[x] = (unsigned short)((R >> 3) << 11 | (G >> 2) << 5 | (B >> 3));
        }
    }
    sHavePicture = 1;
    sSerial++;
}

// ---------------------------------------------------------------------------------------------------------------
// Frames

static void Keep(void)
{
    unsigned int at = sKeptNext % KEEP;
    size_t size;

    if (!sRollback || sFrames < BOOT_FRAMES)
        return;
    if (sKept[at].data == NULL && (sKept[at].data = malloc(STATE_CAP)) == NULL)
        return;
    size = Arcade_SaveState(sKept[at].data, STATE_CAP);
    if (size == 0)
        return;
    sKept[at].size = size;
    sKept[at].frame = sFrames;
    sKept[at].pads = sPads;
    sKeptNext++;
    if (sKeptCount < KEEP)
        sKeptCount++;
}

static void Forget(void)
{
    sKeptCount = 0;
    sKeptNext = 0;
}

static void End(int result)
{
    sResult = result;
    sActive = 0;
    Arcade_Close();
    Forget();
    sAudioPairs = 0;
}

static void RunFrame(const unsigned short buttons[2], int quiet)
{
    uint16_t p1 = BoardButtons(0, buttons[0]), p2 = BoardButtons(1, buttons[1]);
    int16_t scratch[2048 * 2];

    Arcade_Frame(p1, p2);
    sFrames++;
    if (quiet)
    {
        while (Arcade_TakeAudio(scratch, 2048) > 0)
            ;
    }
    else
    {
        int room = (int)(sizeof(sAudio) / sizeof(sAudio[0]) / 2) - sAudioPairs;
        if (room < 1024)
        {
            sAudioPairs = 0;   // nobody's taking it (the sound is off): start again
            room = (int)(sizeof(sAudio) / sizeof(sAudio[0]) / 2);
        }
        sAudioPairs += Arcade_TakeAudio(sAudio + sAudioPairs * 2, room);
        Picture();
    }
    Keep();
}

int ArcFight_PackSummary(int game, struct MfPack *out)
{
    char why[160];
    int ok = Arcade_Check(game, why, sizeof(why));

    memset(out, 0, sizeof(*out));
    out->core = ok ? 0xA2C0DE00u + (unsigned int)game : 0;
    return ok;
}

int ArcFight_Begin(int game, const struct MfSession *session)
{
    if (sActive)
        End(MF_RESULT_CANCELLED);
    sError[0] = 0;
    sResult = MF_RESULT_NONE;
    if (!Arcade_Open(game))
    {
        snprintf(sError, sizeof(sError), "the %s set (%s) doesn't load", Arcade_Title(game), Arcade_SetFiles(game));
        sResult = MF_RESULT_ERROR;
        return 0;
    }
    sGame = game;
    sRollback = session->rollback;
    sFrames = 0;
    memset(&sPads, 0, sizeof(sPads));
    sPads.seq[0] = sPads.seq[1] = -1;
    Forget();
    sAudioPairs = 0;
    sHavePicture = 0;
    memset(sPicture, 0, sizeof(sPicture));
    sActive = 1;
    return 1;
}

int ArcFight_IsActive(void)
{
    return sActive;
}

int ArcFight_Step(const unsigned short buttons[2])
{
    if (!sActive)
        return 0;
    RunFrame(buttons, 0);
    if (sPads.leaveAt != 0 && sFrames >= sPads.leaveAt + (sRollback ? LEAVE_SURE : 0))
        End(MF_RESULT_CANCELLED);
    return 1;
}

unsigned int ArcFight_FrameCount(void)
{
    return sFrames;
}

int ArcFight_Kept(unsigned int *oldest)
{
    unsigned int newest;

    if (!sActive || sKeptCount == 0)
        return 0;
    newest = sKept[(sKeptNext - 1) % KEEP].frame;
    if (oldest)
        *oldest = sKept[(sKeptNext - sKeptCount) % KEEP].frame;
    return newest == sFrames;
}

int ArcFight_RollBack(unsigned int frame)
{
    unsigned int i;

    for (i = 0; i < sKeptCount; i++)
    {
        unsigned int at = (sKeptNext - 1 - i) % KEEP;
        if (sKept[at].frame != frame)
            continue;
        if (!Arcade_LoadState(sKept[at].data, sKept[at].size))
            return 0;
        sFrames = frame;
        sPads = sKept[at].pads;
        // the frames after it are run again (and kept again)
        sKeptNext -= i;
        sKeptCount -= i;
        return 1;
    }
    return 0;
}

void ArcFight_StepQuiet(const unsigned short buttons[2])
{
    if (sActive)
        RunFrame(buttons, 1);
}

unsigned int ArcFight_Hash(void)
{
    unsigned int h = Arcade_Hash();
    return h ^ (sPads.leaveAt * 2654435761u) ^ ((unsigned int)sPads.held[0] << 8) ^ ((unsigned int)sPads.held[1] << 20);
}

// ---------------------------------------------------------------------------------------------------------------
// Desync recovery: the board's save state as the "globals" region (4 KB pages), the cabinet's START handling in
// the host values; no heap

static int Serialize(uint8_t **buf, size_t *size)
{
    if (*buf == NULL && (*buf = malloc(STATE_CAP)) == NULL)
        return 0;
    *size = Arcade_SaveState(*buf, STATE_CAP);
    return *size != 0;
}

int ArcFight_StateInfo(struct MfStateInfo *out)
{
    memset(out, 0, sizeof(*out));
    if (!sActive || !Serialize(&sStateBuf, &sStateSize))
        return 0;
    out->globalsSize = (unsigned int)sStateSize;
    out->layout = 0xA2C00000u ^ (unsigned int)sGame;
    out->pages[MF_STATE_GLOBALS] = (unsigned int)((sStateSize + MF_STATE_PAGE - 1) / MF_STATE_PAGE);
    memcpy(out->host, &sPads, sizeof(sPads));
    return 1;
}

static unsigned int PageHash(const uint8_t *p, size_t n)
{
    unsigned int h = 2166136261u;
    size_t i;
    for (i = 0; i < n; i++)
        h = (h ^ p[i]) * 16777619u;
    return h | 1;
}

int ArcFight_StateHashes(int region, unsigned int first, unsigned int count, unsigned int *out)
{
    unsigned int i;

    for (i = 0; i < count; i++)
    {
        size_t at = (size_t)(first + i) * MF_STATE_PAGE;
        if (region != MF_STATE_GLOBALS || at >= sStateSize)
            out[i] = 0;
        else
            out[i] = PageHash(sStateBuf + at, sStateSize - at < MF_STATE_PAGE ? sStateSize - at : MF_STATE_PAGE);
    }
    return 1;
}

int ArcFight_StatePage(int region, unsigned int index, unsigned char *out)
{
    size_t at = (size_t)index * MF_STATE_PAGE;

    memset(out, 0, MF_STATE_PAGE);
    if (region != MF_STATE_GLOBALS || at >= sStateSize)
        return 0;
    memcpy(out, sStateBuf + at, sStateSize - at < MF_STATE_PAGE ? sStateSize - at : MF_STATE_PAGE);
    return 1;
}

int ArcFight_StateBegin(void)
{
    Forget();
    return sActive && Serialize(&sPutBuf, &sPutSize);
}

int ArcFight_StatePut(int region, unsigned int index, const unsigned char *page)
{
    size_t at = (size_t)index * MF_STATE_PAGE;

    if (region != MF_STATE_GLOBALS || sPutBuf == NULL || at >= sPutSize)
        return 0;
    memcpy(sPutBuf + at, page, sPutSize - at < MF_STATE_PAGE ? sPutSize - at : MF_STATE_PAGE);
    return 1;
}

int ArcFight_StateEnd(const struct MfStateInfo *from)
{
    if (!sActive || sPutBuf == NULL || from->globalsSize != sPutSize)
        return 0;
    if (!Arcade_LoadState(sPutBuf, sPutSize))
        return 0;
    memcpy(&sPads, from->host, sizeof(sPads));
    Keep();
    return 1;
}

void ArcFight_TestPoke(int delta)
{
    (void)delta;   // (the MUGEN engine's test; a board's memory isn't changed from outside)
}

// ---------------------------------------------------------------------------------------------------------------
// Picture, sound, results

const unsigned short *ArcFight_Frame(void)
{
    return sHavePicture ? sPicture : NULL;
}

unsigned int ArcFight_FrameSerial(void)
{
    return sSerial;
}

int ArcFight_Result(void)
{
    return sActive ? MF_RESULT_NONE : sResult;
}

void ArcFight_Cancel(void)
{
    if (sActive)
        End(MF_RESULT_CANCELLED);
}

const char *ArcFight_Error(void)
{
    return sError;
}

// The board's sound since the last call, stretched over `frames` samples (the game asks once a frame)
void ArcFight_MixAudio(short *stereo, int frames, int rate)
{
    int i;

    (void)rate;
    if (sAudioPairs == 0)
        return;
    for (i = 0; i < frames; i++)
    {
        double pos = (i + 0.5) * sAudioPairs / frames - 0.5;
        int a = pos < 0 ? 0 : (int)pos, b = a + 1 < sAudioPairs ? a + 1 : a;
        double t = pos < 0 ? 0 : pos - a;
        stereo[i * 2] = (short)(sAudio[a * 2] * (1 - t) + sAudio[b * 2] * t);
        stereo[i * 2 + 1] = (short)(sAudio[a * 2 + 1] * (1 - t) + sAudio[b * 2 + 1] * t);
    }
    sAudioPairs = 0;
}
