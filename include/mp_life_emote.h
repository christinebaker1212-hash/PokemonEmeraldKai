#ifndef GUARD_MP_LIFE_EMOTE_H
#define GUARD_MP_LIFE_EMOTE_H

// Moemon Co-op: MOéMON GARDEN, Pokémon emotes (src/mp/mp_life_emote.c): a round pink bubble over a
// Pokémon, never like the Trainers' square emotes.
enum
{
    LIFE_EMOTE_NOTE,       // happy, playful
    LIFE_EMOTE_HEART,      // affection
    LIFE_EMOTE_ZZZ,        // sleepy, napping
    LIFE_EMOTE_EXCLAIM,    // surprise
    LIFE_EMOTE_QUESTION,   // curious
    LIFE_EMOTE_SWEAT,      // nervous, afraid
    LIFE_EMOTE_ANGRY,      // frustrated
    LIFE_EMOTE_TEAR,       // sad
    LIFE_EMOTE_SPARKLE,    // proud
    LIFE_EMOTE_FOOD,       // hungry, eating
    LIFE_EMOTE_DOTS,       // restless, bored, unsure
    LIFE_EMOTE_CALM,       // calm, content
    LIFE_EMOTE_SUN,        // basking
    LIFE_EMOTE_WATER,      // soaking, splashing
    LIFE_EMOTE_BALL,       // wants to play (an invitation)
    LIFE_EMOTE_LEAF,       // in the leaves, outdoors
    LIFE_EMOTE_COUNT
};
#define LIFE_EMOTE_NONE 0xFF

#define LIFE_EMOTE_SLOT_LOCAL      0
#define LIFE_EMOTE_SLOT_REMOTE(id) (1 + (id))
#define LIFE_EMOTE_SLOT_GARDEN(i)  (1 + MP_MAX_PLAYERS + (i))
#define LIFE_EMOTE_SLOTS           (1 + MP_MAX_PLAYERS + 24)
#define LIFE_EMOTE_SLOT_BATTLER(b) LIFE_EMOTE_SLOT_GARDEN(b)   // in battle (no Garden then): battler b's Pokémon

void MP_LifeEmote_Show(u8 slot, u8 icon);
u8 MP_LifeEmote_Current(u8 slot);
void MP_LifeEmote_Place(u8 slot, bool8 visible, s16 x, s16 top, u8 subpriority);
void MP_LifeEmote_HideAll(void);

#endif // GUARD_MP_LIFE_EMOTE_H
