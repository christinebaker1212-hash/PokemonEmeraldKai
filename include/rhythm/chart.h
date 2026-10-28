/* Moemon Rhythm: StepMania's simfiles (.sm / .ssc) for the DANCE machine (docs/RHYTHM_DECISIONS.md).
 * Plain C with no game headers, so tools/rhythm can test it on every chart. */
#ifndef GUARD_RHYTHM_CHART_H
#define GUARD_RHYTHM_CHART_H

#include <stdint.h>

#define RH_COLS          4      /* dance-single: LEFT DOWN UP RIGHT */
#define RH_ROWS_PER_BEAT 48     /* StepMania's rows */
#define RH_TEXT          64

enum { RH_BEGINNER, RH_BASIC, RH_DIFFICULT, RH_EXPERT, RH_CHALLENGE, RH_DIFFS };
enum { RH_RADAR_STREAM, RH_RADAR_VOLTAGE, RH_RADAR_AIR, RH_RADAR_FREEZE, RH_RADAR_CHAOS, RH_RADARS };

struct RhBpm
{
    double beat, bpm;
    double time;                /* when it starts (worked out by RhChart_Prepare) */
};

struct RhStop
{
    double beat, seconds;
    double time;                /* when it starts */
};

struct RhChartInfo
{
    uint8_t present;
    uint8_t meter;              /* feet, 1-10 (higher in some packs: shown as they are) */
    float radar[RH_RADARS];
    uint32_t notesAt, notesLen; /* the notes' text in the file */
    uint32_t hash;              /* of the notes as read (records go by this) */
    uint16_t steps, holds;      /* rows with steps, freeze arrows */
};

struct RhSong
{
    char dir[260];              /* the song's folder */
    char file[260];             /* the simfile */
    char group[RH_TEXT];        /* the folder it's in (the wheel's sections) */
    char title[RH_TEXT], subtitle[RH_TEXT], artist[RH_TEXT];
    char titleTranslit[RH_TEXT];
    char music[RH_TEXT], banner[RH_TEXT], background[RH_TEXT];
    double offset;              /* seconds: beat 0 is at -offset in the music */
    double sampleStart, sampleLength;
    double displayBpmLo, displayBpmHi;
    uint8_t displayBpmRandom;   /* "*": the BPM shown spins */
    struct RhBpm *bpms;
    int bpmCount;
    struct RhStop *stops;
    int stopCount;
    struct RhChartInfo charts[RH_DIFFS];
    double lastBeat;            /* the last step's beat in any chart */
};

enum { RH_NOTE_TAP, RH_NOTE_HOLD, RH_NOTE_MINE };

struct RhNote
{
    int32_t row;                /* 48 a beat */
    uint8_t col, type;
    uint8_t quant;              /* 4th 8th 12th 16th 24th 32nd 48th 64th: 0-7 (the arrow's color) */
    int32_t endRow;             /* a freeze arrow's end */
    double beat, time;          /* seconds into the song (not the music: add the offset) */
    double endBeat, endTime;
};

struct RhChart
{
    struct RhNote *notes;       /* by row, then column */
    int count;
    int rows;                   /* rows with taps or freeze heads */
    int holds;
    double lastTime;
};

/* Reads a simfile's header and its charts' places (not the notes). 0: not a song we can play */
int RhSong_Load(struct RhSong *song, const char *path, const char *group);
void RhSong_Free(struct RhSong *song);
/* The notes of one chart (RH_BEGINNER...) */
int RhChart_Load(const struct RhSong *song, int diff, struct RhChart *chart);
void RhChart_Free(struct RhChart *chart);

/* Timing: seconds at a beat and the beat at a time (stops hold the beat) */
double RhSong_TimeAt(const struct RhSong *song, double beat);
double RhSong_BeatAt(const struct RhSong *song, double time);
double RhSong_BpmAt(const struct RhSong *song, double beat);

/* FNV-1a, for the charts' hashes and the records' keys */
uint32_t Rh_Hash(const void *data, uint32_t len, uint32_t h);

#endif
