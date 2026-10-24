// Moemon Co-op: watching other players' screens.
//
// A player streams their screen to whoever is watching: changed 8x8 tiles
// (run-length or palette packed), 15-20 times a second, plus the battle
// music's song number. On the map the picture scrolls: the update says how far
// and only the tiles that came into view and the ones that changed go along.
//
// Watching a battle: walk up to a player marked IN BATTLE and press A. The
// watcher's own game pauses while the feed is shown (src/platform/sdl2.c asks
// MP_Spectate_IsWatching every frame) and resumes exactly where it was when
// they press B or the battle ends.
//
// The SPECTATE panel (CO-OP > SPECTATE): every friend's screen, all the time,
// in a row under your own (src/platform/display.c draws it), 1-4 times.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "money.h"
#include "battle.h"
#include "constants/battle.h"
#include "map_name_popup.h"
#include "main.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "menu.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "m4a.h"
#include "constants/songs.h"
#include "field_camera.h"
#include "palette.h"
#include "platform/display.h"
#include "mp_net.h"
#include "mp_join.h"

#define FRAME_W 240
#define FRAME_H 160
#define TILES_X (FRAME_W / 8)
#define TILES_Y (FRAME_H / 8)
#define NUM_TILES (TILES_X * TILES_Y)
#define SEND_EVERY 3              // frames between updates (20 per second)
#define KEYFRAME_EVERY (60 * 4)   // full refresh now and then, in case anything was lost
#define MAX_PAYLOAD 7600
#define LAST_SONG 610

enum {
    MPR_SPEC_JOIN = MPR_SPECTATE_FIRST,
    MPR_SPEC_LEAVE,
    MPR_SPEC_FRAME,     // u16 seq, u8 flags, u16 song, u8 count, tiles...
    MPR_SPEC_END,       // the battle is over (or wasn't running)
    MPR_SPEC_PANEL_JOIN,   // send me your screen for my SPECTATE panel (any time, not only battles)
    MPR_SPEC_PANEL_LEAVE,
};

// MPR_SPEC_FRAME: u8 type, u16 seq, u8 flags, u16 song, u8 tile count, s8 shiftX, s8 shiftY, tiles...
#define FRAME_HEADER 9
#define FRAMEF_KEY   1   // this update starts a full frame
#define FRAMEF_SHIFT 2   // first move the picture: new (x, y) = old (x + shiftX, y + shiftY)
#define FRAMEF_LAST  4   // the update's last message (always sent: viewers know the sender is there)
#define PANEL_SEND_EVERY 4   // 15 updates a second when only panels watch
#define SEND_BACKLOG (24 * 1024)   // more than this still waiting to go out: skip a picture (the game's own messages first)

const u16 *Platform_GetFrame(void);
u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);

// ---- Streaming (the player in the battle) ----
static struct
{
    u8 viewers;                    // bitmask of player ids watching our battle
    u8 panel;                      // ... and of those with us in their SPECTATE panel
    bool8 needKey;
    u16 seq;
    u16 timer;
    u16 keyTimer;
    u16 lastSong;
    u16 ref[FRAME_W * FRAME_H];    // what the viewers have
} sStream;

// ---- Watching ----
static struct
{
    u8 target;                     // player id, 0xFF = not watching
    bool8 gotFrame;
    bool8 ending;
    u16 endTimer;
    u16 song;
    u16 prevKeys;
    u16 updates;                   // frame updates received (log)
    u8 name[MP_NAME_LEN + 1];
    u16 frame[FRAME_W * FRAME_H];
    u16 shown[FRAME_W * FRAME_H];  // frame plus the banner
} sWatch = {0xFF};

static u8 sOut[MAX_PAYLOAD + 16];

// Tests (MOEMON_SPECTRACE=1): each update's picture checksum, the sender's (what viewers should have) and each
// viewer's (what it made of it), so a test can prove they're the same pixel for pixel
static s8 sTrace = -1;
static bool8 Tracing(void)
{
    if (sTrace < 0)
        sTrace = getenv("MOEMON_SPECTRACE") != NULL;
    return sTrace;
}

static u32 FrameSum(const u16 *buf)
{
    u32 h = 2166136261u;
    int i;
    for (i = 0; i < FRAME_W * FRAME_H; i++)
        h = (h ^ buf[i]) * 16777619u;
    return h;
}

bool8 MP_Spectate_IsWatching(void)
{
    return sWatch.target != 0xFF;
}

bool8 MP_Spectate_IsStreaming(void)
{
    return (sStream.viewers | sStream.panel) != 0;
}

// ---------------------------------------------------------------------------
// Encoding

static u16 CurrentSong(void)
{
    u16 i;
    if (gMPlayInfo_BGM.songHeader == NULL || (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE))
        return 0;
    for (i = 0; i < LAST_SONG; i++)
        if (gSongTable[i].header == gMPlayInfo_BGM.songHeader)
            return i;
    return 0;
}

static bool8 TileChanged(const u16 *frame, int t)
{
    int tx = (t % TILES_X) * 8, ty = (t / TILES_X) * 8, y;
    for (y = 0; y < 8; y++)
        if (memcmp(frame + (ty + y) * FRAME_W + tx, sStream.ref + (ty + y) * FRAME_W + tx, 16) != 0)
            return TRUE;
    return FALSE;
}

// Bits per pixel of a palette-packed tile with n colors
static int IndexBits(int n)
{
    return n <= 2 ? 1 : (n <= 4 ? 2 : 4);
}

