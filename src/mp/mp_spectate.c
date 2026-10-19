// Moemon Co-op: watching other players' battles.
//
// A player in a battle streams their screen to whoever is watching: changed
// 8x8 tiles, run-length packed, about 20 times a second, plus the battle
// music's song number. The watcher's own game pauses while the feed is shown
// (src/platform/sdl2.c asks MP_Spectate_IsWatching every frame) and resumes
// exactly where it was when they press B or the battle ends.
//
// Walk up to a player marked IN BATTLE and press A to watch.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
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
#include "gba/m4a_internal.h"
#include "m4a.h"
#include "constants/songs.h"

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
};

#define FRAMEF_KEY  1   // this update starts a full frame

const u16 *Platform_GetFrame(void);
void MPNet_Log(const char *fmt, ...);

// ---- Streaming (the player in the battle) ----
static struct
{
    u8 viewers;                    // bitmask of player ids
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
    u8 name[MP_NAME_LEN + 1];
    u16 frame[FRAME_W * FRAME_H];
    u16 shown[FRAME_W * FRAME_H];  // frame plus the banner
} sWatch = {0xFF};

static u8 sOut[MAX_PAYLOAD + 16];

bool8 MP_Spectate_IsWatching(void)
{
    return sWatch.target != 0xFF;
}

