// Moemon Kai: CANDIES. Every released Moemon leaves a CANDY of its first type; they're kept in the CANDY JAR
// (a key item PROF. BIRCH brings the first time, in person, once you're back from the PC). The jar's menu shows
// them filling a jar; CONVERT turns each 10 of a kind into a RARE CANDY.
// The counts go in "<save>.bag" with the save (mp_bag.c), so they always match the saved game.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "bg.h"
#include "gpu_regs.h"
#include "item_icon.h"
#include "item_menu.h"
#include "random.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"

#define CANDY_MAX 9999
#define CANDY_MAGIC 0x59444E43   // "CNDY"
#define LOCALID_BIRCH 0xDA

u16 gMpCandies[NUMBER_OF_MON_TYPES];

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

// ---------------------------------------------------------------------------
// Counts

static u8 CandyType(u16 species)
{
    u8 type;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return TYPE_NORMAL;
    type = gSpeciesInfo[species].types[0];
    if (type >= NUMBER_OF_MON_TYPES || type == TYPE_MYSTERY)
        type = TYPE_NORMAL;
    return type;
}

// A Moemon was released (never an EGG: the PC doesn't release those)
void MP_Candy_OnRelease(u16 species)
{
    u8 type = CandyType(species);
    if (gMpCandies[type] < CANDY_MAX)
        gMpCandies[type]++;
    MPNet_Log("candy: released species %d -> type %d candy (%d now)", species, type, gMpCandies[type]);
}

u32 MP_Candy_Total(void)
{
    u32 n = 0;
    u8 t;
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        n += gMpCandies[t];
    return n;
}

void MP_Candy_Reset(void)
{
    memset(gMpCandies, 0, sizeof(gMpCandies));
}

// RARE CANDIES CONVERT would make now
u16 MP_Candy_ConvertCount(void)
{
    u16 n = 0;
    u8 t;
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        n += gMpCandies[t] / 10;
    return n;
}

// Each 10 of a kind -> a RARE CANDY. FALSE (nothing changed) when there's nothing to convert or no room in the BAG.
bool8 MP_Candy_Convert(u16 *made)
{
    u16 n = MP_Candy_ConvertCount();
    u8 t;
    *made = n;
    if (n == 0 || !CheckBagHasSpace(ITEM_RARE_CANDY, n))
        return FALSE;
    AddBagItem(ITEM_RARE_CANDY, n);
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        gMpCandies[t] %= 10;
    MPNet_Log("candy: converted into %d RARE CANDY", n);
    return TRUE;
}

// "<save>.bag" (mp_bag.c): after the item marks
void MP_Candy_Write(FILE *f)
{
    u32 magic = CANDY_MAGIC;
    u16 count = NUMBER_OF_MON_TYPES;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&count, sizeof(count), 1, f);
    fwrite(gMpCandies, sizeof(gMpCandies[0]), NUMBER_OF_MON_TYPES, f);
}

void MP_Candy_Read(FILE *f)
{
    u32 magic = 0;
    u16 count = 0, i, v;
    MP_Candy_Reset();
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != CANDY_MAGIC || fread(&count, sizeof(count), 1, f) != 1)
        return;   // a sidecar from before the candies
    for (i = 0; i < count; i++)
    {
        if (fread(&v, sizeof(v), 1, f) != 1)
            return;
        if (i < NUMBER_OF_MON_TYPES)
            gMpCandies[i] = v > CANDY_MAX ? CANDY_MAX : v;
    }
}

// ---------------------------------------------------------------------------
// PROF. BIRCH brings the CANDY JAR: after a release, the first time the player is free on the map without one, he
// walks up (from up to 4 free tiles away, on the side with the most room), explains, hands it over and leaves.

static EWRAM_DATA struct
{
    u8 state;
    u8 objId;
    u8 steps, walked;
    u8 dir;          // Birch's way toward the player
    u16 timer;
} sBirch = {0};

static bool8 TileFree(s16 x, s16 y, u8 elevation)
{
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (MapGridGetElevationAt(x, y) != elevation && MapGridGetElevationAt(x, y) != 0 && elevation != 0)
        return FALSE;
    return TRUE;
}

