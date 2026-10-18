from common import *
REPOINT={'gPokedexMenu_Gfx':0xf96040,'gPokedexList_Tilemap':0xf96c00,'gIntroCopyright_Gfx':0xf953c0,
 'gIntroCopyright_Tilemap':0xf957c0,'gPokenavHeader_Gfx':0xf96dc0,'gPokenavHeader_Tilemap':0xf97040,
 'gTitleScreenEmeraldVersionGfx':0xf95980,'gItemIcon_PokeDoll':0xf93e00,'gItemIconPalette_PokeDoll':0xf93f80}
INPLACE=['gTitleScreenPokemonLogoGfx','gTitleScreenEmeraldVersionPal','gTilesetTiles_CableClub',
 'gFontNormalLatinGlyphs','gFontNarrowLatinGlyphs','gFontShortLatinGlyphs','gFontSmallLatinGlyphs']
for s,p in REPOINT.items():
    rel=write_blob(s,K[p:p+lz_len(K,p)[0]]); replace_def_with_incbin(s,rel)
for s in INPLACE:
    a,n=BYNAME[s]
    data=K[a:a+lz_len(K,a)[0]] if V[a]==0x10 and K[a]==0x10 and s.endswith('Gfx') and 'Logo' in s else K[a:a+n]
    if V[a]==0x10: data=K[a:a+lz_len(K,a)[0]]
    rel=write_blob(s,data); replace_def_with_incbin(s,rel); print(s,len(data),n)
