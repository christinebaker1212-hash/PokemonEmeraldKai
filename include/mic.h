#ifndef GUARD_MIC_H
#define GUARD_MIC_H

// Moemon Kai: the microphone, for CHATOT's CHATTER (src/platform/mic.c; docs/COOP_NOTES.md "CHATTER as it was in
// Platinum"). OPTIONS > CONTROLS > MICROPHONE picks one of the PC's recording devices (LEFT / RIGHT), or A listens to
// all of them and takes the one spoken into, as A on CONTROLLER takes the pad a button is pressed on. A recording is
// Platinum's: one second of signed 8-bit sound at 2000 samples a second (MIC_RECORD_SAMPLES).

#define MIC_RECORD_RATE     2000
#define MIC_RECORD_SAMPLES  2000

int Mic_Count(void);                       // the PC's recording devices (MOEMON_MIC_WAV: one, the test file)
const char *Mic_Name(int i);
const char *Mic_SelectedName(void);        // "" = the PC's default
void Mic_SetSelected(const char *name);    // (moemon_coop.ini microphone =)
void Mic_CycleSelected(int dir);           // DEFAULT, then each device
bool8 Mic_IsAvailable(void);               // a device to record from (the chosen one is plugged in, or the default)

void Mic_StartPick(void);                  // listens to every device; the first spoken into becomes the one
void Mic_StopPick(void);
bool8 Mic_IsPicking(void);
void Mic_Update(void);                     // every frame (mp_main.c): the pick listens, a recording fills

bool8 Mic_StartRecording(void);            // FALSE: no device could be opened
bool8 Mic_RecordingDone(void);             // one second heard
bool8 Mic_RecordingStalled(void);          // the device stopped sending (3 seconds and not done)
void Mic_TakeRecording(s8 *out);           // MIC_RECORD_SAMPLES samples; closes the device
void Mic_StopRecording(void);              // (left early: closes the device, nothing kept)
u8 Mic_Level(void);                        // 0-16: how loud the device is now (while picking or recording)

#endif // GUARD_MIC_H