static void BirchText(const char *ascii)
{
    static u8 sText[480];
    char buf[480], name[16];
    const char *p;
    int n = 0;
    MP_GameToAscii(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    // "@" is the player's name
    for (p = ascii; *p && n < (int)sizeof(buf) - 16; p++)
    {
        if (*p == '@')
            n += snprintf(buf + n, sizeof(buf) - n, "%s", name);
        else
            buf[n++] = *p;
    }
    buf[n] = 0;
    MP_AsciiToGame(sText, buf, sizeof(sText) - 1);
    MP_ShowFieldMessageFitted(sText);
}

static void Task_BirchCandyJar(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *birch = sBirch.objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[sBirch.objId] : NULL;
    static const s8 sDx[] = {0, 0, 0, -1, 1}, sDy[] = {0, 1, -1, 0, 0};   // DIR_SOUTH, NORTH, WEST, EAST

    switch (sBirch.state)
    {
    case 0:
    {
        // where Birch comes from: the side with the longest free line (up to 4 tiles), south first
        static const u8 sOrder[] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
        s16 px = player->currentCoords.x, py = player->currentCoords.y;
        u8 i, best = 0, bestLen = 0;
        for (i = 0; i < ARRAY_COUNT(sOrder); i++)
        {
            u8 d = sOrder[i], len = 0;
            while (len < 4 && TileFree(px + sDx[d] * (len + 1), py + sDy[d] * (len + 1), player->currentElevation))
                len++;
            if (len > bestLen)
            {
                bestLen = len;
                best = d;
            }
        }
        sBirch.objId = OBJECT_EVENTS_COUNT;
        if (bestLen > 0)
            sBirch.objId = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_PROF_BIRCH, MOVEMENT_TYPE_NONE, LOCALID_BIRCH,
                                                                px + sDx[best] * bestLen, py + sDy[best] * bestLen,
                                                                player->currentElevation);
        if (sBirch.objId >= OBJECT_EVENTS_COUNT)
        {
            // nowhere to stand: he calls instead
            MPNet_Log("candy: Birch can't stand anywhere, the jar comes by message");
            sBirch.state = 10;
            break;
        }
        // he faces the player, who turns to him
        sBirch.dir = (best == DIR_SOUTH) ? DIR_NORTH : (best == DIR_NORTH) ? DIR_SOUTH : (best == DIR_WEST) ? DIR_EAST : DIR_WEST;
        sBirch.steps = bestLen - 1;
        sBirch.walked = 0;
        SetObjectEventDirection(&gObjectEvents[sBirch.objId], sBirch.dir);
        ObjectEventTurn(player, best);
        MPNet_Log("candy: Birch comes from %d tiles away (dir %d)", bestLen, best);
        PlaySE(SE_PIN);
        sBirch.timer = 0;
        sBirch.state++;
        break;
    }
    case 1:
        if (++sBirch.timer < 20)
            break;
        sBirch.state++;
        // fallthrough
    case 2:
        // walk up to the player
        if (ObjectEventIsHeldMovementActive(birch) && !ObjectEventClearHeldMovementIfFinished(birch))
            break;
        if (sBirch.walked < sBirch.steps)
        {
            ObjectEventSetHeldMovement(birch, GetWalkNormalMovementAction(sBirch.dir));
            sBirch.walked++;
            break;
        }
        sBirch.state = 3;
        break;
    case 3:
        BirchText("PROF. BIRCH: Hi, @! I was in the\nneighborhood and saw you at the PC.\f"
                  "Did you just let a MO~MON go?\nThen you've seen it, haven't you?\f"
                  "When a MO~MON is released, it leaves\nsomething behind: a CANDY full of its\ntype's power!\f"
                  "Nobody quite knows how it happens.\nIt's one of the mysteries I study!\f"
                  "You'll want somewhere to keep them.\nHere, take this!");
        sBirch.state++;
        break;
    case 4:
        if (IsTextPrinterActive(0))
            break;
        if (!JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        // fallthrough
    case 10:
        AddBagItem(ITEM_CANDY_JAR, 1);
        PlayFanfare(MUS_OBTAIN_ITEM);
        BirchText("@ received the CANDY JAR!");
        MPNet_Log("candy: got the CANDY JAR (%d candies in it)", (int)MP_Candy_Total());
        sBirch.state = (sBirch.state == 10) ? 11 : 5;
        break;
    case 5:
    case 11:
        if (IsTextPrinterActive(0) || !IsFanfareTaskInactive())
            break;
        if (!JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        BirchText(sBirch.state == 5
                  ? "Every CANDY from a released MO~MON\ngoes in the jar by itself.\f"
                    "Ten of one kind? CONVERT them into\na RARE CANDY! Use the jar from your\nKEY ITEMS.\f"
                    "Well, I'll be off. Take care, @!"
                  : "(PROF. BIRCH sent it over with a note:\nreleased MO~MON leave CANDIES behind,\n"
                    "and ten of one kind make a RARE\nCANDY. CONVERT them in the jar!)");
        sBirch.state = (sBirch.state == 5) ? 6 : 12;
        break;
    case 6:
    case 12:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        if (sBirch.state == 12)
        {
            sBirch.state = 9;
            break;
        }
        sBirch.walked = 0;
        sBirch.state = 7;
        break;
    case 7:
        // back the way he came, then gone
        if (ObjectEventIsHeldMovementActive(birch) && !ObjectEventClearHeldMovementIfFinished(birch))
            break;
        if (sBirch.walked < sBirch.steps + 1)
        {
            u8 away = (sBirch.dir == DIR_NORTH) ? DIR_SOUTH : (sBirch.dir == DIR_SOUTH) ? DIR_NORTH
                    : (sBirch.dir == DIR_WEST) ? DIR_EAST : DIR_WEST;
            ObjectEventSetHeldMovement(birch, GetWalkNormalMovementAction(away));
            sBirch.walked++;
            break;
        }
        RemoveObjectEventByLocalIdAndMap(LOCALID_BIRCH, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        sBirch.state = 9;
        break;
    case 9:
        UnlockPlayerFieldControls();
        MPNet_Log("candy: Birch's visit is over");
        DestroyTask(taskId);
        break;
    }
}

// Every frame (MP_FrameUpdate): Birch comes once the player is free on the map
void MP_Candy_Update(void)
{
    if (gMain.inBattle || FuncIsActiveTask(Task_BirchCandyJar) || !MP_IsInOverworld() || !MP_IsLocalFree())
        return;
    if (MP_Candy_Total() == 0 || CheckBagHasItem(ITEM_CANDY_JAR, 1))
        return;
    LockPlayerFieldControls();
    memset(&sBirch, 0, sizeof(sBirch));
    CreateTask(Task_BirchCandyJar, 80);
}

// ---------------------------------------------------------------------------
// The CANDY JAR's menu: the jar on the left filling up with candies (sprites, up to JAR_SHOWN; past that the jar is
// full and the mix follows the counts), each type's count on the right, A = CONVERT, B = back.


#define JAR_X 8
#define JAR_Y 16
#define JAR_SHOWN 96
#define TAG_CANDY 0x5C40   // + type (tiles and palette)

static const u32 sJar_Gfx[] = INCGFX_U32("graphics/moemon/candies/jar.png", ".4bpp");
static const u32 sJar_Pal[] = INCGFX_U32("graphics/moemon/candies/jar.png", ".gbapal");
static const u32 sCandyGfx_Normal[] = INCGFX_U32("graphics/moemon/candies/normal.png", ".4bpp");
static const u32 sCandyPal_Normal[] = INCGFX_U32("graphics/moemon/candies/normal.png", ".gbapal");
static const u32 sCandyGfx_Fighting[] = INCGFX_U32("graphics/moemon/candies/fighting.png", ".4bpp");
static const u32 sCandyPal_Fighting[] = INCGFX_U32("graphics/moemon/candies/fighting.png", ".gbapal");
static const u32 sCandyGfx_Flying[] = INCGFX_U32("graphics/moemon/candies/flying.png", ".4bpp");
static const u32 sCandyPal_Flying[] = INCGFX_U32("graphics/moemon/candies/flying.png", ".gbapal");
static const u32 sCandyGfx_Poison[] = INCGFX_U32("graphics/moemon/candies/poison.png", ".4bpp");
static const u32 sCandyPal_Poison[] = INCGFX_U32("graphics/moemon/candies/poison.png", ".gbapal");
static const u32 sCandyGfx_Ground[] = INCGFX_U32("graphics/moemon/candies/ground.png", ".4bpp");
static const u32 sCandyPal_Ground[] = INCGFX_U32("graphics/moemon/candies/ground.png", ".gbapal");
static const u32 sCandyGfx_Rock[] = INCGFX_U32("graphics/moemon/candies/rock.png", ".4bpp");
static const u32 sCandyPal_Rock[] = INCGFX_U32("graphics/moemon/candies/rock.png", ".gbapal");
static const u32 sCandyGfx_Bug[] = INCGFX_U32("graphics/moemon/candies/bug.png", ".4bpp");
static const u32 sCandyPal_Bug[] = INCGFX_U32("graphics/moemon/candies/bug.png", ".gbapal");
static const u32 sCandyGfx_Ghost[] = INCGFX_U32("graphics/moemon/candies/ghost.png", ".4bpp");
static const u32 sCandyPal_Ghost[] = INCGFX_U32("graphics/moemon/candies/ghost.png", ".gbapal");
static const u32 sCandyGfx_Steel[] = INCGFX_U32("graphics/moemon/candies/steel.png", ".4bpp");
static const u32 sCandyPal_Steel[] = INCGFX_U32("graphics/moemon/candies/steel.png", ".gbapal");
static const u32 sCandyGfx_Fire[] = INCGFX_U32("graphics/moemon/candies/fire.png", ".4bpp");
static const u32 sCandyPal_Fire[] = INCGFX_U32("graphics/moemon/candies/fire.png", ".gbapal");
static const u32 sCandyGfx_Water[] = INCGFX_U32("graphics/moemon/candies/water.png", ".4bpp");
static const u32 sCandyPal_Water[] = INCGFX_U32("graphics/moemon/candies/water.png", ".gbapal");
static const u32 sCandyGfx_Grass[] = INCGFX_U32("graphics/moemon/candies/grass.png", ".4bpp");
static const u32 sCandyPal_Grass[] = INCGFX_U32("graphics/moemon/candies/grass.png", ".gbapal");
static const u32 sCandyGfx_Electric[] = INCGFX_U32("graphics/moemon/candies/electric.png", ".4bpp");
static const u32 sCandyPal_Electric[] = INCGFX_U32("graphics/moemon/candies/electric.png", ".gbapal");
static const u32 sCandyGfx_Psychic[] = INCGFX_U32("graphics/moemon/candies/psychic.png", ".4bpp");
static const u32 sCandyPal_Psychic[] = INCGFX_U32("graphics/moemon/candies/psychic.png", ".gbapal");
static const u32 sCandyGfx_Ice[] = INCGFX_U32("graphics/moemon/candies/ice.png", ".4bpp");
static const u32 sCandyPal_Ice[] = INCGFX_U32("graphics/moemon/candies/ice.png", ".gbapal");
static const u32 sCandyGfx_Dragon[] = INCGFX_U32("graphics/moemon/candies/dragon.png", ".4bpp");
static const u32 sCandyPal_Dragon[] = INCGFX_U32("graphics/moemon/candies/dragon.png", ".gbapal");
static const u32 sCandyGfx_Dark[] = INCGFX_U32("graphics/moemon/candies/dark.png", ".4bpp");
static const u32 sCandyPal_Dark[] = INCGFX_U32("graphics/moemon/candies/dark.png", ".gbapal");

static const struct { const u32 *gfx; const u32 *pal; const char *name; } sCandyArt[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL] = {sCandyGfx_Normal, sCandyPal_Normal, "NOR"},
    [TYPE_FIGHTING] = {sCandyGfx_Fighting, sCandyPal_Fighting, "FIG"},
    [TYPE_FLYING] = {sCandyGfx_Flying, sCandyPal_Flying, "FLY"},
    [TYPE_POISON] = {sCandyGfx_Poison, sCandyPal_Poison, "POI"},
    [TYPE_GROUND] = {sCandyGfx_Ground, sCandyPal_Ground, "GRO"},
    [TYPE_ROCK] = {sCandyGfx_Rock, sCandyPal_Rock, "ROC"},
    [TYPE_BUG] = {sCandyGfx_Bug, sCandyPal_Bug, "BUG"},
    [TYPE_GHOST] = {sCandyGfx_Ghost, sCandyPal_Ghost, "GHO"},
    [TYPE_STEEL] = {sCandyGfx_Steel, sCandyPal_Steel, "STE"},
    [TYPE_MYSTERY] = {NULL, NULL, NULL},
    [TYPE_FIRE] = {sCandyGfx_Fire, sCandyPal_Fire, "FIR"},
    [TYPE_WATER] = {sCandyGfx_Water, sCandyPal_Water, "WAT"},
    [TYPE_GRASS] = {sCandyGfx_Grass, sCandyPal_Grass, "GRA"},
    [TYPE_ELECTRIC] = {sCandyGfx_Electric, sCandyPal_Electric, "ELE"},
    [TYPE_PSYCHIC] = {sCandyGfx_Psychic, sCandyPal_Psychic, "PSY"},
    [TYPE_ICE] = {sCandyGfx_Ice, sCandyPal_Ice, "ICE"},
    [TYPE_DRAGON] = {sCandyGfx_Dragon, sCandyPal_Dragon, "DRA"},
    [TYPE_DARK] = {sCandyGfx_Dark, sCandyPal_Dark, "DAR"},
};

enum { WIN_JAR, WIN_LIST, WIN_HELP, WIN_JAR_COUNT };

static const struct BgTemplate sJarBgs[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0},
};

static const struct WindowTemplate sJarWindows[] = {
    [WIN_JAR] = {.bg = 1, .tilemapLeft = JAR_X / 8, .tilemapTop = JAR_Y / 8, .width = 12, .height = 16, .paletteNum = 1, .baseBlock = 1},
    [WIN_LIST] = {.bg = 0, .tilemapLeft = 15, .tilemapTop = 1, .width = 14, .height = 16, .paletteNum = 15, .baseBlock = 1},
    [WIN_HELP] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 18, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 1 + 14 * 16},
    DUMMY_WIN_TEMPLATE,
};