// Appends one tile; returns bytes written. Mode 0 = raw, 1 = runs of a color, 2 = up to 16 colors and their
// indices (1, 2 or 4 bits a pixel), whichever is smallest
static int PackTile(const u16 *frame, int t, u8 *out)
{
    int tx = (t % TILES_X) * 8, ty = (t / TILES_X) * 8, x, y, i;
    u16 px[64], cols[16];
    u8 runs[64 * 3], idx[64];
    int nRuns = 0, len, nCols = 0, runLen, palLen = 0x7FFF;

    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
            px[y * 8 + x] = frame[(ty + y) * FRAME_W + tx + x];
    for (i = 0; i < 64; )
    {
        int run = 1;
        while (i + run < 64 && px[i + run] == px[i])
            run++;
        runs[nRuns * 3] = run;
        runs[nRuns * 3 + 1] = px[i] & 0xFF;
        runs[nRuns * 3 + 2] = px[i] >> 8;
        nRuns++;
        i += run;
    }
    for (i = 0; i < 64 && nCols <= 16; i++)
    {
        int k;
        for (k = 0; k < nCols && cols[k] != px[i]; k++)
            ;
        if (k == nCols)
        {
            if (nCols == 16)
            {
                nCols = 17;
                break;
            }
            cols[nCols++] = px[i];
        }
        idx[i] = k;
    }
    if (nCols <= 16)
        palLen = 4 + nCols * 2 + 64 * IndexBits(nCols) / 8;
    runLen = 4 + nRuns * 3;
    out[0] = t & 0xFF;
    out[1] = t >> 8;
    if (palLen < runLen && palLen < 3 + 128)
    {
        int bits = IndexBits(nCols), perByte = 8 / bits;
        u8 *p = out + 4 + nCols * 2;
        out[2] = 2;
        out[3] = nCols;
        for (i = 0; i < nCols; i++)
        {
            out[4 + i * 2] = cols[i] & 0xFF;
            out[4 + i * 2 + 1] = cols[i] >> 8;
        }
        memset(p, 0, 64 / perByte);
        for (i = 0; i < 64; i++)
            p[i / perByte] |= idx[i] << ((i % perByte) * bits);
        len = palLen;
    }
    else if (runLen < 3 + 128)
    {
        out[2] = 1;
        out[3] = nRuns;
        memcpy(out + 4, runs, nRuns * 3);
        len = runLen;
    }
    else
    {
        out[2] = 0;
        for (i = 0; i < 64; i++)
        {
            out[3 + i * 2] = px[i] & 0xFF;
            out[3 + i * 2 + 1] = px[i] >> 8;
        }
        len = 3 + 128;
    }
    for (y = 0; y < 8; y++)
        memcpy(sStream.ref + (ty + y) * FRAME_W + tx, frame + (ty + y) * FRAME_W + tx, 16);
    return len;
}

static void SendToViewers(const u8 *data, u16 len)
{
    u8 i, to = sStream.viewers | sStream.panel, others = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present)
            others |= 1 << i;
    if (to != 0 && (to & others) == others)
    {
        MP_SendRelay(0xFF, data, len);   // everyone watches: the server copies it, we send it once
        return;
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (to & (1 << i))
            MP_SendRelay(i, data, len);
}

static void SendSimple(u8 target, u8 type)
{
    MP_SendRelay(target, &type, 1);
}

// Moves a picture: new (x, y) = old (x + ox, y + oy); what came into view is black until its tiles arrive
static void ShiftFrame(u16 *buf, int ox, int oy)
{
    static u16 sTmp[FRAME_W * FRAME_H];
    int y;
    memcpy(sTmp, buf, sizeof(sTmp));
    for (y = 0; y < FRAME_H; y++)
    {
        int sy = y + oy, x0, x1;
        u16 *row = buf + y * FRAME_W;
        if (sy < 0 || sy >= FRAME_H)
        {
            memset(row, 0, FRAME_W * 2);
            continue;
        }
        x0 = ox < 0 ? -ox : 0;            // first x with a source
        x1 = ox > 0 ? FRAME_W - ox : FRAME_W;
        if (x0 >= FRAME_W || x1 <= 0)
        {
            memset(row, 0, FRAME_W * 2);
            continue;
        }
        memset(row, 0, FRAME_W * 2);
        memcpy(row + x0, sTmp + sy * FRAME_W + x0 + ox, (x1 - x0) * 2);
    }
}

// Pixels sampled to find the scroll: a fixed spread over the picture
#define SHIFT_SAMPLES 512
static u16 sSampleX[SHIFT_SAMPLES], sSampleY[SHIFT_SAMPLES];

static int ShiftMatch(const u16 *frame, int ox, int oy, int *inside)
{
    int i, match = 0;
    *inside = 0;
    for (i = 0; i < SHIFT_SAMPLES; i++)
    {
        int x = sSampleX[i], y = sSampleY[i], sx = x + ox, sy = y + oy;
        if (sx < 0 || sx >= FRAME_W || sy < 0 || sy >= FRAME_H)
            continue;
        (*inside)++;
        if (frame[y * FRAME_W + x] == sStream.ref[sy * FRAME_W + sx])
            match++;
    }
    return match;
}

// On the map the picture scrolls with the camera: how far since the viewers' picture (FALSE = not a scroll).
// The camera's move says roughly how far; the pixels say exactly and which way.
static bool8 FindShift(const u16 *frame, int *outX, int *outY)
{
    static bool8 sCamValid, sSamplesMade;
    static u16 sCamX, sCamY;
    int dx = 0, dy = 0, sign, ex, ey, baseIn, base, best = -1, bestIn = 1, bx = 0, by = 0;
    bool8 moved = FALSE;
    if (!sSamplesMade)
    {
        u32 r = 12345;
        int i;
        for (i = 0; i < SHIFT_SAMPLES; i++)
        {
            r = r * 1103515245 + 12345;
            sSampleX[i] = (r >> 8) % FRAME_W;
            r = r * 1103515245 + 12345;
            sSampleY[i] = (r >> 8) % FRAME_H;
        }
        sSamplesMade = TRUE;
    }
    if (MP_IsInOverworld() && !gPaletteFade.active)
    {
        u16 cx = gTotalCameraPixelOffsetX, cy = gTotalCameraPixelOffsetY;
        if (sCamValid)
        {
            dx = (s16)(cx - sCamX);
            dy = (s16)(cy - sCamY);
            moved = (dx != 0 || dy != 0) && dx >= -48 && dx <= 48 && dy >= -48 && dy <= 48;
        }
        sCamX = cx;
        sCamY = cy;
        sCamValid = TRUE;
    }
    else
        sCamValid = FALSE;
    if (!moved)
        return FALSE;
    base = ShiftMatch(frame, 0, 0, &baseIn);
    for (sign = -1; sign <= 1; sign += 2)
        for (ey = -3; ey <= 3; ey++)
            for (ex = -3; ex <= 3; ex++)
            {
                int ox = sign * dx + ex, oy = sign * dy + ey, in, m;
                if (ox == 0 && oy == 0)
                    continue;
                m = ShiftMatch(frame, ox, oy, &in);
                if (in >= SHIFT_SAMPLES / 2 && (best < 0 || m * bestIn > best * in))
                {
                    best = m;
                    bestIn = in;
                    bx = ox;
                    by = oy;
                }
            }
    // a clear winner: most samples match, clearly more than not moving
    if (best < 0 || best * 4 < bestIn * 3 || (baseIn > 0 && best * baseIn <= base * bestIn + bestIn * baseIn / 10))
        return FALSE;
    *outX = bx;
    *outY = by;
    return TRUE;
}

