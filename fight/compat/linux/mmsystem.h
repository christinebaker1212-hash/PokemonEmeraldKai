/* Moemon Fight: <mmsystem.h> for the Linux test build (the engine's clock is the game's: mf_port.h). */
#ifndef MF_LINUX_MMSYSTEM_H
#define MF_LINUX_MMSYSTEM_H
static inline DWORD timeGetTime(void)
{
    return 0;
}
static inline unsigned int timeBeginPeriod(unsigned int period)
{
    return 0;
}
static inline unsigned int timeEndPeriod(unsigned int period)
{
    return 0;
}
#endif
