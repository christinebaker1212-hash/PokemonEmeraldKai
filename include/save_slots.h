// Moemon Co-op: the saves folder and the twelve save files (src/platform/save_slots.c)
#ifndef GUARD_SAVE_SLOTS_H
#define GUARD_SAVE_SLOTS_H

#define SAVE_SLOT_COUNT 12

struct SaveSlotSummary
{
    bool8 exists, known;   // a save is there; its summary was read (a save from before 1.61.5 has none yet)
    bool8 header;          // the name and time read from the save itself
    char name[16];
    u8 region, badges, minutes;
    u16 hours;
};

void SaveSlots_Init(bool8 readOnly);   // before the save is read: the old save moved in, the slot in use picked
int SaveSlots_Current(void);        // 1-SAVE_SLOT_COUNT, 0 with --save / MOEMON_SAVE (no slots)
void SaveSlots_WriteSummary(void);  // with every save
bool8 SaveSlots_Summary(int slot, struct SaveSlotSummary *out);
void SaveSlots_Switch(int slot);    // goes on with that save file in place (main_menu.c reloads the save)
const char *SaveSlots_File(const char *name, const char *shared);   // that save file's settings file NAME (File 1's
                                                                    // with the file's own changes over it)
void SaveSlots_Written(const char *name);   // after writing it: another file than File 1 keeps what it changed
int SaveSlots_Erase(int slot);       // every file of that save file gone (how many)

#endif