static void StreamFrame(void)
{
    const u16 *frame = Platform_GetFrame();
    bool8 key = sStream.needKey || sStream.keyTimer == 0;
    int t = 0, pos, count, ox = 0, oy = 0, bytes = 0;
    u8 flags = key ? FRAMEF_KEY : 0;
    static u8 sForced[NUM_TILES];

    if (!frame)
        return;
    sStream.needKey = FALSE;
    if (key)
        sStream.keyTimer = KEYFRAME_EVERY;
    memset(sForced, 0, sizeof(sForced));
    if (!key && FindShift(frame, &ox, &oy))
    {
        // Viewers move their picture the same way; the tiles that came into view go along
        int tx, ty;
        flags |= FRAMEF_SHIFT;
        ShiftFrame(sStream.ref, ox, oy);
        for (ty = 0; ty < TILES_Y; ty++)
            for (tx = 0; tx < TILES_X; tx++)
            {
                int x0 = tx * 8, y0 = ty * 8;
                if ((ox > 0 && x0 + 8 > FRAME_W - ox) || (ox < 0 && x0 < -ox)
                 || (oy > 0 && y0 + 8 > FRAME_H - oy) || (oy < 0 && y0 < -oy))
                    sForced[ty * TILES_X + tx] = TRUE;
            }
    }
    sStream.lastSong = CurrentSong();
    // One or more messages; the first carries the flags (and the shift)
    do
    {
        sOut[0] = MPR_SPEC_FRAME;
        sOut[1] = sStream.seq & 0xFF;
        sOut[2] = sStream.seq >> 8;
        sOut[3] = flags;
        sOut[4] = sStream.lastSong & 0xFF;
        sOut[5] = sStream.lastSong >> 8;
        sOut[7] = (flags & FRAMEF_SHIFT) ? (u8)(s8)ox : 0;
        sOut[8] = (flags & FRAMEF_SHIFT) ? (u8)(s8)oy : 0;
        pos = FRAME_HEADER;
        count = 0;
        for (; t < NUM_TILES && pos + 140 < MAX_PAYLOAD && count < 255; t++)
        {
            if (!key && !sForced[t] && !TileChanged(frame, t))
                continue;
            pos += PackTile(frame, t, sOut + pos);
            count++;
        }
        sOut[6] = count;
        if (t >= NUM_TILES)
            sOut[3] |= FRAMEF_LAST;
        if (count > 0 || sOut[3])
        {
            SendToViewers(sOut, pos);
            bytes += pos;
        }
        flags = 0;
    } while (t < NUM_TILES);
    if (Tracing())
        MPNet_Log("spectrace: send seq %d sum %08x bytes %d%s", sStream.seq, FrameSum(sStream.ref), bytes, (ox || oy) ? " shift" : "");
    sStream.seq++;
}

static bool8 LocalInBattle(void)
{
    return gMain.inBattle || MP_Battle_IsActive();
}

static void Stream_Update(void)
{
    if (!(sStream.viewers | sStream.panel))
        return;
    if (sStream.viewers && !LocalInBattle())
    {
        u8 i;
        // The result goes along, for anyone who bet on it
        u8 end[2] = {MPR_SPEC_END, gBattleOutcome & 0x7F};
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (sStream.viewers & (1 << i))
                MP_SendRelay(i, end, 2);
        sStream.viewers = 0;
        if (!sStream.panel)
            return;
    }
    if (sStream.keyTimer)
        sStream.keyTimer--;
    if (++sStream.timer >= (sStream.viewers ? SEND_EVERY : PANEL_SEND_EVERY)
     && (MPNet_ClientQueued() < SEND_BACKLOG || sStream.timer >= 60))
    {
        // (a picture only sends what changed since the last one, so skipping some loses nothing)
        sStream.timer = 0;
        StreamFrame();
    }
}

// ---------------------------------------------------------------------------
// Watching

// Tiny 3x5 font for the banner
static const u8 sFont[][5] = {
    // A-Z
    {7,5,7,5,5},{6,5,6,5,6},{7,4,4,4,7},{6,5,5,5,6},{7,4,6,4,7},{7,4,6,4,4},{7,4,5,5,7},{5,5,7,5,5},
    {7,2,2,2,7},{1,1,1,5,7},{5,5,6,5,5},{4,4,4,4,7},{5,7,7,5,5},{7,5,5,5,5},{7,5,5,5,7},{7,5,7,4,4},
    {7,5,5,7,1},{7,5,6,5,5},{7,4,7,1,7},{7,2,2,2,2},{5,5,5,5,7},{5,5,5,5,2},{5,5,7,7,5},{5,5,2,5,5},
    {5,5,7,2,2},{7,1,2,4,7},
    // 0-9
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},{7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},
    {7,5,7,5,7},{7,5,7,1,7},
    // : . - ' !
    {0,2,0,2,0},{0,0,0,0,2},{0,0,7,0,0},{2,2,0,0,0},{2,2,2,0,2},
};

static int GlyphIndex(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + c - '0';
    switch (c)
    {
    case ':': return 36;
    case '.': return 37;
    case '-': return 38;
    case '\'': return 39;
    case '!': return 40;
    }
    return -1;
}

