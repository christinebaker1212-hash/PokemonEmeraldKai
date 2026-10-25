// Moemon Co-op: the PATCH NOTES read on the CO-OP setup screen (mp_setup.c), every version back to the first build.
// The text is tools/release/notes/v<ver>.md, turned into src/data/mp_patchnotes.h by tools/release/gen_patchnotes.py
// at build time (json_data_rules.mk): a release's notes file is all it takes.
#include "global.h"
#include "mp.h"

#include "../data/mp_patchnotes.h"

u16 MP_PatchNotes_Count(void)
{
    return ARRAY_COUNT(sMpPatchNotes);
}

// Newest first (0 = this version)
const struct MpPatchNote *MP_PatchNotes_Get(u16 index)
{
    return index < ARRAY_COUNT(sMpPatchNotes) ? &sMpPatchNotes[index] : NULL;
}
