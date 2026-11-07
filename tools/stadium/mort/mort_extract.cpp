// POKeMON STADIUM's announcer from the player's own ROM (Pokemon Stadium (USA) 1.0): the speech archive (an S1
// container at 0x197C1E0: 165, 1 and 657 MORT streams = clips 0-822, 16000 Hz) decoded by SubDrag's MORT decoder.
//   mort_extract ROM OUTDIR            one WAV per clip (OUTDIR/NNN.wav), for listening and transcribing
//   mort_extract ROM -pack OUT.bin     the game's pack: "STAN" u16 version u16 count, then per clip
//                                      u32 offset u32 samples, then 16-bit mono 16000 Hz PCM, little-endian
// The ROM and what comes out of it are never committed or shipped (tools/stadium/mort/README.txt).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "../../../mort/MORTDecoder.h"

static uint32_t U32(const unsigned char* p) { return ((uint32_t)p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static uint16_t U16(const unsigned char* p) { return (uint16_t)((p[0] << 8) | p[1]); }

#define SPEECH_ARCHIVE 0x197C1E0

static void Put32(FILE* f, uint32_t v) { fwrite(&v, 4, 1, f); }
static void Put16(FILE* f, uint16_t v) { fwrite(&v, 2, 1, f); }

static void WriteWav(const char* path, const std::vector<unsigned short>& pcm, int rate)
{
    FILE* f = fopen(path, "wb");
    uint32_t data = (uint32_t)pcm.size() * 2;
    if (!f)
        return;
    fwrite("RIFF", 1, 4, f); Put32(f, 36 + data); fwrite("WAVEfmt ", 1, 8, f);
    Put32(f, 16); Put16(f, 1); Put16(f, 1); Put32(f, rate); Put32(f, rate * 2); Put16(f, 2); Put16(f, 16);
    fwrite("data", 1, 4, f); Put32(f, data);
    fwrite(pcm.data(), 2, pcm.size(), f);   // (the decoder gives the samples in the machine's order)
    fclose(f);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: mort_extract ROM OUTDIR | mort_extract ROM -pack OUT.bin\n");
        return 2;
    }
    FILE* f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<unsigned char> rom(size);
    if (fread(rom.data(), 1, size, f) != (size_t)size) { fclose(f); return 1; }
    fclose(f);
    bool pack = strcmp(argv[2], "-pack") == 0 && argc >= 4;
    if (size < SPEECH_ARCHIVE + 64 || memcmp(rom.data() + SPEECH_ARCHIVE, "S1", 2) != 0
     || U16(rom.data() + SPEECH_ARCHIVE + 2) != 3)
    {
        fprintf(stderr, "mort_extract: no speech archive at %#x (Pokemon Stadium (USA) 1.0 is needed)\n", SPEECH_ARCHIVE);
        return 1;
    }
    const unsigned char* a = rom.data() + SPEECH_ARCHIVE;
    std::vector<std::vector<unsigned short> > clips;
    int bad = 0;
    for (int c = 0; c < 3; c++)
    {
        uint32_t childOff = SPEECH_ARCHIVE + U32(a + 4 + c * 8);
        const unsigned char* ch = rom.data() + childOff;
        int n = U16(ch + 2);
        if (memcmp(ch, "S1", 2) != 0) { fprintf(stderr, "mort_extract: child %d isn't S1\n", c); return 1; }
        for (int i = 0; i < n; i++)
        {
            uint32_t off = childOff + U32(ch + 4 + i * 8), len = U32(ch + 8 + i * 8);
            std::vector<unsigned short> pcm;
            CMORTDecoder dec;
            if (memcmp(rom.data() + off, "MORT", 4) != 0 || !dec.Decode(rom.data(), (int)size, off, len, pcm))
            {
                fprintf(stderr, "mort_extract: clip %zu didn't decode\n", clips.size());
                bad++;
            }
            clips.push_back(pcm);
        }
    }
    if (!pack)
    {
        char path[1024];
        for (size_t i = 0; i < clips.size(); i++)
        {
            snprintf(path, sizeof(path), "%s/%03zu.wav", argv[2], i);
            WriteWav(path, clips[i], 16000);
        }
        printf("mort_extract: %zu clips to %s (%d failed)\n", clips.size(), argv[2], bad);
        return bad ? 1 : 0;
    }
    FILE* o = fopen(argv[3], "wb");
    if (!o) { perror(argv[3]); return 1; }
    uint16_t count = (uint16_t)clips.size();
    fwrite("STAN", 1, 4, o); Put16(o, 1); Put16(o, count);
    uint32_t at = 8 + 8 * count;
    for (size_t i = 0; i < clips.size(); i++)
    {
        Put32(o, at); Put32(o, (uint32_t)clips[i].size());
        at += (uint32_t)clips[i].size() * 2;
    }
    for (size_t i = 0; i < clips.size(); i++)
        fwrite(clips[i].data(), 2, clips[i].size(), o);
    fclose(o);
    printf("mort_extract: %zu clips, %u bytes to %s (%d failed)\n", clips.size(), at, argv[3], bad);
    return bad ? 1 : 0;
}