static void DrawText(u16 *buf, int x, int y, const char *text, u16 color, int scale)
{
    for (; *text; text++, x += 4 * scale)
    {
        int g = GlyphIndex(*text), gx, gy, sx, sy;
        if (g < 0)
            continue;
        for (gy = 0; gy < 5; gy++)
            for (gx = 0; gx < 3; gx++)
                if (sFont[g][gy] & (4 >> gx))
                    for (sy = 0; sy < scale; sy++)
                        for (sx = 0; sx < scale; sx++)
                        {
                            int px = x + gx * scale + sx, py = y + gy * scale + sy;
                            if (px >= 0 && px < FRAME_W && py >= 0 && py < FRAME_H)
                                buf[py * FRAME_W + px] = color;
                        }
    }
}

static void Banner(u16 *buf, const char *text, bool8 bottom)
{
    int y0 = bottom ? FRAME_H - 9 : 0, x, y;
    for (y = y0; y < y0 + 9; y++)
        for (x = 0; x < FRAME_W; x++)
        {
            u16 c = buf[y * FRAME_W + x];
            // darken behind the text
            buf[y * FRAME_W + x] = 0x8000 | ((c & 0x1F) >> 2) | ((((c >> 5) & 0x1F) >> 2) << 5) | ((((c >> 10) & 0x1F) >> 2) << 10);
        }
    DrawText(buf, (FRAME_W - (int)strlen(text) * 4) / 2, y0 + 2, text, 0xFFFF, 1);
}

// ---------------------------------------------------------------------------
// Betting: put money on the player you watch (win: double back)

#define BET_STAKE 500

static s32 sBetStake;
static bool8 sBetOnWin;

static void ResolveBet(u8 outcome)
{
    char msg[96], name[16];
    if (sBetStake == 0)
        return;
    MP_GameToAscii(name, sWatch.name, MP_NAME_LEN);
    if (outcome != 0)
        MP_Events_Did(MP_DID_BET);
    if (outcome == 0)
    {
        AddMoney(&gSaveBlock1Ptr->money, sBetStake);
        snprintf(msg, sizeof(msg), "No result for %s's battle.\nYour $%d bet was returned.", name, (int)sBetStake);
    }
    else if ((outcome == B_OUTCOME_WON) == sBetOnWin)
    {
        MP_Events_Did(MP_DID_BETWIN);
        AddMoney(&gSaveBlock1Ptr->money, sBetStake * 2);
        snprintf(msg, sizeof(msg), "You won your bet on %s!\n+$%d", name, (int)sBetStake);
    }
    else
        snprintf(msg, sizeof(msg), "You lost your bet on %s.\n-$%d", name, (int)sBetStake);
    MP_Events_QueueToast(msg);
    MP_Social_Log("BET", msg);
    sBetStake = 0;
}

static void StopWatching(void)
{
    if (sBetStake && !sWatch.ending)
    {
        MP_Events_QueueToast("You walked away early:\nyour bet is lost.");
        sBetStake = 0;
    }
    if (sWatch.target != 0xFF && gMpPlayers[sWatch.target].present)
        SendSimple(sWatch.target, MPR_SPEC_LEAVE);
    if (sWatch.target != 0xFF)
        MPNet_Log("spectate: stopped watching, %d updates", sWatch.updates);
    sWatch.target = 0xFF;
    // Back to the overworld exactly where we were
    if (MP_IsInOverworld())
    {
        // The battle's music played over ours: start the map's music again
        // (it only restarts when it thinks a different song is playing)
        Overworld_ResetMapMusic();
        Overworld_PlaySpecialMapMusic();
        UnlockPlayerFieldControls();
    }
}

static void StartWatching(u8 id)
{
    memset(&sWatch, 0, sizeof(sWatch));
    sWatch.target = id;
    sWatch.updates = 0;
    StringCopyN(sWatch.name, gMpPlayers[id].name, MP_NAME_LEN);
    sWatch.name[MP_NAME_LEN] = EOS;
    sWatch.prevKeys = 0xFFFF; // ignore the A press that started this
    LockPlayerFieldControls();
    SendSimple(id, MPR_SPEC_JOIN);
    MPNet_Log("watching player %d's battle", id + 1);
}

// Called by the platform layer every frame while watching (the game is paused)
const u16 *MP_Spectate_WatchFrame(u16 keys)
{
    char name[16], text[64];
    u16 pressed = keys & ~sWatch.prevKeys;
    sWatch.prevKeys = keys;

    if (sWatch.target == 0xFF)
        return NULL;
    if (!MP_IsConnected() || !gMpPlayers[sWatch.target].present)
    {
        ResolveBet(0);   // they left: the bet is returned
        StopWatching();
        MP_ShowNotice("The battle you were watching\nisn't available anymore.");
        return NULL;
    }
    if (pressed & (B_BUTTON | START_BUTTON))
    {
        PlaySE(SE_SELECT);
        StopWatching();
        return NULL;
    }
    if (sWatch.ending && ++sWatch.endTimer > 90)
    {
        char msg[80];
        MP_GameToAscii(name, sWatch.name, MP_NAME_LEN);
        StopWatching();
        snprintf(msg, sizeof(msg), "%s's battle is over.", name);
        MP_ShowNotice(msg);
        return NULL;
    }

    MP_GameToAscii(name, sWatch.name, MP_NAME_LEN);
    if (!sWatch.gotFrame)
    {
        int i;
        for (i = 0; i < FRAME_W * FRAME_H; i++)
            sWatch.shown[i] = 0x8000;
        snprintf(text, sizeof(text), "CONNECTING TO %s'S BATTLE...", name);
        DrawText(sWatch.shown, (FRAME_W - (int)strlen(text) * 8) / 2, 70, text, 0xFFFF, 2);
    }
    else
        memcpy(sWatch.shown, sWatch.frame, sizeof(sWatch.shown));
    if (sWatch.ending)
        snprintf(text, sizeof(text), "%s'S BATTLE IS OVER", name);
    else
        snprintf(text, sizeof(text), "WATCHING %s  -  {B} STOP WATCHING", name);
    Banner(sWatch.shown, text, FALSE);
    return sWatch.shown;
}

