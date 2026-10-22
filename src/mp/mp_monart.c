// Moemon Co-op: OPTIONS > CREATURES: Kai's MOéMON pictures (the default) or the original
// POKéMON ones, in this game only (nothing is stored in the Pokémon or sent).
// include/data.h turns the picture, palette, coordinate, elevation and animation tables
// into a pick between Kai's (_Moe) and the original game's (_Pokemon, mp_monart_data.c).
// The word MOéMON / POKéMON follows the same option (mp_textopts.c).
#include "global.h"
#include "data.h"
#include "mp.h"

bool8 MP_MonArtPokemon(void)
{
    return gSaveBlock2Ptr != NULL && gSaveBlock2Ptr->optionsPokemonArt;
}

void MP_MonArt_Changed(void)
{
    MPNet_Log("options: creatures %s", MP_MonArtPokemon() ? "POKEMON" : "MOEMON");
}
