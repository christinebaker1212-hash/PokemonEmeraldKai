// Moemon Kai: the game's own overworld sprites past the port's (ids OBJ_EVENT_GFX_KAI_FIRST and up, in the JOHTO /
// KANTO range: event_object_movement.c QzObjectEventGraphicsInfo / QzLoadObjectPalette)
// - JOHTO's APRICORN trees: Crystal's fruit tree (pokecrystal gfx/sprites/fruit_tree.png, its four shades as the
//   tree's greens), the same all day as in Crystal (src/mp/mp_apricorn.c)
const u32 gObjectEventPic_KaiFruitTree[] = INCGFX_U32("graphics/object_events/pics/kai/fruit_tree.png", ".4bpp", "-mwidth 2 -mheight 2");
#include "constants/rgb.h"
static const u16 sObjectEventPal_KaiFruitTree[16] = {
    RGB(31, 0, 16), RGB(3, 5, 3), RGB(7, 16, 5), RGB(17, 25, 12),
};
#define KAI_PAL_TAG_FRUIT_TREE 0xA0F0

static const struct SpriteFrameImage sPicTable_KaiFruitTree[] = {
    overworld_frame(gObjectEventPic_KaiFruitTree, 2, 2, 0),
};

static const struct ObjectEventGraphicsInfo sObjectEventGraphicsInfo_KaiFruitTree = {
    .tileTag = TAG_NONE,
    .paletteTag = KAI_PAL_TAG_FRUIT_TREE,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = 128,
    .width = 16,
    .height = 16,
    .paletteSlot = PALSLOT_QZ,
    .shadowSize = SHADOW_SIZE_S,
    .inanimate = TRUE,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = TRACKS_NONE,
    .oam = &gObjectEventBaseOam_16x16,
    .subspriteTables = sOamTables_16x16,
    .anims = sAnimTable_Inanimate,
    .images = sPicTable_KaiFruitTree,
    .affineAnims = gDummySpriteAffineAnimTable,
};

static const struct SpritePalette sKaiObjectEventPalettes[] = {
    {sObjectEventPal_KaiFruitTree, KAI_PAL_TAG_FRUIT_TREE},
    {NULL, TAG_NONE},
};