bool8 MP_Spectate_IsStreaming(void)
{
    return sStream.viewers != 0;
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

// Appends one tile; returns bytes written. Mode 0 = raw, 1 = runs of a color
static int PackTile(const u16 *frame, int t, u8 *out)
{
    int tx = (t % TILES_X) * 8, ty = (t / TILES_X) * 8, x, y, i;
    u16 px[64];
    u8 runs[64 * 3];
    int nRuns = 0, len;

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
    out[0] = t & 0xFF;
    out[1] = t >> 8;
    if (nRuns * 3 + 1 < 128)
    {
        out[2] = 1;
        out[3] = nRuns;
        memcpy(out + 4, runs, nRuns * 3);
        len = 4 + nRuns * 3;
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
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (sStream.viewers & (1 << i))
            MP_SendRelay(i, data, len);
}

static void SendSimple(u8 target, u8 type)
{
    MP_SendRelay(target, &type, 1);
}

static void StreamFrame(void)
{
    const u16 *frame = Platform_GetFrame();
    bool8 key = sStream.needKey || sStream.keyTimer == 0;
    int t = 0, pos, count;
    u8 flags = key ? FRAMEF_KEY : 0;

    if (!frame)
        return;
    sStream.needKey = FALSE;
    if (key)
        sStream.keyTimer = KEYFRAME_EVERY;
    sStream.lastSong = CurrentSong();
    // One or more messages; the first carries the key flag
    do
    {
        sOut[0] = MPR_SPEC_FRAME;
        sOut[1] = sStream.seq & 0xFF;
        sOut[2] = sStream.seq >> 8;
        sOut[3] = flags;
        sOut[4] = sStream.lastSong & 0xFF;
        sOut[5] = sStream.lastSong >> 8;
        pos = 7;
        count = 0;
        for (; t < NUM_TILES && pos + 140 < MAX_PAYLOAD && count < 255; t++)
        {
            if (!key && !TileChanged(frame, t))
                continue;
            pos += PackTile(frame, t, sOut + pos);
            count++;
        }
        sOut[6] = count;
        if (count > 0 || flags)
            SendToViewers(sOut, pos);
        flags = 0;
    } while (t < NUM_TILES);
    sStream.seq++;
}

static bool8 LocalInBattle(void)
{
    return gMain.inBattle || MP_Battle_IsActive();
}

static void Stream_Update(void)
{
    if (!sStream.viewers)
        return;
    if (!LocalInBattle())
    {
        u8 i;
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (sStream.viewers & (1 << i))
                SendSimple(i, MPR_SPEC_END);
        sStream.viewers = 0;
        return;
    }
    if (sStream.keyTimer)
        sStream.keyTimer--;
    if (++sStream.timer >= SEND_EVERY)
    {
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

static void StopWatching(void)
{
    if (sWatch.target != 0xFF && gMpPlayers[sWatch.target].present)
        SendSimple(sWatch.target, MPR_SPEC_LEAVE);
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
        snprintf(text, sizeof(text), "WATCHING %s  -  B: STOP WATCHING", name);
    Banner(sWatch.shown, text, FALSE);
    return sWatch.shown;
}

static void ApplyFrame(const u8 *data, u16 len)
{
    u16 song;
    int pos = 7, count, i;
    if (len < 7)
        return;
    if (data[3] & FRAMEF_KEY)
        sWatch.gotFrame = TRUE;
    if (!sWatch.gotFrame)
        return; // wait for a full frame first
    song = data[4] | (data[5] << 8);
    if (song && song != sWatch.song && song < LAST_SONG)
    {
        sWatch.song = song;
        m4aSongNumStart(song);
    }
    count = data[6];
    for (i = 0; i < count && pos + 3 <= len; i++)
    {
        int t = data[pos] | (data[pos + 1] << 8), mode = data[pos + 2], k;
        int tx, ty;
        u16 px[64];
        if (t >= NUM_TILES)
            return;
        if (mode == 1)
        {
            int nRuns, n = 0;
            if (pos + 4 > len)
                return;
            nRuns = data[pos + 3];
            pos += 4;
            if (pos + nRuns * 3 > len)
                return;
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
        else
        {
            pos += 3;
            if (pos + 128 > len)
                return;
            for (k = 0; k < 64; k++)
                px[k] = data[pos + k * 2] | (data[pos + k * 2 + 1] << 8);
            pos += 128;
        }
        tx = (t % TILES_X) * 8;
        ty = (t / TILES_X) * 8;
        for (k = 0; k < 8; k++)
            memcpy(sWatch.frame + (ty + k) * FRAME_W + tx, px + k * 8, 16);
    }
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
    case MPR_SPEC_FRAME:
        if (from == sWatch.target && !sWatch.ending)
            ApplyFrame(data, len);
        break;
    case MPR_SPEC_END:
        if (from == sWatch.target)
            sWatch.ending = TRUE;
        break;
    }
}

void MP_Spectate_OnPlayerLeft(u8 id)
{
    sStream.viewers &= ~(1 << id);
    // the watch loop notices the missing player and stops
}

void MP_Spectate_FrameUpdate(void)
{
    Stream_Update();
}

// ---------------------------------------------------------------------------
// "Watch BOB's battle?" when pressing A in front of a player in a battle

#define tState data[0]
#define tTarget data[1]

static EWRAM_DATA u8 sPrompt[96];

static void Task_WatchPrompt(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        ShowFieldMessage(sPrompt);
        tState++;
        break;
    case 1:
        if (!IsTextPrinterActive(0))
        {
            DisplayYesNoMenuDefaultYes();
            tState++;
        }
        break;
    case 2:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        HideFieldMessageBox();
        if (answer == 0 && gMpPlayers[tTarget].present && (gMpPlayers[tTarget].state.flags & MPF_IN_BATTLE))
        {
            UnlockPlayerFieldControls();
            StartWatching(tTarget);
        }
        else
            UnlockPlayerFieldControls();
        DestroyTask(taskId);
        break;
    }
    }
}

// Called from the overworld update; TRUE when it started the prompt
bool8 MP_Spectate_TryInteract(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;

    if (!JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || !MP_IsConnected() || FuncIsActiveTask(Task_WatchPrompt))
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
            snprintf(text, sizeof(text), "%s is in a battle.\nWatch it?", name);
            MP_AsciiToGame(sPrompt, text, sizeof(sPrompt) - 1);
        }
        LockPlayerFieldControls();
        taskId = CreateTask(Task_WatchPrompt, 80);
        gTasks[taskId].tTarget = i;
        return TRUE;
    }
    return FALSE;
}
