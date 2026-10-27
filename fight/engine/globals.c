/* The global variables, with the initial values of the original's .data / .rdata;
 * addresses are those of the original mugen.exe. */

#include "mugen.h"

const ResamplerVtbl g_resamplerVtbl = { Resampler_Open, Resampler_Create, Resampler_Close, Resampler_InputNeeded, Resampler_State, Resampler_Write, Resampler_Read, Resampler_InFormat, Resampler_OutFormat, Resampler_Reset, Resampler_InBlock, Resampler_OutBlock }; /* 0x004f1890 */
SpriteEncodeFn g_spriteEncoders[13] = { NULL, NULL, (SpriteEncodeFn)Rle8_Encode, (SpriteEncodeFn)Rle5_Encode, (SpriteEncodeFn)Lz5_Encode, NULL, NULL, NULL, NULL, NULL, (SpriteEncodeFn)Png_Encode8, (SpriteEncodeFn)Png_Encode24, (SpriteEncodeFn)Png_Encode32 }; /* 0x004f19e0 */
SpriteDecodeFn g_spriteDecoders[13] = { NULL, NULL, (SpriteDecodeFn)Rle8_Decode, (SpriteDecodeFn)Rle5_Decode, (SpriteDecodeFn)Lz5_Decode, NULL, NULL, NULL, NULL, NULL, (SpriteDecodeFn)Png_Decode, (SpriteDecodeFn)Png_Decode, (SpriteDecodeFn)Png_Decode }; /* 0x004f1a14 */
int g_triggersV10[3] = { 25, 182, -1 }; /* 0x004f1a64 */
int g_triggersV11[10] = { 22, 212, 213, 214, 215, 216, 217, 218, 219, -1 }; /* 0x004f1a70 (Moemon Fight: the end marker at 0x004f1a94, which Trigger_Parse reads) */
int g_sctrlsV10[2] = { 0x8C, -1 }; /* 0x004f1a98 */
int g_sctrlsV11[2] = { 0x97, -1 }; /* 0x004f1aa0 */
TriggerConstInfo g_triggerConsts[88] = { { "data.life", 0, 0 }, { "data.power", 0, 0 }, { "data.attack", 0, 0 }, { "size.attack.z.width.front", 0, 1 }, { "size.attack.z.width.back", 0, 1 }, { "data.airjuggle", 0, 0 }, { "size.attack.dist", 0, 1 }, { "size.proj.attack.dist", 0, 1 }, { "data.defence", 0, 0 }, { "data.fall.defence_mul", 1, 0 }, { "data.liedown.time", 0, 0 }, { "size.xscale", 1, 0 }, { "size.yscale", 1, 0 }, { "size.ground.back", 0, 1 }, { "size.ground.front", 0, 1 }, { "size.air.back", 0, 1 }, { "size.air.front", 0, 1 }, { "size.height", 0, 1 }, { "size.proj.doscale", 0, 0 }, { "size.head.pos.x", 0, 1 }, { "size.head.pos.y", 0, 1 }, { "size.mid.pos.x", 0, 1 }, { "size.mid.pos.y", 0, 1 }, { "size.shadowoffset", 0, 1 }, { "velocity.walk.fwd.x", 1, 1 }, { "velocity.walk.back.x", 1, 1 }, { "velocity.walk.up.x", 1, 1 }, { "velocity.walk.down.x", 1, 1 }, { "velocity.run.fwd.x", 1, 1 }, { "velocity.run.fwd.y", 1, 1 }, { "velocity.run.back.x", 1, 1 }, { "velocity.run.back.y", 1, 1 }, { "velocity.run.down.x", 1, 1 }, { "velocity.run.down.y", 1, 1 }, { "velocity.run.up.x", 1, 1 }, { "velocity.run.up.y", 1, 1 }, { "velocity.jump.y", 1, 1 }, { "velocity.jump.neu.x", 1, 1 }, { "velocity.jump.fwd.x", 1, 1 }, { "velocity.jump.back.x", 1, 1 }, { "velocity.jump.up.x", 1, 1 }, { "velocity.jump.down.x", 1, 1 }, { "velocity.runjump.y", 1, 1 }, { "velocity.runjump.fwd.x", 1, 1 }, { "velocity.runjump.back.x", 1, 1 }, { "velocity.runjump.up.x", 1, 1 }, { "velocity.runjump.down.x", 1, 1 }, { "movement.airjump.num", 0, 0 }, { "movement.airjump.height", 0, 1 }, { "velocity.airjump.y", 1, 1 }, { "velocity.airjump.neu.x", 1, 1 }, { "velocity.airjump.fwd.x", 1, 1 }, { "velocity.airjump.back.x", 1, 1 }, { "velocity.airjump.up.x", 1, 1 }, { "velocity.airjump.down.x", 1, 1 }, { "movement.yaccel", 1, 1 }, { "movement.stand.friction", 1, 0 }, { "movement.crouch.friction", 1, 0 }, { "data.sparkno", 0, 0 }, { "data.guard.sparkno", 0, 0 }, { "data.ko.echo", 0, 0 }, { "size.draw.offset.x", 0, 1 }, { "size.draw.offset.y", 0, 1 }, { "data.intpersistindex", 0, 0 }, { "data.floatpersistindex", 0, 0 }, { "velocity.air.gethit.groundrecover.x", 1, 1 }, { "velocity.air.gethit.groundrecover.y", 1, 1 }, { "velocity.air.gethit.airrecover.mul.x", 1, 0 }, { "velocity.air.gethit.airrecover.mul.y", 1, 0 }, { "velocity.air.gethit.airrecover.add.x", 1, 1 }, { "velocity.air.gethit.airrecover.add.y", 1, 1 }, { "velocity.air.gethit.airrecover.back", 1, 1 }, { "velocity.air.gethit.airrecover.fwd", 1, 1 }, { "velocity.air.gethit.airrecover.up", 1, 1 }, { "velocity.air.gethit.airrecover.down", 1, 1 }, { "movement.stand.friction.threshold", 1, 1 }, { "movement.crouch.friction.threshold", 1, 1 }, { "movement.air.gethit.groundlevel", 1, 1 }, { "movement.air.gethit.groundrecover.ground.threshold", 1, 1 }, { "movement.air.gethit.groundrecover.groundlevel", 1, 1 }, { "movement.air.gethit.airrecover.threshold", 1, 1 }, { "movement.air.gethit.airrecover.yaccel", 1, 1 }, { "movement.air.gethit.trip.groundlevel", 1, 1 }, { "movement.down.bounce.offset.x", 1, 1 }, { "movement.down.bounce.offset.y", 1, 1 }, { "movement.down.bounce.yaccel", 1, 1 }, { "movement.down.bounce.groundlevel", 1, 1 }, { "movement.down.friction.threshold", 1, 1 } }; /* 0x004f1aa8 */
const KeybindName g_keybindNames[105] = { { "esc", 27 }, { "f1", 282 }, { "f2", 283 }, { "f3", 284 }, { "f4", 285 }, { "f5", 286 }, { "f6", 287 }, { "f7", 288 }, { "f8", 289 }, { "f9", 290 }, { "f10", 291 }, { "f11", 292 }, { "f12", 293 }, { "f13", 294 }, { "f14", 295 }, { "f15", 296 }, { "printscreen", 316 }, { "scrolllock", 302 }, { "pause", 19 }, { "tilde", 96 }, { "1", 49 }, { "2", 50 }, { "3", 51 }, { "4", 52 }, { "5", 53 }, { "6", 54 }, { "7", 55 }, { "8", 56 }, { "9", 57 }, { "0", 48 }, { "minus", 45 }, { "plus", 43 }, { "backspace", 8 }, { "tab", 9 }, { "q", 113 }, { "w", 119 }, { "e", 101 }, { "r", 114 }, { "t", 116 }, { "y", 121 }, { "u", 117 }, { "i", 105 }, { "o", 111 }, { "p", 112 }, { "openbrackets", 91 }, { "closebrackets", 93 }, { "backslash", 92 }, { "capslock", 301 }, { "a", 97 }, { "s", 115 }, { "d", 100 }, { "f", 102 }, { "g", 103 }, { "h", 104 }, { "j", 106 }, { "k", 107 }, { "l", 108 }, { "semicolon", 58 }, { "quotes", 39 }, { "enter", 13 }, { "leftshift", 304 }, { "z", 122 }, { "x", 120 }, { "c", 99 }, { "v", 118 }, { "b", 98 }, { "n", 110 }, { "m", 109 }, { "comma", 44 }, { "period", 46 }, { "question", 47 }, { "rightshift", 303 }, { "leftctrl", 306 }, { "leftalt", 308 }, { "space", 32 }, { "rightalt", 307 }, { "rightctrl", 305 }, { "insert", 277 }, { "home", 278 }, { "pageup", 280 }, { "delete", 127 }, { "end", 279 }, { "pagedown", 281 }, { "up", 273 }, { "left", 276 }, { "down", 274 }, { "right", 275 }, { "numlock", 300 }, { "divide", 267 }, { "multiple", 268 }, { "subtract", 269 }, { "numpad7", 263 }, { "numpad8", 264 }, { "numpad9", 265 }, { "add", 270 }, { "numpad4", 260 }, { "numpad5", 261 }, { "numpad6", 262 }, { "numpad1", 257 }, { "numpad2", 258 }, { "numpad3", 259 }, { "numpadenter", 271 }, { "numpad0", 256 }, { "numpadperiod", 266 }, { NULL, 0 } }; /* 0x004f24d0 */
const char * g_glDetectVertexShader = "#version 120\nvoid main()\n{\n  gl_TexCoord[0] = gl_MultiTexCoord0;\n  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n}\n"; /* 0x00500050 */
const char * g_sdlmixErrors[5] = { "OK", "Can't change looping while file is playing", "Can't change volume during fade", "No music file loaded!", "Error attempting to load the file." }; /* 0x0050005c */
float g_glOffsetX = 0.375f; /* 0x00500094 */
float g_glOffsetY = 0.375f; /* 0x00500098 */
float g_glPalOffset = 0.375f; /* 0x005000a0 */
int g_glMaxTextureSize = 64; /* 0x005000a4 */
const char ** g_glShaderDefines[GL_PROGRAM_COUNT] = { (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_NONE", "PALFX_DISABLE", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_NONE", "PALFX_DISABLE_COLORBAL", "PALFX_DISABLE_INVERTALL", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_NONE", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_STANDARD", "PALFX_DISABLE", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_STANDARD", "PALFX_DISABLE_COLORBAL", "PALFX_DISABLE_INVERTALL", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_STANDARD", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_EVEN", "PALFX_DISABLE", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_EVEN", "PALFX_DISABLE_COLORBAL", "PALFX_DISABLE_INVERTALL", NULL }, (const char *[]){ "INDEX_SAFE", "PERSPECTIVE_EVEN", NULL } }; /* 0x00500138 */
float g_clsnScaleX = 1.0f; /* 0x0050015c */
float g_clsnScaleY = 1.0f; /* 0x00500160 */
void * g_luaHook1 = (void *)Stub_ReturnZero; /* 0x00500164 */
void * g_luaHook2 = (void *)Stub_Nop; /* 0x00500168 */
int g_stereoDirection = 1; /* 0x0050016c */
double g_timerMeasuredFps = -1.0; /* 0x00500170 */
int g_keyboardKeepRepeat = 1; /* 0x00500178 */
int g_keyboardNoReleases = 1; /* 0x0050017c */
GfxClearFn g_gfxClear = (GfxClearFn)Gfx_SoftClear; /* 0x00500b8c */
const char * g_fontDefaultDirs[2] = { "", NULL }; /* 0x00500d94 */
GfxClearFn g_gfxClearFns[GFX_RENDERER_COUNT] = { (GfxClearFn)Gfx_SoftClear, (GfxClearFn)Gfx_SoftClear, (GfxClearFn)Gfx_GlClear, (GfxClearFn)Stub_Nop }; /* 0x00500d9c */
GfxFn g_gfxPutPixelFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftPutPixel, (GfxFn)Gfx_SoftPutPixel, (GfxFn)Gfx_GlPutPixel, (GfxFn)Stub_Nop }; /* 0x00500dac */
GfxFn g_gfxPutPixel = (GfxFn)Gfx_SoftPutPixel; /* 0x00500dbc */
GfxFn g_gfxAddPixelFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftAddPixel, (GfxFn)Gfx_SoftAddPixel, (GfxFn)Gfx_GlAddPixel, (GfxFn)Stub_Nop }; /* 0x00500dc0 */
GfxFn g_gfxAddPixel = (GfxFn)Gfx_SoftAddPixel; /* 0x00500dd0 */
GfxFn g_gfxInvertPixelFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftInvertPixel, (GfxFn)Gfx_SoftInvertPixel, (GfxFn)Gfx_GlInvertPixel, (GfxFn)Stub_Nop }; /* 0x00500dd4 */
GfxFn g_gfxInvertPixel = (GfxFn)Gfx_SoftInvertPixel; /* 0x00500de4 */
GfxFn g_gfxDrawLineFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawLine, (GfxFn)Gfx_SoftDrawLine, (GfxFn)Gfx_GlDrawLine, (GfxFn)Stub_Nop }; /* 0x00500de8 */
GfxFn g_gfxDrawLine = (GfxFn)Gfx_SoftDrawLine; /* 0x00500df8 */
GfxFn g_gfxDrawLineAddFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawLineAdd, (GfxFn)Gfx_SoftDrawLineAdd, (GfxFn)Gfx_GlDrawLineAdd, (GfxFn)Stub_Nop }; /* 0x00500dfc */
GfxFn g_gfxDrawLineAdd = (GfxFn)Gfx_SoftDrawLineAdd; /* 0x00500e0c */
GfxFn g_gfxDrawRectFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawRect, (GfxFn)Gfx_SoftDrawRect, (GfxFn)Gfx_GlDrawRect, (GfxFn)Stub_Nop }; /* 0x00500e10 */
GfxFn g_gfxDrawRect = (GfxFn)Gfx_SoftDrawRect; /* 0x00500e20 */
GfxFn g_gfxDrawRectAddFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawRectAdd, (GfxFn)Gfx_SoftDrawRectAdd, (GfxFn)Gfx_GlDrawRectAdd, (GfxFn)Stub_Nop }; /* 0x00500e24 */
GfxFn g_gfxDrawRectAdd = (GfxFn)Gfx_SoftDrawRectAdd; /* 0x00500e34 */
GfxFn g_gfxDrawRectBlendFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawRectBlend, (GfxFn)Gfx_SoftDrawRectBlend, (GfxFn)Gfx_GlDrawRectBlend, (GfxFn)Stub_Nop }; /* 0x00500e38 */
GfxFn g_gfxDrawRectBlend = (GfxFn)Gfx_SoftDrawRectBlend; /* 0x00500e48 */
GfxFn g_gfxDrawRectSubFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftDrawRectSub, (GfxFn)Gfx_SoftDrawRectSub, (GfxFn)Gfx_GlDrawRectSub, (GfxFn)Stub_Nop }; /* 0x00500e4c */
GfxFn g_gfxDrawRectSub = (GfxFn)Gfx_SoftDrawRectSub; /* 0x00500e5c */
GfxRectFn g_gfxFillRectFns[GFX_RENDERER_COUNT] = { (GfxRectFn)Gfx_SoftFillRect, (GfxRectFn)Gfx_SoftFillRect, (GfxRectFn)Gfx_GlFillRect, (GfxRectFn)Stub_Nop }; /* 0x00500e60 */
GfxRectFn g_gfxFillRect = (GfxRectFn)Gfx_SoftFillRect; /* 0x00500e70 */
GfxFillRectFn g_gfxFillRectColorFns[GFX_RENDERER_COUNT] = { (GfxFillRectFn)Gfx_SoftFillRectAdd, (GfxFillRectFn)Gfx_SoftFillRectAdd, (GfxFillRectFn)Gfx_GlFillRectAdd, (GfxFillRectFn)Stub_Nop }; /* 0x00500e74 */
GfxFillRectFn g_gfxFillRectColor = (GfxFillRectFn)Gfx_SoftFillRectAdd; /* 0x00500e84 */
GfxBlendRectFn g_gfxBlendRectFns[GFX_RENDERER_COUNT] = { (GfxBlendRectFn)Gfx_SoftBlendRect, (GfxBlendRectFn)Gfx_SoftBlendRect, (GfxBlendRectFn)Gfx_GlBlendRect, (GfxBlendRectFn)Stub_Nop }; /* 0x00500e88 */
GfxBlendRectFn g_gfxBlendRect = (GfxBlendRectFn)Gfx_SoftBlendRect; /* 0x00500e98 */
GfxFn g_gfxFillRectSubFns[GFX_RENDERER_COUNT] = { (GfxFn)Gfx_SoftFillRectSub, (GfxFn)Gfx_SoftFillRectSub, (GfxFn)Gfx_GlFillRectSub, (GfxFn)Stub_Nop }; /* 0x00500e9c */
GfxFn g_gfxFillRectSub = (GfxFn)Gfx_SoftFillRectSub; /* 0x00500eac */
GfxCaptureFn g_gfxCaptureFns[GFX_RENDERER_COUNT] = { NULL, NULL, (GfxCaptureFn)Gl_Capture, NULL }; /* 0x00500eb0 */
GfxPresentFn g_gfxPresentFns[GFX_RENDERER_COUNT] = { (GfxPresentFn)Gfx_Present, (GfxPresentFn)Gfx_Present, (GfxPresentFn)Gl_Present, NULL }; /* 0x00500ec0 */
LONG g_savedWindowStyle = 0x10000000; /* 0x00500ed4 */
double g_glNear = 1000.0; /* 0x00500ed8 */
double g_glFar = 10000.0; /* 0x00500ee0 */
double g_glZScale = 1.0; /* 0x00500ee8 */
unsigned int g_palFxSerial = 1; /* 0x00500ef0 */
int g_nokoLatch = 1; /* 0x00500ef4 */
const char * g_teamModeNames[4] = { "single", "simul", "tag", "turns" }; /* 0x00500f00 */
const char * g_palKeymapKeys[12] = { "a", "b", "c", "x", "y", "z", "a2", "b2", "c2", "x2", "y2", "z2" }; /* 0x00500f10 */
int g_exprStackTop = -1; /* 0x00500f40 */
SelectInfo * g_selectInfo; /* 0x00503388 */
int g_selectRows; /* 0x0050338c */
int g_selectColumns; /* 0x00503390 */
SelectCell * g_selectCells; /* 0x00503394 */
int * g_screenKeys[2]; /* 0x00503398 */
PalFx g_shadowPalFx; /* 0x005033a0 */
PalFx g_palFxApplied; /* 0x005033c8 */
PalFx g_palFx; /* 0x005033ec */
RECT g_savedWindowRect; /* 0x00503414 */
RECT g_savedClientRect; /* 0x00503424 */
GlTex * g_glScreenTexCur; /* 0x00503434 */
int g_glScreenUploadW; /* 0x00503438 */
int g_glScreenUploadH; /* 0x0050343c */
GlTex g_glScreenTex; /* 0x00503440 */
int g_unused503468; /* 0x00503468 */
int g_sndUnused346C; /* 0x0050346c */
int g_keyboardQueue[INPUT_RING_SIZE]; /* 0x00503470 */
int g_keyboardQueueRead; /* 0x00503870 */
int g_keyboardQueueWrite; /* 0x00503874 */
int g_sndUnusedVolume; /* 0x00503878 */
AudioFormat g_sndFormat; /* 0x00503880 */
int g_padsChanged; /* 0x005038c4 */
int g_padQueue[INPUT_RING_SIZE]; /* 0x005038c8 */
int g_padQueueRead; /* 0x00503cc8 */
int g_padQueueWrite; /* 0x00503ccc */
int g_timerFpsInterval; /* 0x00503cd0 */
int g_timerFpsFrames; /* 0x00503cd4 */
long long g_timerFrameTicks; /* 0x00503cd8 */
long long g_timerStart; /* 0x00503ce0 */
long long g_timerFpsStart; /* 0x00503ce8 */
int g_timerFrameRate; /* 0x00503cf0 */
SdlmixInstance * g_sdlmixInstance; /* 0x00504040 */
BgmHostCallbacks g_sdlmixHost; /* 0x00504044 */
AudioFormat g_sdlmixFormat; /* 0x00504050 */
double g_sdlmixVolumes[6]; /* 0x00504060 */
Ini * g_selectIni; /* 0x00504090 */
Log * g_log; /* 0x00504094 */
void * g_richEditLib; /* 0x00504098 */
int g_screenshotNumber; /* 0x0050409c */
double g_gameCoord[4]; /* 0x005040a0 */
int g_sysElemSerial; /* 0x005040c0 */
int * g_gameflowState; /* 0x005040c4 */
GameVars * g_game; /* 0x005040e8 */
CmdLineArgs * g_cmdline; /* 0x005040ec */
int g_shadowPalFxReady; /* 0x005040f0 */
Console * g_console; /* 0x005040f4 */
int g_titleInfoShown; /* 0x005040f8 */
struct lua_State * g_luaState; /* 0x005040fc */
int g_playerCacheSerial; /* 0x00504100 */
int g_outOfMemory; /* 0x0050410c */
int g_libraryError; /* 0x00504110 */
int g_palConvertCount; /* 0x00504138 */
PalFx * g_palFxCur; /* 0x0050413c */
PalFx g_palFxStatic; /* 0x00504140 */
PalRemap * g_palRemapCur; /* 0x00504168 */
void * g_videoScratch; /* 0x0050416c */
PalRemap g_palRemapStatic; /* 0x00504170 */
int g_videoGameWidth; /* 0x005041f4 */
int g_videoGameHeight; /* 0x005041f8 */
int g_videoGamePixels; /* 0x005041fc */
int g_videoWidth; /* 0x00504200 */
int g_videoHeight; /* 0x00504204 */
int g_clipX1; /* 0x00504208 */
int g_clipX2; /* 0x0050420c */
int g_clipY1; /* 0x00504210 */
int g_clipY2; /* 0x00504214 */
int g_clipW; /* 0x00504218 */
int g_clipH; /* 0x0050421c */
int g_clipRowSkip; /* 0x00504220 */
int g_renderMode; /* 0x00504224 */
int g_videoDepth; /* 0x00504228 */
int g_videoBytesPerPixel; /* 0x0050422c */
int g_unused504230; /* 0x00504230 */
int g_unused504234; /* 0x00504234 */
int g_glProjection; /* 0x00504238 */
int * g_sinTable; /* 0x0050423c */
SDL_Surface * g_screenSurface; /* 0x00504240 */
SDL_Surface * g_unusedSurface1; /* 0x00504244 */
SDL_Surface * g_unusedSurface2; /* 0x00504248 */
int g_unused50424C; /* 0x0050424c */
unsigned int g_videoFlags; /* 0x00504250 */
Uint32 g_videoSdlFlags; /* 0x00504254 */
int g_vretrace; /* 0x00504258 */
ActivateFn g_onActivate; /* 0x0050425c */
int g_videoFullscreen; /* 0x00504260 */
LONG g_savedWindowExStyle; /* 0x00504264 */
unsigned int g_glSpriteBufferFlags; /* 0x00504268 */
int g_streamError; /* 0x0050426c */
int g_gfxFlags; /* 0x00504270 */
int g_bufferedRead; /* 0x00504274 */
int g_iniCaseSensitive; /* 0x00504278 */
char g_iniLang[3]; /* 0x0050427c */
int g_quitRequested; /* 0x00504280 */
int * g_keyMap; /* 0x00504284 */
IntArray g_keyQueue; /* 0x00504288 */
GfxCaptureFn g_gfxCapture; /* 0x00504298 */
GfxPresentFn g_gfxPresent; /* 0x0050429c */
int g_glScreenTexReady; /* 0x005042a0 */
SpriteSet * g_renderTarget; /* 0x005042a4 */
int g_renderTargetSprite; /* 0x005042a8 */
int g_glScreenResized; /* 0x005042ac */
GfxOverlayFn g_gfxPrePresent; /* 0x005042b0 */
Log * g_matchLog; /* 0x005042b4 */
int g_fontWrapMode; /* 0x005042b8 */
int g_fontTtfRender; /* 0x005042bc */
char * g_systemFontDirPtr; /* 0x005042c0 */
int g_fontLineCount; /* 0x005042c4 */
char g_systemFontDir[0x200]; /* 0x005042c8 */
InputHandlerFn g_keyHandler; /* 0x005044c8 */
KeyEventFn g_keyEventHandler; /* 0x005044cc */
int g_keyboardActive; /* 0x005044d0 */
int g_keyModState; /* 0x005044d4 */
int g_unused5044D8; /* 0x005044d8 */
int g_sndBufferedRead; /* 0x005044dc */
int g_sndActive; /* 0x005044e0 */
int g_sndOpen; /* 0x005044e4 */
int g_sndSlotCount; /* 0x005044e8 */
int g_sndMixChannels; /* 0x005044ec */
int g_sndChunkSize; /* 0x005044f0 */
int g_sndMuted; /* 0x005044f4 */
SoundSlot * g_sndSlots; /* 0x005044f8 */
int g_sndResampleMethod; /* 0x005044fc */
Resampler * g_sndResampler; /* 0x00504500 */
int g_padsActive; /* 0x00504504 */
int g_padCount; /* 0x00504508 */
InputHandlerFn g_padHandler; /* 0x0050450c */
char g_storyboardFontDir[0x200]; /* 0x00504510 */
long long g_timerFrameStart; /* 0x00504710 */
long long g_timerFrame; /* 0x00504718 */
int g_timerInitialized; /* 0x00504720 */
int g_timerPerformance; /* 0x00504724 */
int g_timerResyncs; /* 0x00504728 */
GrowBuf * g_pngOwnedBuffer; /* 0x0050472c */
GrowBuf * g_pngBuffer; /* 0x00504730 */
int g_pngBufferPos; /* 0x00504734 */
Stream * g_pngStream; /* 0x00504738 */
DecompBuffer * g_spriteDecompBuffer; /* 0x0050473c */
char g_language[4]; /* 0x00504740 */
char g_systemLanguage[4]; /* 0x00504744 */
unsigned long long g_glTexMemory; /* 0x00504748 */
unsigned long long g_glTexMemoryPeak; /* 0x00504750 */
int g_glInitialized; /* 0x00504758 */
unsigned int g_glCurrentProgram; /* 0x0050475c */
GlProgram g_glProgramsIndexed[GL_PROGRAM_COUNT]; /* 0x00504760 */
GlProgram g_glProgramsRgba[GL_PROGRAM_COUNT]; /* 0x00504910 */
GlTex * g_glSysTex[2]; /* 0x00504ac0 */
unsigned long long g_glStatSpriteTexture; /* 0x00504ac8 */
unsigned long long g_glStatSpriteStream; /* 0x00504ad0 */
unsigned long long g_glStatPalTexture; /* 0x00504ad8 */
unsigned long long g_glStatPalStream; /* 0x00504ae0 */
unsigned long long g_glStatTexCompress; /* 0x00504ae8 */
float g_glRgbaOffsetX; /* 0x00504af0 */
float g_glRgbaOffsetY; /* 0x00504af4 */
int g_glIndexMode; /* 0x00504af8 */
int g_glBoundFramebuffer; /* 0x00504afc */
unsigned int g_glFramebuffer; /* 0x00504b00 */
long long g_timerFrequency; /* 0x00504b28 */
int g_frameRate; /* 0x00504b30 */
int g_timerTicksPerSecond; /* 0x00504b34 */
unsigned char g_padButtonState[INPUT_PAD_COUNT+1]; /* 0x00504b40 */
Pad g_pads[PAD_MAX]; /* 0x00504be0 */
char g_pauseToggled; /* 0x00504c30 */
unsigned char g_keyState[0x201]; /* 0x00504c40 */
unsigned char g_inputState[INPUT_CODE_COUNT]; /* 0x00504e60 */
SDL_Surface * g_videoSurface; /* 0x005050e4 */
int g_targetWidth; /* 0x005050e8 */
int g_targetBytesPerPixel; /* 0x005050ec */
int g_targetHeight; /* 0x005050f0 */
int g_targetPixels; /* 0x005050f4 */
int g_targetBpp; /* 0x005050f8 */
char g_libraryErrorText[0x100]; /* 0x00505100 */
char g_unused505200; /* 0x00505200 */
int g_exprStackType[128]; /* 0x00505300 */
ExprValue g_exprStackValue[128]; /* 0x00505500 */
