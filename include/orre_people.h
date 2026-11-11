#ifndef GUARD_ORRE_PEOPLE_H
#define GUARD_ORRE_PEOPLE_H

// ORRE's people (src/orre/orre_people.c): each Colosseum character model as a LOOK > CREATE recipe (their field
// sprite, object graphics OBJ_EVENT_GFX_ORRE_FIRST + person) and a Pokemon Showdown trainer sprite (their battle
// picture, 80x80 as drawn). The table is tools/orre/people.py's (src/data/orre_people.h).

#define ORRE_PERSON_NONE 0xFFFF

struct Sprite;
struct MpCreated;

u16 Orre_People_Count(void);
u16 Orre_People_Find(const char *model);                       // a model's person ("ken_b1"), ORRE_PERSON_NONE if none
const char *Orre_People_Model(u16 person);
bool8 Orre_People_Recipe(u16 person, struct MpCreated *out);
bool8 Orre_People_Field(u16 person, const u8 **frames, const u16 **pal);   // 9 walking frames of 32x32 (4bpp tiles)
const char *Orre_People_PicName(u16 person);                   // the battle picture's Showdown name
const char *Orre_People_PicArtist(u16 person);                 // ... its artist ("" the games' own sprite)

// The battle picture: the next battle's foe A / B is PERSON (the converted scripts set it before a trainer battle;
// in ORRE only, until the battle ends)
void Orre_People_SetFoe(u8 which, u16 person);
void Orre_People_SetFoeDefault(u8 which, u16 person);          // ... if none is set
void Orre_SetFoePerson(void);                                  // (special) VAR_0x8004 person, VAR_0x8005 0 A / 1 B
void Orre_People_BattleTrainerPic(struct Sprite *sprite, u8 battler, u16 standIn);   // the battle controllers' hook
void Orre_People_DrawPic(struct Sprite *sprite, u16 person, u16 standIn);           // PERSON's picture on SPRITE
void Orre_People_BattleEnd(void);
bool8 Orre_People_PicPixels(u16 person, const u8 **pixels, u8 *bpp, const u16 **pal, u8 *colors);   // 80x80

#endif // GUARD_ORRE_PEOPLE_H