// Applies one frame message to a picture. *gotKey: it has had a full frame (updates before that are skipped).
// FALSE when the message is broken.
static bool8 ApplyUpdate(u16 *buf, bool8 *gotKey, const u8 *data, u16 len)
{
    int pos = FRAME_HEADER, count, i;
    if (len < FRAME_HEADER)
        return FALSE;
    if (data[3] & FRAMEF_KEY)
        *gotKey = TRUE;
    if (!*gotKey)
        return TRUE; // wait for a full frame first
    if (data[3] & FRAMEF_SHIFT)
        ShiftFrame(buf, (s8)data[7], (s8)data[8]);
    count = data[6];
    for (i = 0; i < count && pos + 3 <= len; i++)
    {
        int t = data[pos] | (data[pos + 1] << 8), mode = data[pos + 2], k;
        int tx, ty;
        u16 px[64];
        if (t >= NUM_TILES)
            return FALSE;
        if (mode == 1)
        {
            int nRuns, n = 0;
            if (pos + 4 > len)
                return FALSE;
            nRuns = data[pos + 3];
            pos += 4;
            if (pos + nRuns * 3 > len)
                return FALSE;
            for (k = 0; k < nRuns; k++)
            {
                int run = data[pos + k * 3];
                u16 c = data[pos + k * 3 + 1] | (data[pos + k * 3 + 2] << 8);
                while (run-- > 0 && n < 64)
                    px[n++] = c;
            }
            while (n < 64)
                px[n++] = 0;
            pos += nRuns * 3;
        }
        else if (mode == 2)
        {
            int nCols, bits, perByte;
            u16 cols[16];
            if (pos + 4 > len)
                return FALSE;
            nCols = data[pos + 3];
            if (nCols < 1 || nCols > 16)
                return FALSE;
            bits = IndexBits(nCols);
            perByte = 8 / bits;
            pos += 4;
            if (pos + nCols * 2 + 64 / perByte > len)
                return FALSE;
            for (k = 0; k < nCols; k++)
                cols[k] = data[pos + k * 2] | (data[pos + k * 2 + 1] << 8);
            pos += nCols * 2;
            for (k = 0; k < 64; k++)
            {
                int c = (data[pos + k / perByte] >> ((k % perByte) * bits)) & ((1 << bits) - 1);
                px[k] = c < nCols ? cols[c] : 0;
            }
            pos += 64 / perByte;
        }
        else
        {
            pos += 3;
            if (pos + 128 > len)
                return FALSE;
            for (k = 0; k < 64; k++)
                px[k] = data[pos + k * 2] | (data[pos + k * 2 + 1] << 8);
            pos += 128;
        }
        tx = (t % TILES_X) * 8;
        ty = (t / TILES_X) * 8;
        for (k = 0; k < 8; k++)
            memcpy(buf + (ty + k) * FRAME_W + tx, px + k * 8, 16);
    }
    return TRUE;
}

static void ApplyFrame(const u8 *data, u16 len)
{
    u16 song;
    sWatch.updates++;
    if (len < FRAME_HEADER)
        return;
    if (!ApplyUpdate(sWatch.frame, &sWatch.gotFrame, data, len) || !sWatch.gotFrame)
        return;
    song = data[4] | (data[5] << 8);
    if (song && song != sWatch.song && song < LAST_SONG)
    {
        sWatch.song = song;
        m4aSongNumStart(song);
    }
}

// ---------------------------------------------------------------------------
// The SPECTATE panel: friends' screens under ours (src/platform/display.c draws it)

#define PANEL_ASK_AGAIN (60 * 9)   // nothing heard this long (they send a full frame every 4 s): ask again

static const u32 sPanelAccents[MP_MAX_PLAYERS] = {0xFF5A6E, 0x3E9BFF, 0x2BD487, 0xFFB52E};

static struct
{
    u8 subscribed;                          // players we asked for their screen
    u8 shown[DISPLAY_PANEL_MAX];            // player ids on the cards, in order
    u8 count;
    bool8 gotKey[MP_MAX_PLAYERS];
    bool8 changed[MP_MAX_PLAYERS];
    bool8 labelChanged[MP_MAX_PLAYERS];
    u16 heard[MP_MAX_PLAYERS];              // frames since their last update
    u16 updates[MP_MAX_PLAYERS];
    u32 labelKey[MP_MAX_PLAYERS];           // what the label shows (redrawn when it changes)
    u16 frame[MP_MAX_PLAYERS][FRAME_W * FRAME_H];
    u32 label[MP_MAX_PLAYERS][DISPLAY_PANEL_LABEL_W * DISPLAY_PANEL_LABEL_H];
} sPanel;
static u16 sPanelWaiting[FRAME_W * FRAME_H];   // "CONNECTING..." until a full frame arrives
static bool8 sPanelWaitingMade;

static void Panel_Forget(u8 id)
{
    sPanel.subscribed &= ~(1 << id);
    sPanel.gotKey[id] = FALSE;
    sPanel.heard[id] = 0;
    sPanel.labelKey[id] = 0;
}

static void Panel_OnFrame(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || !(sPanel.subscribed & (1 << from)))
        return;
    sPanel.heard[from] = 0;
    if (ApplyUpdate(sPanel.frame[from], &sPanel.gotKey[from], data, len))
    {
        // the update's last message: the picture is complete (they're split at 255 tiles / MAX_PAYLOAD)
        if (Tracing() && sPanel.gotKey[from] && (data[3] & FRAMEF_LAST))
            MPNet_Log("spectrace: player %d seq %d sum %08x", from + 1, data[1] | (data[2] << 8), FrameSum(sPanel.frame[from]));
        if (sPanel.gotKey[from])
        {
            sPanel.changed[from] = TRUE;
            if (sPanel.updates[from]++ == 0 || (data[3] & FRAMEF_KEY))
                MPNet_Log("spectate panel: player %d frame%s, %d tiles", from + 1, (data[3] & FRAMEF_KEY) ? " (full)" : "", data[6]);
        }
    }
    else
    {
        // A broken update: ask for a full frame
        sPanel.gotKey[from] = FALSE;
        SendSimple(from, MPR_SPEC_PANEL_JOIN);
    }
}