static const struct OamData sOam_Candy = {.shape = SPRITE_SHAPE(16x16), .size = SPRITE_SIZE(16x16), .priority = 1};

static EWRAM_DATA struct
{
    MainCallback back;
    u8 state;
    u8 listSprites[NUMBER_OF_MON_TYPES];
    bool8 leaving;
    float r, targetR;       // the candies' radius now and where it's going
    u8 pourTimer;
    u8 shakeTimer;
    s8 shakeX;
} sJar = {0};

static const u8 sTextInk[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void JarText(u8 win, u8 x, u8 y, const char *ascii)
{
    u8 buf[96];
    MP_AsciiToGame(buf, ascii, sizeof(buf) - 1);
    AddTextPrinterParameterized3(win, FONT_SMALL, x, y, sTextInk, TEXT_SKIP_DRAW, buf);
}

static void JarHelp(const char *ascii)
{
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    JarText(WIN_HELP, 4, 2, ascii);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static u8 CandySprite(u8 type, s16 x, s16 y, u8 subpriority)
{
    struct SpriteTemplate t = {
        .tileTag = TAG_CANDY + type, .paletteTag = TAG_CANDY + type, .oam = &sOam_Candy, .anims = gDummySpriteAnimTable,
        .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy,
    };
    return CreateSprite(&t, x, y, subpriority);
}

// ---------------------------------------------------------------------------
// The jar's physics. Each candy sprite is a ball (radius sJar.r, the same for all: it shrinks as the jar fills) inside
// the jar's glass, which is read from the jar picture: its outline pixels are the walls, its glass pixels the inside.
// Balls fall, bounce off the glass and each other and roll (their turn picks one of ROT_STEPS shared rotation + scale
// matrices). Up to JAR_SHOWN sprites; past that each one stands for several candies and the mix follows the counts.
// The lid (rows 0-17 of the picture) is three sprites: it flies up off the screen while candies pour in or leave.

#define ROT_STEPS 16
#define SUBSTEPS 4
#define BALL_PASSES 4   // rounds of pushing balls apart in each substep
#define GRAVITY (0.22f / SUBSTEPS)
#define MAX_SPEED 6.0f
#define JAR_W 96
#define JAR_H 128
#define LID_ROWS 18            // the lid's rows in the picture
#define NECK_L 19              // the opening (picture x, inclusive)
#define NECK_R 76
#define MAX_WALL_POINTS 640
#define TAG_LID 0x5C60
#define TAG_PRIZE 0x5C61
#define LID_UP_Y (-40)         // the lid's offset when it's off the jar
#define GATHER_X (JAR_X + 48)  // where leaving candies meet and the RARE CANDY shows
#define GATHER_Y 18

struct Ball
{
    float x, y, vx, vy, angle;   // screen pixels
    float px, py;                // the last place it was inside the glass
    float sx, sy;                // where it was when this substep began (its speed is how far it got)
    s16 shownX, shownY;          // where its sprite is
    bool8 touching;              // against the glass or another candy this frame
    bool8 contact;               // ... this substep
    float vx0, vy0;              // its speed before this substep's pushes
    u8 type, spriteId;
    u8 state;                    // BALL_*
};
enum { BALL_UNUSED, BALL_WAITING, BALL_IN, BALL_LEAVING };
enum { PHASE_POUR, PHASE_IDLE, PHASE_CONVERT, PHASE_SETTLE };

static EWRAM_DATA struct Ball sBalls[JAR_SHOWN] = {0};
static EWRAM_DATA u8 sMatrices[ROT_STEPS + 2] = {0};   // then the RARE CANDY's and the list's (smaller candies)
#define LIST_MATRIX (ROT_STEPS + 1)
static EWRAM_DATA u8 sInside[JAR_H][JAR_W] = {0};      // 1 = glass (a ball's center may be there)
static EWRAM_DATA u8 sWallX[MAX_WALL_POINTS] = {0}, sWallY[MAX_WALL_POINTS] = {0};
static EWRAM_DATA u16 sWallCount = 0;
static EWRAM_DATA u8 sLidTiles[3][32 * 32 / 2] = {0};
static EWRAM_DATA u8 sLidSprites[3] = {0};
static EWRAM_DATA struct
{
    u8 phase;
    u8 timer;
    float lidY, lidVy;   // the lid's offset from its place
    bool8 lidWanted;     // TRUE: it should be on the jar
    u8 prize;            // the RARE CANDY sprite (MAX_SPRITES: none)
    u8 arrived, leaving; // CONVERT: candies that reached the meeting point / still on their way
    float prizeScale;
} sFx = {0};

static float sqrtf_(float v)
{
    // Newton's method (no libm needed on every platform)
    float g = v > 1 ? v / 2 : 1;
    int i;
    if (v <= 0)
        return 0;
    for (i = 0; i < 12; i++)
        g = 0.5f * (g + v / g);
    return g;
}

static u8 JarPixel(u32 x, u32 y)
{
    const u8 *p = (const u8 *)sJar_Gfx;
    u32 tile = (y / 8) * (JAR_W / 8) + x / 8, b = p[tile * 32 + (y % 8) * 4 + (x % 8) / 2];
    return (x & 1) ? (b >> 4) : (b & 0xF);
}

// Reads the glass and its walls from the picture: color 1 is the outline, 6-8 the glass
static void JarReadGlass(void)
{
    u32 x, y;
    sWallCount = 0;
    for (y = 0; y < JAR_H; y++)
        for (x = 0; x < JAR_W; x++)
        {
            u8 c = JarPixel(x, y);
            sInside[y][x] = (y >= LID_ROWS && c >= 6 && c <= 8) || (y < LID_ROWS && x >= NECK_L && x <= NECK_R);
        }
    for (y = LID_ROWS; y < JAR_H; y++)
        for (x = 0; x < JAR_W; x++)
        {
            bool8 touches;
            if (JarPixel(x, y) != 1)
                continue;
            touches = (x > 0 && sInside[y][x - 1]) || (x + 1 < JAR_W && sInside[y][x + 1])
                   || (sInside[y - 1][x]) || (y + 1 < JAR_H && sInside[y + 1][x]);
            if (touches && sWallCount < MAX_WALL_POINTS)
            {
                sWallX[sWallCount] = x;
                sWallY[sWallCount] = y;
                sWallCount++;
            }
        }
}

// Is a ball's center at screen (x, y) inside the glass or the open air above the neck?
static bool8 InGlass(float x, float y)
{
    int jx = (int)(x - JAR_X - sJar.shakeX), jy = (int)(y - JAR_Y);
    if (jx < 0 || jx >= JAR_W || jy >= JAR_H)
        return FALSE;
    if (jy < 0)
        return jx >= NECK_L && jx <= NECK_R;
    return sInside[jy][jx];
}

// The ball radius for n balls: they fill about half the glass
static float BallRadius(u32 n)
{
    float area = 80.0f * 90.0f * 0.55f, r;
    if (n == 0)
        return 7.5f;
    r = sqrtf_(area / n / 3.14159f);
    if (r > 7.5f)
        r = 7.5f;
    if (r < 3.0f)
        r = 3.0f;
    return r;
}

static void SetMatrices(float r)
{
    u8 i;
    s16 scale = (s16)(256.0f * r / 8.0f + 0.5f);
    for (i = 0; i < ROT_STEPS; i++)
        SetOamMatrixRotationScaling(sMatrices[i], scale, scale, i * (0x10000 / ROT_STEPS));
}

// Which types the balls show: one per candy up to JAR_SHOWN, else each type's share (every type there gets one)
static u8 BallTypes(u8 *order)
{
    u8 n = 0, t, i;
    u32 total = MP_Candy_Total();
    u16 shown[NUMBER_OF_MON_TYPES] = {0};
    if (total <= JAR_SHOWN)
    {
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            shown[t] = gMpCandies[t];
    }
    else
    {
        u32 left = JAR_SHOWN, rem[NUMBER_OF_MON_TYPES], have = 0, types = 0;
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
            if (gMpCandies[t])
                types++;
        left -= types;
        for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        {
            shown[t] = gMpCandies[t] ? 1 + (u32)gMpCandies[t] * left / total : 0;
            rem[t] = gMpCandies[t] ? (u32)gMpCandies[t] * left % total : 0;
            have += shown[t];
        }
        while (have < JAR_SHOWN)
        {
            u8 best = 0;
            for (t = 1; t < NUMBER_OF_MON_TYPES; t++)
                if (rem[t] > rem[best])
                    best = t;
            shown[best]++;
            rem[best] = 0;
            have++;
        }
    }
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        for (i = 0; i < shown[t] && n < JAR_SHOWN; i++)
            order[n++] = t;
    return n;
}

static u32 sRand;
static float Rand01(void)
{
    sRand = sRand * 1103515245 + 12345;
    return ((sRand >> 16) & 0x7FFF) / 32768.0f;
}

// A sprite that turns and scales with one of our matrices (never an affine animation's: those would rewrite it)
static void UseMatrix(struct Sprite *sp, u8 matrix)
{
    sp->affineAnims = gDummySpriteAffineAnimTable;
    sp->affineAnimPaused = TRUE;
    sp->oam.affineMode = ST_OAM_AFFINE_NORMAL;
    sp->oam.matrixNum = matrix;
}

static void BallSprite(struct Ball *b)
{
    b->spriteId = CandySprite(b->type, -32, -32, 100);
    if (b->spriteId != MAX_SPRITES)
    {
        UseMatrix(&gSprites[b->spriteId], sMatrices[0]);
        gSprites[b->spriteId].invisible = TRUE;
    }
}

static u8 CountBalls(u8 state)
{
    u8 i, n = 0;
    for (i = 0; i < JAR_SHOWN; i++)
        if (sBalls[i].state == state)
            n++;
    return n;
}

// The balls for the counts now. Balls already in the jar stay where they are while their type still has them; the
// extra ones leave (CONVERT: up and out); new ones wait to be poured in.
static void JarSyncBalls(void)
{
    u8 order[JAR_SHOWN], n = BallTypes(order), i, j, newCount = 0;
    u8 want[NUMBER_OF_MON_TYPES] = {0}, have[NUMBER_OF_MON_TYPES] = {0};
    u8 newSlots[JAR_SHOWN];

    for (i = 0; i < n; i++)
        want[order[i]]++;
    for (i = 0; i < JAR_SHOWN; i++)
        if (sBalls[i].state == BALL_IN || sBalls[i].state == BALL_WAITING)
        {
            if (have[sBalls[i].type] < want[sBalls[i].type])
                have[sBalls[i].type]++;
            else if (sBalls[i].state == BALL_WAITING)
            {
                if (sBalls[i].spriteId != MAX_SPRITES)
                    DestroySprite(&gSprites[sBalls[i].spriteId]);
                sBalls[i].spriteId = MAX_SPRITES;
                sBalls[i].state = BALL_UNUSED;
            }
            else
            {
                sBalls[i].state = BALL_LEAVING;
            }
        }
    for (i = 0; i < n; i++)
    {
        u8 t = order[i];
        if (have[t] >= want[t])
            continue;
        for (j = 0; j < JAR_SHOWN && sBalls[j].state != BALL_UNUSED; j++)
            ;
        if (j == JAR_SHOWN)
            break;   // the rest come once the leaving ones are gone (JarSyncBalls again)
        have[t]++;
        memset(&sBalls[j], 0, sizeof(sBalls[j]));
        sBalls[j].type = t;
        sBalls[j].state = BALL_WAITING;
        sBalls[j].angle = Rand01() * 6.28f;
        sBalls[j].spriteId = MAX_SPRITES;
        newSlots[newCount++] = j;
    }
    // shuffle the new ones' types so the colors mix as they pour in
    for (i = newCount; i > 1; i--)
    {
        u8 k = (u8)(Rand01() * i), a = newSlots[i - 1], b = newSlots[k], t = sBalls[a].type;
        sBalls[a].type = sBalls[b].type;
        sBalls[b].type = t;
    }
    for (i = 0; i < newCount; i++)
        BallSprite(&sBalls[newSlots[i]]);
    sJar.targetR = BallRadius(n);
}

// A ball against the glass: pushed out of the nearest wall pixel, bouncing off it
static void CollideGlass(struct Ball *b, float r)
{
    float bx = b->x - JAR_X - sJar.shakeX - 0.5f, by = b->y - JAR_Y - 0.5f, reach = r + 0.5f;
    float best = reach * reach, nx = 0, ny = 0;
    u16 i;
    bool8 hit = FALSE;
    for (i = 0; i < sWallCount; i++)
    {
        float dx = bx - sWallX[i], dy = by - sWallY[i], d2;
        if (dx > reach || dx < -reach || dy > reach || dy < -reach)
            continue;
        d2 = dx * dx + dy * dy;
        if (d2 < best)
        {
            best = d2;
            nx = dx;
            ny = dy;
            hit = TRUE;
        }
    }
    if (hit)
    {
        float d = sqrtf_(best);
        if (d < 0.001f)
            return;
        nx /= d;
        ny /= d;
        b->x += nx * (reach - d);
        b->y += ny * (reach - d);
        b->touching = b->contact = TRUE;
    }
    // above the neck (the lid's rows: no wall pixels there) the opening's sides are walls too
    if (by < LID_ROWS + r)
    {
        float lo = NECK_L + r + 0.5f, hi = NECK_R - r + 0.5f;
        if (bx < lo)
        {
            b->x += lo - bx;
            b->touching = b->contact = TRUE;
        }
        else if (bx > hi)
        {
            b->x -= bx - hi;
            b->touching = b->contact = TRUE;
        }
    }
    // the lid (on, or nearly) keeps them in
    if (b->state == BALL_IN && sFx.lidY > -8 && b->y < JAR_Y + LID_ROWS + r)
    {
        b->y = JAR_Y + LID_ROWS + r;
        b->touching = b->contact = TRUE;
    }
    if (InGlass(b->x, b->y))
    {
        b->px = b->x;
        b->py = b->y;
    }
    else
    {
        // squeezed through the glass: back to where it was
        b->x = b->px;
        b->y = b->py;
    }
}

static void Integrate(struct Ball *b, float r)
{
    float speed2;
    if (b->state == BALL_LEAVING)
    {
        // up through the neck, then to the meeting point above the jar
        float tx, ty, dx, dy, d;
        if (b->y > JAR_Y + LID_ROWS)
        {
            tx = JAR_X + (NECK_L + NECK_R) / 2.0f;
            ty = JAR_Y;
        }
        else
        {
            tx = GATHER_X;
            ty = GATHER_Y;
        }
        dx = tx - b->x;
        dy = ty - b->y;
        d = sqrtf_(dx * dx + dy * dy);
        if (d > 0.001f)
        {
            b->vx += (dx / d * 4.0f - b->vx) * (0.25f / SUBSTEPS);
            b->vy += (dy / d * 4.0f - b->vy) * (0.25f / SUBSTEPS);
        }
    }
    else
    {
        b->vy += GRAVITY;
    }
    speed2 = b->vx * b->vx + b->vy * b->vy;
    if (speed2 > MAX_SPEED * MAX_SPEED)
    {
        float k = MAX_SPEED / sqrtf_(speed2);
        b->vx *= k;
        b->vy *= k;
    }
    b->sx = b->x;
    b->sy = b->y;
    b->vx0 = b->vx;
    b->vy0 = b->vy;
    b->x += b->vx / SUBSTEPS;
    b->y += b->vy / SUBSTEPS;
}

static void CollideBalls(float r)
{
    u8 i, j;
    float min2 = 4 * r * r;
    for (i = 0; i < JAR_SHOWN; i++)
    {
        struct Ball *a = &sBalls[i];
        if (a->state != BALL_IN)
            continue;
        for (j = i + 1; j < JAR_SHOWN; j++)
        {
            struct Ball *b = &sBalls[j];
            float dx, dy, d2, d, overlap, nx, ny;
            if (b->state != BALL_IN)
                continue;
            dx = b->x - a->x;
            dy = b->y - a->y;
            if (dx >= 2 * r || dx <= -2 * r || dy >= 2 * r || dy <= -2 * r)
                continue;
            d2 = dx * dx + dy * dy;
            if (d2 >= min2)
                continue;
            d = sqrtf_(d2);
            if (d < 0.001f)
            {
                dx = (i & 1) ? 0.01f : -0.01f;
                dy = 0;
                d = 0.01f;
            }
            nx = dx / d;
            ny = dy / d;
            overlap = 2 * r - d;
            {
                // the upper one gives way more (a pile settles from the bottom up instead of bouncing)
                float wb = 0.5f + 0.3f * ny;   // ny > 0: b is below a
                wb = 1.0f - wb;
                a->x -= nx * overlap * (1.0f - wb);
                a->y -= ny * overlap * (1.0f - wb);
                b->x += nx * overlap * wb;
                b->y += ny * overlap * wb;
            }
            a->touching = b->touching = a->contact = b->contact = TRUE;
        }
    }
}

// Drops the next waiting ball in through the neck; FALSE when none is left
static bool8 PourOne(float r)
{
    u8 i;
    for (i = 0; i < JAR_SHOWN; i++)
        if (sBalls[i].state == BALL_WAITING)
        {
            struct Ball *b = &sBalls[i];
            b->state = BALL_IN;
            b->x = JAR_X + NECK_L + 2 + r + Rand01() * (NECK_R - NECK_L - 4 - 2 * r);
            b->y = -r;
            b->px = b->x;
            b->py = b->y;
            b->vx = (Rand01() - 0.5f) * 1.0f;
            b->vy = 1.5f;
            return TRUE;
        }
    return FALSE;
}

static void JarPhysics(void)
{
    u8 i, step;
    float r;

    // the size eases toward the size for the count
    sJar.r += (sJar.targetR - sJar.r) * 0.08f;
    r = sJar.r;
    SetMatrices(r);
    if (sJar.shakeTimer)
    {
        sJar.shakeTimer--;
        sJar.shakeX = (sJar.shakeTimer == 0) ? 0 : ((sJar.shakeTimer / 2) & 1) ? 2 : -2;
        ChangeBgX(1, -sJar.shakeX * 256, BG_COORD_SET);
    }
    for (step = 0; step < SUBSTEPS; step++)
    {
        for (i = 0; i < JAR_SHOWN; i++)
            if (sBalls[i].state == BALL_IN || sBalls[i].state == BALL_LEAVING)
                Integrate(&sBalls[i], r);
        u8 pass;
        for (pass = 0; pass < BALL_PASSES; pass++)
        {
            CollideBalls(r);
            for (i = 0; i < JAR_SHOWN; i++)
                if (sBalls[i].state == BALL_IN || (sBalls[i].state == BALL_LEAVING && sBalls[i].y > JAR_Y + LID_ROWS))
                    CollideGlass(&sBalls[i], r);
        }
        // the speed is how far each one really got (the pushes take away what the glass and the others stopped)
        for (i = 0; i < JAR_SHOWN; i++)
            if (sBalls[i].state == BALL_IN || sBalls[i].state == BALL_LEAVING)
            {
                struct Ball *b = &sBalls[i];
                float s2;
                b->vx = (b->x - b->sx) * SUBSTEPS;
                b->vy = (b->y - b->sy) * SUBSTEPS;
                b->angle += (b->x - b->sx) / r;
                if (b->contact && b->state == BALL_IN)
                {
                    // in a pile the pushes would turn into speed (candies popping out of it): a push can't throw a
                    // candy up, or sideways, faster than it was already going (a shake still can)
                    float up = (b->vy0 < 0 ? b->vy0 : 0) - 0.3f, side = (b->vx0 < 0 ? -b->vx0 : b->vx0) + 0.8f;
                    if (b->vy < up)
                        b->vy = up;
                    if (b->vx > side)
                        b->vx = side;
                    if (b->vx < -side)
                        b->vx = -side;
                }
                b->contact = FALSE;
                s2 = b->vx * b->vx + b->vy * b->vy;
                if (s2 > MAX_SPEED * MAX_SPEED)
                {
                    float k = MAX_SPEED / sqrtf_(s2);
                    b->vx *= k;
                    b->vy *= k;
                }
            }
    }
    for (i = 0; i < JAR_SHOWN; i++)
    {
        struct Ball *b = &sBalls[i];
        if (b->state != BALL_IN)
            continue;
        // against the glass (or resting in a pile), candies slow down; tiny drifts stop
        if (b->touching)
        {
            b->vx *= 0.9f;
            b->touching = FALSE;
        }
        b->vx *= 0.99f;
        if (b->vx * b->vx + b->vy * b->vy < 0.01f)
            b->vx = b->vy = 0;
    }
    // the sprites follow
    for (i = 0; i < JAR_SHOWN; i++)
    {
        struct Ball *b = &sBalls[i];
        struct Sprite *sp;
        int rot;
        if (b->state == BALL_UNUSED || b->spriteId == MAX_SPRITES)
            continue;
        sp = &gSprites[b->spriteId];
        if (b->state == BALL_WAITING)
        {
            sp->invisible = TRUE;
            continue;
        }
        if (b->state == BALL_LEAVING)
        {
            float dx = GATHER_X - b->x, dy = GATHER_Y - b->y;
            if (b->y <= JAR_Y + LID_ROWS && dx * dx + dy * dy < 16)
            {
                DestroySprite(sp);
                b->spriteId = MAX_SPRITES;
                b->state = BALL_UNUSED;
                sFx.arrived++;
                continue;
            }
        }
        sp->invisible = FALSE;
        // a resting candy keeps its pixel unless it really moved (no flicker between two pixels)
        if (b->x - b->shownX > 0.8f || b->x - b->shownX < -0.8f)
            b->shownX = (s16)(b->x + 0.5f);
        if (b->y - b->shownY > 0.8f || b->y - b->shownY < -0.8f)
            b->shownY = (s16)(b->y + 0.5f);
        sp->x = b->shownX;
        sp->y = b->shownY;
        rot = (int)(b->angle / 6.28318f * ROT_STEPS + 0.5f) % ROT_STEPS;
        if (rot < 0)
            rot += ROT_STEPS;
        sp->oam.matrixNum = sMatrices[rot];
        // the lower ones in front, like a real pile; leaving ones in front of all
        sp->subpriority = (b->state == BALL_LEAVING) ? 10 : 130 - (s16)(b->y / 2);
    }
}

// SELECT: the jar shakes and the candies tumble
static void JarShake(void)
{
    u8 i;
    for (i = 0; i < JAR_SHOWN; i++)
        if (sBalls[i].state == BALL_IN)
        {
            sBalls[i].vx += (Rand01() - 0.5f) * 4.0f;
            sBalls[i].vy -= 1.5f + Rand01() * 2.5f;
        }
    sJar.shakeTimer = 24;
    PlaySE(SE_BALL_BOUNCE_1);
}

// The lid: three 32x32 sprites cut from the picture's top rows
static void JarMakeLid(void)
{
    u8 part, i;
    static const struct OamData sOam_Lid = {.shape = SPRITE_SHAPE(32x32), .size = SPRITE_SIZE(32x32), .priority = 1};
    memset(sLidTiles, 0, sizeof(sLidTiles));
    for (part = 0; part < 3; part++)
    {
        u32 x, y;
        for (y = 0; y < LID_ROWS; y++)
            for (x = 0; x < 32; x++)
            {
                u8 c = JarPixel(part * 32 + x, y), *dst = &sLidTiles[part][((y / 8) * 4 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                *dst |= (x & 1) ? (c << 4) : c;
            }
    }
    {
        struct SpritePalette pal = {(const u16 *)sJar_Pal, TAG_LID};
        LoadSpritePalette(&pal);
    }
    for (part = 0; part < 3; part++)
    {
        struct SpriteSheet sheet = {sLidTiles[part], sizeof(sLidTiles[part]), TAG_LID + 0x10 + part};
        struct SpriteTemplate t = {
            .tileTag = TAG_LID + 0x10 + part, .paletteTag = TAG_LID, .oam = &sOam_Lid, .anims = gDummySpriteAnimTable,
            .images = NULL, .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy,
        };
        LoadSpriteSheet(&sheet);
        sLidSprites[part] = CreateSprite(&t, JAR_X + part * 32 + 16, JAR_Y + 16, 1);
    }
    for (i = 0; i < 3; i++)
        if (sLidSprites[i] != MAX_SPRITES)
            gSprites[sLidSprites[i]].y2 = (s16)sFx.lidY;
}

// The lid flies up when it isn't wanted and drops back (with a clink) when it is
static void JarLid(void)
{
    u8 i;
    if (!sFx.lidWanted)
    {
        sFx.lidVy -= 0.6f;
        sFx.lidY += sFx.lidVy;
        if (sFx.lidY < LID_UP_Y)
        {
            sFx.lidY = LID_UP_Y;
            sFx.lidVy = 0;
        }
    }
    else if (sFx.lidY < 0 || sFx.lidVy != 0)
    {
        sFx.lidVy += 0.6f;
        sFx.lidY += sFx.lidVy;
        if (sFx.lidY >= 0)
        {
            sFx.lidY = 0;
            if (sFx.lidVy > 1.5f)
            {
                PlaySE(SE_CLICK);
                sFx.lidVy = -sFx.lidVy * 0.25f;   // a small hop
            }
            else
            {
                sFx.lidVy = 0;
            }
        }
    }
    for (i = 0; i < 3; i++)
        if (sLidSprites[i] != MAX_SPRITES)
        {
            gSprites[sLidSprites[i]].y2 = (s16)sFx.lidY;
            gSprites[sLidSprites[i]].x2 = sJar.shakeX;
        }
}

static bool8 LidIsUp(void)
{
    return sFx.lidY <= LID_UP_Y + 0.5f;
}

static bool8 LidIsOn(void)
{
    return sFx.lidY == 0 && sFx.lidVy == 0;
}

static bool8 BallsResting(void)
{
    u8 i;
    for (i = 0; i < JAR_SHOWN; i++)
        if (sBalls[i].state == BALL_IN && (sBalls[i].vy * sBalls[i].vy + sBalls[i].vx * sBalls[i].vx > 0.8f
                                           || sBalls[i].y < JAR_Y + LID_ROWS + 4))
            return FALSE;
    return TRUE;
}

static void JarFreePrize(void)
{
    if (sFx.prize != MAX_SPRITES)
    {
        DestroySpriteAndFreeResources(&gSprites[sFx.prize]);
        sFx.prize = MAX_SPRITES;
    }
}

// The RARE CANDY that the leaving candies make: it grows with each one that arrives
static void JarPrize(void)
{
    struct Sprite *sp;
    if (sFx.prize == MAX_SPRITES)
    {
        if (sFx.arrived == 0)
            return;
        sFx.prize = AddItemIconSprite(TAG_PRIZE, TAG_PRIZE, ITEM_RARE_CANDY);
        if (sFx.prize == MAX_SPRITES)
            return;
        sp = &gSprites[sFx.prize];
        sp->x = GATHER_X;
        sp->y = GATHER_Y;
        sp->oam.priority = 0;
        sp->subpriority = 0;
        UseMatrix(sp, sMatrices[ROT_STEPS]);
        sFx.prizeScale = 0.3f;
    }
    sp = &gSprites[sFx.prize];
    {
        float total = sFx.arrived + sFx.leaving, want = total ? 0.4f + 0.6f * sFx.arrived / total : 1;
        if (sFx.arrived >= sFx.leaving && sFx.timer > 30)
            want = 0;   // all of them made it: the RARE CANDY goes into the BAG
        else if (sFx.arrived >= sFx.leaving && sFx.timer > 8)
            want = 1.25f;   // a little pop first
        sFx.prizeScale += (want - sFx.prizeScale) * 0.3f;
        if (sFx.prizeScale < 0.05f)
        {
            JarFreePrize();
            return;
        }
        SetOamMatrixRotationScaling(sMatrices[ROT_STEPS], (s16)(256 * sFx.prizeScale), (s16)(256 * sFx.prizeScale), 0);
    }
}

// The jar's animation, once a frame: pouring, CONVERT and settling
static void JarAnimate(void)
{
    JarPhysics();
    JarLid();
    switch (sFx.phase)
    {
    case PHASE_POUR:
        sFx.lidWanted = FALSE;
        if (!LidIsUp())
            break;
        // one candy a frame (two when there are many)
        if (!PourOne(sJar.r) || (CountBalls(BALL_WAITING) > 48 && !PourOne(sJar.r)))
        {
            if (CountBalls(BALL_WAITING) == 0)
            {
                sFx.phase = PHASE_SETTLE;
                sFx.timer = 0;
            }
        }
        break;
    case PHASE_SETTLE:
        if (++sFx.timer > 20 && (BallsResting() || sFx.timer > 240))
        {
            sFx.lidWanted = TRUE;
            if (LidIsOn())
            {
                JarFreePrize();
                sFx.phase = PHASE_IDLE;
            }
        }
        break;
    case PHASE_CONVERT:
        sFx.lidWanted = FALSE;
        if (!LidIsUp())
            break;
        if (sFx.timer == 0)
        {
            sFx.timer = 1;
            sFx.arrived = 0;
            JarSyncBalls();   // the converted ones leave
            sFx.leaving = CountBalls(BALL_LEAVING);
        }
        JarPrize();
        if (CountBalls(BALL_LEAVING) == 0)
            sFx.timer++;
        if (CountBalls(BALL_LEAVING) == 0 && sFx.timer > 30 && sFx.prize == MAX_SPRITES)
        {
            JarSyncBalls();   // any the leaving ones made room for
            sFx.phase = PHASE_POUR;
        }
        break;
    }
}

bool8 MP_Candy_JarBusy(void)
{
    return sFx.phase != PHASE_IDLE;
}

// Test harness (mp jarstate): the balls' state, and anything wrong with them
void MP_Candy_LogJar(void)
{
    u8 i, j, outside = 0, overlaps = 0, visibleOff = 0;
    float maxSpeed = 0, worst = 0, r = sJar.r;
    for (i = 0; i < JAR_SHOWN; i++)
    {
        struct Ball *a = &sBalls[i];
        float s;
        if (a->state != BALL_IN)
            continue;
        if (!InGlass(a->x, a->y))
            outside++;
        s = a->vx * a->vx + a->vy * a->vy;
        if (s > maxSpeed)
            maxSpeed = s;
        if (a->spriteId == MAX_SPRITES || gSprites[a->spriteId].invisible)
            visibleOff++;
        for (j = i + 1; j < JAR_SHOWN; j++)
        {
            struct Ball *b = &sBalls[j];
            float dx, dy, d;
            if (b->state != BALL_IN)
                continue;
            dx = b->x - a->x;
            dy = b->y - a->y;
            d = sqrtf_(dx * dx + dy * dy);
            if (d < 2 * r - 1.0f)
            {
                overlaps++;
                if (2 * r - d > worst)
                    worst = 2 * r - d;
            }
        }
    }
    for (i = 0, j = 0; i < JAR_SHOWN && j < 4; i++)
    {
        struct Ball *a = &sBalls[i];
        if (a->state == BALL_IN && a->vx * a->vx + a->vy * a->vy > 1)
        {
            MPNet_Log("candy: moving ball %d at %d,%d (jar %d,%d) speed %d,%d/10 last good %d,%d", i, (int)a->x, (int)a->y,
                      (int)(a->x - JAR_X), (int)(a->y - JAR_Y), (int)(a->vx * 10), (int)(a->vy * 10), (int)a->px, (int)a->py);
            j++;
        }
    }
    MPNet_Log("candy: jar phase %d, %d in, %d waiting, %d leaving, radius %d/10, %d outside the glass, %d overlapping "
              "(worst %d/10 px), top speed %d/10, %d hidden, lid %d, prize %s",
              sFx.phase, CountBalls(BALL_IN), CountBalls(BALL_WAITING), CountBalls(BALL_LEAVING), (int)(r * 10), outside,
              overlaps, (int)(worst * 10), (int)(sqrtf_(maxSpeed) * 10), visibleOff, (int)sFx.lidY,
              sFx.prize != MAX_SPRITES ? "shown" : "none");
}

static void JarDrawList(void)
{
    u8 i = 0, t;
    char line[24];
    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(1));
    JarText(WIN_LIST, 4, 1, "CANDY JAR");
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
    {
        u8 col, row;
        if (sCandyArt[t].gfx == NULL)
            continue;
        col = i / 9;
        row = i % 9;
        snprintf(line, sizeof(line), "%s %d", sCandyArt[t].name, gMpCandies[t]);
        JarText(WIN_LIST, col * 56 + 17, 15 + row * 12, line);
        if (sJar.listSprites[t] == MAX_SPRITES)
        {
            sJar.listSprites[t] = CandySprite(t, 15 * 8 + col * 56 + 9, 8 + 15 + row * 12 + 7, 0);
            if (sJar.listSprites[t] != MAX_SPRITES)
            {
                gSprites[sJar.listSprites[t]].oam.priority = 0;   // over the list's window
                UseMatrix(&gSprites[sJar.listSprites[t]], sMatrices[LIST_MATRIX]);
            }
        }
        i++;
    }
    CopyWindowToVram(WIN_LIST, COPYWIN_GFX);
}

static void VBlankCB_Jar(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Jar(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void JarIdleHelp(void)
{
    JarHelp(MP_Candy_ConvertCount() ? "A: CONVERT  SELECT: SHAKE  B: BACK" : "Ten of one kind make a RARE CANDY.  B: BACK");
}

static void Task_Jar(u8 taskId)
{
    JarAnimate();
    if (sJar.leaving)
    {
        if (!gPaletteFade.active)
        {
            u8 i;
            for (i = 0; i <= LIST_MATRIX; i++)
                FreeOamMatrix(sMatrices[i]);
            ChangeBgX(1, 0, BG_COORD_SET);
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            SetMainCallback2(sJar.back);
        }
        return;
    }
    if (gPaletteFade.active)
        return;
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sJar.leaving = TRUE;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        return;
    }
    if (sFx.phase != PHASE_IDLE)
        return;
    if (JOY_NEW(SELECT_BUTTON))
    {
        if (CountBalls(BALL_IN))
            JarShake();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u16 made;
        char text[48];
        if (MP_Candy_ConvertCount() == 0)
        {
            PlaySE(SE_FAILURE);
            JarHelp("Ten of one kind make a RARE CANDY.");
        }
        else if (!MP_Candy_Convert(&made))
        {
            PlaySE(SE_FAILURE);
            JarHelp("There's no room for RARE CANDY in the BAG.");
        }
        else
        {
            PlaySE(SE_SHOP);
            snprintf(text, sizeof(text), "You got %d RARE CANDY!", made);
            JarHelp(text);
            JarDrawList();
            sFx.phase = PHASE_CONVERT;
            sFx.timer = 0;
            MPNet_Log("candy: converted, %d RARE CANDY made, %d candies left", made, (int)MP_Candy_Total());
        }
    }
}

static void CB2_OpenJar(void)
{
    u8 i;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sJarBgs, ARRAY_COUNT(sJarBgs));
    for (i = 0; i < 2; i++)
    {
        // the map's scroll would still be there
        ChangeBgX(i, 0, BG_COORD_SET);
        ChangeBgY(i, 0, BG_COORD_SET);
    }
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sJarWindows);
    DeactivateAllTextPrinters();
    // a soft backdrop, the jar's colors, the text's
    {
        static const u16 sBackdrop = RGB(29, 27, 24);
        LoadPalette(&sBackdrop, BG_PLTT_ID(0), 2);
    }
    LoadPalette(sJar_Pal, BG_PLTT_ID(1), PLTT_SIZE_4BPP);
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
    {
        if (sCandyArt[i].gfx != NULL)
        {
            struct SpriteSheet sheet = {sCandyArt[i].gfx, 16 * 16 / 2, TAG_CANDY + i};
            struct SpritePalette pal = {(const u16 *)sCandyArt[i].pal, TAG_CANDY + i};
            LoadSpriteSheet(&sheet);
            LoadSpritePalette(&pal);
        }
        sJar.listSprites[i] = MAX_SPRITES;
    }
    for (i = 0; i <= LIST_MATRIX; i++)
        sMatrices[i] = AllocOamMatrix();
    SetOamMatrixRotationScaling(sMatrices[LIST_MATRIX], 0xB0, 0xB0, 0);   // the list's candies: 11 pixels
    memset(sBalls, 0, sizeof(sBalls));
    for (i = 0; i < JAR_SHOWN; i++)
        sBalls[i].spriteId = MAX_SPRITES;
    memset(&sFx, 0, sizeof(sFx));
    sFx.prize = MAX_SPRITES;
    sFx.lidWanted = TRUE;
    sJar.shakeX = 0;
    sJar.shakeTimer = 0;
    sRand = Random() | (Random() << 16);
    JarReadGlass();
    FillWindowPixelBuffer(WIN_JAR, PIXEL_FILL(0));
    BlitBitmapToWindow(WIN_JAR, (const u8 *)sJar_Gfx, 0, 0, 96, 128);
    FillWindowPixelRect(WIN_JAR, PIXEL_FILL(0), 0, 0, 96, LID_ROWS);   // the lid is sprites
    PutWindowTilemap(WIN_JAR);
    CopyWindowToVram(WIN_JAR, COPYWIN_FULL);
    LoadUserWindowBorderGfx(WIN_LIST, 0x200, BG_PLTT_ID(14));
    DrawStdFrameWithCustomTileAndPalette(WIN_LIST, FALSE, 0x200, 14);
    PutWindowTilemap(WIN_LIST);
    PutWindowTilemap(WIN_HELP);
    JarDrawList();
    JarMakeLid();
    JarSyncBalls();
    sJar.r = sJar.targetR;
    SetMatrices(sJar.r);
    // drop in on open: the lid comes off and the candies pour in
    sFx.phase = CountBalls(BALL_WAITING) ? PHASE_POUR : PHASE_IDLE;
    JarIdleHelp();
    CopyBgTilemapBufferToVram(0);
    CopyBgTilemapBufferToVram(1);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    sJar.leaving = FALSE;
    CreateTask(Task_Jar, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Jar);
    SetMainCallback2(CB2_Jar);
    MPNet_Log("candy: jar opened (%d candies, %d to convert)", (int)MP_Candy_Total(), MP_Candy_ConvertCount());
}

// Opens the jar's menu; back() when it closes
void MP_Candy_OpenJar(MainCallback back)
{
    sJar.back = back;
    SetMainCallback2(CB2_OpenJar);
}

bool8 MP_Candy_JarOpen(void)
{
    return gMain.callback2 == CB2_Jar && !sJar.leaving && !gPaletteFade.active;
}
