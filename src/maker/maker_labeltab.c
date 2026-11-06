// Moemon Maker, M4b: the scripts' global labels and the game's C names they point at (gStringVar4, a callnative's
// function...), as name -> address for the Maker's raw lines (src/maker/maker_raw.c Maker_RawLabel). In a file of its
// own: the generated `extern const u8 NAME[]` lines never meet the game's own declarations of those names.
typedef unsigned char u8;
struct MakerLabel { const char *name; const u8 *at; };
#include "constants/maker_labels.h"
