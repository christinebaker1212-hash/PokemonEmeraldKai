// Moemon Co-op: the saves folder and the three save files (src/platform/save_slots.c)
#ifndef GUARD_SAVE_SLOTS_H
#define GUARD_SAVE_SLOTS_H

#define SAVE_SLOT_COUNT 3

struct SaveSlotSummary
{
    bool8 exists, known;   // a save is there; its summary was read (a save from before 1.61.5 has none yet)
    bool8 header;          // the name and time read from the save itself
    char name[16];
    u8 region, badges, minutes;
    u16 hours;
};

void SaveSlots_Init(bool8 readOnly);   // before the save is read: the old save moved in, the slot in use picked
int SaveSlots_Current(void);        // 1-3, 0 with --save / MOEMON_SAVE (no slots)
void SaveSlots_WriteSummary(void);  // with every save
bool8 SaveSlots_Summary(int slot, struct SaveSlotSummary *out);
void SaveSlots_Switch(int slot);    // starts the game again on that save file
int SaveSlots_Erase(int slot);       // every file of that save file gone (how many)

#endif