// A friend's SPECTATE settings, by their name (CREATE: a slot for them, the oldest dropped when all 16 are used).
// Not created: the defaults (below, the NEW FRIENDS size, shown) when they have none.
static struct MpSpecFriend *SpecFriend(u8 id, bool8 create)
{
    static struct MpSpecFriend sDefault;
    char name[MP_NAME_LEN + 1];
    int i;
    MP_GameToAscii(name, gMpPlayers[id].name, MP_NAME_LEN);
    name[MP_NAME_LEN] = 0;
    for (i = 0; i < MP_SPEC_FRIENDS; i++)
        if (gMpConfig.specFriends[i].name[0] && strcmp(gMpConfig.specFriends[i].name, name) == 0)
            return &gMpConfig.specFriends[i];
    memset(&sDefault, 0, sizeof(sDefault));
    snprintf(sDefault.name, sizeof(sDefault.name), "%s", name);
    sDefault.side = MP_SPEC_BELOW;
    sDefault.size = gMpConfig.spectateScale < 1 ? 1 : gMpConfig.spectateScale;
    sDefault.show = TRUE;
    if (!create)
        return &sDefault;
    for (i = 0; i < MP_SPEC_FRIENDS && gMpConfig.specFriends[i].name[0]; i++)
        ;
    if (i == MP_SPEC_FRIENDS)
    {
        memmove(&gMpConfig.specFriends[0], &gMpConfig.specFriends[1], sizeof(gMpConfig.specFriends[0]) * (MP_SPEC_FRIENDS - 1));
        i = MP_SPEC_FRIENDS - 1;
    }
    gMpConfig.specFriends[i] = sDefault;
    return &gMpConfig.specFriends[i];
}

// Asks the friends we show for their screen, and stops asking the rest
static void Panel_Update(void)
{
    u8 want = 0, n = 0, i;
    if (gMpConfig.spectatePanel && MP_IsConnected() && gMpLocalId >= 0)
        for (i = 0; i < MP_MAX_PLAYERS && n < DISPLAY_PANEL_MAX; i++)
            if (i != gMpLocalId && gMpPlayers[i].present && SpecFriend(i, FALSE)->show)
            {
                want |= 1 << i;
                sPanel.shown[n++] = i;
            }
    sPanel.count = n;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u8 bit = 1 << i;
        if ((want & bit) && !(sPanel.subscribed & bit))
        {
            Panel_Forget(i);
            sPanel.subscribed |= bit;
            sPanel.labelChanged[i] = TRUE;
            sPanel.updates[i] = 0;
            SendSimple(i, MPR_SPEC_PANEL_JOIN);
            MPNet_Log("spectate panel: asking player %d for their screen", i + 1);
        }
        else if (!(want & bit) && (sPanel.subscribed & bit))
        {
            if (gMpPlayers[i].present && MP_IsConnected())
                SendSimple(i, MPR_SPEC_PANEL_LEAVE);
            Panel_Forget(i);
        }
        else if ((sPanel.subscribed & bit) && ++sPanel.heard[i] >= PANEL_ASK_AGAIN)
        {
            sPanel.heard[i] = 0;
            sPanel.gotKey[i] = FALSE;
            SendSimple(i, MPR_SPEC_PANEL_JOIN);
            MPNet_Log("spectate panel: nothing from player %d, asking again", i + 1);
        }
    }
}

// The label under a card: a dot in the player's color, their name, and where they are (or IN BATTLE)
static void LabelPixel(u32 *buf, int x, int y, u32 argb)
{
    if (x >= 0 && x < DISPLAY_PANEL_LABEL_W && y >= 0 && y < DISPLAY_PANEL_LABEL_H)
        buf[y * DISPLAY_PANEL_LABEL_W + x] = argb;
}

static int LabelText(u32 *buf, int x, int y, const u8 *str, u32 fg, u32 shadow, bool8 measureOnly)
{
    int start = x;
    for (; *str != EOS; str++)
    {
        int w = MP_RenderSmallGlyph(*str, 1, 0, 2), gx, gy;
        if (!measureOnly)
            for (gy = 0; gy < 16; gy++)
            {
                const u32 *rows = (gy < 8) ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
                for (gx = 0; gx < w && gx < 16; gx++)
                {
                    u32 row = rows[(gx < 8 ? 0 : 8) + (gy % 8)];
                    u8 c = (row >> ((gx % 8) * 4)) & 0xF;
                    if (c == 1)
                        LabelPixel(buf, x + gx, y + gy, fg);
                    else if (c == 2)
                        LabelPixel(buf, x + gx, y + gy, shadow);
                }
            }
        x += w;
    }
    return x - start;
}

