#ifndef GUARD_MP_WEATHER_H
#define GUARD_MP_WEATHER_H
// The hourly weather over every region's outdoor maps (src/mp/mp_weather.c)

enum { MP_WX_CLEAR, MP_WX_RAIN, MP_WX_STORM, MP_WX_SUN, MP_WX_HAIL, MP_WX_SAND, MP_WX_COUNT };
enum { MP_WXF_COLD = 1, MP_WXF_DESERT = 2, MP_WXF_NIGHT = 4 };

struct WildPokemon;
void MP_Weather_ServerTime(s64 serverUnix);    // the server's clock (MPMSG_S_WELCOME): everyone's hour alike
u32 MP_Weather_Hour(void);                     // hours since 1970, UTC
u8 MP_Weather_Sky(u8 region, s32 x, s32 y, u32 hour, u8 flags);   // MP_WX_* at a place of a region's grid
u8 MP_Weather_MapSky(u8 group, u8 num, u8 base, u32 hour);         // ... over a map (0xFF: the map keeps its own)
u8 MP_Weather_ForCurrentMap(u8 base);          // the current map's field weather as it loads
bool8 MP_Weather_Ours(void);                   // the field weather is the hour's
bool8 MP_Weather_GentleSun(void);              // ... and it's the bright sun (gentler than GROUDON's)
void MP_Weather_Update(void);                  // each field frame: the hour turning
u8 MP_Weather_Likes(u16 species);              // the weather a POKéMON likes (MP_WX_CLEAR: none)
u8 MP_Weather_WildIndex(const struct WildPokemon *mons, const u8 *rates, u8 count, u8 index, u32 roll);
void MP_Weather_Debug(const char *arg);

#endif
