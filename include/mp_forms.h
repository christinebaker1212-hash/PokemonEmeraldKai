#ifndef GUARD_MP_FORMS_H
#define GUARD_MP_FORMS_H
// Moemon Kai: POKéMON forms outside battle (src/mp/mp_forms.c; data src/mp/mp_forms_data.c from
// tools/moemon_forms/gen_forms.py). Form 0 is the species' own entry everywhere.

struct MpFormData
{
    u16 species;
    u8 form;
    u8 types[2];
    u8 stats[6];               // (HP, Attack, Defense, Speed, Sp. Atk, Sp. Def)
    u16 abilities[2];
    const u32 *front[2];       // ([0] the MOéMON picture, [1] the original POKéMON one: OPTIONS > CREATURES)
    const u32 *back[2];
    const u32 *pal[2];
    const u32 *shiny[2];
    const u8 *icon;
    u8 iconPal;
};

extern const struct MpFormData gMpForms[];
extern const u32 gMpFormCount;

struct BoxPokemon;
struct Pokemon;
u8 MP_Form_Of(const struct BoxPokemon *box);                       // stored or worked out (season, held item)
void MP_Form_Set(struct BoxPokemon *box, u8 form);
u8 MP_Form_OfPersonality(u16 species, u32 personality);           // (the pictures: the POKéMON found by personality)
const struct MpFormData *MP_Form_Data(u16 species, u8 form);       // NULL: the species' own
bool8 MP_Form_HasForms(u16 species);
#endif