static void Panel_DrawLabel(u8 id)
{
    u32 *buf = sPanel.label[id];
    u8 name[PLAYER_NAME_LENGTH + 1], where[40];
    u32 accent = 0xFF000000 | sPanelAccents[id], key = 5381;
    bool8 battle = gMpPlayers[id].hasState && (gMpPlayers[id].state.flags & MPF_IN_BATTLE);
    int i, x, w, y;

    StringCopyN(name, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    name[PLAYER_NAME_LENGTH] = EOS;
    MP_Friends_LocationName(id, where);
    for (i = 0; name[i] != EOS; i++)
        key = key * 33 + name[i];
    for (i = 0; where[i] != EOS; i++)
        key = key * 33 + where[i];
    key = key * 33 + battle + 1;
    if (key == sPanel.labelKey[id])
        return;
    sPanel.labelKey[id] = key;
    sPanel.labelChanged[id] = TRUE;
    memset(buf, 0, sizeof(sPanel.label[id]));
    // a round dot in the player's color
    for (y = 0; y < 5; y++)
        for (x = 0; x < 5; x++)
            if (!((x == 0 || x == 4) && (y == 0 || y == 4)))
                LabelPixel(buf, 5 + x, 4 + y, accent);
    LabelText(buf, 13, -2, name, 0xFFFFFFFF, 0xFF3A3F4E, FALSE);
    // where they are, on the right: IN BATTLE in a red pill
    w = LabelText(buf, 0, 0, where, 0, 0, TRUE);
    x = DISPLAY_PANEL_LABEL_W - 6 - w;
    if (battle)
    {
        for (y = 2; y < DISPLAY_PANEL_LABEL_H - 1; y++)
            for (i = x - 4; i < x + w + 3; i++)
                if (!((i == x - 4 || i == x + w + 2) && (y == 2 || y == DISPLAY_PANEL_LABEL_H - 2)))
                    LabelPixel(buf, i, y, 0xFFD8394C);
        LabelText(buf, x, -2, where, 0xFFFFFFFF, 0xFF7A1C2A, FALSE);
    }
    else
        LabelText(buf, x, -2, where, 0xFFB4BCCC, 0xFF2A2F3B, FALSE);
}

static void Panel_MakeWaiting(void)
{
    int i;
    const char *text = "CONNECTING...";
    if (sPanelWaitingMade)
        return;
    for (i = 0; i < FRAME_W * FRAME_H; i++)
        sPanelWaiting[i] = 0x8000 | (3 << 10) | (2 << 5) | 2;
    DrawText(sPanelWaiting, (FRAME_W - (int)strlen(text) * 8) / 2, 75, text, 0xFFFF, 2);
    sPanelWaitingMade = TRUE;
}

// sdl2.c, every drawn frame: the panel's cards to the display
void MP_Spectate_PanelSync(void)
{
    struct DisplayPanelCard cards[DISPLAY_PANEL_MAX];
    static char sTitles[DISPLAY_PANEL_MAX][40];
    int closed;
    u8 i, n;
    // A pop-out window the player closed: that friend's screen is off (back on in CO-OP > SPECTATE)
    while ((closed = Display_TakeClosedPopout()) >= 0)
    {
        if (closed < MP_MAX_PLAYERS && gMpPlayers[closed].present)
        {
            SpecFriend(closed, TRUE)->show = FALSE;
            MP_SaveConfig();
        }
        MPNet_Log("spectate panel: player %d's window closed, not shown", closed + 1);
    }
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        sPanel.count = 0;   // left the server: nobody to show (Panel_Update only runs in a room)
        sPanel.subscribed = 0;
    }
    if (sPanel.count == 0)
    {
        Display_SetPanel(0, NULL);
        return;
    }
    Panel_MakeWaiting();
    for (i = 0, n = 0; i < sPanel.count; i++)
    {
        u8 id = sPanel.shown[i];
        struct DisplayPanelCard *card = &cards[n];
        if (!SpecFriend(id, FALSE)->show)
            continue;   // just hidden (its window closed): Panel_Update drops it next
        Panel_DrawLabel(id);
        card->frame = sPanel.gotKey[id] ? (const void *)sPanel.frame[id] : (const void *)sPanelWaiting;
        card->frameChanged = sPanel.changed[id] || !sPanel.gotKey[id];
        card->label = sPanel.label[id];
        card->labelChanged = sPanel.labelChanged[id];
        card->accent = sPanelAccents[id];
        {
            const struct MpSpecFriend *fr = SpecFriend(id, FALSE);
            char name[MP_NAME_LEN + 1];
            card->side = fr->side;
            card->scale = fr->size;
            card->popout = fr->popout;
            card->id = id;
            MP_GameToAscii(name, gMpPlayers[id].name, MP_NAME_LEN);
            name[MP_NAME_LEN] = 0;
            snprintf(sTitles[n], sizeof(sTitles[n]), "%s - Moemon Co-op", name);
            card->title = sTitles[n];
        }
        sPanel.changed[id] = FALSE;
        sPanel.labelChanged[id] = FALSE;
        n++;
    }
    Display_SetPanel(n, cards);
}

void MP_Spectate_OnRelay(u8 from, const u8 *data, u16 len)
{
    switch (data[0])
    {
    case MPR_SPEC_JOIN:
        if (LocalInBattle())
        {
            sStream.viewers |= 1 << from;
            sStream.needKey = TRUE;
        }
        else
            SendSimple(from, MPR_SPEC_END);
        break;
    case MPR_SPEC_LEAVE:
        sStream.viewers &= ~(1 << from);
        break;
    case MPR_SPEC_PANEL_JOIN:
        if (!(sStream.panel & (1 << from)))
            MPNet_Log("spectate panel: player %d shows our screen", from + 1);
        sStream.panel |= 1 << from;
        sStream.needKey = TRUE;
        break;
    case MPR_SPEC_PANEL_LEAVE:
        if (sStream.panel & (1 << from))
            MPNet_Log("spectate panel: player %d stopped showing our screen", from + 1);
        sStream.panel &= ~(1 << from);
        break;
    case MPR_SPEC_FRAME:
        if (from == sWatch.target && !sWatch.ending)
            ApplyFrame(data, len);
        Panel_OnFrame(from, data, len);
        break;
    case MPR_SPEC_END:
        if (from == sWatch.target)
        {
            sWatch.ending = TRUE;
            ResolveBet(len >= 2 ? data[1] : 0);
        }
        break;
    }
}

void MP_Spectate_OnPlayerLeft(u8 id)
{
    sStream.viewers &= ~(1 << id);
    sStream.panel &= ~(1 << id);
    Panel_Forget(id);
    // the watch loop notices the missing player and stops
}

void MP_Spectate_FrameUpdate(void)
{
    Stream_Update();
    Panel_Update();
}

// ---------------------------------------------------------------------------
// "Watch BOB's battle?" when pressing A in front of a player in a battle

#define tState data[0]
#define tTarget data[1]

static EWRAM_DATA u8 sPrompt[96];

static void OpenBetMenu(u8 target);

// The choice window beside the message: SPECTATE / BET / JOIN / CANCEL (the yes / no box's tiles: it isn't up)
static const struct WindowTemplate sWatchChoiceWindow =
{
    .bg = 0,
    .tilemapLeft = 20,
    .tilemapTop = 6,
    .width = 9,
    .height = 8,
    .paletteNum = 15,
    .baseBlock = 0x125,
};
static const u8 sText_ChoiceSpectate[] = _("SPECTATE");
static const u8 sText_ChoiceBet[] = _("BET");
static const u8 sText_ChoiceJoin[] = _("JOIN");
static const u8 sText_ChoiceCancel[] = _("CANCEL");
static const struct MenuAction sWatchChoices[] =
{
    {sText_ChoiceSpectate, {NULL}},
    {sText_ChoiceBet, {NULL}},
    {sText_ChoiceJoin, {NULL}},
    {sText_ChoiceCancel, {NULL}},
};
enum { WATCH_SPECTATE, WATCH_BET, WATCH_JOIN, WATCH_CANCEL };
static EWRAM_DATA u8 sWatchWindowId = 0;

