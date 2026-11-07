// The game's way into SubDrag's MORT decoder (POKeMON STADIUM's announcer: src/mp/mp_stadium_announcer.c).
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "MORTDecoder.h"

// One clip of a speech archive held in memory (`buf`, `size` bytes): the MORT stream at `off`, `len` bytes long.
// *out gets malloc'ed 16-bit samples at the stream's own rate (16000 Hz), *count how many. 0: it didn't decode.
extern "C" int Mort_Decode(const unsigned char *buf, int size, unsigned off, unsigned len, short **out, unsigned *count)
{
    *out = NULL;
    *count = 0;
    if (buf == NULL || off + 8 > (unsigned)size || memcmp(buf + off, "MORT", 4) != 0)
        return 0;
    CMORTDecoder *dec = new CMORTDecoder();
    std::vector<unsigned short> pcm;
    // (the decoder reads but never writes the buffer it's given)
    bool ok = dec->Decode(const_cast<unsigned char *>(buf), size, off, len, pcm);
    delete dec;
    if (!ok || pcm.empty())
        return 0;
    *out = (short *)malloc(pcm.size() * sizeof(short));
    if (*out == NULL)
        return 0;
    memcpy(*out, pcm.data(), pcm.size() * sizeof(short));
    *count = (unsigned)pcm.size();
    return 1;
}
