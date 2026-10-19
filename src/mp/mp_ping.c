// Moemon Co-op: map pings. Left stick click (or V on the keyboard) drops a
// "look here!" marker on the tile in front of you: an item, a person, a ledge...
// Friends on the same map see a bubble with your name over that tile for a few
// seconds, and a blinking arrow in your color at the edge of the screen while
// it's out of view.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "sound.h"
#include "string_util.h"
#include "constants/songs.h"
#include "constants/characters.h"

#define PING_FRAMES   (6 * 60)
#define PING_COOLDOWN 45

struct Ping
{
    u16 timer;                 // frames left, 0 = none
    u8 mapGroup, mapNum;
    s16 x, y;                  // map coords (with MAP_OFFSET)
};

static struct Ping sPings[MP_MAX_PLAYERS];
static u16 sCooldown;

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

static void SetPing(u8 id, u8 group, u8 num, s16 x, s16 y)
{
    sPings[id].mapGroup = group;
    sPings[id].mapNum = num;
    sPings[id].x = x;
    sPings[id].y = y;
    sPings[id].timer = PING_FRAMES;
}

void MP_Ping_Place(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 buf[7];

    if (!MP_IsConnected() || gMpLocalId < 0 || !MP_IsInOverworld() || sCooldown
     || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    SetPing(gMpLocalId, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, x, y);
    sCooldown = PING_COOLDOWN;
    PlaySE(SE_PIN);
    buf[0] = MPR_PING;
    buf[1] = gSaveBlock1Ptr->location.mapGroup;
    buf[2] = gSaveBlock1Ptr->location.mapNum;
    buf[3] = x & 0xFF;
    buf[4] = (x >> 8) & 0xFF;
    buf[5] = y & 0xFF;
    buf[6] = (y >> 8) & 0xFF;
    MP_SendRelay(0xFF, buf, sizeof(buf));   // everyone
}

void MP_Ping_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || from == gMpLocalId || len < 7)
        return;
    SetPing(from, data[1], data[2], (s16)(data[3] | (data[4] << 8)), (s16)(data[5] | (data[6] << 8)));
    if (data[1] == gSaveBlock1Ptr->location.mapGroup && data[2] == gSaveBlock1Ptr->location.mapNum)
        PlaySE(SE_PIN);
}

// Every overworld frame (from MP_Emote_Update, before the bubbles are drawn)
void MP_Ping_Update(void)
{
    struct ObjectEvent *me = gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT ? &gObjectEvents[gPlayerAvatar.objectEventId] : NULL;
    u8 id;

    if (sCooldown)
        sCooldown--;
    for (id = 0; id < MP_MAX_PLAYERS; id++)
    {
        struct Ping *p = &sPings[id];
        bool8 here = p->timer && me && MP_IsConnected() && (id == gMpLocalId || gMpPlayers[id].present)
                  && p->mapGroup == gSaveBlock1Ptr->location.mapGroup && p->mapNum == gSaveBlock1Ptr->location.mapNum;
        s32 dx = 0, dy = 0;
        bool8 onScreen = FALSE;

        if (p->timer)
            p->timer--;
        if (here)
        {
            dx = p->x - me->currentCoords.x;
            dy = p->y - me->currentCoords.y;
            onScreen = abs(dx) <= 7 && abs(dy) <= 5;
        }
        if (here && onScreen)
        {
            u8 text[32], name[PLAYER_NAME_LENGTH + 1];
            static const u8 sLook[] = _(": Look here!");
            if (id == gMpLocalId)
                StringCopyN(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
            else
                StringCopyN(name, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
            name[PLAYER_NAME_LENGTH] = EOS;
            StringCopy(text, name);
            StringAppend(text, sLook);
            MP_Emote_PingBubble(id, text, p->x, p->y);
        }
        else
        {
            MP_Emote_PingBubble(id, NULL, 0, 0);
        }
        MP_Compass_PointAt(id, id, dx, dy, here && !onScreen);
    }
}
