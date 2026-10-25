// Moemon Music Studio: Standard MIDI File reader.
#ifndef MIDI_H
#define MIDI_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t time;           // absolute ticks
    uint8_t status;          // 0x80-0xEF channel message, 0xF0/0xF7 sysex, 0xFF meta
    uint8_t a, b;            // data bytes (channel messages)
    uint8_t meta;            // meta type
    uint32_t len;            // meta / sysex length
    const uint8_t *data;     // into Midi.raw
} MEvent;

typedef struct {
    int nev;
    MEvent *ev;
    uint32_t end;            // time of End of Track
    char name[64];
} MTrack;

typedef struct {
    int format, ntracks, division;
    MTrack *tracks;
    uint8_t *raw;
    size_t rawlen;
} Midi;

int midi_parse(Midi *m, const uint8_t *buf, size_t len, char *err, int errlen);
void midi_free(Midi *m);

#endif