static void Task_WatchPrompt(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(sPrompt);
        tState++;
        break;
    case 1:
        if (!IsTextPrinterActive(0))
        {
            sWatchWindowId = AddWindow(&sWatchChoiceWindow);
            DrawStdWindowFrame(sWatchWindowId, FALSE);
            PrintMenuTable(sWatchWindowId, ARRAY_COUNT(sWatchChoices), sWatchChoices);
            InitMenuInUpperLeftCornerNormal(sWatchWindowId, ARRAY_COUNT(sWatchChoices), 0);
            CopyWindowToVram(sWatchWindowId, COPYWIN_FULL);
            tState++;
        }
        break;
    case 2:
    {
        s8 answer = Menu_ProcessInputNoWrap();
        u8 t = tTarget;
        bool8 stillOn;
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        ClearStdWindowAndFrameToTransparent(sWatchWindowId, TRUE);
        RemoveWindow(sWatchWindowId);
        HideFieldMessageBox();
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        stillOn = gMpPlayers[t].present && (gMpPlayers[t].state.flags & MPF_IN_BATTLE);
        if (answer == MENU_B_PRESSED || answer == WATCH_CANCEL)
            break;
        if (!stillOn)
        {
            MP_ShowNotice("That battle is already over.");
            break;
        }
        if (answer == WATCH_SPECTATE)
        {
            sBetStake = 0;
            StartWatching(t);
        }
        else if (answer == WATCH_BET)
            OpenBetMenu(t);
        else
            MP_Join_Ask(t);
        break;
    }
    }
}

bool8 MP_Spectate_ChoiceUp(void)
{
    u8 t = FindTaskIdByFunc(Task_WatchPrompt);
    return t != TASK_NONE && gTasks[t].tState == 2;
}

static EWRAM_DATA struct { u8 target; u16 cursor; } sBetUi = {0};

static void DrawBetMenu(void)
{
    char name[16], line[48];
    MP_GameToAscii(name, gMpPlayers[sBetUi.target].name, MP_NAME_LEN);
    MpUi_Begin("PLACE A BET?");
    snprintf(line, sizeof(line), "$%d says %s WINS", BET_STAKE, name);
    MpUi_RowAscii(0, sBetUi.cursor == 0, line, NULL);
    snprintf(line, sizeof(line), "$%d says %s LOSES", BET_STAKE, name);
    MpUi_RowAscii(1, sBetUi.cursor == 1, line, NULL);
    MpUi_RowAscii(2, sBetUi.cursor == 2, "JUST WATCH", NULL);
    MpUi_End();
    MpUi_HelpAscii("Right: double back. Leave early and the bet is lost.");
}

static void Task_BetMenu(u8 taskId)
{
    if (MpUi_MoveCursor(&sBetUi.cursor, 3))
    {
        DrawBetMenu();
        return;
    }
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        u8 choice = JOY_NEW(B_BUTTON) ? 2 : sBetUi.cursor;
        PlaySE(SE_SELECT);
        MpUi_Close();
        DestroyTask(taskId);
        if (!gMpPlayers[sBetUi.target].present || !(gMpPlayers[sBetUi.target].state.flags & MPF_IN_BATTLE))
        {
            MP_ShowNotice("That battle is already over.");
            return;
        }
        sBetStake = 0;
        if (choice < 2)
        {
            if (!IsEnoughMoney(&gSaveBlock1Ptr->money, BET_STAKE))
            {
                MP_ShowNotice("You don't have enough money to bet.");
                return;
            }
            RemoveMoney(&gSaveBlock1Ptr->money, BET_STAKE);
            sBetStake = BET_STAKE;
            sBetOnWin = (choice == 0);
            PlaySE(SE_SHOP);
        }
        StartWatching(sBetUi.target);
    }
}

static void OpenBetMenu(u8 target)
{
    sBetUi.target = target;
    sBetUi.cursor = 2;
    MpUi_Open();
    CreateTask(Task_BetMenu, 80);
    DrawBetMenu();
}

// Called from the overworld update; TRUE when it started the prompt
bool8 MP_Spectate_TryInteract(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;

    if (!JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || !MP_IsConnected() || FuncIsActiveTask(Task_WatchPrompt) || MP_Join_IsAsking())
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 rx, ry;
        char name[16];
        u8 taskId;
        if (i == gMpLocalId || !gMpPlayers[i].present || !(gMpPlayers[i].state.flags & MPF_IN_BATTLE))
            continue;
        if (!MP_GetRemoteMapPos(i, &rx, &ry) || rx != x || ry != y)
            continue;
        MP_GameToAscii(name, gMpPlayers[i].name, MP_NAME_LEN);
        {
            char text[80];
            snprintf(text, sizeof(text), "%s is in a battle! Would you like\nto spectate, bet, or join?", name);
            MP_AsciiToGame(sPrompt, text, sizeof(sPrompt) - 1);
        }
        LockPlayerFieldControls();
        taskId = CreateTask(Task_WatchPrompt, 80);
        gTasks[taskId].tTarget = i;
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// The friends' screens' settings are in OPTIONS > CO-OP (option_menu.c): FRIENDS' SCREENS on / off, one row per friend
// online (A: their page: SHOW, PLACE = below / above / left / right / own window, SIZE), NEW FRIENDS

static u8 SetFriends(u8 *ids)
{
    u8 i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS && n < DISPLAY_PANEL_MAX; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && MP_IsConnected())
            ids[n++] = i;
    return n;
}

// The size friend id's card shows at now (0: not shown)
static u8 ShownFit(u8 id)
{
    u8 i, n = 0;
    for (i = 0; i < sPanel.count; i++)
    {
        if (!SpecFriend(sPanel.shown[i], FALSE)->show)
            continue;   // not given to the display (MP_Spectate_PanelSync's order)
        if (sPanel.shown[i] == id)
            return Display_GetCardScale(n);
        n++;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// OPTIONS > CO-OP (option_menu.c): the friends online and their screens' settings

u8 MP_Spectate_ListFriends(u8 *ids)
{
    return SetFriends(ids);
}

struct MpSpecFriend *MP_Spectate_Friend(u8 id, bool8 create)
{
    return SpecFriend(id, create);
}

// The size friend id's screen really shows at (0: not shown now)
u8 MP_Spectate_ShownSize(u8 id)
{
    return ShownFit(id);
}
