/* Prototypes and global variables of the whole program; addresses are those
 * of the original mugen.exe. */

#ifndef MUGEN_SYMBOLS_H
#define MUGEN_SYMBOLS_H

/* ---- global variables ---- */
extern BgmSubsystem g_bgmSdlmixSubsystem; /* 0x004f17a0 */
extern BgmSubsystem g_bgmMpg123Subsystem; /* 0x004f1818 */
extern const ResamplerVtbl g_resamplerVtbl; /* 0x004f1890 */
extern SpriteEncodeFn g_spriteEncoders[13]; /* 0x004f19e0 */
extern SpriteDecodeFn g_spriteDecoders[13]; /* 0x004f1a14 */
extern int g_triggersV10[3]; /* 0x004f1a64 */
extern int g_triggersV11[10]; /* 0x004f1a70 */
extern int g_sctrlsV10[2]; /* 0x004f1a98 */
extern int g_sctrlsV11[2]; /* 0x004f1aa0 */
extern TriggerConstInfo g_triggerConsts[88]; /* 0x004f1aa8 */
extern const KeybindName g_keybindNames[105]; /* 0x004f24d0 */
extern const char * g_glDetectVertexShader; /* 0x00500050 */
extern const char * g_sdlmixErrors[5]; /* 0x0050005c */
extern const char * g_mpg123Errors[7]; /* 0x00500070 */
extern float g_glOffsetX; /* 0x00500094 */
extern float g_glOffsetY; /* 0x00500098 */
extern float g_glPalOffset; /* 0x005000a0 */
extern int g_glMaxTextureSize; /* 0x005000a4 */
extern const char ** g_glShaderDefines[GL_PROGRAM_COUNT]; /* 0x00500138 */
extern float g_clsnScaleX; /* 0x0050015c */
extern float g_clsnScaleY; /* 0x00500160 */
extern void * g_luaHook1; /* 0x00500164 */
extern void * g_luaHook2; /* 0x00500168 */
extern int g_stereoDirection; /* 0x0050016c */
extern double g_timerMeasuredFps; /* 0x00500170 */
extern int g_keyboardKeepRepeat; /* 0x00500178 */
extern int g_keyboardNoReleases; /* 0x0050017c */
extern GfxClearFn g_gfxClear; /* 0x00500b8c */
extern const char * g_fontDefaultDirs[2]; /* 0x00500d94 */
extern GfxClearFn g_gfxClearFns[GFX_RENDERER_COUNT]; /* 0x00500d9c */
extern GfxFn g_gfxPutPixelFns[GFX_RENDERER_COUNT]; /* 0x00500dac */
extern GfxFn g_gfxPutPixel; /* 0x00500dbc */
extern GfxFn g_gfxAddPixelFns[GFX_RENDERER_COUNT]; /* 0x00500dc0 */
extern GfxFn g_gfxAddPixel; /* 0x00500dd0 */
extern GfxFn g_gfxInvertPixelFns[GFX_RENDERER_COUNT]; /* 0x00500dd4 */
extern GfxFn g_gfxInvertPixel; /* 0x00500de4 */
extern GfxFn g_gfxDrawLineFns[GFX_RENDERER_COUNT]; /* 0x00500de8 */
extern GfxFn g_gfxDrawLine; /* 0x00500df8 */
extern GfxFn g_gfxDrawLineAddFns[GFX_RENDERER_COUNT]; /* 0x00500dfc */
extern GfxFn g_gfxDrawLineAdd; /* 0x00500e0c */
extern GfxFn g_gfxDrawRectFns[GFX_RENDERER_COUNT]; /* 0x00500e10 */
extern GfxFn g_gfxDrawRect; /* 0x00500e20 */
extern GfxFn g_gfxDrawRectAddFns[GFX_RENDERER_COUNT]; /* 0x00500e24 */
extern GfxFn g_gfxDrawRectAdd; /* 0x00500e34 */
extern GfxFn g_gfxDrawRectBlendFns[GFX_RENDERER_COUNT]; /* 0x00500e38 */
extern GfxFn g_gfxDrawRectBlend; /* 0x00500e48 */
extern GfxFn g_gfxDrawRectSubFns[GFX_RENDERER_COUNT]; /* 0x00500e4c */
extern GfxFn g_gfxDrawRectSub; /* 0x00500e5c */
extern GfxRectFn g_gfxFillRectFns[GFX_RENDERER_COUNT]; /* 0x00500e60 */
extern GfxRectFn g_gfxFillRect; /* 0x00500e70 */
extern GfxFillRectFn g_gfxFillRectColorFns[GFX_RENDERER_COUNT]; /* 0x00500e74 */
extern GfxFillRectFn g_gfxFillRectColor; /* 0x00500e84 */
extern GfxBlendRectFn g_gfxBlendRectFns[GFX_RENDERER_COUNT]; /* 0x00500e88 */
extern GfxBlendRectFn g_gfxBlendRect; /* 0x00500e98 */
extern GfxFn g_gfxFillRectSubFns[GFX_RENDERER_COUNT]; /* 0x00500e9c */
extern GfxFn g_gfxFillRectSub; /* 0x00500eac */
extern GfxCaptureFn g_gfxCaptureFns[GFX_RENDERER_COUNT]; /* 0x00500eb0 */
extern GfxPresentFn g_gfxPresentFns[GFX_RENDERER_COUNT]; /* 0x00500ec0 */
extern LONG g_savedWindowStyle; /* 0x00500ed4 */
extern double g_glNear; /* 0x00500ed8 */
extern double g_glFar; /* 0x00500ee0 */
extern double g_glZScale; /* 0x00500ee8 */
extern unsigned int g_palFxSerial; /* 0x00500ef0 */
extern int g_nokoLatch; /* 0x00500ef4 */
extern const char * g_teamModeNames[4]; /* 0x00500f00 */
extern const char * g_palKeymapKeys[12]; /* 0x00500f10 */
extern int g_exprStackTop; /* 0x00500f40 */
extern GlEnumFn __glewActiveTexture; /* 0x00500f70 */
#define g_glewActiveTexture __glewActiveTexture
extern GlMultiTexCoord2fFn __glewMultiTexCoord2f; /* 0x00500fcc */
#define g_glewMultiTexCoord2f __glewMultiTexCoord2f
extern GlMultiTexCoord4fFn __glewMultiTexCoord4f; /* 0x0050100c */
#define g_glewMultiTexCoord4f __glewMultiTexCoord4f
extern GlBlendColorFn __glewBlendColor; /* 0x00501028 */
#define g_glewBlendColor __glewBlendColor
extern GlEnumFn __glewBlendEquation; /* 0x0050102c */
#define g_glewBlendEquation __glewBlendEquation
extern GlAttachShaderFn __glewAttachShader; /* 0x00501130 */
#define g_glewAttachShader __glewAttachShader
extern GlUintFn __glewCompileShader; /* 0x0050113c */
#define g_glewCompileShader __glewCompileShader
extern GlCreateProgramFn __glewCreateProgram; /* 0x00501140 */
#define g_glewCreateProgram __glewCreateProgram
extern GlCreateShaderFn __glewCreateShader; /* 0x00501144 */
#define g_glewCreateShader __glewCreateShader
extern GlUintFn __glewDeleteProgram; /* 0x00501148 */
#define g_glewDeleteProgram __glewDeleteProgram
extern GlUintFn __glewDeleteShader; /* 0x0050114c */
#define g_glewDeleteShader __glewDeleteShader
extern GlGetInfoLogFn __glewGetProgramInfoLog; /* 0x00501170 */
#define g_glewGetProgramInfoLog __glewGetProgramInfoLog
extern GlGetObjectivFn __glewGetProgramiv; /* 0x00501174 */
#define g_glewGetProgramiv __glewGetProgramiv
extern GlGetInfoLogFn __glewGetShaderInfoLog; /* 0x00501178 */
#define g_glewGetShaderInfoLog __glewGetShaderInfoLog
extern GlGetObjectivFn __glewGetShaderiv; /* 0x00501180 */
#define g_glewGetShaderiv __glewGetShaderiv
extern GlGetUniformLocationFn __glewGetUniformLocation; /* 0x00501184 */
#define g_glewGetUniformLocation __glewGetUniformLocation
extern GlIsShaderFn __glewIsShader; /* 0x005011a4 */
#define g_glewIsShader __glewIsShader
extern GlUintFn __glewLinkProgram; /* 0x005011a8 */
#define g_glewLinkProgram __glewLinkProgram
extern GlShaderSourceFn __glewShaderSource; /* 0x005011ac */
#define g_glewShaderSource __glewShaderSource
extern GlUniform1fFn __glewUniform1f; /* 0x005011bc */
#define g_glewUniform1f __glewUniform1f
extern GlUniform1iFn __glewUniform1i; /* 0x005011c4 */
#define g_glewUniform1i __glewUniform1i
extern GlUniform2fFn __glewUniform2f; /* 0x005011cc */
#define g_glewUniform2f __glewUniform2f
extern GlUniform3fFn __glewUniform3f; /* 0x005011dc */
#define g_glewUniform3f __glewUniform3f
extern GlUintFn __glewUseProgram; /* 0x00501208 */
#define g_glewUseProgram __glewUseProgram
extern GlBindFramebufferFn __glewBindFramebufferEXT; /* 0x0050204c */
#define g_glewBindFramebufferEXT __glewBindFramebufferEXT
extern GlBindFramebufferFn __glewBindRenderbufferEXT; /* 0x00502050 */
#define g_glewBindRenderbufferEXT __glewBindRenderbufferEXT
extern GlCheckFramebufferStatusFn __glewCheckFramebufferStatusEXT; /* 0x00502054 */
#define g_glewCheckFramebufferStatusEXT __glewCheckFramebufferStatusEXT
extern GlGenDeleteFn __glewDeleteRenderbuffersEXT; /* 0x0050205c */
#define g_glewDeleteRenderbuffersEXT __glewDeleteRenderbuffersEXT
extern GlFramebufferRenderbufferFn __glewFramebufferRenderbufferEXT; /* 0x00502060 */
#define g_glewFramebufferRenderbufferEXT __glewFramebufferRenderbufferEXT
extern GlFramebufferTexture2DFn __glewFramebufferTexture2DEXT; /* 0x00502068 */
#define g_glewFramebufferTexture2DEXT __glewFramebufferTexture2DEXT
extern GlGenDeleteFn __glewGenFramebuffersEXT; /* 0x00502070 */
#define g_glewGenFramebuffersEXT __glewGenFramebuffersEXT
extern GlGenDeleteFn __glewGenRenderbuffersEXT; /* 0x00502074 */
#define g_glewGenRenderbuffersEXT __glewGenRenderbuffersEXT
extern GlRenderbufferStorageFn __glewRenderbufferStorageEXT; /* 0x0050208c */
#define g_glewRenderbufferStorageEXT __glewRenderbufferStorageEXT
extern unsigned char __GLEW_VERSION_2_0; /* 0x00502ac1 */
#define g_glewVersion20 __GLEW_VERSION_2_0
extern unsigned char __GLEW_ARB_fragment_program; /* 0x00502af1 */
#define g_glewArbFragmentProgram __GLEW_ARB_fragment_program
extern unsigned char __GLEW_ARB_texture_non_power_of_two; /* 0x00502b24 */
#define g_glewArbTextureNpot __GLEW_ARB_texture_non_power_of_two
extern unsigned char __GLEW_EXT_blend_logic_op; /* 0x00502b54 */
#define g_glewExtBlendLogicOp __GLEW_EXT_blend_logic_op
extern unsigned char __GLEW_EXT_framebuffer_object; /* 0x00502b68 */
#define g_glewExtFramebufferObject __GLEW_EXT_framebuffer_object
extern SelectInfo * g_selectInfo; /* 0x00503388 */
extern int g_selectRows; /* 0x0050338c */
extern int g_selectColumns; /* 0x00503390 */
extern SelectCell * g_selectCells; /* 0x00503394 */
extern int * g_screenKeys[2]; /* 0x00503398 */
extern PalFx g_shadowPalFx; /* 0x005033a0 */
extern PalFx g_palFxApplied; /* 0x005033c8 */
extern PalFx g_palFx; /* 0x005033ec */
extern RECT g_savedWindowRect; /* 0x00503414 */
extern RECT g_savedClientRect; /* 0x00503424 */
extern GlTex * g_glScreenTexCur; /* 0x00503434 */
extern int g_glScreenUploadW; /* 0x00503438 */
extern int g_glScreenUploadH; /* 0x0050343c */
extern GlTex g_glScreenTex; /* 0x00503440 */
extern int g_unused503468; /* 0x00503468 */
extern int g_sndUnused346C; /* 0x0050346c */
extern int g_keyboardQueue[INPUT_RING_SIZE]; /* 0x00503470 */
extern int g_keyboardQueueRead; /* 0x00503870 */
extern int g_keyboardQueueWrite; /* 0x00503874 */
extern int g_sndUnusedVolume; /* 0x00503878 */
extern BgmHandleFn g_bgmGetHandle; /* 0x0050387c */
extern AudioFormat g_sndFormat; /* 0x00503880 */
extern int g_bgmInitialized; /* 0x00503890 */
extern int g_bgmActive; /* 0x00503894 */
extern double g_bgmVolume; /* 0x00503898 */
extern AudioFormat g_bgmFormat; /* 0x005038a0 */
extern int g_bgmChunk; /* 0x005038b0 */
extern int g_bgmBackgroundMode; /* 0x005038b4 */
extern Resampler * g_bgmResampler; /* 0x005038b8 */
extern DynArray * g_bgmSubsystems; /* 0x005038bc */
extern DynArray * g_bgmExtensions; /* 0x005038c0 */
extern int g_padsChanged; /* 0x005038c4 */
extern int g_padQueue[INPUT_RING_SIZE]; /* 0x005038c8 */
extern int g_padQueueRead; /* 0x00503cc8 */
extern int g_padQueueWrite; /* 0x00503ccc */
extern int g_timerFpsInterval; /* 0x00503cd0 */
extern int g_timerFpsFrames; /* 0x00503cd4 */
extern long long g_timerFrameTicks; /* 0x00503cd8 */
extern long long g_timerStart; /* 0x00503ce0 */
extern long long g_timerFpsStart; /* 0x00503ce8 */
extern int g_timerFrameRate; /* 0x00503cf0 */
extern int g_bgmQueueCount; /* 0x00503cf4 */
extern unsigned int g_bgmChunkBytes; /* 0x00503cf8 */
extern int g_bgmPlayedSeq; /* 0x00503cfc */
extern BgmCommand g_bgmQueue[BGM_QUEUE_SIZE]; /* 0x00503d00 */
extern pthread_mutex_t g_bgmMutex; /* 0x00503e80 (Moemon Fight: the real type) */
extern int g_bgmMuted; /* 0x00503e84 */
extern BgmThreadId g_bgmThread; /* 0x00503e88 */
extern int g_bgmThreadRunning; /* 0x00503e90 */
extern BgmStream * g_bgmStream; /* 0x00503e94 */
extern AudioFormat g_bgmThreadFormat; /* 0x00503e98 */
extern void * g_bgmScratch; /* 0x00503ea8 */
extern int g_bgmFillSeq; /* 0x00503eac */
extern int g_bgmBackgroundChanged; /* 0x00503eb0 */
extern BgmRing * g_bgmRing; /* 0x00503eb4 */
extern unsigned int g_bgmPauseFlags; /* 0x00503eb8 */
extern int g_bgmEventCount; /* 0x00503ebc */
extern BgmEvent g_bgmEvents[BGM_EVENT_QUEUE_SIZE]; /* 0x00503ec0 */
extern int g_bgmHooked; /* 0x00503f80 */
extern Mpg123InfoFn g_mpg123_info; /* 0x00503f84 */
extern Mpg123HandleIntFn g_mpg123_open_feed; /* 0x00503f88 */
extern void * g_mpg123_current_decoder; /* 0x00503f8c */
extern int g_mpg123Initialized; /* 0x00503f90 */
extern void * g_mpg123_getvolume; /* 0x00503f94 */
extern Mpg123FormatFn g_mpg123_format; /* 0x00503f98 */
extern Mpg123HandleIntFn g_mpg123_format_none; /* 0x00503f9c */
extern Mpg123ParamFn g_mpg123_param; /* 0x00503fa0 */
extern HMODULE g_mpg123Lib; /* 0x00503fa4 */
extern Mpg123ReadFn g_mpg123_read; /* 0x00503fa8 */
extern Mpg123VolumeFn g_mpg123_volume; /* 0x00503fac */
extern Mpg123NewFn g_mpg123_new; /* 0x00503fb0 */
extern int g_mpg123Ntom; /* 0x00503fb4 */
extern Mpg123PlainStrErrorFn g_mpg123_plain_strerror; /* 0x00503fb8 */
extern Mpg123HandleFn g_mpg123_delete; /* 0x00503fbc */
extern void * g_mpg123_getparam; /* 0x00503fc0 */
extern Mpg123FeedSeekFn g_mpg123_feedseek; /* 0x00503fc4 */
extern unsigned int g_mpg123ReadBufferSize; /* 0x00503fc8 */
extern Mpg123HandleIntFn g_mpg123_close; /* 0x00503fcc */
extern double g_mpg123Volume; /* 0x00503fd0 */
extern Mpg123InitFn g_mpg123_init; /* 0x00503fd8 */
extern Mpg123FeedFn g_mpg123_feed; /* 0x00503fdc */
extern Mpg123GetFormatFn g_mpg123_getformat; /* 0x00503fe0 */
extern Mpg123StrErrorFn g_mpg123_strerror; /* 0x00503fe4 */
extern AudioFormat g_mpg123Format; /* 0x00503fe8 */
extern Mpg123ExitFn g_mpg123_exit; /* 0x00503ff8 */
extern void * g_mpg123_getstate; /* 0x00503ffc */
extern char g_mpg123Decoder[0x40]; /* 0x00504000 */
extern SdlmixInstance * g_sdlmixInstance; /* 0x00504040 */
extern BgmHostCallbacks g_sdlmixHost; /* 0x00504044 */
extern AudioFormat g_sdlmixFormat; /* 0x00504050 */
extern double g_sdlmixVolumes[6]; /* 0x00504060 */
extern Ini * g_selectIni; /* 0x00504090 */
extern Log * g_log; /* 0x00504094 */
extern void * g_richEditLib; /* 0x00504098 */
extern int g_screenshotNumber; /* 0x0050409c */
extern double g_gameCoord[4]; /* 0x005040a0 */
extern int g_sysElemSerial; /* 0x005040c0 */
extern int * g_gameflowState; /* 0x005040c4 */
extern GameVars * g_game; /* 0x005040e8 */
extern CmdLineArgs * g_cmdline; /* 0x005040ec */
extern int g_shadowPalFxReady; /* 0x005040f0 */
extern Console * g_console; /* 0x005040f4 */
extern int g_titleInfoShown; /* 0x005040f8 */
extern struct lua_State * g_luaState; /* 0x005040fc */
extern int g_playerCacheSerial; /* 0x00504100 */
extern int g_outOfMemory; /* 0x0050410c */
extern int g_libraryError; /* 0x00504110 */
extern int g_palConvertCount; /* 0x00504138 */
extern PalFx * g_palFxCur; /* 0x0050413c */
extern PalFx g_palFxStatic; /* 0x00504140 */
extern PalRemap * g_palRemapCur; /* 0x00504168 */
extern void * g_videoScratch; /* 0x0050416c */
extern PalRemap g_palRemapStatic; /* 0x00504170 */
extern int g_videoGameWidth; /* 0x005041f4 */
extern int g_videoGameHeight; /* 0x005041f8 */
extern int g_videoGamePixels; /* 0x005041fc */
extern int g_videoWidth; /* 0x00504200 */
extern int g_videoHeight; /* 0x00504204 */
extern int g_clipX1; /* 0x00504208 */
extern int g_clipX2; /* 0x0050420c */
extern int g_clipY1; /* 0x00504210 */
extern int g_clipY2; /* 0x00504214 */
extern int g_clipW; /* 0x00504218 */
extern int g_clipH; /* 0x0050421c */
extern int g_clipRowSkip; /* 0x00504220 */
extern int g_renderMode; /* 0x00504224 */
extern int g_videoDepth; /* 0x00504228 */
extern int g_videoBytesPerPixel; /* 0x0050422c */
extern int g_unused504230; /* 0x00504230 */
extern int g_unused504234; /* 0x00504234 */
extern int g_glProjection; /* 0x00504238 */
extern int * g_sinTable; /* 0x0050423c */
extern SDL_Surface * g_screenSurface; /* 0x00504240 */
extern SDL_Surface * g_unusedSurface1; /* 0x00504244 */
extern SDL_Surface * g_unusedSurface2; /* 0x00504248 */
extern int g_unused50424C; /* 0x0050424c */
extern unsigned int g_videoFlags; /* 0x00504250 */
extern Uint32 g_videoSdlFlags; /* 0x00504254 */
extern int g_vretrace; /* 0x00504258 */
extern ActivateFn g_onActivate; /* 0x0050425c */
extern int g_videoFullscreen; /* 0x00504260 */
extern LONG g_savedWindowExStyle; /* 0x00504264 */
extern unsigned int g_glSpriteBufferFlags; /* 0x00504268 */
extern int g_streamError; /* 0x0050426c */
extern int g_gfxFlags; /* 0x00504270 */
extern int g_bufferedRead; /* 0x00504274 */
extern int g_iniCaseSensitive; /* 0x00504278 */
extern char g_iniLang[3]; /* 0x0050427c */
extern int g_quitRequested; /* 0x00504280 */
extern int * g_keyMap; /* 0x00504284 */
extern IntArray g_keyQueue; /* 0x00504288 */
extern GfxCaptureFn g_gfxCapture; /* 0x00504298 */
extern GfxPresentFn g_gfxPresent; /* 0x0050429c */
extern int g_glScreenTexReady; /* 0x005042a0 */
extern SpriteSet * g_renderTarget; /* 0x005042a4 */
extern int g_renderTargetSprite; /* 0x005042a8 */
extern int g_glScreenResized; /* 0x005042ac */
extern GfxOverlayFn g_gfxPrePresent; /* 0x005042b0 */
extern Log * g_matchLog; /* 0x005042b4 */
extern int g_fontWrapMode; /* 0x005042b8 */
extern int g_fontTtfRender; /* 0x005042bc */
extern char * g_systemFontDirPtr; /* 0x005042c0 */
extern int g_fontLineCount; /* 0x005042c4 */
extern char g_systemFontDir[0x200]; /* 0x005042c8 */
extern InputHandlerFn g_keyHandler; /* 0x005044c8 */
extern KeyEventFn g_keyEventHandler; /* 0x005044cc */
extern int g_keyboardActive; /* 0x005044d0 */
extern int g_keyModState; /* 0x005044d4 */
extern int g_unused5044D8; /* 0x005044d8 */
extern int g_sndBufferedRead; /* 0x005044dc */
extern int g_sndActive; /* 0x005044e0 */
extern int g_sndOpen; /* 0x005044e4 */
extern int g_sndSlotCount; /* 0x005044e8 */
extern int g_sndMixChannels; /* 0x005044ec */
extern int g_sndChunkSize; /* 0x005044f0 */
extern int g_sndMuted; /* 0x005044f4 */
extern SoundSlot * g_sndSlots; /* 0x005044f8 */
extern int g_sndResampleMethod; /* 0x005044fc */
extern Resampler * g_sndResampler; /* 0x00504500 */
extern int g_padsActive; /* 0x00504504 */
extern int g_padCount; /* 0x00504508 */
extern InputHandlerFn g_padHandler; /* 0x0050450c */
extern char g_storyboardFontDir[0x200]; /* 0x00504510 */
extern long long g_timerFrameStart; /* 0x00504710 */
extern long long g_timerFrame; /* 0x00504718 */
extern int g_timerInitialized; /* 0x00504720 */
extern int g_timerPerformance; /* 0x00504724 */
extern int g_timerResyncs; /* 0x00504728 */
extern GrowBuf * g_pngOwnedBuffer; /* 0x0050472c */
extern GrowBuf * g_pngBuffer; /* 0x00504730 */
extern int g_pngBufferPos; /* 0x00504734 */
extern Stream * g_pngStream; /* 0x00504738 */
extern DecompBuffer * g_spriteDecompBuffer; /* 0x0050473c */
extern char g_language[4]; /* 0x00504740 */
extern char g_systemLanguage[4]; /* 0x00504744 */
extern unsigned long long g_glTexMemory; /* 0x00504748 */
extern unsigned long long g_glTexMemoryPeak; /* 0x00504750 */
extern int g_glInitialized; /* 0x00504758 */
extern unsigned int g_glCurrentProgram; /* 0x0050475c */
extern GlProgram g_glProgramsIndexed[GL_PROGRAM_COUNT]; /* 0x00504760 */
extern GlProgram g_glProgramsRgba[GL_PROGRAM_COUNT]; /* 0x00504910 */
extern GlTex * g_glSysTex[2]; /* 0x00504ac0 */
extern unsigned long long g_glStatSpriteTexture; /* 0x00504ac8 */
extern unsigned long long g_glStatSpriteStream; /* 0x00504ad0 */
extern unsigned long long g_glStatPalTexture; /* 0x00504ad8 */
extern unsigned long long g_glStatPalStream; /* 0x00504ae0 */
extern unsigned long long g_glStatTexCompress; /* 0x00504ae8 */
extern float g_glRgbaOffsetX; /* 0x00504af0 */
extern float g_glRgbaOffsetY; /* 0x00504af4 */
extern int g_glIndexMode; /* 0x00504af8 */
extern int g_glBoundFramebuffer; /* 0x00504afc */
extern unsigned int g_glFramebuffer; /* 0x00504b00 */
extern int g_mpg123Rva; /* 0x00504b04 */
extern long long g_timerFrequency; /* 0x00504b28 */
extern int g_frameRate; /* 0x00504b30 */
extern int g_timerTicksPerSecond; /* 0x00504b34 */
extern unsigned char g_padButtonState[INPUT_PAD_COUNT+1]; /* 0x00504b40 */
extern Pad g_pads[PAD_MAX]; /* 0x00504be0 */
extern char g_pauseToggled; /* 0x00504c30 */
extern unsigned char g_keyState[0x201]; /* 0x00504c40 */
extern unsigned char g_inputState[INPUT_CODE_COUNT]; /* 0x00504e60 */
extern SDL_Surface * g_videoSurface; /* 0x005050e4 */
extern int g_targetWidth; /* 0x005050e8 */
extern int g_targetBytesPerPixel; /* 0x005050ec */
extern int g_targetHeight; /* 0x005050f0 */
extern int g_targetPixels; /* 0x005050f4 */
extern int g_targetBpp; /* 0x005050f8 */
extern char g_libraryErrorText[0x100]; /* 0x00505100 */
extern char g_unused505200; /* 0x00505200 */
extern int g_exprStackType[128]; /* 0x00505300 */
extern ExprValue g_exprStackValue[128]; /* 0x00505500 */

/* ---- functions ---- */
/* 0x00401000 */ int __cdecl AfterImage_Init(AfterImage *ai);
/* 0x004010e0 */ int __cdecl AfterImage_Update(const DrawParams *dp, AfterImage *ai, const PalRemap *remap, int active);
/* 0x004012c0 */ void __cdecl AfterImage_Draw(AfterImage *ai, int layer);
/* 0x00401690 */ int __cdecl Expr_IntPow(int *out, int base, int exp);
/* 0x004016f0 */ int __cdecl Expr_IsRightAssoc(int op);
/* 0x00401720 */ int __cdecl Expr_Precedence(int op);
/* 0x004017f0 */ int __cdecl Expr_OpAcceptsMode(int op, int mode);
/* 0x00401840 */ int __cdecl Expr_ReadOp(const char *s);
/* 0x004019d0 */ int __cdecl Expr_Parse(const char *s, ExprValue *out_tokens, char *out_types, CharData *owner, int max, const char **end, int want);
/* 0x004023d0 */ int __cdecl Expr_Compile(const char *s, Expr *e, CharData *owner, const char **end, int want);
/* 0x004024f0 */ void __cdecl Expr_Free(Expr *e);
/* 0x00402550 */ void __cdecl ExprList_Free(KeyedArray *list);
/* 0x004025d0 */ void __cdecl Expr_SetInt(Expr *e, int value);
/* 0x00402600 */ void __cdecl Expr_SetFloat(Expr *e, float value);
/* 0x00402630 */ int __cdecl Expr_Copy(Expr *dst, const Expr *src);
/* 0x004026e0 */ int __cdecl Expr_Clone(Expr *dst, const Expr *src);
/* 0x00402770 */ void __cdecl Expr_MulFloat(Expr *e, float k);
/* 0x00402810 */ void __cdecl Expr_Halve(Expr *e);
/* 0x004028c0 */ int __cdecl Expr_EvalInt(Player *p, const Expr *e, int warn);
/* 0x004029a0 */ float __cdecl Expr_EvalFloat(Player *p, const Expr *e);
/* 0x00402a20 */ int __cdecl Expr_Eval(Player *p, const Expr *e, int *iout, float *fout);
/* 0x004036a0 */ int __cdecl Select_OpenDef(const char *path);
/* 0x00403760 */ int __cdecl Select_Init(void);
/* 0x00403820 */ int __cdecl Select_InitCharInfo(void);
/* 0x004038d0 */ int __cdecl Select_LoadDef(void);
/* 0x00405d20 */ int __cdecl Charsel_Init(ScreenLoop *screen, CharselState *st);
/* 0x00405dd0 */ int __cdecl Charsel_End(ScreenLoop *screen, CharselState *st);
/* 0x00406020 */ int __cdecl Charsel_Update(ScreenLoop *screen, CharselState *st);
/* 0x00406db0 */ int __cdecl Charsel_Tick(ScreenLoop *screen, CharselState *st);
/* 0x00406e10 */ int __cdecl Charsel_Draw(ScreenLoop *screen, CharselState *st);
/* 0x004089e0 */ int __cdecl Charsel_Setup(ScreenLoop *screen);
/* 0x00408a80 */ int __cdecl Charsel_Run(void);
/* 0x00408b20 */ void __cdecl Charsel_SetupMode(CharselState *st, int mode, int side);
/* 0x00409040 */ void __cdecl Charsel_ResetBoxes(int *phase);
/* 0x00409090 */ void __cdecl Charsel_FlashCell(int *phase, int col, int row);
/* 0x004090e0 */ int __cdecl Charsel_MoveCursor(unsigned int input, int input_side, int *col, int *row, int side);
/* 0x004093c0 */ int __cdecl StageSelect_Move(int *stage, unsigned int input, int input_side, int side);
/* 0x00409430 */ int __cdecl TeamMenu_Move(TeamMenu *menu, unsigned int input, int input_side, int dir);
/* 0x00409500 */ int __cdecl Select_LoadLayout(Ini *ini);
/* 0x0040a2b0 */ void __cdecl Arcade_BuildOrder(void);
/* 0x0040a370 */ void __cdecl Arcade_ShuffleOrder(ArcadeOrder *order, int team_size, const int *max_matches);
/* 0x0040a620 */ int __cdecl Arcade_NextTeam(void);
/* 0x0040a780 */ int __cdecl Arcade_CountNext(void);
/* 0x0040a7f0 */ int __cdecl Arcade_PeekNext(int *out, int skip_current);
/* 0x0040a8a0 */ void __cdecl Team_Randomize(TeamSel *team, int side, int keep);
/* 0x0040aae0 */ int __cdecl Select_FindCell(int char_index, int *col, int *row);
/* 0x0040ab30 */ CharselStep *__cdecl CharselScript_Next(CharselScript *sc);
/* 0x0040ab70 */ void __cdecl Charsel_StartChars(int side, CharselState *st, const CharselStep *step);
/* 0x0040ad00 */ void __cdecl TeamSel_Fill(int count, TeamSel *team, int side, int mode, int per_member);
/* 0x0040ad80 */ void __cdecl TeamMenu_Init(int side, CharselState *st, const CharselStep *step);
/* 0x0040aea0 */ int __cdecl PalReserve_Request(int char_index, int pal);
/* 0x0040af90 */ void __cdecl PalReserve_Release(int char_index, int pal);
/* 0x0040b010 */ void __cdecl PalReserve_ResetAll(void);
/* 0x0040b070 */ void __cdecl CmdLine_Parse(int argc, char **argv);
/* 0x0040b960 */ __declspec(noreturn) void __cdecl CmdLine_Help(void);
/* 0x0040b9d0 */ void __cdecl Video_HandleFullscreenKey(int *key);
/* 0x0040baa0 */ void __cdecl Player_Warn(Player *p, const char *msg);
/* 0x0040bae0 */ void __cdecl Player_Warnf(Player *p, const char *fmt, ...);
/* 0x0040bb50 */ void __cdecl Player_WarnAt(Player *p, const StateCtrl *sc, const char *fmt, ...);
/* 0x0040bc20 */ void __cdecl Player_WarnRedirected(Player *p, Player *from, const char *msg);
/* 0x0040bc70 */ void __cdecl Clipboard_Printf(const char *fmt, ...);
/* 0x0040bcc0 */ void __cdecl Clipboard_Draw(TextRing *ring, int max_lines, Font *font, int y);
/* 0x0040bd90 */ __declspec(noreturn) void __cdecl Error_Exit(void);
/* 0x0040c0f0 */ __declspec(noreturn) void __cdecl Error_Fatal(const char *fmt, ...);
/* 0x0040c140 */ void __cdecl Error_Printf(const char *fmt, ...);
/* 0x0040c190 */ void __cdecl Error_OutOfMemory(void);
/* 0x0040c1d0 */ int __cdecl File_Exists(const char *path);
/* 0x0040c200 */ int __cdecl File_Find(const char *path, char *out, char *dir_out, const char *base_dir);
/* 0x0040c360 */ int __cdecl Ini_ReadString(Ini *ini, const char *key, unsigned int size, char *out);
/* 0x0040c3f0 */ int __cdecl Log_Init(void);
/* 0x0040c4a0 */ void __cdecl Log_Printf(const char *fmt, ...);
/* 0x0040c770 */ int __cdecl Error_ShowDialog(const char *text, const char *title);
/* 0x0040c7d0 */ int __cdecl Game_LoopInit(ScreenLoop *screen);
/* 0x0040c8e0 */ int __cdecl Game_LoopDeinit(ScreenLoop *screen);
/* 0x0040c950 */ int __cdecl Game_ReadInput(ScreenLoop *screen);
/* 0x0040ca20 */ int __cdecl Game_DebugKeys(ScreenLoop *screen);
/* 0x0040cbf0 */ int __cdecl Game_Tick(ScreenLoop *screen);
/* 0x0040d1c0 */ int __cdecl Game_Draw(ScreenLoop *screen);
/* 0x0040dec0 */ void __cdecl Game_PrePresent(SpriteSet *set, int sprite);
/* 0x0040e080 */ void __cdecl Game_DrawFps(void);
/* 0x0040e110 */ int __cdecl Screen_Init(ScreenLoop *screen);
/* 0x0040e280 */ int __cdecl Screen_Start(ScreenLoop *screen);
/* 0x0040e330 */ int __cdecl Screen_Finish(ScreenLoop *screen);
/* 0x0040e420 */ int __cdecl Screen_ReadInput(ScreenLoop *screen);
/* 0x0040e5d0 */ int __cdecl Screen_Tick(ScreenLoop *screen);
/* 0x0040e7c0 */ int __cdecl Screen_Run(ScreenLoop *screen);
/* 0x0040ebc0 */ int __cdecl Continue_Init(ScreenLoop *screen, void *state);
/* 0x0040edf0 */ int __cdecl Continue_End(ScreenLoop *screen, void *state);
/* 0x0040ee20 */ int __cdecl Continue_Update(ScreenLoop *screen, void *state);
/* 0x0040eee0 */ int __cdecl Continue_Tick(ScreenLoop *screen, void *state);
/* 0x0040efb0 */ int __cdecl Continue_Draw(ScreenLoop *screen, void *state);
/* 0x0040f400 */ int __cdecl Continue_Setup(ScreenLoop *screen);
/* 0x0040f4b0 */ int __cdecl Continue_Run(void);
/* 0x0040f540 */ Coord *__cdecl Coord_MakeScaled(Coord *out, const CharData *c, double x, double y);
/* 0x0040f5c0 */ double __cdecl Player_FromLocalCoord(Player *p, double v);
/* 0x0040f610 */ double __cdecl Player_ToLocalCoord(Player *p, double v);
/* 0x0040f660 */ Coord *__cdecl Coord_MakeUnit(Coord *out, double x, double y);
/* 0x0040f6a0 */ int __cdecl Version_ParseTriple(const char *s);
/* 0x0040f700 */ void __cdecl Screen_EndFrame(void);
/* 0x0040f780 */ void __cdecl Frame_UpdateSkip(void);
/* 0x0040f830 */ void __cdecl Screen_Present(void);
/* 0x0040f9e0 */ void __cdecl Screen_SaveShot(void);
/* 0x0040fb50 */ SysDrawList *__cdecl Explods_Create(int count, struct SpriteQueue *layer);
/* 0x0040fd00 */ void __cdecl Explods_Free(void *list);
/* 0x0040fdb0 */ void __cdecl SysDraw_Start(SysDrawList *list, int index);
/* 0x0040ff90 */ Coord *__cdecl SysDraw_ScreenPos(const SysDrawItem *e, Coord *out);
/* 0x004100c0 */ void __cdecl SysDraw_MakeParams(DrawParams *dp, const SysDrawItem *e);
/* 0x004101f0 */ void __cdecl SysDraw_Update(SysDrawItem *e);
/* 0x004103e0 */ void __cdecl Explods_DrawShadows(SysDrawList *list);
/* 0x004104f0 */ void __cdecl SysDraw_DrawShadow(const SysDrawItem *e, const int *a, const int *b);
/* 0x00410550 */ void __cdecl SysDraw_Clear(SysDrawList *list);
/* 0x004105b0 */ void __cdecl SysDrawItem_Reset(SysDrawItem *e);
/* 0x00410740 */ void __cdecl Explod_Create(Player *p, ExplodParams *ep, StateRunCtx *ctx, StateCtrl *sc);
/* 0x00410950 */ void __cdecl Explod_Modify(Player *p, ExplodParams *ep, StateRunCtx *ctx, StateCtrl *sc);
/* 0x004109e0 */ void __cdecl Explod_SetParams(Player *p, SysDrawItem *e, const ExplodParams *ep, StateRunCtx *ctx, int modify, const StateCtrl *sc);
/* 0x00411280 */ int __cdecl Explods_Add(SpriteSet *sprites, SysDrawList *list, int action, Anim *anim);
/* 0x00411360 */ int __cdecl SysDraw_AddSprite(int group, int number, SysDrawList *list, SpriteSet *sprites);
/* 0x004113e0 */ int __cdecl SysDraw_AddText(SysDrawList *list, const char *text, struct Font *font, int bank, int align, int r, int g, int b);
/* 0x00411480 */ void __cdecl SysDrawItem_Free(SysDrawItem *e);
/* 0x004114c0 */ void __cdecl SysDraw_Remove(int index, SysDrawList *list);
/* 0x00411510 */ void __cdecl Explods_RemoveOwner(SysDrawList *list, int owner_id, int id);
/* 0x00411580 */ void __cdecl Explods_RemoveOnHit(SysDrawList *list, int owner_id);
/* 0x004115f0 */ void __cdecl Explods_Unbind(Player *p, SysDrawList *list);
/* 0x00411710 */ int __cdecl SysDraw_Find(const SysDrawList *list, int id, int owner_id);
/* 0x00411750 */ void __cdecl Explods_SetBindTime(SysDrawList *list, int owner_id, int id, int time);
/* 0x00411790 */ int __cdecl Explods_Count(SysDrawList *list, int owner_id, int id);
/* 0x004117d0 */ int __cdecl SysElem_Read(SysElem *elem, const char *prefix, Ini *ini);
/* 0x00411b80 */ int __cdecl SysElem_Draw(int layer, SysDrawList *list, SpriteSet *sprites, Anim *anims, const SysElem *elem, void **fonts, float x, float y, float scale, float sprite_scale);
/* 0x00411c00 */ int __cdecl SysElem_Add(const SysElem *elem, SysDrawList *list, SpriteSet *sprites, Anim *anims, void **fonts, float x, float y);
/* 0x00411f30 */ int __cdecl SysElem_AddFight(const SysElem *elem, float x, float y);
/* 0x00411fb0 */ int __cdecl SysElem_AddMotif(const SysElem *elem, float x, float y);
/* 0x00412030 */ int __cdecl SysElem_DrawFight(const SysElem *elem, int layer, float x, float y);
/* 0x004120d0 */ void __cdecl SysElem_DrawMotif(const SysElem *elem, float x, float y);
/* 0x00412120 */ void __cdecl SysElem_SetScale(int index, float scale, float sprite_scale);
/* 0x00412430 */ int __cdecl SysDraw_DrawItem(SysDrawList *list, int index);
/* 0x004124a0 */ void __cdecl Gameflow_Run(void);
/* 0x00412880 */ int __cdecl PlayerRef_Set(PlayerRef *ref, Player *p, int flag);
/* 0x004128b0 */ int __cdecl PlayerRef_Is(const PlayerRef *ref, const Player *p);
/* 0x00412910 */ int __cdecl PlayerRef_GetPos(const int *ref_id, float *x, float *y, int *facing);
/* 0x00412970 */ Coord *__cdecl Coord_ScreenToGame(Coord *out, double x, double y, double z, double unit, int type, int pad);
/* 0x00412a40 */ Coord *__cdecl Coord_GameToScreen(Coord *out, double x, double y, double z, double unit, int type, int pad);
/* 0x00412b10 */ Coord *__cdecl Coord_ScreenToStage(Coord *out, double x, double y, double z, double unit, int type, int pad);
/* 0x00412bf0 */ Coord *__cdecl Coord_Unzoom(Coord *out, double x, double y, double z, double unit, int type, int pad);
/* 0x00412c50 */ int __cdecl PlayerRef_SetById(PlayerRef *ref, int id);
/* 0x00412ca0 */ int __cdecl Match_Init(ScreenLoop *screen, void *state);
/* 0x00412cc0 */ int __cdecl Match_End(ScreenLoop *screen, void *state);
/* 0x00412df0 */ int __cdecl Match_Update(ScreenLoop *screen, void *state);
/* 0x00412e60 */ int __cdecl Match_Tick(ScreenLoop *screen, void *state);
/* 0x00412eb0 */ int __cdecl Screen_TickNop(ScreenLoop *screen, void *state);
/* 0x00412ec0 */ int __cdecl Match_Setup(ScreenLoop *screen);
/* 0x00412fa0 */ int __cdecl Match_Run(void);
/* 0x00413170 */ int __cdecl Round_Tick(ScreenLoop *screen);
/* 0x00413e60 */ int __cdecl Demo_TickFight(ScreenLoop *screen);
/* 0x00413f00 */ int __cdecl Round_RunState(void);
/* 0x00415430 */ void __cdecl Round_AddAnnouncements(void);
/* 0x004157b0 */ void __cdecl Camera_UpdateZoom(void);
/* 0x00415a10 */ void __cdecl Camera_Reset(void);
/* 0x00415a70 */ void __cdecl Camera_Set(float x, float y);
/* 0x00415ae0 */ void __cdecl Camera_Update(void);
/* 0x00416dc0 */ void __cdecl Camera_UpdateCrop(void);
/* 0x00416ea0 */ void __cdecl Combo_AddHits(int team, int hits);
/* 0x00416f10 */ void __cdecl Combo_Draw(int team);
/* 0x004172f0 */ void __cdecl Combo_Tick(int team);
/* 0x00417400 */ void __cdecl Player_UpdateCombo(Player *p);
/* 0x004174c0 */ void __cdecl Stage_UpdateBgCamera(ScreenLoop *screen);
/* 0x004177a0 */ void __cdecl Match_DrawStageBack(void);
/* 0x00417880 */ void __cdecl Fight_DrawHud(int layer);
/* 0x00418790 */ void __cdecl Lifebar_Draw(Player *p, FightBar *bar, int layer);
/* 0x00418c60 */ void __cdecl Lifebars_Tick(void);
/* 0x00418cd0 */ void __cdecl Lifebar_TickLife(Player *p);
/* 0x00418e20 */ void __cdecl Powerbar_Draw(int team, Player *p, int x1, int x2, int layer);
/* 0x00418fb0 */ void __cdecl Lifebar_TickPower(Player *p);
/* 0x00419080 */ AfterImage *__cdecl AfterImages_FindFree(KeyedArray *list);
/* 0x004190d0 */ void __cdecl Projectiles_UpdateAfterImages(ProjectileList *pl);
/* 0x00419160 */ void __cdecl Projectiles_DrawAfterImages(ProjectileList *pl);
/* 0x004191d0 */ void __cdecl Clsn_Draw(const Clsn *clsn, float x, float y, float sx, float sy, unsigned int color, int flip, int filled);
/* 0x00419350 */ int __cdecl Team_LifeRecovery(int team);
/* 0x004193e0 */ void __cdecl Round_SetKoWinType(int attacker_team, int team, unsigned int attr);
/* 0x004194b0 */ void __cdecl Round_SetWinType(int team, int type, unsigned int attr);
/* 0x00419570 */ int __cdecl Team_HasFullLife(int side);
/* 0x004195d0 */ void __cdecl Debug_Draw(int index);
/* 0x00419a50 */ Player *__cdecl Helper_Create(Player *p, HelperParams *hp, StateRunCtx *ctx, StateCtrl *sc);
/* 0x0041a230 */ void __cdecl Helper_Remove(Player *p, int recursive, int remove_explods);
/* 0x0041a410 */ int __cdecl Helpers_Count(Player *p, int id);
/* 0x0041a480 */ Player *__cdecl Helper_Find(Player *p, int id);
/* 0x0041a500 */ void __cdecl System_SetWindowIcon(void);
/* 0x0041a560 */ void __cdecl System_Init(void);
/* 0x0041ae10 */ int __cdecl Console_Init(void);
/* 0x0041aea0 */ void __cdecl System_Deinit(void);
/* 0x0041b1e0 */ void __cdecl Input_InitPads(void);
/* 0x0041b340 */ void __cdecl Input_Init(void);
/* 0x0041b380 */ int __cdecl Input_Remap(void);
/* 0x0041b500 */ int __cdecl Fight_Load(const char *path);
/* 0x0041d9b0 */ void __cdecl Fight_Free(void);
/* 0x0041dab0 */ int __cdecl GameVars_Init(void);
/* 0x0041de60 */ int __cdecl System_LoadFonts(void);
/* 0x0041dee0 */ void __cdecl GameVars_Free(void);
/* 0x0041e170 */ KeyedArray *__cdecl AfterImages_Create(int count);
/* 0x0041e1e0 */ void __cdecl AfterImages_Free(KeyedArray *list);
/* 0x0041e260 */ void __cdecl System_OnActivate(int gained);
/* 0x0041e280 */ void __cdecl System_OnFocusLost(int gained);
/* 0x0041e370 */ void __cdecl System_OnFocusGained(int gained);
/* 0x0041e470 */ int __cdecl Game_GetFrameCount(void);
/* 0x0041e480 */ KeybindList *__cdecl KeybindList_Create(void);
/* 0x0041e510 */ int __cdecl Keybind_Add(KeybindList *list, const char *cmd, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid);
/* 0x0041e600 */ int __cdecl Keybind_Remove(KeybindList *list, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid);
/* 0x0041e690 */ void __cdecl KeybindList_Clear(KeybindList *list);
/* 0x0041e6e0 */ const char *__cdecl Keybind_Find(KeybindList *list, unsigned int kb_mods, int kb_key, int kb_unicode, int kb_valid);
/* 0x0041e730 */ int __cdecl Keybind_Parse(const char *str, KeyBind *out);
/* 0x0041e7f0 */ int __cdecl Keybind_Match(const KeyBind *e, const KeyBind *kb);
/* 0x0041e880 */ int __cdecl LuaPlayer_GetPlayer(struct lua_State *L);
/* 0x0041e930 */ int __cdecl LuaPlayer_GetPlayerList(struct lua_State *L);
/* 0x0041e9b0 */ int __cdecl LuaPlayer_IndexFromId(struct lua_State *L);
/* 0x0041e9f0 */ int __cdecl LuaPlayer_IndexIsValid(struct lua_State *L);
/* 0x0041ea50 */ int __cdecl LuaPlayer_Enabled(struct lua_State *L);
/* 0x0041ead0 */ int __cdecl LuaPlayer_EnableSet(struct lua_State *L);
/* 0x0041eb40 */ Player *__cdecl LuaPlayer_Check(struct lua_State *L);
/* 0x0041eba0 */ int __cdecl LuaPlayer_Life(struct lua_State *L);
/* 0x0041ebd0 */ int __cdecl LuaPlayer_LifeSet(struct lua_State *L);
/* 0x0041ec00 */ int __cdecl LuaPlayer_LifeMax(struct lua_State *L);
/* 0x0041ec30 */ int __cdecl LuaPlayer_Power(struct lua_State *L);
/* 0x0041ec80 */ int __cdecl LuaPlayer_PowerSet(struct lua_State *L);
/* 0x0041ecb0 */ int __cdecl LuaPlayer_PowerMax(struct lua_State *L);
/* 0x0041ed00 */ int __cdecl LuaPlayer_WinPerfect(struct lua_State *L);
/* 0x0041ed60 */ int __cdecl LuaPlayer_Win(struct lua_State *L);
/* 0x0041ed90 */ int __cdecl LuaPlayer_WinKO(struct lua_State *L);
/* 0x0041ede0 */ int __cdecl LuaPlayer_WinTime(struct lua_State *L);
/* 0x0041ee30 */ int __cdecl LuaPlayer_Lose(struct lua_State *L);
/* 0x0041ee80 */ int __cdecl LuaPlayer_LoseKO(struct lua_State *L);
/* 0x0041eee0 */ int __cdecl LuaPlayer_LoseTime(struct lua_State *L);
/* 0x0041ef40 */ int __cdecl LuaPlayer_DrawGame(struct lua_State *L);
/* 0x0041ef90 */ int __cdecl LuaPlayer_MatchOver(struct lua_State *L);
/* 0x0041efc0 */ int __cdecl LuaPlayer_AnimTime(struct lua_State *L);
/* 0x0041f000 */ int __cdecl LuaPlayer_Anim(struct lua_State *L);
/* 0x0041f040 */ int __cdecl LuaPlayer_AnimElemNo(struct lua_State *L);
/* 0x0041f080 */ int __cdecl LuaPlayer_AnimElemTime(struct lua_State *L);
/* 0x0041f100 */ int __cdecl LuaPlayer_AnimExist(struct lua_State *L);
/* 0x0041f150 */ int __cdecl LuaPlayer_SelfAnimExist(struct lua_State *L);
/* 0x0041f1b0 */ int __cdecl LuaPlayer_Pos(struct lua_State *L);
/* 0x0041f1f0 */ int __cdecl LuaPlayer_Vel(struct lua_State *L);
/* 0x0041f250 */ int __cdecl LuaPlayer_ScreenPos(struct lua_State *L);
/* 0x0041f2a0 */ int __cdecl LuaPlayer_RoundsExisted(struct lua_State *L);
/* 0x0041f2d0 */ int __cdecl LuaPlayer_TeamSide(struct lua_State *L);
/* 0x0041f2f0 */ int __cdecl LuaPlayer_TeamMode(struct lua_State *L);
/* 0x0041f340 */ int __cdecl LuaPlayer_IsHomeTeam(struct lua_State *L);
/* 0x0041f3a0 */ int __cdecl LuaPlayer_BackEdge(struct lua_State *L);
/* 0x0041f440 */ int __cdecl LuaPlayer_FrontEdge(struct lua_State *L);
/* 0x0041f4e0 */ int __cdecl LuaPlayer_FrontEdgeDist(struct lua_State *L);
/* 0x0041f590 */ int __cdecl LuaPlayer_BackEdgeDist(struct lua_State *L);
/* 0x0041f640 */ int __cdecl LuaPlayer_FrontEdgeBodyDist(struct lua_State *L);
/* 0x0041f680 */ int __cdecl LuaPlayer_BackEdgeBodyDist(struct lua_State *L);
/* 0x0041f6c0 */ int __cdecl LuaPlayer_Alive(struct lua_State *L);
/* 0x0041f6f0 */ int __cdecl LuaPlayer_AiEnabled(struct lua_State *L);
/* 0x0041f720 */ int __cdecl LuaPlayer_AiEnableSet(struct lua_State *L);
/* 0x0041f770 */ int __cdecl LuaPlayer_AiLevel(struct lua_State *L);
/* 0x0041f7d0 */ int __cdecl LuaPlayer_HitCount(struct lua_State *L);
/* 0x0041f800 */ int __cdecl LuaPlayer_UniqHitCount(struct lua_State *L);
/* 0x0041f830 */ int __cdecl LuaPlayer_MoveHit(struct lua_State *L);
/* 0x0041f870 */ int __cdecl LuaPlayer_MoveGuarded(struct lua_State *L);
/* 0x0041f8b0 */ int __cdecl LuaPlayer_MoveContact(struct lua_State *L);
/* 0x0041f900 */ int __cdecl LuaPlayer_MoveReversed(struct lua_State *L);
/* 0x0041f940 */ int __cdecl LuaPlayer_InGuardDist(struct lua_State *L);
/* 0x0041f970 */ int __cdecl LuaPlayer_StateNo(struct lua_State *L);
/* 0x0041f9a0 */ int __cdecl LuaPlayer_PrevStateNo(struct lua_State *L);
/* 0x0041f9d0 */ int __cdecl LuaPlayer_StateType(struct lua_State *L);
/* 0x0041fa70 */ int __cdecl LuaPlayer_MoveType(struct lua_State *L);
/* 0x0041faf0 */ int __cdecl LuaPlayer_Ctrl(struct lua_State *L);
/* 0x0041fb20 */ int __cdecl LuaPlayer_Var(struct lua_State *L);
/* 0x0041fb70 */ int __cdecl LuaPlayer_FVar(struct lua_State *L);
/* 0x0041fbd0 */ int __cdecl LuaPlayer_SysVar(struct lua_State *L);
/* 0x0041fc20 */ int __cdecl LuaPlayer_SysFVar(struct lua_State *L);
/* 0x0041fc80 */ int __cdecl LuaPlayer_HitDefAttrMatch(struct lua_State *L);
/* 0x0041fd10 */ int __cdecl LuaPlayer_NumProj(struct lua_State *L);
/* 0x0041fdb0 */ int __cdecl LuaPlayer_ProjCancelTime(struct lua_State *L);
/* 0x0041fe50 */ int __cdecl LuaPlayer_ProjHitTime(struct lua_State *L);
/* 0x0041fef0 */ int __cdecl LuaPlayer_ProjGuardedTime(struct lua_State *L);
/* 0x0041ff90 */ int __cdecl LuaPlayer_ProjContactTime(struct lua_State *L);
/* 0x00420030 */ int __cdecl LuaPlayer_Name(struct lua_State *L);
/* 0x00420050 */ int __cdecl LuaPlayer_AuthorName(struct lua_State *L);
/* 0x00420070 */ int __cdecl LuaPlayer_NumPartner(struct lua_State *L);
/* 0x004200a0 */ int __cdecl LuaPlayer_NumEnemy(struct lua_State *L);
/* 0x004200d0 */ int __cdecl LuaPlayer_Id(struct lua_State *L);
/* 0x004200f0 */ int __cdecl LuaPlayer_Facing(struct lua_State *L);
/* 0x00420120 */ int __cdecl LuaPlayer_HitFall(struct lua_State *L);
/* 0x00420150 */ int __cdecl LuaPlayer_HitShakeOver(struct lua_State *L);
/* 0x00420180 */ int __cdecl LuaPlayer_HitOver(struct lua_State *L);
/* 0x004201b0 */ int __cdecl LuaPlayer_HitVel(struct lua_State *L);
/* 0x00420220 */ int __cdecl LuaPlayer_HitPauseTime(struct lua_State *L);
/* 0x00420250 */ int __cdecl LuaPlayer_CanRecover(struct lua_State *L);
/* 0x004202a0 */ int __cdecl LuaPlayer_PalNo(struct lua_State *L);
/* 0x004202d0 */ int __cdecl LuaPlayer_NumExplod(struct lua_State *L);
/* 0x00420390 */ int __cdecl LuaPlayer_NumTarget(struct lua_State *L);
/* 0x00420420 */ int __cdecl LuaPlayer_GetHitVar(struct lua_State *L);
/* 0x00420590 */ int __cdecl LuaPlayer_IsHelper(struct lua_State *L);
/* 0x00420650 */ int __cdecl LuaPlayer_NumHelper(struct lua_State *L);
/* 0x004206e0 */ int __cdecl LuaPlayer_ParentDist(struct lua_State *L);
/* 0x004207c0 */ int __cdecl LuaPlayer_RootDist(struct lua_State *L);
/* 0x00420880 */ int __cdecl LuaPlayer_Command(struct lua_State *L);
/* 0x00420910 */ int __cdecl LuaPlayer_UnitSize(struct lua_State *L);
/* 0x00420940 */ int __cdecl LuaPlayer_ChangeState(struct lua_State *L);
/* 0x00420ae0 */ int __cdecl LuaBind_ReadExplod(struct lua_State *L, Player *p, SysDrawItem *e);
/* 0x00420ed0 */ int __cdecl LuaPlayer_Explod(struct lua_State *L);
/* 0x00421150 */ int __cdecl LuaPlayer_VelSet(struct lua_State *L);
/* 0x004211c0 */ int __cdecl LuaPlayer_IsValid(struct lua_State *L);
/* 0x00421230 */ int __cdecl LuaPlayer_ToString(struct lua_State *L);
/* 0x00421260 */ int __cdecl LuaPlayer_Rc(struct lua_State *L);
/* 0x00421290 */ int __cdecl LuaBind_Register(void);
/* 0x00421300 */ int __cdecl CharLua_SetConstInt(CharData *c, struct lua_State *L, const char *name, int value);
/* 0x00421380 */ int __cdecl CharLua_SetConstFloat(CharData *c, struct lua_State *L, const char *name, double value);
/* 0x00421400 */ int __cdecl CharLua_SetConstCoord(CharData *c, struct lua_State *L, const char *name, double x, double y, double z, double unit, int type, int pad);
/* 0x00421490 */ int __cdecl CharLua_GetConstCoord(CharData *c, struct lua_State *L, Coord *out, const char *name);
/* 0x00421500 */ int __cdecl SDL_main(int argc, char **argv);
/* 0x004216f0 */ int __cdecl Screen_IsSpecial(void);
/* 0x00421720 */ float __cdecl Units_From320(float v);
/* 0x00421750 */ float __cdecl Camera_GetRawX(void);
/* 0x00421780 */ float __cdecl Camera_GetRawY(void);
/* 0x004217b0 */ float __cdecl Camera_GetX(void);
/* 0x004217e0 */ float __cdecl Camera_GetY(void);
/* 0x00421820 */ int __cdecl Str_IsVersionNumber(const char *s);
/* 0x00421850 */ unsigned int __cdecl Version_ParseDate(const char *s);
/* 0x004218e0 */ unsigned int __cdecl Version_Parse(const char *s);
/* 0x00421990 */ void __cdecl Rng_Seed(int seed);
/* 0x004219f0 */ int __cdecl Random_Next(void);
/* 0x00421a40 */ void __cdecl Debug_ResetLifePowerTime(void);
/* 0x00421bd0 */ int __cdecl Round_GetStartTime(void);
/* 0x00421c10 */ void __cdecl Debug_ClearClipboards(void);
/* 0x00421c80 */ void __cdecl Debug_ToggleTrainingMode(void);
/* 0x00421d00 */ void __cdecl Debug_TogglePlayer(int n);
/* 0x00421d80 */ void __cdecl Debug_ToggleAi(int n);
/* 0x00421dd0 */ void __cdecl Debug_SetPlayerState(int n, int state);
/* 0x00421e80 */ void __cdecl Match_LoadAssets(void);
/* 0x00422000 */ void __cdecl Demo_SetupTeams(void);
/* 0x004221c0 */ int __cdecl Gameflow_SetupMatch(void);
/* 0x00422240 */ void __cdecl QuickVs_SetupTeams(void);
/* 0x004227d0 */ void __cdecl Gameflow_PickStage(void);
/* 0x00422960 */ int __cdecl Stage_Load(Stage *st);
/* 0x00423580 */ int __cdecl Stage_Activate(Stage *st);
/* 0x004236e0 */ int __cdecl Stage_LoadBg(Stage *st, Ini *ini);
/* 0x00423890 */ void __cdecl Helpers_Alloc(void);
/* 0x004239c0 */ void __cdecl Team_ReloadMember(int team);
/* 0x00423a90 */ int __cdecl Team_CreatePlayers(int team);
/* 0x00423d70 */ int __cdecl Team_FreePlayers(int team);
/* 0x00423e90 */ void __cdecl Players_Free(void);
/* 0x00423fc0 */ void __cdecl Stage_Unload(Stage *st);
/* 0x00424020 */ int __cdecl Match_LoadTeamFaces(void);
/* 0x004242c0 */ void __cdecl Players_ReleaseSlots(void);
/* 0x00424310 */ void __cdecl Match_NextRound(void);
/* 0x00424480 */ void __cdecl Match_Start(void);
/* 0x00424a20 */ void __cdecl Match_Unload(void);
/* 0x00424b60 */ void __cdecl Team_ResetLife(TeamSel *ts, int force);
/* 0x00424bf0 */ void __cdecl Round_Reset(void);
/* 0x004252d0 */ void __cdecl Round_PlacePlayers(void);
/* 0x00425480 */ int __cdecl Match_CheckOver(void);
/* 0x00425540 */ unsigned int __cdecl Match_SeedRng(unsigned int seed);
/* 0x00425630 */ int __cdecl Match_Random(void);
/* 0x00425680 */ void __cdecl Shadow_Draw(AnimState *st, const DrawParams *dp, float y, float offset, float sx, float sy, const int *shadow, const int *reflection);
/* 0x004259f0 */ int __cdecl ConLines_RemoveFirst(DynArray *a);
/* 0x00425a50 */ void *__cdecl PtrArray_Push(DynArray *a, void *elem);
/* 0x00425aa0 */ int __cdecl PtrArray_RemoveAt(DynArray *a, int index);
/* 0x00425ae0 */ ConText *__cdecl ConText_Create(int cap);
/* 0x00425b30 */ void __cdecl ConText_Copy(ConText *dst, const ConText *src);
/* 0x00425ba0 */ void __cdecl ConText_Set(ConText *t, const char *s);
/* 0x00425c00 */ int __cdecl ConText_Append(ConText *dst, const ConText *src);
/* 0x00425c40 */ void __cdecl ConText_AppendStr(const char *s, ConText *dst);
/* 0x00425d10 */ void __cdecl ConText_SetUtf16(ConText *t, const unsigned short *s);
/* 0x00425e00 */ int __cdecl ConCursor_Move(ConCursor *c, int n);
/* 0x00425e80 */ int __cdecl ConCursor_Insert(ConCursor *c, const ConText *src);
/* 0x00425f00 */ int __cdecl ConCursor_Delete(ConCursor *c);
/* 0x00425f80 */ int __cdecl ConCursor_SetChars(ConCursor *c, int chars);
/* 0x00425fc0 */ int __cdecl ConCursor_SeekBytes(ConCursor *c, int bytes);
/* 0x00426020 */ void __cdecl PtrArray_FreeAll(DynArray *a);
/* 0x00426060 */ int __cdecl Con_CmdClear(CmdTable *ctx, char *name, char *args);
/* 0x004260a0 */ int __cdecl Con_CmdConsoleToggle(CmdTable *ctx, char *name, char *args);
/* 0x004260e0 */ int __cdecl Con_CmdFrameStep(CmdTable *ctx, char *name, char *args);
/* 0x00426130 */ void __cdecl Console_ListCommands(void);
/* 0x004261f0 */ int __cdecl Con_CmdHelp(CmdTable *ctx, char *name, char *args);
/* 0x004262e0 */ int __cdecl Con_CmdPause(CmdTable *ctx, char *name, char *args);
/* 0x00426390 */ int __cdecl Con_CmdQuit(CmdTable *ctx, char *name, char *args);
/* 0x004263d0 */ int __cdecl Con_CmdLifeReset(CmdTable *ctx, char *name, char *args);
/* 0x00426430 */ int __cdecl Con_CmdRoundReset(CmdTable *ctx, char *name, char *args);
/* 0x004264b0 */ int __cdecl Con_CmdMatchReset(CmdTable *ctx, char *name, char *args);
/* 0x00426530 */ int __cdecl Con_CmdClearClipboard(CmdTable *ctx, char *name, char *args);
/* 0x00426570 */ int __cdecl Con_CmdSetRoundTime(CmdTable *ctx, char *name, char *args);
/* 0x00426600 */ int __cdecl Con_CmdMaxPower(CmdTable *ctx, char *name, char *args);
/* 0x00426690 */ int __cdecl Con_CmdMaxPowerMode(CmdTable *ctx, char *name, char *args);
/* 0x00426710 */ int __cdecl Con_CmdToggleLifebars(CmdTable *ctx, char *name, char *args);
/* 0x00426780 */ int __cdecl Con_CmdShowClsn(CmdTable *ctx, char *name, char *args);
/* 0x00426800 */ int __cdecl Con_CmdFps(CmdTable *ctx, char *name, char *args);
/* 0x00426850 */ int __cdecl Con_CmdSetLife(CmdTable *ctx, char *name, char *args);
/* 0x004268f0 */ int __cdecl Con_CmdToggleAi(CmdTable *ctx, char *name, char *args);
/* 0x00426970 */ int __cdecl Con_CmdTogglePlayer(CmdTable *ctx, char *name, char *args);
/* 0x004269f0 */ int __cdecl Con_CmdForceStand(CmdTable *ctx, char *name, char *args);
/* 0x00426b70 */ int __cdecl Con_CmdChangeState(CmdTable *ctx, char *name, char *args);
/* 0x00426c00 */ int __cdecl Con_CmdScreenshot(CmdTable *ctx, char *name, char *args);
/* 0x00426c50 */ int __cdecl Con_CmdInvalid(CmdTable *ctx, char *name, char *args);
/* 0x00426c70 */ int __cdecl Console_Create(void);
/* 0x00427510 */ void __cdecl Console_Free(void);
/* 0x00427650 */ int __cdecl ConsoleView_Init(ConsoleView *v);
/* 0x00427720 */ void __cdecl ConsoleView_Free(ConsoleView *v);
/* 0x00427790 */ int __cdecl ConsoleInput_Init(ConsoleInput *in);
/* 0x00427810 */ void __cdecl Console_Toggle(void);
/* 0x004278b0 */ void __cdecl Console_Draw(void);
/* 0x00427c50 */ int __cdecl Console_Tick(void);
/* 0x00427cd0 */ void __cdecl Console_HandleKey(unsigned int unicode, int key, int pressed);
/* 0x00428090 */ void __cdecl Console_AddHistory(const char *line);
/* 0x004281a0 */ void __cdecl Console_HistoryMove(int dir);
/* 0x004282c0 */ void __cdecl Console_QueueCommand(const char *cmd);
/* 0x004283b0 */ int __cdecl Console_AddLine(ConText *t, int copy);
/* 0x00428460 */ int __cdecl Console_Print(const char *s);
/* 0x004284e0 */ void __cdecl Console_Printf(const char *fmt, ...);
/* 0x00428530 */ void __cdecl Console_ClearLines(void);
/* 0x00428590 */ void __cdecl Console_ResetInput(void);
/* 0x004285c0 */ int __cdecl Str_IsNumber(const char *s);
/* 0x00428620 */ int __cdecl Con_ArgInt(char **args, int *out);
/* 0x00428690 */ int __cdecl Con_ArgNumber(char **args, double *out);
/* 0x00428710 */ int __cdecl Con_NoMoreArgs(char **args);
/* 0x00428740 */ int __cdecl Storyboard_Update(ScreenLoop *screen, void *state);
/* 0x004287d0 */ void __cdecl Storyboard_SetupScreen(ScreenLoop *screen, Cutscene *cs);
/* 0x00428850 */ void __cdecl Storyboard_PlayFile(const char *path);
/* 0x00428af0 */ int __cdecl Title_Init(ScreenLoop *screen, TitleState *st);
/* 0x00428d90 */ int __cdecl Title_End(ScreenLoop *screen, TitleState *st);
/* 0x00428dd0 */ int __cdecl Title_Update(ScreenLoop *screen, TitleState *st);
/* 0x00429020 */ int __cdecl Title_Tick(ScreenLoop *screen, TitleState *st);
/* 0x004291d0 */ int __cdecl Title_Draw(ScreenLoop *screen, TitleState *st);
/* 0x00429b80 */ int __cdecl Title_Setup(ScreenLoop *screen);
/* 0x00429c20 */ int __cdecl Title_Run(void);
/* 0x00429cb0 */ Font *__cdecl Options_GetFont(void);
/* 0x00429d30 */ int __cdecl Options_Init(ScreenLoop *screen, OptionsState *st);
/* 0x00429f60 */ int __cdecl Options_End(ScreenLoop *screen, OptionsState *st);
/* 0x00429f90 */ int __cdecl Options_Update(ScreenLoop *screen, OptionsState *st);
/* 0x0042ab00 */ int __cdecl Options_Draw(ScreenLoop *screen, OptionsState *st);
/* 0x0042b960 */ int __cdecl Options_Setup(ScreenLoop *screen);
/* 0x0042ba00 */ int __cdecl Options_Run(void);
/* 0x0042ba60 */ int __cdecl KeyConfig_Run(int joystick, int ticks);
/* 0x0042d750 */ int __cdecl Training_HandleKey(int code);
/* 0x0042db50 */ void __cdecl Training_Draw(void);
/* 0x0042e260 */ void __cdecl Infobox_DrawText(const char *text, int x, int y);
/* 0x0042e2f0 */ int __cdecl Lua_Init(void);
/* 0x0042e390 */ int __cdecl Lua_RunInit(void);
/* 0x0042e400 */ int __cdecl LuaMugen_LifeReset(struct lua_State *L);
/* 0x0042e410 */ int __cdecl LuaMugen_ClipboardPrint(struct lua_State *L);
/* 0x0042e430 */ int __cdecl LuaMugen_ConsolePrint(struct lua_State *L);
/* 0x0042e450 */ int __cdecl LuaMugen_Log(struct lua_State *L);
/* 0x0042e470 */ int __cdecl LuaMugen_WinningTeam(struct lua_State *L);
/* 0x0042e490 */ int __cdecl LuaMugen_GameTime(struct lua_State *L);
/* 0x0042e4b0 */ int __cdecl LuaMugen_RoundNo(struct lua_State *L);
/* 0x0042e4d0 */ int __cdecl LuaMugen_RoundState(struct lua_State *L);
/* 0x0042e4f0 */ int __cdecl LuaMugen_MatchNo(struct lua_State *L);
/* 0x0042e510 */ int __cdecl LuaMugen_TicksPerSecond(struct lua_State *L);
/* 0x0042e530 */ int __cdecl LuaMugen_GameWidth(struct lua_State *L);
/* 0x0042e570 */ int __cdecl LuaMugen_GameHeight(struct lua_State *L);
/* 0x0042e5b0 */ int __cdecl LuaMugen_ScreenWidth(struct lua_State *L);
/* 0x0042e5e0 */ int __cdecl LuaMugen_ScreenHeight(struct lua_State *L);
/* 0x0042e610 */ int __cdecl LuaMugen_CameraPos(struct lua_State *L);
/* 0x0042e650 */ int __cdecl LuaMugen_CameraZoom(struct lua_State *L);
/* 0x0042e680 */ int __cdecl LuaMugen_LeftEdge(struct lua_State *L);
/* 0x0042e6e0 */ int __cdecl LuaMugen_RightEdge(struct lua_State *L);
/* 0x0042e740 */ int __cdecl LuaMugen_TopEdge(struct lua_State *L);
/* 0x0042e7b0 */ int __cdecl LuaMugen_BottomEdge(struct lua_State *L);
/* 0x0042e7e0 */ int __cdecl LuaMugen_Random(struct lua_State *L);
/* 0x0042e860 */ int __cdecl LuaMugen_RoundTimeLeft(struct lua_State *L);
/* 0x0042e880 */ int __cdecl LuaMugen_RoundTimeMax(struct lua_State *L);
/* 0x0042e8d0 */ int __cdecl LuaMugen_SetRoundTimeLeft(struct lua_State *L);
/* 0x0042e920 */ int __cdecl LuaMugen_KeybindAdd(struct lua_State *L);
/* 0x0042e9a0 */ int __cdecl LuaMugen_KeybindDel(struct lua_State *L);
/* 0x0042ea10 */ int __cdecl LuaMugen_KeybindGet(struct lua_State *L);
/* 0x0042eab0 */ int __cdecl LuaMugen_GameModeCode(struct lua_State *L);
/* 0x0042ead0 */ int __cdecl LuaMugen_Language(struct lua_State *L);
/* 0x0042eaf0 */ int __cdecl LuaMugen_ToggleConsole(struct lua_State *L);
/* 0x0042eb00 */ int __cdecl LuaMugen_ToggleMaxPowerMode(struct lua_State *L);
/* 0x0042eb30 */ int __cdecl LuaMugen_RoundReset(struct lua_State *L);
/* 0x0042eb60 */ int __cdecl LuaMugen_MatchReload(struct lua_State *L);
/* 0x0042eb90 */ int __cdecl LuaMugen_ClearClipboards(struct lua_State *L);
/* 0x0042eba0 */ int __cdecl LuaMugen_ToggleBarDisplay(struct lua_State *L);
/* 0x0042ebc0 */ int __cdecl LuaMugen_ToggleClsnDisplay(struct lua_State *L);
/* 0x0042ebf0 */ int __cdecl LuaMugen_ToggleTrainingMode(struct lua_State *L);
/* 0x0042ec00 */ int __cdecl Lua_LogError(struct lua_State *L);
/* 0x0042ec20 */ int __cdecl Lua_Panic(struct lua_State *L);
/* 0x0042ec40 */ int __cdecl Lua_PushVector(struct lua_State *L, double x, double y, double z, double unit, int type, int pad);
/* 0x0042ed20 */ int __cdecl Lua_ReadVector(struct lua_State *L, Coord *out);
/* 0x0042ede0 */ int __cdecl LuaTable_GetVector(struct lua_State *L, const char *key, Coord *out);
/* 0x0042ee40 */ int __cdecl LuaTable_GetVectorNumber(const char *key, struct lua_State *L, double *out);
/* 0x0042ef50 */ int __cdecl Ai_InitPlayer(Player *p);
/* 0x0042ef90 */ int __cdecl Ai_Load(const char *path, AiData *ai);
/* 0x0042f1f0 */ int __cdecl Ai_Save(const char *path, const AiData *ai);
/* 0x0042f360 */ int __cdecl Ai_Alloc(AiData *ai);
/* 0x0042f3c0 */ void __cdecl Ai_Free(AiData *ai);
/* 0x0042f420 */ void __cdecl Ai_Reset(AiData *ai);
/* 0x0042f4a0 */ int __cdecl Ai_Merge(AiData *dst, const AiData *src);
/* 0x0042f5c0 */ void __cdecl Ai_ResetPlayer(Player *p);
/* 0x0042f640 */ void __cdecl Ai_Tick(Player *p);
/* 0x0042f750 */ void __cdecl Ai_UpdatePlayer(Player *p);
/* 0x0042f7a0 */ int __cdecl Ai_AddCommand(int *list, int label);
/* 0x0042f7d0 */ void __cdecl Ai_CopyCommands(const int *src, int *dst);
/* 0x0042f800 */ int __cdecl Ai_RecordMove(Player *p);
/* 0x0042f9c0 */ void __cdecl Ai_UpdateLearn(Player *p, int tick);
/* 0x00430020 */ void __cdecl Ai_TrackBox(Player *p, AiMove *move);
/* 0x004301e0 */ void __cdecl Ai_TrackBounds(Player *p);
/* 0x00430330 */ int __cdecl Ai_FinishMove(Player *p, int store);
/* 0x004305f0 */ void __cdecl Ai_LearnMove(Player *p);
/* 0x00430660 */ int __cdecl Ai_Broadcast(Player *sender, Player *from, int type, int value, KeyedArray *table);
/* 0x00430710 */ AiMsg *__cdecl Ai_FindMsg(AiMsg *out, Player *p, Player *sender, int type);
/* 0x00430780 */ AiMsg *__cdecl Ai_TakeMsg(AiMsg *out, Player *p, Player *sender, int type);
/* 0x00430800 */ void __cdecl Ai_Think(Player *p, int level);
/* 0x00430900 */ int __cdecl Ai_Decide(Player *p, int level);
/* 0x00430fb0 */ int __cdecl Ai_MakePath(Player *p, int y, int x);
/* 0x00431110 */ int __cdecl Ai_StepDone(Player *p, int index);
/* 0x00431200 */ int __cdecl Ai_ModeIdle(Player *p);
/* 0x004312a0 */ int __cdecl Ai_ModeMove(Player *p);
/* 0x004314f0 */ int __cdecl Ai_ModeAttack(Player *p);
/* 0x00431680 */ int __cdecl Ai_ModeGuard(Player *p);
/* 0x00431740 */ int __cdecl Ai_HitTest(Player *p, Player *enemy, AiMove *move);
/* 0x00431970 */ int __cdecl Ai_FindCounters(Player *p);
/* 0x00431ba0 */ int __cdecl Ai_ModeFight(Player *enemy, Player *p, int level);
/* 0x004325b0 */ void __cdecl Ai_TrainingDummy(Player *enemy, Player *p);
/* 0x00432c00 */ int __cdecl Ai_AnyInGuardDist(Player *p);
/* 0x00432c60 */ void __cdecl Ai_ClearLearn(int *ai_learn);
/* 0x00432c90 */ void __cdecl Ai_MergeRecord(AiMove *dst, const AiMove *src, int contact);
/* 0x00433b90 */ void __cdecl Ai_ReleaseDirs(Player *p);
/* 0x00433d10 */ void __cdecl Ai_ReleaseDirsExcept(Player *p, int keep);
/* 0x00433d40 */ void __cdecl Ai_ReleaseAll(Player *p);
/* 0x00433dd0 */ void __cdecl Ai_ReleaseKeysExcept(Player *p, int keep);
/* 0x00433e00 */ Player *__cdecl Player_FindEnemy(Player *p);
/* 0x00433fd0 */ Player *__cdecl Player_GetPartner(Player *p, int n);
/* 0x00434030 */ int __cdecl Player_CountPartners(Player *p);
/* 0x00434090 */ Player *__cdecl Player_GetEnemy(Player *p, int n);
/* 0x00434100 */ int __cdecl Player_CountEnemies(Player *p);
/* 0x00434160 */ Player *__cdecl Player_GetNearEnemy(Player *p, int n);
/* 0x00434250 */ Player *__cdecl Player_GetActive(int index);
/* 0x00434280 */ Player *__cdecl Player_FindById(int id);
/* 0x004342a0 */ int __cdecl Player_FindIndexById(int id);
/* 0x004342f0 */ int __cdecl Player_AtScreenEdge(Player *p);
/* 0x00434360 */ Coord *__cdecl Player_GetScreenPos(Player *p, Coord *out);
/* 0x004343d0 */ double __cdecl Player_EdgeDist(Player *p, int body, int front);
/* 0x004345a0 */ float __cdecl Player_DistX(Player *p, Player *other);
/* 0x004345f0 */ float __cdecl Player_BodyDistX(Player *p, Player *other);
/* 0x00434720 */ int __cdecl Player_InEnemyGuardDist(Player *p);
/* 0x004347a0 */ int __cdecl Player_InGuardDist(Player *attacker, Player *p);
/* 0x00434900 */ int __cdecl HitDef_AffectsTeam(const HitDef *hd, int team);
/* 0x00434930 */ int __cdecl Player_AnimExists(Player *p, int action);
/* 0x00434960 */ int __cdecl Player_SelfAnimExists(Player *p, int action);
/* 0x004349b0 */ TargetEntry *__cdecl Targets_FindPlayer(KeyedArray *targets, Player *p);
/* 0x00434a00 */ Player **__cdecl Targets_Find(KeyedArray *targets, int hit_id);
/* 0x00434a60 */ int __cdecl Targets_Count(KeyedArray *targets, int hit_id);
/* 0x00434ac0 */ int __cdecl Player_ScaleDamage(Player *p, int damage);
/* 0x00434b20 */ void __cdecl Player_GetPosOrigin(PlayerRef *bind, Player *p, int postype, Player **p2, int space, float *x, float *y, float *offset_x, float *offset_y, int *facing);
/* 0x00434d50 */ void __cdecl Player_UpdateInputFacing(Player *p);
/* 0x00434db0 */ void __cdecl Player_UpdateInput(Player *p);
/* 0x004355d0 */ CharData *__cdecl CharCache_Get(int index);
/* 0x00435600 */ CharData *__cdecl PlayerCache_Load(int index);
/* 0x00435870 */ void __cdecl PlayerCache_Release(int index, int serial);
/* 0x004358e0 */ void __cdecl PlayerCache_Clear(void);
/* 0x004359e0 */ int __cdecl PlayerCache_HasRoom(void);
/* 0x00435a30 */ CharData *__cdecl Char_Load(const char *dir, const char *def);
/* 0x00435b30 */ CharData *__cdecl Char_LoadDef(const char *dir, Stream *s);
/* 0x00436d60 */ int __cdecl Const_ReadInt(Ini *ini, const char *key, int min, int max, CharData *c, int def, const char *name);
/* 0x00436dd0 */ int __cdecl Const_ReadFloat(Ini *ini, const char *key, CharData *c, const char *name, double def, double min, double max);
/* 0x00436e60 */ int __cdecl Const_ReadDist(CharData *c, const char *key, Ini *ini, const char *name, double def, double min, double max);
/* 0x00436f70 */ int __cdecl Const_ReadAirRecoverMul(CharData *c, Ini *ini, double dx, double dy, double dz, double dunit, int dtype, int dpad, double minx, double miny, double minz, double minunit, int mintype, int minpad, double maxx, double maxy, double maxz, double maxunit, int maxtype, int maxpad);
/* 0x00437040 */ int __cdecl Const_ReadVector(CharData *c, Ini *ini, const char *key, const char *name, double dx, double dy, double dz, double dunit, int dtype, int dpad, double minx, double miny, double minz, double minunit, int mintype, int minpad, double maxx, double maxy, double maxz, double maxunit, int maxtype, int maxpad);
/* 0x004371a0 */ Coord *__cdecl Const_GetCoord(Coord *out, const char *name, CharData *c);
/* 0x004371f0 */ int __cdecl Const_Alias(CharData *c, const char *name, const char *alias);
/* 0x00437290 */ int __cdecl Char_LoadConstants(CharData *c, Ini *ini);
/* 0x00439920 */ int __cdecl Char_LoadCmd(Ini *ini, CmdSet *cmds);
/* 0x00439f20 */ int __cdecl Cmd_Parse(const char *s, CmdEntry *entry, const CmdRemap *remap);
/* 0x0043a150 */ int __cdecl Cmd_ParseKey(const char *s, unsigned int *out, const CmdRemap *remap);
/* 0x0043a290 */ void __cdecl CmdSet_FreeNames(CmdSet *cmds);
/* 0x0043a2e0 */ void __cdecl CmdSet_Free(CmdSet *cmds);
/* 0x0043a330 */ void __cdecl Char_Free(CharData *c);
/* 0x0043a450 */ Player *__cdecl Player_Create(CharData *c);
/* 0x0043a7c0 */ int __cdecl Player_GetAiLevel(void);
/* 0x0043a880 */ void __cdecl Player_Free(Player *p);
/* 0x0043a9a0 */ int __cdecl Player_CheckRequired(Player *p);
/* 0x0043b1b0 */ void __cdecl PersistVars_Reset(int team);
/* 0x0043b210 */ void __cdecl PersistVars_Save(const Player *p, PersistVars *dst);
/* 0x0043b280 */ void __cdecl Players_ForceAi(void);
/* 0x0043b320 */ int __cdecl Player_Unlink(Player *p);
/* 0x0043b460 */ void __cdecl Team_RotateMembers(int team);
/* 0x0043b6b0 */ float __cdecl Math_Sign(float v);
/* 0x0043b6f0 */ void __cdecl Player_Reset(Player *p);
/* 0x0043be60 */ void __cdecl Players_SetActive(int index, int active);
/* 0x0043bf40 */ int __cdecl Player_ChangeState(int state, Player *p);
/* 0x0043bfc0 */ void __cdecl Player_SetLife(Player *p, int life);
/* 0x0043c0b0 */ void __cdecl Player_SetPower(Player *p, int power);
/* 0x0043c120 */ void __cdecl Player_SetAction(Player *p, int action);
/* 0x0043c190 */ void __cdecl Player_SetActionElem(Player *p, int action, int elem);
/* 0x0043c220 */ void __cdecl Player_ChangeAnimSelf(Player *p, int action);
/* 0x0043c240 */ void __cdecl Player_ChangeAnimSelfElem(Player *p, int action, int elem);
/* 0x0043c280 */ void __cdecl Player_ChangeAnim2(Player *p, int action);
/* 0x0043c300 */ void __cdecl Player_ChangeAnim2Elem(Player *p, int action, int elem);
/* 0x0043c390 */ void __cdecl Player_FaceTowards(Player *p, Player *other);
/* 0x0043c460 */ void __cdecl Player_Turn(Player *p);
/* 0x0043c480 */ void __cdecl Player_Update(Player *p);
/* 0x0043d650 */ void __cdecl Player_UpdateBind(Player *p);
/* 0x0043d750 */ void __cdecl Player_ScreenBound(Player *p);
/* 0x0043d8c0 */ void __cdecl Player_ClampToStage(Player *p);
/* 0x0043d950 */ void __cdecl Players_CheckHits(void);
/* 0x0043de40 */ void __cdecl Players_Push(Player *a, Player *b);
/* 0x0043df60 */ void __cdecl Players_PushApart(Player *a, Player *b);
/* 0x0043e280 */ int __cdecl Player_CanHit(Player *att, Player *tgt);
/* 0x0043e480 */ void __cdecl Player_Hit(Player *att, Player *tgt);
/* 0x0043ece0 */ int __cdecl HitDef_Apply(HitOverride *ov, Player *tgt, Player *att, const HitDef *hd, float x, float y, int dir, int ko);
/* 0x0043f8d0 */ void __cdecl Player_ApplyDamage(Player *p, int damage, int kill, int absolute);
/* 0x0043f930 */ int __cdecl Player_HitDamage(Player *p, int damage, int kill);
/* 0x0043f990 */ int __cdecl Player_Guard(Player *tgt, Player *att, int dir, const HitDef *hd);
/* 0x0043fc60 */ int __cdecl Player_CanReverse(Player *rev, Player *att);
/* 0x0043fd80 */ void __cdecl Player_Reverse(Player *rev, Player *att);
/* 0x004405c0 */ void __cdecl Player_HitProjectiles(Player *p, Player *owner);
/* 0x00440880 */ void __cdecl Projectiles_HitPlayer(Player *owner, Player *tgt);
/* 0x00440f50 */ void __cdecl Projectiles_ApplyHits(Player *p);
/* 0x00440fd0 */ void __cdecl Projectiles_Collide(Player *a, Player *b);
/* 0x00441200 */ int __cdecl Player_HitFlagMatches(Player *p, unsigned int flags);
/* 0x004412a0 */ int __cdecl Player_HitByAllows(Player *p, unsigned int attr);
/* 0x004412d0 */ void __cdecl HitDef_SetStateAttr(Player *p);
/* 0x00441310 */ void __cdecl Targets_Release(Player *p, Player *keep);
/* 0x00441410 */ int __cdecl Spark_Create(int sparkno, Player *p, int x, int y, int own, int flip, int guard);
/* 0x004415d0 */ void __cdecl Player_MakeDust(Player *p);
/* 0x00441820 */ DrawParams *__cdecl Player_UpdateDrawParams(Player *p, int bound);
/* 0x00441a20 */ void __cdecl Player_Draw(Player *p);
/* 0x00441b90 */ void __cdecl Player_DrawShadow(Player *p, const int *a, const int *b);
/* 0x00441cf0 */ void __cdecl Player_DrawDebug(Player *p);
/* 0x00442560 */ int __cdecl PalGroups_Add(PalFx *fx, unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4);
/* 0x00442760 */ void __cdecl PalGroups_Build(void);
/* 0x004428e0 */ void __cdecl Match_FreeAssets(void);
/* 0x00442940 */ void __cdecl PalGroups_ResetAll(void);
/* 0x004429a0 */ void __cdecl PalGroups_AddColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int r, int g, int b);
/* 0x00442a90 */ void __cdecl PalGroups_MulColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int r, int g, int b);
/* 0x00442ba0 */ void __cdecl PalGroups_Invert(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int invert);
/* 0x00442c60 */ void __cdecl PalGroups_SetColor(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int color);
/* 0x00442d30 */ void __cdecl PalGroups_Reset(unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4);
/* 0x00442e10 */ void __cdecl PalGroups_Apply(PalFxDef *def, unsigned int m0, unsigned int m1, unsigned int m2, unsigned int m3, unsigned int m4, int global);
/* 0x004430c0 */ int __cdecl Precache_Init(int enabled);
/* 0x00443110 */ int __cdecl Precache_QueueTeam(TeamMember *m);
/* 0x00443180 */ int __cdecl Precache_QueueArcade(int skip_current);
/* 0x004431f0 */ int __cdecl Precache_QueueChar(int char_index);
/* 0x00443270 */ int __cdecl Precache_Start(void);
/* 0x00443360 */ void __cdecl Precache_RequestStop(void);
/* 0x004433a0 */ void __cdecl Precache_Stop(void);
/* 0x00443410 */ void *__cdecl Precache_ThreadMain(void *arg);
/* 0x004435f0 */ int __cdecl Precache_QueueAdd(int *queue, int index);
/* 0x00443630 */ ProjectileList *__cdecl Projectiles_Create(int count, Anim *anim, SpriteSet *sprites);
/* 0x00443740 */ void __cdecl Projectiles_Free(ProjectileList *pl);
/* 0x004437e0 */ int __cdecl Projectiles_Add(ProjectileList *pl, Player *p, const Projectile *proj, int action);
/* 0x004438a0 */ void __cdecl Projectiles_Remove(int slot, ProjectileList *pl);
/* 0x004438e0 */ void __cdecl Projectiles_Update(ProjectileList *pl);
/* 0x00443e30 */ void __cdecl Projectiles_UpdateDraw(ProjectileList *pl);
/* 0x00444040 */ int __cdecl Projectiles_Count(ProjectileList *pl);
/* 0x004440a0 */ int __cdecl Projectiles_CountId(ProjectileList *pl, int id);
/* 0x00444100 */ void __cdecl Projectiles_Draw(ProjectileList *pl);
/* 0x004441b0 */ void __cdecl Projectiles_DrawShadow(ProjectileList *pl);
/* 0x004442a0 */ void __cdecl Projectiles_DrawClsn(ProjectileList *pl);
/* 0x004442f0 */ void __cdecl Projectile_DrawClsn(Projectile *e);
/* 0x00444450 */ int __cdecl Result_Init(ScreenLoop *screen, ResultState *st);
/* 0x00444710 */ int __cdecl Result_End(ScreenLoop *screen, ResultState *st);
/* 0x00444740 */ int __cdecl Result_Update(ScreenLoop *screen, ResultState *st);
/* 0x004447e0 */ int __cdecl Result_Tick(ScreenLoop *screen, ResultState *st);
/* 0x004448f0 */ int __cdecl Result_Draw(ScreenLoop *screen, ResultState *st);
/* 0x00444980 */ int __cdecl Result_Setup(ScreenLoop *screen);
/* 0x00444a40 */ int __cdecl Result_Run(void);
/* 0x00444aa0 */ void __cdecl Sound_PlaySystem(Snd *snd, int group, int channel, int number, double volume);
/* 0x00444b20 */ void __cdecl Sound_PlayPanned(Snd *snd, int group, int x, int number, double volume);
/* 0x00444bf0 */ int __cdecl Sound_Pan(int x);
/* 0x00444c80 */ void __cdecl Sound_SetVolume(double volume);
/* 0x00444d00 */ int __cdecl Sound_PlayerChannel(int channel, Player *p);
/* 0x00444d30 */ int __cdecl Sound_ReadRef(int *group, int *number, Ini *ini, const char *key);
/* 0x00444d50 */ int __cdecl Player_IsActive(Player *p);
/* 0x00444d70 */ int __cdecl StateCtrl_VersionOk(int type, unsigned int version);
/* 0x00444dd0 */ int __cdecl StateCtrl_ParseParams(Ini *ini, StateCtrl *sc, CharData *owner);
/* 0x00448a10 */ int __cdecl StateCtrl_Parse(Ini *ini, StateDef *sd, int number, CharData *owner);
/* 0x00448f50 */ int __cdecl StateCtrl_Run(Player *p, StateCtrl *sc, StateRunCtx *ctx);
/* 0x00449020 */ int __cdecl StateCtrl_Apply(Player *p, StateCtrl *sc, StateRunCtx *ctx);
/* 0x0044c360 */ void __cdecl StateCtrl_Free(StateCtrl *sc);
/* 0x0044c570 */ float __cdecl StateCtrl_Clamp(const char *name, Player *p, StateCtrl *sc, float value, float min, float max);
/* 0x0044c610 */ char *__cdecl Ini_GetValuePrefixed(const char *key, const char *prefix, Ini *ini);
/* 0x0044c640 */ int __cdecl AfterImage_ParseParams(AfterImageParams *ai, Ini *ini, CharData *owner, const char *prefix);
/* 0x0044cdf0 */ int __cdecl AfterImage_Apply(Player *p, AfterImageParams *params, AfterImage *ai, AnimState *anim, StateCtrl *sc);
/* 0x0044d0c0 */ void __cdecl AfterImageParams_Free(AfterImageParams *ai);
/* 0x0044d1c0 */ int __cdecl EnvShake_ParseParams(const char *prefix, EnvShakeParams *es, Ini *ini, CharData *owner);
/* 0x0044d420 */ int __cdecl EnvShake_Apply(EnvShake *out, EnvShakeParams *es, Player *p, StateCtrl *sc);
/* 0x0044d540 */ void __cdecl EnvShakeParams_Free(EnvShakeParams *es);
/* 0x0044d580 */ int __cdecl Explod_ParseParams(ExplodParams *ep, Ini *ini, CharData *owner, int modify);
/* 0x0044dd70 */ void __cdecl ExplodParams_Free(ExplodParams *ep);
/* 0x0044df00 */ int __cdecl PalFx_ParseParams(PalFxParams *pf, Ini *ini, CharData *owner, int hitdef);
/* 0x0044e1a0 */ int __cdecl PalFx_Apply(PalFxDef *dst, PalFxParams *pf, Player *p, StateCtrl *sc);
/* 0x0044e330 */ void __cdecl PalFxParams_Free(PalFxParams *pf);
/* 0x0044e3c0 */ int __cdecl PlaySnd_ParseParams(PlaySndParams *ps, Ini *ini, CharData *owner);
/* 0x0044e5e0 */ void __cdecl PlaySnd_Run(PlaySndParams *ps, Player *p, StateCtrl *sc);
/* 0x0044e890 */ void __cdecl PlaySndParams_Free(PlaySndParams *ps);
/* 0x0044e8f0 */ int __cdecl HitOverride_ParseParams(Ini *ini, CharData *owner, HitOverrideParams *ho);
/* 0x0044ea20 */ int __cdecl HitOverride_Apply(HitOverride *ho, Player *p, HitOverrideParams *params, StateCtrl *sc);
/* 0x0044eac0 */ int __cdecl HitDef_ParseParams(Ini *ini, HitDefParams *hd, CharData *owner);
/* 0x00450300 */ int __cdecl HitDef_SetFromParams(Player *p, HitDefParams *hp, HitDef *hd, StateCtrl *sc);
/* 0x00450f70 */ void __cdecl HitDefParams_Free(HitDefParams *hd);
/* 0x00451370 */ int __cdecl Projectile_ParseParams(ProjectileParams *pp, CharData *owner, Ini *ini);
/* 0x00451da0 */ int __cdecl Projectile_Fire(ProjectileParams *pp, Player *p, StateRunCtx *ctx, Player *owner, StateCtrl *sc);
/* 0x00452ab0 */ void __cdecl ProjectileParams_Free(ProjectileParams *pp);
/* 0x00452c70 */ int __cdecl Helper_ParseParams(HelperParams *hp, Ini *ini, CharData *owner);
/* 0x00453170 */ void __cdecl HelperParams_Free(HelperParams *hp);
/* 0x00453290 */ int __cdecl Pause_ParseParams(PauseParams *pp, Ini *ini, CharData *owner);
/* 0x00453400 */ int __cdecl Pause_Apply(int kind, Player *p, PauseParams *pp, StateCtrl *sc);
/* 0x00453520 */ void __cdecl PauseParams_Free(PauseParams *pp);
/* 0x00453550 */ int __cdecl SuperPause_ParseParams(Ini *ini, CharData *owner, SuperPauseParams *sp);
/* 0x00453790 */ void __cdecl SuperPauseParams_Free(SuperPauseParams *sp);
/* 0x00453810 */ void __cdecl ExprStack_Push(int value, int type);
/* 0x00453860 */ int __cdecl ExprStack_Pop(int *value, int *type);
/* 0x00453890 */ int __cdecl Trigger_IsStageVarString(const Trigger *t);
/* 0x004538c0 */ int __cdecl Trigger_Parse(const char *s, CharData *owner, ExprValue *tokens, char *types, int max, const char **end);
/* 0x00455e60 */ int __cdecl Trigger_ParseCompare(const char *s, Trigger *t, const char **end);
/* 0x004561b0 */ int __cdecl Trigger_ArgCount(int id);
/* 0x004561f0 */ int __cdecl Trigger_TestList(KeyedArray *list, Player *p, int count);
/* 0x004562c0 */ int __cdecl Trigger_TestExpr(Player *p, const Expr *e);
/* 0x00456350 */ void __cdecl Trigger_Eval(Player *p, const Trigger *t);
/* 0x00458830 */ void __cdecl Trigger_Free(Trigger *t);
/* 0x004588a0 */ int __cdecl Trigger_Copy(const Trigger *src, Trigger *dst);
/* 0x004589c0 */ Coord *__cdecl Camera_GetPos(Coord *out);
/* 0x00458a50 */ double __cdecl Screen_GetTopEdge(void);
/* 0x00458aa0 */ double __cdecl Screen_GetBottomEdge(void);
/* 0x00458b00 */ int __cdecl Trigger_PopInt(Player *p, int *out);
/* 0x00458bb0 */ int __cdecl Trigger_PopFloat(float *out);
/* 0x00458c10 */ __declspec(noreturn) void __cdecl ExprStack_Underflow(void);
/* 0x00458c30 */ int __cdecl Cns_CompileInt(Ini *ini, const char *key, Expr *e, CharData *owner, int def);
/* 0x00458c90 */ int __cdecl Cns_CompileFloat(Ini *ini, const char *key, Expr *e, CharData *owner, float def);
/* 0x00458ce0 */ int __cdecl Cns_ParseValues(const char *s, const char *fmt, CharData *owner, const char **end, ...);
/* 0x00458ea0 */ int __cdecl Cns_ParseExprList(const char *s, CharData *owner, const char **end, Expr **out);
/* 0x00459050 */ int __cdecl Cns_ParseArgs(const char *s, const char *fmt, CharData *owner, const char **end, ExprValue *tokens, char *types, int *count, int max);
/* 0x004591f0 */ int __cdecl Cns_ParsePosType(const char *s);
/* 0x004592a0 */ int __cdecl AssertSpecial_ParseFlag(const char *name, int *kind, int *index);
/* 0x00459830 */ int __cdecl Cns_ParseAnimRef(const char *s, int *fight, Expr *e, CharData *owner);
/* 0x004598b0 */ int __cdecl Cns_ParseSoundRef(const char *s, int *fight, Expr *group, Expr *index, CharData *owner);
/* 0x00459930 */ int __cdecl Cns_ParseVelocity(const char *s, Expr *x, Expr *y, CharData *owner);
/* 0x00459ab0 */ int __cdecl HitDefAttr_Parse(const char *s, const char **end);
/* 0x00459fd0 */ unsigned int __cdecl HitFlags_Parse(const char *s);
/* 0x0045a0c0 */ int __cdecl GetHitVar_Lookup(const char *name, int old_version, int *index, int *is_float);
/* 0x0045aa90 */ int __cdecl GetHitVar_Get(Player *p, int index, int *ival, float *fval, int *is_float, int *scaled);
/* 0x0045acf0 */ int __cdecl Cmd_FindLabel(const char *s, int *label, CharData *owner, const char **end);
/* 0x0045add0 */ int __cdecl CnsText_ParseFormat(const char *s, int *len);
/* 0x0045af40 */ CnsText *__cdecl CnsText_Parse(const char *s);
/* 0x0045b210 */ void __cdecl StateRunCtx_Init(StateRunCtx *ctx, Player *p);
/* 0x0045b2d0 */ StateDefs *__cdecl StateDefs_Load(Ini *ini, CharData *owner);
/* 0x0045b470 */ int __cdecl StateDefs_LoadMore(Ini *ini, StateDefs *defs, CharData *owner);
/* 0x0045b5e0 */ int __cdecl StateDef_Parse(const char *header, StateDef *sd, Ini *ini, CharData *owner);
/* 0x0045bce0 */ void __cdecl StateDefs_Free(StateDefs *defs);
/* 0x0045be80 */ int __cdecl Player_RunState(Player *p);
/* 0x0045c2c0 */ void __cdecl Player_RunSpecialState(Player *p, int state, int flags);
/* 0x0045c3d0 */ void __cdecl Player_SetCustomState(Player *p, const CustomStateInfo *info);
/* 0x0045c420 */ void __cdecl Player_ClearCustomState(Player *p);
/* 0x0045c480 */ void __cdecl Player_GetCustomState(Player *p, CustomStateInfo *out);
/* 0x0045c4d0 */ int __cdecl System_LoadMotif(void);
/* 0x0045d8f0 */ void __cdecl System_FreeMotif(void);
/* 0x0045daa0 */ void __cdecl System_ReloadMotif(void);
/* 0x0045dd80 */ void __cdecl System_UnloadMotif(void);
/* 0x0045de80 */ int __cdecl Config_Load(void);
/* 0x0045f100 */ void __cdecl Options_SetDefaults(void);
/* 0x0045f170 */ void __cdecl Options_Load(void);
/* 0x0045f5b0 */ void __cdecl Options_Save(void);
/* 0x0045fa30 */ int __cdecl Config_ReadKeys(Ini *ini, const char *group, int *keys);
/* 0x0045fb90 */ void __cdecl Config_WriteKeys(Ini *ini, const int *keys);
/* 0x0045fc80 */ int __cdecl FontSpec_Read(FontSpec *spec, Ini *ini, const char *key);
/* 0x0045fd00 */ void __cdecl FontSpec_DrawText(const FontSpec *spec, const char *text, int x, int y);
/* 0x0045fda0 */ int __cdecl Game_SetResolution(int width, int height);
/* 0x0045ff40 */ void __cdecl Gfx_SetCoordSpace(int width, int height, int fit);
/* 0x00460010 */ int __cdecl Versus_Init(ScreenLoop *screen, void *state);
/* 0x00460110 */ int __cdecl Versus_End(ScreenLoop *screen, void *state);
/* 0x00460140 */ int __cdecl Versus_Update(ScreenLoop *screen, void *state);
/* 0x004601c0 */ int __cdecl Versus_Tick(ScreenLoop *screen, void *state);
/* 0x00460280 */ int __cdecl Versus_Draw(ScreenLoop *screen, void *state);
/* 0x004605e0 */ int __cdecl Versus_Setup(ScreenLoop *screen);
/* 0x00460680 */ int __cdecl Versus_Run(void);
/* 0x00460750 */ int __cdecl Victory_Init(ScreenLoop *screen, void *state);
/* 0x00460860 */ int __cdecl Victory_End(ScreenLoop *screen, void *state);
/* 0x00460890 */ int __cdecl Victory_Update(ScreenLoop *screen, void *state);
/* 0x00460910 */ int __cdecl Victory_Tick(ScreenLoop *screen, void *state);
/* 0x004609e0 */ int __cdecl Victory_Draw(ScreenLoop *screen, void *state);
/* 0x00460c10 */ int __cdecl Victory_Setup(ScreenLoop *screen);
/* 0x00460cb0 */ int __cdecl Victory_Run(void);
/* 0x00460d30 */ int __cdecl WinScreen_Init(ScreenLoop *screen, void *state);
/* 0x00460fc0 */ int __cdecl WinScreen_End(ScreenLoop *screen, void *state);
/* 0x00460fe0 */ int __cdecl WinScreen_Update(ScreenLoop *screen, void *state);
/* 0x00461080 */ int __cdecl WinScreen_Tick(ScreenLoop *screen, void *state);
/* 0x00461190 */ int __cdecl WinScreen_Draw(ScreenLoop *screen, void *state);
/* 0x004611c0 */ int __cdecl WinScreen_Setup(ScreenLoop *screen);
/* 0x00461280 */ int __cdecl WinScreen_Run(void);
/* 0x004612e0 */ void *__cdecl Mem_Calloc(size_t count, size_t size);
/* 0x00461300 */ int __cdecl File_GetType(const char *path);
/* 0x00461470 */ int __cdecl Char_IsSpace(int c);
/* 0x00461490 */ int __cdecl Char_IsDigit(int c);
/* 0x004614b0 */ int __cdecl Char_ToLower(int c);
/* 0x004614d0 */ int __cdecl Str_StartsWithI(const char *s, const char *prefix);
/* 0x00461530 */ int __cdecl Str_PrefixMatchI(const char *a, const char *b, int n);
/* 0x004615a0 */ int __cdecl Str_PrefixMatch(const char *a, const char *b, int n);
/* 0x004615e0 */ int __cdecl Str_ReadIdent(const char *src, char *dst);
/* 0x00461650 */ void __cdecl Str_ToLowerCopy(const char *src, char *dst);
/* 0x00461680 */ void __cdecl Path_Split(const char *path, char *dir, char *file);
/* 0x00461700 */ void __cdecl Path_Join(const char *file, const char *dir, char *out);
/* 0x004617a0 */ void __cdecl Str_Append(char *dst, const char *src, unsigned int size);
/* 0x004617d0 */ float __cdecl Math_Approach(float cur, float target, float step, float t, float snap);
/* 0x00461890 */ int __cdecl Math_Round(float x);
/* 0x004618c0 */ int __cdecl Pal_Init(Palette *pal, int count, void *colors, int flags);
/* 0x00461ab0 */ void __cdecl Pal_Free(Palette *pal);
/* 0x00461b00 */ PalFx *__cdecl PalFx_Init(PalFx *fx, const PalFx *src);
/* 0x00461b60 */ int __cdecl PalFx_CurIsExternal(void);
/* 0x00461b80 */ PalFx *__cdecl PalFx_GetApplied(void);
/* 0x00461bc0 */ void __cdecl Pal_Convert(Palette *pal);
/* 0x00461c30 */ void __cdecl PalRemap_Compact(PalRemap *r);
/* 0x00461cb0 */ int __cdecl PalRemap_Set(PalRemap *r, const PalRemapEntry *e);
/* 0x00461d40 */ int __cdecl PalRemap_CurIsExternal(void);
/* 0x00461d60 */ void __cdecl PalRemap_Lookup(const PalRemap *r, int group, int index, int *out_group, int *out_index);
/* 0x00461db0 */ int __cdecl PalFx_Combine(PalFx *out, const PalFx *a, const PalFx *b);
/* 0x00461e20 */ PalFx *__cdecl PalFx_ScaleMul(PalFx *fx, int r, int g, int b);
/* 0x00461e70 */ int __cdecl PalFx_InitGlobal(void);
/* 0x00461ee0 */ void __cdecl PalFx_ResetGlobal(void);
/* 0x00461f40 */ void __cdecl PalFx_ScaleGlobalMul(int r, int g, int b);
/* 0x00461fa0 */ unsigned int __cdecl PalFx_Apply565(unsigned int color);
/* 0x00461ff0 */ void __cdecl PalFx_ColorMul(const unsigned char *src, unsigned char *dst, int mul_r, int mul_g, int mul_b);
/* 0x00462080 */ void __cdecl PalFx_ColorAdd(const unsigned char *src, unsigned char *dst, int add_r, int add_g, int add_b);
/* 0x00462110 */ void __cdecl PalFx_ColorDesaturate(const unsigned char *src, unsigned char *dst, int amount);
/* 0x004621b0 */ void __cdecl PalFx_ColorInvert(const unsigned char *src, unsigned char *dst);
/* 0x004621f0 */ void __cdecl PalFx_ApplyColor(const unsigned char *src, unsigned char *dst);
/* 0x00462260 */ int __cdecl Pal_ReadAct(Stream *s, unsigned char *out);
/* 0x00462380 */ void __cdecl Pal_ToRGB565(const unsigned char *rgba, int last, unsigned short *out);
/* 0x004623e0 */ void __cdecl PalFx_ToRGB565(const PalFx *fx, int last, const unsigned char *rgba, unsigned short *out);
/* 0x004627e0 */ Coord *__cdecl Coord_Rescale(Coord *out, double x, double y, double z, double unit, int type, int pad, double new_unit);
/* 0x00462820 */ Coord *__cdecl Coord_Add(Coord *out, double ax, double ay, double az, double aunit, int atype, int apad, double bx, double by, double bz, double bunit, int btype, int bpad);
/* 0x004628a0 */ Coord *__cdecl Coord_Make(Coord *out, double x, double y);
/* 0x004628e0 */ int __cdecl Video_PreInit(void);
/* 0x00462920 */ int __cdecl Video_Deinit(void);
/* 0x00462990 */ int __cdecl Video_SetMode(int game_width, int game_height, int width, int height, int depth, unsigned int flags);
/* 0x00462c50 */ void __cdecl Video_OnResize(int width, int height);
/* 0x00462d80 */ int __cdecl Video_SetFullscreen(int fullscreen);
/* 0x00462f10 */ unsigned int __cdecl Video_MapRGB(int r, int g, int b);
/* 0x00462f70 */ DrawParams *__cdecl DrawParams_Init(DrawParams *dp);
/* 0x00462fb0 */ DrawParams *__cdecl DrawParams_ScalePos(const DrawParams *parent, DrawParams *dp);
/* 0x00463070 */ DrawParams *__cdecl DrawParams_Combine(const DrawParams *parent, DrawParams *dp);
/* 0x00463260 */ void __cdecl DrawParams_FlipToAngle(DrawParams *dp);
/* 0x004633f0 */ void __cdecl DrawParams_FoldScales(DrawParams *dp);
/* 0x00463470 */ DrawParams *__cdecl DrawParams_SetBlend(int mode, DrawParams *dp, int alpha_src, int alpha_dst);
/* 0x00463500 */ SDL_Surface *__cdecl Surface_ToRGBA32(SDL_Surface *src);
/* 0x004635c0 */ void __cdecl Video_SetVRetrace(int vretrace);
/* 0x004635f0 */ int __cdecl Trans_Parse(const char *s, int *alpha_dst, int *alpha_src);
/* 0x004636c0 */ int __cdecl SinTable_Init(void);
/* 0x00463760 */ Anim *__cdecl Anim_Load(const char *path);
/* 0x00463890 */ Anim *__cdecl Anim_Parse(Ini *ini);
/* 0x00463f80 */ int __cdecl Anim_ParseElem(AnimElem *elem, const char *s, const char *header, int index);
/* 0x00464110 */ void __cdecl Anim_ParseElemFlip(const char *s, AnimElem *elem);
/* 0x00464170 */ void __cdecl Anim_ParseElemTrans(const char *s, AnimElem *elem);
/* 0x004642c0 */ void __cdecl Anim_Free(Anim *anim);
/* 0x00464410 */ int __cdecl Anim_Step(AnimState *st);
/* 0x004644b0 */ void __cdecl Anim_Interpolate(AnimState *st);
/* 0x004645b0 */ void __cdecl Anim_SeekElem(AnimState *st);
/* 0x00464620 */ void __cdecl Anim_Reset(AnimState *st);
/* 0x00464660 */ int __cdecl Anim_SetAction(AnimState *st, int action_no);
/* 0x004646e0 */ int __cdecl Anim_SetElem(AnimState *st, int elem);
/* 0x00464770 */ void __cdecl Anim_Draw(AnimState *st, const DrawParams *dp, float sx, float sy);
/* 0x00464970 */ int __cdecl Anim_ParseInterpolate(AnimElem *elem, const char *s);
/* 0x00464b50 */ void __cdecl Anim_DrawAt(AnimState *st, float x, float y, float scale_x, float scale_y, float sx, float sy, unsigned int flip);
/* 0x00464bf0 */ int __cdecl Anim_GetElemStart(AnimState *st, int index);
/* 0x00464c20 */ int __cdecl Anim_ElemAtOffset(AnimState *st, int offset);
/* 0x00464ca0 */ SpriteQueue *__cdecl SpriteQueue_Create(int capacity);
/* 0x00464d20 */ void __cdecl SpriteQueue_Free(SpriteQueue *q);
/* 0x00464d70 */ int __cdecl SpriteQueue_Add(const SpriteQueueItem *item, SpriteQueue *q);
/* 0x00464de0 */ void __cdecl AnimSet_DrawInstance(AnimInstance *inst, int x, int y, float scale);
/* 0x00464e70 */ AnimSet *__cdecl AnimSet_Create(void);
/* 0x00464ec0 */ void __cdecl AnimSet_Free(AnimSet *set);
/* 0x00464f00 */ int __cdecl AnimSet_Add(int x, int y, unsigned int flip, AnimSet *set, Anim *anim, struct SpriteSet *sprites, int action);
/* 0x00464fd0 */ void __cdecl AnimSet_Update(AnimSet *set);
/* 0x00465030 */ void __cdecl AnimSet_Draw(AnimSet *set, int x, int y, unsigned int mask, float scale);
/* 0x004650a0 */ int __cdecl Sprite_Init(unsigned int flags, Sprite *spr, int format, void *data, size_t size, int w, int h, int bpp, int x, int y);
/* 0x004654a0 */ void __cdecl Sprite_Free(Sprite *spr);
/* 0x00465510 */ int __cdecl Sprite_InitRows(Sprite *spr);
/* 0x00465570 */ int __cdecl SpriteSet_LinkPalette(SpriteSet *set, Sprite *spr, int group, int number);
/* 0x004655b0 */ SpriteSet *__cdecl SpriteSet_Create(int sprite_count, int palette_count);
/* 0x00465610 */ int __cdecl SpriteSet_AddSprite(SpriteSet *set, const Sprite *spr, int group, int number);
/* 0x00465670 */ int __cdecl SpriteSet_AddPalette(SpriteSet *set, const Palette *pal, int group, int number);
/* 0x004656e0 */ int __cdecl SpriteSet_Free(SpriteSet *set);
/* 0x00465810 */ Sprite *__cdecl SpriteSet_FindSprite(SpriteSet *set, int group, int number);
/* 0x00465840 */ Palette *__cdecl SpriteSet_GetPalette(SpriteSet *set, int index);
/* 0x00465920 */ Palette *__cdecl SpriteSet_FindPalette(SpriteSet *set, int group, int number);
/* 0x00465950 */ Palette *__cdecl SpriteSet_GetPaletteConverted(SpriteSet *set, int index);
/* 0x00465970 */ void *__cdecl Sprite_GetPixels(SpriteSet *set, int sprite);
/* 0x004659d0 */ void __cdecl Sprite_Draw(SpriteSet *set, int sprite, const DrawParams *dp);
/* 0x00465a00 */ void __cdecl Sprite_DrawWithPalette(SpriteSet *set, int sprite, int pal_index, const DrawParams *dp);
/* 0x00465a80 */ void __cdecl Sprite_DrawSoft(SpriteSet *set, int sprite, const DrawParams *dp);
/* 0x00465b90 */ void __cdecl Sprite_BlitSurface(SpriteSet *set, int sprite, int x, int y);
/* 0x00465c10 */ void __cdecl Sprite_DrawAt(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y);
/* 0x00465cb0 */ void __cdecl Sprite_DrawAdd(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y);
/* 0x00465d60 */ void __cdecl Sprite_DrawAlpha(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y, int alpha_src, int alpha_dst);
/* 0x00465e20 */ void __cdecl Sprite_DrawSub(SpriteSet *set, int sprite, float x, float y, float scale_x, float scale_y);
/* 0x00465ed0 */ SpriteSet *__cdecl SpriteSet_FromPixels(int bpp, void *pixels, int w, int h, Palette *pal, unsigned int flags);
/* 0x00465fe0 */ void __cdecl SpriteSet_MemUse(SpriteSet *set, unsigned int *sys_bytes, unsigned int *tex_bytes);
/* 0x00466100 */ void *__cdecl Sprite_Downscale(const void *pixels, int w, int h, int step_x, int step_y, int bpp);
/* 0x004661e0 */ TextRing *__cdecl TextRing_Create(int width, int count);
/* 0x00466280 */ void __cdecl TextRing_Free(TextRing *ring);
/* 0x004662b0 */ void __cdecl TextRing_SetPrintf(TextRing *ring, const char *fmt, ...);
/* 0x00466320 */ void __cdecl TextRing_Printf(TextRing *ring, const char *fmt, ...);
/* 0x00466380 */ void __cdecl TextRing_VPrintf(TextRing *ring, va_list ap, const char *fmt);
/* 0x004663e0 */ void __cdecl TextRing_Append(TextRing *ring, const char *text);
/* 0x00466480 */ void __cdecl TextRing_Dump(TextRing *ring, char *out, int max_lines, unsigned int size);
/* 0x00466550 */ KeyedArray *__cdecl KeyedArray_Create(int capacity, int elem_size);
/* 0x00466600 */ void __cdecl KeyedArray_Free(KeyedArray *list);
/* 0x00466630 */ int __cdecl KeyedArray_Add(KeyedArray *list, const void *elem, int key1, int key2);
/* 0x004666b0 */ void __cdecl KeyedArray_Remove(int index, KeyedArray *list);
/* 0x00466740 */ int __cdecl KeyedArray_Grow(KeyedArray *list, int capacity);
/* 0x004667d0 */ int __cdecl KeyedArray_FreeSlot(KeyedArray *list);
/* 0x00466840 */ int __cdecl KeyedArray_Find(KeyedArray *list, int key1, int key2);
/* 0x00466970 */ void *__cdecl KeyedArray_Get(KeyedArray *list, int key1, int key2);
/* 0x00466990 */ void *__cdecl KeyedArray_First(KeyedArray *list);
/* 0x004669b0 */ void *__cdecl KeyedArray_Next(KeyedArray *list);
/* 0x004669e0 */ void __cdecl KeyedArray_ClearCache(KeyedArray *list);
/* 0x00466a30 */ int __cdecl Stream_GetByte(Stream *s);
/* 0x00466a70 */ char *__cdecl Stream_Gets(Stream *s, char *buf, int size);
/* 0x00466b20 */ size_t __cdecl Stream_Read(void *buf, size_t size, size_t count, Stream *s);
/* 0x00466bf0 */ int __cdecl Stream_Eof(Stream *s);
/* 0x00466c20 */ Stream *__cdecl Stream_OpenFile(const char *path, const char *mode);
/* 0x00466c90 */ Stream *__cdecl Stream_OpenMemory(void *data, int size);
/* 0x00466ce0 */ int __cdecl Stream_ZipLoad(Stream *s, const char *name);
/* 0x00466db0 */ int __cdecl Stream_ZipUnload(Stream *s);
/* 0x00466e20 */ Stream *__cdecl Stream_OpenZip(const char *zip_path, const char *name);
/* 0x00466e90 */ void __cdecl Stream_FreeBuffer(Stream *s);
/* 0x00466ee0 */ int __cdecl Stream_Close(Stream *s);
/* 0x00466f50 */ int __cdecl Stream_Seek(Stream *s, long offset, int origin);
/* 0x00466ff0 */ long __cdecl Stream_Tell(Stream *s);
/* 0x00467020 */ long __cdecl Stream_GetSize(Stream *s);
/* 0x00467130 */ int __cdecl Stream_ReadU8(Stream *s, unsigned char *out);
/* 0x00467180 */ int __cdecl Stream_ReadU16(Stream *s, unsigned short *out);
/* 0x00467210 */ int __cdecl Stream_ReadU32(Stream *s, unsigned int *out);
/* 0x00467420 */ SDL_RWops *__cdecl Stream_ToRWops(Stream *s);
/* 0x004674c0 */ SDL_RWops *__cdecl Stream_ToSDLRWops(Stream *s);
/* 0x00467510 */ SpriteSet *__cdecl Sff_Load(const char *path);
/* 0x00467680 */ SpriteSet *__cdecl Sff_LoadStream(Stream *s);
/* 0x004677d0 */ SpriteSet *__cdecl Sff1_Load(const Sff1Header *hdr, Stream *s);
/* 0x00467b30 */ SpriteSet *__cdecl Sff_LoadSelected(Stream *s, const int *select);
/* 0x00467c70 */ SpriteSet *__cdecl Sff1_LoadSelected(Stream *s, const Sff1Header *hdr, const int *select);
/* 0x004680f0 */ SpriteSet *__cdecl Sff2_Load(Stream *s, const Sff2Header *hdr, const int *select);
/* 0x00468c60 */ void *__cdecl Pcx_Decode(Palette *pal, int *w_out, Stream *s, int *h_out, int *bpp_out, size_t *size_out, int flags);
/* 0x00469410 */ SpriteSet *__cdecl Pcx_Load(Stream *s, int flags);
/* 0x00469550 */ int __cdecl Sff_ReadHeader(Stream *s, SffHeader *hdr, unsigned int *version);
/* 0x00469600 */ int __cdecl Sff1_ReadHeader(Sff1Header *hdr, Stream *s);
/* 0x004696c0 */ int __cdecl Sff2_ReadHeader(Sff2Header *hdr, Stream *s);
/* 0x00469810 */ int __cdecl Sff1_ReadSubHeader(Sff1SubHeader *sub, Stream *s);
/* 0x004698c0 */ int __cdecl Sff2_ReadSpriteNode(Sff2SpriteNode *node, Stream *s);
/* 0x004699d0 */ int __cdecl Sff2_ReadPaletteNode(Sff2PaletteNode *node, Stream *s);
/* 0x00469a50 */ void __cdecl Ini_Free(Ini *ini);
/* 0x00469b00 */ Ini *__cdecl Ini_Open(const char *path);
/* 0x00469b40 */ Ini *__cdecl Ini_OpenMode(const char *path, int mode);
/* 0x00469b80 */ Ini *__cdecl Ini_OpenEditable(const char *path);
/* 0x00469bc0 */ Ini *__cdecl Ini_Parse(Stream *s, int mode);
/* 0x0046a150 */ int __cdecl Ini_Save(Ini *ini, const char *path);
/* 0x0046a1d0 */ int __cdecl Ini_FindGroup(Ini *ini, const char *name);
/* 0x0046a2f0 */ int __cdecl Ini_SelectGroup(Ini *ini, const char *name);
/* 0x0046a360 */ int __cdecl Ini_SelectGroupIndex(Ini *ini, int index);
/* 0x0046a3b0 */ void __cdecl Ini_Rewind(Ini *ini);
/* 0x0046a3f0 */ const char *__cdecl Ini_NextGroup(Ini *ini);
/* 0x0046a450 */ int __cdecl Ini_CountGroups(Ini *ini, const char *prefix);
/* 0x0046a4e0 */ int __cdecl Ini_FindKeyLine(Ini *ini, const char *key);
/* 0x0046a5e0 */ char *__cdecl Ini_GetValue(Ini *ini, const char *key);
/* 0x0046a740 */ char *__cdecl Ini_GetValueDup(Ini *ini, const char *key);
/* 0x0046a7a0 */ char *__cdecl Ini_GetNextValue(Ini *ini, const char *key);
/* 0x0046a8a0 */ int __cdecl Ini_GetFloat(Ini *ini, const char *key, float *out);
/* 0x0046a8d0 */ void __cdecl Ini_GetDoubleDef(Ini *ini, const char *key, double *out, double def);
/* 0x0046a900 */ int __cdecl Ini_GetInt(Ini *ini, const char *key, int *out);
/* 0x0046a930 */ void __cdecl Ini_GetIntDef(Ini *ini, const char *key, int *out, int def);
/* 0x0046a960 */ int __cdecl Ini_Scanf(Ini *ini, const char *key, const char *fmt, ...);
/* 0x0046a990 */ int __cdecl Ini_GetString(Ini *ini, const char *key, char *out, int size);
/* 0x0046a9c0 */ const char *__cdecl Ini_FirstLine(Ini *ini);
/* 0x0046aa30 */ const char *__cdecl Ini_NextLine(Ini *ini);
/* 0x0046aa70 */ const char *__cdecl Ini_PrevLine(Ini *ini);
/* 0x0046aab0 */ int __cdecl Ini_ReadKeyValue(Ini *ini, char *key, char *value);
/* 0x0046abe0 */ int __cdecl Ini_SourceLine(int line, Ini *ini);
/* 0x0046ac60 */ int __cdecl Ini_SetValuef(Ini *ini, const char *key, const char *fmt, ...);
/* 0x0046ae10 */ int __cdecl Str_Unquote(char *s);
/* 0x0046af30 */ int __cdecl Str_VScanf(const char *str, va_list ap, const char *fmt);
/* 0x0046b1d0 */ int __cdecl Ini_SetLanguage(const char *lang);
/* 0x0046b210 */ void __cdecl Events_Poll(void);
/* 0x0046b350 */ void __cdecl Input_OnKey(int key, int pressed);
/* 0x0046b3d0 */ void __cdecl Input_OnPadButton(int button, int pressed);
/* 0x0046b450 */ int __cdecl Events_Init(void);
/* 0x0046b500 */ void __cdecl Events_Deinit(void);
/* 0x0046b560 */ void __cdecl Input_ResetState(void);
/* 0x0046b670 */ void __cdecl Input_QueueCode(int code);
/* 0x0046b6e0 */ int __cdecl Input_CodePending(void);
/* 0x0046b710 */ int __cdecl Input_TranslateKey(int key);
/* 0x0046b740 */ int __cdecl Input_TranslateJoy(int button);
/* 0x0046b770 */ int __cdecl IntArray_Init(IntArray *a, int capacity);
/* 0x0046b7a0 */ void __cdecl Fade_Update(Fade *fade, int advance);
/* 0x0046b830 */ void __cdecl Fade_DrawBars(const Fade *fade);
/* 0x0046b930 */ SpriteSet *__cdecl Screen_Create(int depth, int width, int height);
/* 0x0046ba90 */ int __cdecl Gfx_SetRenderTarget(SpriteSet *target, int index);
/* 0x0046bc10 */ SDL_Surface *__cdecl SpriteSet_GetSurface(const SpriteSet *set, int sprite);
/* 0x0046bc30 */ SDL_Surface *__cdecl Gfx_GetScreenSurface(void);
/* 0x0046bcf0 */ void __cdecl Gfx_Present(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0046bfc0 */ void __cdecl Gfx_SetClipRect(int x1, int y1, int x2, int y2);
/* 0x0046c0c0 */ void __cdecl Gfx_SetClipRectScaled(float x1, float y1, float x2, float y2, float sx, float sy);
/* 0x0046c190 */ void __cdecl Gfx_SelectRenderer(int mode);
/* 0x0046c270 */ void __cdecl Log_Close(void);
/* 0x0046c2e0 */ void __cdecl Log_SetMirror(TextRing *ring);
/* 0x0046c320 */ void __cdecl Log_VWrite(Log *log, va_list ap, int level, const char *fmt);
/* 0x0046c3c0 */ void __cdecl MatchLog_Printf(int level, const char *fmt, ...);
/* 0x0046c3e0 */ void __cdecl Font_DrawLine(const DrawParams *dp, Font *font, const char *text, int len, int bank, int r, int g, int b);
/* 0x0046c6f0 */ void __cdecl Font_DrawLineTTF(const char *text, const DrawParams *dp, int len, Font *font, int r, int g, int b);
/* 0x0046ca80 */ void __cdecl Font_DrawTextEx(const char *text, Font *font, const DrawParams *dp, int align, int bank, int r, int g, int b);
/* 0x0046cbc0 */ void __cdecl Font_DrawLines(const char *text, int len, Font *font, const DrawParams *dp, int align, int bank, int r, int g, int b);
/* 0x0046ce40 */ void __cdecl Font_DrawText(const char *text, Font *font, int x, int y, int align, int bank, int r, int g, int b);
/* 0x0046ced0 */ void __cdecl Font_DrawTextScaled(const char *text, Font *font, float x, float y, float scale_x, float scale_y, int align, int bank, int r, int g, int b);
/* 0x0046cf90 */ void __cdecl Font_Printf(Font *font, int x, int y, int align, int bank, int r, int g, int b, const char *fmt, ...);
/* 0x0046d030 */ Font *__cdecl Font_Load(const char *path, int height);
/* 0x0046d100 */ Font *__cdecl Font_LoadStream(Stream *s, const char *dir, int height);
/* 0x0046d260 */ Font *__cdecl Font_LoadFnt1(Stream *s, const FntHeader *hdr);
/* 0x0046d700 */ int __cdecl Font_SplitGlyphs(Font *font, const FntMap *map);
/* 0x0046d8c0 */ void __cdecl Font_CopyGlyph(const Sprite *image, void *dst, int x, int w);
/* 0x0046d910 */ Font *__cdecl Font_LoadDef(Stream *s, const char *dir, int height);
/* 0x0046dc60 */ int __cdecl Font_Free(Font *font);
/* 0x0046dca0 */ int __cdecl Font_GlyphWidth(int c, Font *font);
/* 0x0046dd60 */ int __cdecl Font_TextWidth(const char *s, Font *font);
/* 0x0046de60 */ int __cdecl Font_WrapLength(const char *text, Font *font, int max_width, int word_wrap);
/* 0x0046dfd0 */ int __cdecl Font_WrapLengthTTF(Font *font, const char *text, int max_width, int word_wrap);
/* 0x0046e120 */ void __cdecl Fonts_Load(Ini *ini, Font **fonts, const char *const *dirs);
/* 0x0046e230 */ char *__cdecl Font_DupLine(int max, const char *s, int *out_len);
/* 0x0046e290 */ void __cdecl Keyboard_HandleKey(int key, int pressed);
/* 0x0046e2f0 */ int __cdecl Keyboard_Init(void);
/* 0x0046e350 */ void __cdecl Keyboard_SyncState(void);
/* 0x0046e3b0 */ int __cdecl Keyboard_SetEventHandler(KeyEventFn fn);
/* 0x0046e3e0 */ void __cdecl Keyboard_QueueCode(int code);
/* 0x0046e430 */ int __cdecl Sound_Init(int buffer_log2, int channels, int bits, int mix_channels, int sample_rate, int resample_method, int resample_quality);
/* 0x0046e730 */ void __cdecl Sound_Deinit(void);
/* 0x0046e7c0 */ Snd *__cdecl Snd_Create(int capacity);
/* 0x0046e810 */ int __cdecl Snd_Add(Snd *snd, int group, int number, struct Mix_Chunk *chunk);
/* 0x0046e870 */ void __cdecl Snd_Free(Snd *snd);
/* 0x0046e920 */ int __cdecl Sound_FindFreeSlot(void);
/* 0x0046e990 */ int __cdecl Sound_PlayOnSlot(int slot, Snd *snd, int group, int number, double volume);
/* 0x0046ea50 */ int __cdecl Sound_PrepareSlot(int slot, Snd *snd, int group, int number);
/* 0x0046eb20 */ void __cdecl Sound_PlaySlot(int slot);
/* 0x0046eb80 */ void __cdecl Sound_SetSlotVolume(int slot, double volume);
/* 0x0046ebf0 */ void __cdecl Sound_RestoreVolumes(void);
/* 0x0046ec50 */ void __cdecl Sound_SetSlotPan(int slot, int pan);
/* 0x0046eca0 */ void __cdecl Sound_SetSlotLoop(int slot, int loop);
/* 0x0046ecc0 */ void __cdecl Sound_ReleaseStopped(void);
/* 0x0046ed30 */ int __cdecl Sound_Play(int id, Snd *snd, int group, int number, double volume);
/* 0x0046ed90 */ int __cdecl Sound_Prepare(int id, Snd *snd, int group, int number);
/* 0x0046ede0 */ void __cdecl Sound_Stop(int id);
/* 0x0046ee30 */ int __cdecl Sound_IsPlaying(int id);
/* 0x0046ee90 */ int __cdecl Sound_FindSlot(int id);
/* 0x0046eec0 */ int __cdecl Sound_GetSlot(int id);
/* 0x0046ef00 */ int __cdecl Sound_FindFreeChannel(void);
/* 0x0046ef30 */ Snd *__cdecl Snd_Load(const char *path);
/* 0x0046f120 */ struct Mix_Chunk *__cdecl Snd_DecodeWav(Stream *s, int *status);
/* 0x0046f150 */ struct Mix_Chunk *__cdecl Snd_DecodeWavResampled(Stream *s, int *status);
/* 0x0046f520 */ struct Mix_Chunk *__cdecl Snd_DecodeWavSDL(Stream *s, int *status);
/* 0x0046f620 */ Snd *__cdecl Snd_Parse(Stream *s, int *status);
/* 0x0046f860 */ Stream *__cdecl Snd_RepairWav(Stream *s);
/* 0x0046fb50 */ void __cdecl Bgm_FreeSubsystems(void);
/* 0x0046fbb0 */ int __cdecl Bgm_RegisterSubsystem(const BgmSubsystem *sub);
/* 0x0046fc80 */ void __cdecl BgmPlugins_Free(DynArray *plugins);
/* 0x0046fd30 */ DynArray *__cdecl BgmPlugins_Load(Ini *ini);
/* 0x00470130 */ int __cdecl Bgm_ExtensionInList(const char *ext, const char *list);
/* 0x00470200 */ const char *__cdecl Bgm_GetExtension(const char *path);
/* 0x00470240 */ void __cdecl Bgm_GetLoopInfo(const BgmInfo *info, int *loop);
/* 0x00470270 */ const BgmSubsystem *__cdecl Bgm_FindSubsystem(const char *path);
/* 0x00470310 */ int __cdecl Bgm_Init(DynArray *plugins, int resample_method, int resample_quality, double volume);
/* 0x00470740 */ void __cdecl Bgm_Deinit(void);
/* 0x004707f0 */ double __cdecl Bgm_ScaleVolume(double volume);
/* 0x00470820 */ void __cdecl Bgm_SetVolume(double volume);
/* 0x00470860 */ int __cdecl Bgm_ReadInfo(Ini *ini, const char *key, BgmInfo *info);
/* 0x00470910 */ int __cdecl Bgm_ReadInfoKeys(BgmInfo *info, const char *key, Ini *ini, const char *volume_key, const char *loop_key, const char *start_key, const char *end_key);
/* 0x00470a50 */ void __cdecl Bgm_Pause(void);
/* 0x00470ab0 */ void __cdecl Bgm_Unpause(void);
/* 0x00470b10 */ void __cdecl Bgm_SetStreamVolume(double volume);
/* 0x00470bc0 */ int __cdecl Bgm_Load(const BgmInfo *info);
/* 0x00470c10 */ int __cdecl Bgm_Play(const BgmInfo *info);
/* 0x00470c60 */ void __cdecl Bgm_Stop(void);
/* 0x00470ca0 */ Bg *__cdecl Bg_Load(const char *name, Ini *ini, const char *dir, SpriteSet *sprites, Anim *anim);
/* 0x00471980 */ int __cdecl BgCtrl_Load(const char *name, Bg *bg, Ini *ini);
/* 0x00471f30 */ Bg *__cdecl Bg_Create(int count);
/* 0x00471fa0 */ void __cdecl Bg_Free(Bg *bg);
/* 0x004721e0 */ void __cdecl Bg_ShiftWindows(Bg *bg, int dx);
/* 0x00472250 */ int __cdecl BgElem_AddSprite(int type, Bg *bg, int index, float start_x, float start_y, float delta_x, float delta_y, int trans, int alpha_src, int alpha_dst, int mask, int group, int number);
/* 0x004723a0 */ int __cdecl BgElem_AddAnim(Bg *bg, int index, int type, float start_x, float start_y, float delta_x, float delta_y, int trans, int alpha_src, int alpha_dst, int action);
/* 0x004724e0 */ void __cdecl Bg_SetScale(Bg *bg, float scale_x, float scale_y);
/* 0x00472540 */ void __cdecl Bg_SetCamera(Bg *bg, float x, float y, float offset_x, float offset_y, float zoom, float center_x, float center_y);
/* 0x004728a0 */ void __cdecl Bg_Draw(Bg *bg, int layer);
/* 0x00472930 */ void __cdecl Bg_Tick(Bg *bg);
/* 0x00472a50 */ void __cdecl BgCtrl_Run(Bg *bg);
/* 0x00472d20 */ void __cdecl Bg_Reset(Bg *bg);
/* 0x00472e40 */ void __cdecl Bg_SetLegacy(Bg *bg);
/* 0x00472eb0 */ BgElem *__cdecl Bg_FindById(Bg *bg, int id);
/* 0x00472f10 */ int __cdecl BgElem_InitParallax(int tile, BgElem *e, Bg *bg, float width_top, float width_bottom, float yscale_start, float yscale_delta, int by_width);
/* 0x00473220 */ void __cdecl BgElem_Draw(BgElem *e, Bg *bg);
/* 0x004734b0 */ void __cdecl BgElem_DrawTiled(BgElem *e, const DrawParams *dp, Bg *bg);
/* 0x00473800 */ void __cdecl BgElem_DrawParallax(BgElem *e, Bg *bg, const DrawParams *dp);
/* 0x004738d0 */ void __cdecl BgElem_DrawParallaxGL(BgElem *e, Bg *bg, const DrawParams *dp);
/* 0x00473c80 */ void __cdecl BgElem_DrawParallaxSoftMasked(Bg *bg, BgElem *e, float scale_x, float scale_y);
/* 0x00474110 */ void __cdecl BgElem_DrawParallaxSoft(Bg *bg, BgElem *e, float scale_x, float scale_y);
/* 0x004745a0 */ float __cdecl Bg_Fmod(float x, float y);
/* 0x004745f0 */ int __cdecl Pads_Init(void);
/* 0x004746d0 */ void __cdecl Input_DeinitPads(void);
/* 0x00474750 */ void __cdecl Pads_SetButton(int code, int pressed);
/* 0x00474790 */ void __cdecl Pads_HandleEvent(const SDL_Event *ev);
/* 0x00474a60 */ void __cdecl Pads_QueueCode(int code);
/* 0x00474ac0 */ int __cdecl Pads_PopCode(void);
/* 0x00474b10 */ int __cdecl Pads_CodePending(void);
/* 0x00474b30 */ long long __cdecl Input_FindKey(int *const *maps, int count, int code);
/* 0x00474cd0 */ Cutscene *__cdecl Storyboard_Load(const char *path);
/* 0x00474e40 */ Cutscene *__cdecl Storyboard_Create(const char *dir, Ini *ini);
/* 0x00474f50 */ Storyboard *__cdecl Storyboard_Parse(const char *dir, Ini *ini);
/* 0x00475d90 */ void __cdecl Storyboard_SetScale(Storyboard *sb, float scale_x, float scale_y);
/* 0x00475de0 */ void __cdecl Storyboard_Reset(Storyboard *sb);
/* 0x00475eb0 */ int __cdecl Storyboard_Tick(Storyboard *sb);
/* 0x00476440 */ void __cdecl Storyboard_ApplyFade(Storyboard *sb);
/* 0x00476570 */ void __cdecl Storyboard_Draw(Storyboard *sb);
/* 0x004769c0 */ void __cdecl Storyboard_Close(Cutscene *cs);
/* 0x004769f0 */ void __cdecl Storyboard_Free(Storyboard *sb);
/* 0x00476bf0 */ int __cdecl Storyboard_Utf8Prefix(const char *s, int chars);
/* 0x00476c90 */ int __cdecl Timer_Init(void);
/* 0x00476d70 */ int64_t __cdecl Timer_GetTicks(void);
/* 0x00476de0 */ float __cdecl Timer_GetFps(void);
/* 0x00476ec0 */ void __cdecl Timer_Reset(void);
/* 0x00476f30 */ int64_t __cdecl Timer_GetFrame(void);
/* 0x00476f90 */ void __cdecl Timer_ResetFrame(void);
/* 0x00476ff0 */ void __cdecl Timer_SetFps(int fps);
/* 0x00477040 */ int64_t __cdecl Timer_GetFrameTimeLeft(void);
/* 0x004770d0 */ int __cdecl Timer_WaitFrame(void);
/* 0x00477140 */ void __cdecl Png_OnWarning(png_structp png, png_const_charp msg);
/* 0x00477160 */ void __cdecl Png_OnError(png_structp png, png_const_charp msg);
/* 0x00477180 */ int __cdecl Png_WriteSurface(SDL_Surface *surface, FILE *fp);
/* 0x00477380 */ void __cdecl Png_ReadData(png_structp png, png_bytep data, png_size_t length);
/* 0x004773a0 */ void __cdecl Png_WriteData(png_structp png, png_bytep data, png_size_t length);
/* 0x00477400 */ unsigned char *__cdecl Png_EncodeToBuffer(const void *pixels, int width, int height, int bpp, int *size_out);
/* 0x00477680 */ int __cdecl Png_ReadStream(Stream *s, void *dest);
/* 0x004778b0 */ unsigned char *__cdecl Png_Encode(const void *pixels, int width, int height, int bpp, int *size_out);
/* 0x00477a00 */ unsigned char *__cdecl Png_Encode8(const void *pixels, int width, int height, int *size_out);
/* 0x00477a20 */ unsigned char *__cdecl Png_Encode24(const void *pixels, int width, int height, int *size_out);
/* 0x00477a40 */ unsigned char *__cdecl Png_Encode32(const void *pixels, int width, int height, int *size_out);
/* 0x00477a60 */ int __cdecl Png_Decode(const void *data, int size, void *dest);
/* 0x00477ac0 */ void __cdecl Input_Clear(InputBuf *buf);
/* 0x00477b10 */ void __cdecl Input_Resync(InputBuf *buf, const int *keys);
/* 0x00477c40 */ void __cdecl Input_SetFlipped(InputBuf *buf, int flipped);
/* 0x00477d00 */ void __cdecl PlayerInput_Push(InputBuf *in, int code);
/* 0x00477d70 */ unsigned int __cdecl Input_Pop(InputBuf *buf);
/* 0x00477da0 */ int __cdecl Keys_IsAttackCode(const int *keys, unsigned int code);
/* 0x00477df0 */ int __cdecl Keys_IsButtonCode(const int *keys, int code);
/* 0x00477e50 */ unsigned int __cdecl Keys_FindCode(unsigned int code, const int *keys);
/* 0x00477ec0 */ void __cdecl SpriteCache_Remove(void *data);
/* 0x00477ef0 */ int __cdecl SpriteCache_Get(void *data, int format, int size, void **out);
/* 0x00477f80 */ DynArray *__cdecl DynArray_Create(int capacity, int elem_size);
/* 0x00477ff0 */ void *__cdecl DynArray_Push(DynArray *a, const void *elem);
/* 0x00478040 */ void __cdecl Stub_Nop(void);
/* 0x00478050 */ int __cdecl Lua_SetPackagePath(void);
/* 0x004780b0 */ void __cdecl Lua_CallFunction(struct lua_State *L, const char *table, const char *func, const char *sig, ...);
/* 0x004783f0 */ int __cdecl LuaTable_GetNumber(struct lua_State *L, const char *key, double *out);
/* 0x00478460 */ int __cdecl LuaTable_GetInt(struct lua_State *L, const char *key, int *out);
/* 0x004784d0 */ int __cdecl LuaTable_GetBool(struct lua_State *L, const char *key, int *out);
/* 0x00478540 */ int __cdecl LuaTable_GetRemapPal(struct lua_State *L);
/* 0x004785a0 */ int __cdecl LuaTable_GetIntAt(struct lua_State *L, int index, int *out);
/* 0x00478610 */ int __cdecl CmdState_AddInput(unsigned int key, CmdState *st, InputBuf *buf, unsigned int frame);
/* 0x004787e0 */ int __cdecl CmdState_Update(const CmdSet *cmds, unsigned char *active, CmdState *st, unsigned int frame, unsigned int type_mask);
/* 0x00478900 */ unsigned int __cdecl Input_GetHeld(const InputBuf *buf);
/* 0x004789a0 */ unsigned int __cdecl Cmd_KeyFromInput(unsigned int code);
/* 0x00478a50 */ int __cdecl CmdStep_Match(unsigned int keys, const CmdState *st, int i);
/* 0x00478b30 */ int __cdecl Cmd_Match(const CmdEntry *cmd, const CmdState *st, unsigned int *frame_out);
/* 0x00478c50 */ void __cdecl CmdState_Push(CmdState *st, unsigned int keys, unsigned int frame, int charge);
/* 0x00478cc0 */ int __cdecl HashTable_Grow(HashTable *ht);
/* 0x00478d60 */ HashTable *__cdecl HashTable_Create(const HashParams *params);
/* 0x00478e40 */ HashEntry *__cdecl HashTable_FindEntry(unsigned int b, unsigned int hash, const HashTable *ht, void *key);
/* 0x00478eb0 */ int __cdecl HashTable_Insert(HashTable *ht, void *key, void *value);
/* 0x00479070 */ void *__cdecl HashTable_Lookup(const HashTable *ht, void *key);
/* 0x004790b0 */ void __cdecl HashTable_Free(HashTable *ht);
/* 0x00479140 */ CountedStr *__cdecl CountedStr_Make(const char *s);
/* 0x00479190 */ void *__cdecl CountedStr_Copy(const void *key, void *ud1, void *ud2);
/* 0x004791d0 */ unsigned int __cdecl CountedStr_Hash(const void *key, void *ud1, void *ud2);
/* 0x004791f0 */ void __cdecl Hash_FreeCallback(void *p, void *ud1, void *ud2);
/* 0x00479200 */ int __cdecl CountedStr_Differ(const CountedStr *a, const CountedStr *b);
/* 0x00479260 */ int __cdecl CountedStr_Compare(const void *a, const void *b, void *ud1, void *ud2);
/* 0x00479270 */ CmdTable *__cdecl CmdTable_Create(void);
/* 0x00479320 */ int __cdecl CmdTable_Add(const char *name, CmdHandlerFn fn, void *arg, CmdTable *t);
/* 0x004793b0 */ int __cdecl CmdTable_Exec(const char *line, CmdTable *t);
/* 0x00479550 */ void *__cdecl CmdTableEntry_Copy(const void *value, void *ud1, void *ud2);
/* 0x00479580 */ int __cdecl Lang_SetCode(const char *code);
/* 0x004795c0 */ Clsn *__cdecl Clsn_Create(int count);
/* 0x00479640 */ void __cdecl Clsn_Release(Clsn *clsn);
/* 0x00479670 */ int __cdecl Clsn_AddBox(Clsn *clsn, int x1, int y1, int x2, int y2);
/* 0x004796e0 */ Clsn *__cdecl Clsn_Parse(const char *line, Ini *ini);
/* 0x004799e0 */ int __cdecl Clsn_HitBox(const Clsn *clsn, const ClsnBox *box, int dx, int dy);
/* 0x00479a40 */ int __cdecl Clsn_HitBoxFlip(const Clsn *clsn, const ClsnBox *box, int dx, int dy);
/* 0x00479ab0 */ int __cdecl Clsn_HitBoxFlipBoth(const Clsn *clsn, const ClsnBox *box, int dx, int dy);
/* 0x00479b20 */ int __cdecl Clsn_HitClsn(const Clsn *a, const Clsn *b, int dx, int dy);
/* 0x00479b80 */ int __cdecl Clsn_Collide(int flip_a, int flip_b, const Clsn *a, const Clsn *b, int dx, int dy);
/* 0x00479ca0 */ GlTex *__cdecl GlTex_Create(void);
/* 0x00479cf0 */ int __cdecl GlTex_Free(GlTex *tex);
/* 0x00479d20 */ int __cdecl GlTex_Init(GlTex *tex);
/* 0x00479d70 */ int __cdecl GlTex_Delete(GlTex *tex);
/* 0x00479e00 */ int __cdecl GlTex_Alloc(GlTex *tex, int w, int h);
/* 0x00479ec0 */ int __cdecl GlTex_Upload2D(int h, int bpp, GlTex *tex, const void *pixels, int w);
/* 0x00479f60 */ int __cdecl GlTex_UploadRGBA(int h, int w, GlTex *tex, const void *pixels);
/* 0x00479fe0 */ int __cdecl GlTex_Upload(GlTex *tex, int h, int internal_format, int w, int format, const void *pixels);
/* 0x0047a0e0 */ int __cdecl GlTex_Upload1D(GlTex *tex, int width, const void *pixels);
/* 0x0047a1f0 */ int __cdecl GlTex_AllocDepth(GlTex *tex);
/* 0x0047a260 */ int __cdecl GlScreen_Upload(int h, int w, const void *pixels);
/* 0x0047a320 */ void __cdecl GlScreen_Draw(int w, int h);
/* 0x0047a460 */ unsigned int __cdecl Gl_TexBytes(unsigned int pixels, int format);
/* 0x0047a640 */ unsigned char *__cdecl Gl_Premultiply(int bytes, const unsigned char *src);
/* 0x0047a6d0 */ int __cdecl Gl_Init(int framebuffers);
/* 0x0047a930 */ int __cdecl Gl_Deinit(void);
/* 0x0047aa00 */ int __cdecl Gl_SetupView(void);
/* 0x0047ab80 */ void __cdecl Gl_ResetModelview(void);
/* 0x0047abd0 */ void __cdecl Gl_DrawQuadRgba(const PalFx *fx, GlTex *tex, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, const DrawParams *dp);
/* 0x0047b0d0 */ void __cdecl Gl_DrawQuadIndexed(const PalFx *fx, GlTex *tex, GlTex *pal, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, const DrawParams *dp);
/* 0x0047b5c0 */ void __cdecl Sprite_DrawGL(struct SpriteSet *set, int sprite, const DrawParams *dp);
/* 0x0047b830 */ void __cdecl Gl_DrawSprite(int sprite, const DrawParams *dp, SpriteSet *set);
/* 0x0047bee0 */ void __cdecl Gl_ApplyClipRect(void);
/* 0x0047bf90 */ void *__cdecl Gl_Capture(void);
/* 0x0047bfb0 */ unsigned char *__cdecl Gl_ReadPixels(int w, int h);
/* 0x0047c100 */ GlTex *__cdecl GlVbuf_Alloc(int depth, int w, int h);
/* 0x0047c1a0 */ int __cdecl GlVbuf_Bind(GlTex *vbuf);
/* 0x0047c1f0 */ void __cdecl Gl_Present(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0047c3b0 */ void __cdecl Gl_AttachFramebuffer(unsigned int texture, unsigned int depth);
/* 0x0047c460 */ char *__cdecl Gl_InsertDefines(const char *src, const char *defines);
/* 0x0047c580 */ int __cdecl Gl_LoadShader(const char **defines, GlProgram *prog, const char *name);
/* 0x0047c990 */ int __cdecl Gl_LoadShaders(void);
/* 0x0047ca50 */ int __cdecl Gl_FreeShaders(void);
/* 0x0047caf0 */ int __cdecl Gl_LogInfo(unsigned int obj);
/* 0x0047cba0 */ char *__cdecl Gl_ReadFile(FILE *f);
/* 0x0047cc10 */ void __cdecl SoftSprite_BlitSub(int y, int x, SpriteSet *set, int sprite);
/* 0x0047ce50 */ void __cdecl SoftSprite_BlitSubH(int y, SpriteSet *set, int sprite, int x);
/* 0x0047d090 */ void __cdecl SoftSprite_BlitSubV(int x, SpriteSet *set, int sprite, int y);
/* 0x0047d2d0 */ void __cdecl SoftSprite_BlitSubHV(SpriteSet *set, int sprite, int x, int y);
/* 0x0047d530 */ void __cdecl SoftSprite_DrawScaledSub(SpriteSet *set, int sprite, float x, float y, float sx, float sy);
/* 0x0047d690 */ void __cdecl SoftSprite_ScaleSub(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0047dce0 */ void __cdecl SoftSprite_ScaleSubH(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0047e330 */ void __cdecl SoftSprite_ScaleSubV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0047e990 */ void __cdecl SoftSprite_ScaleSubHV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x0047f020 */ void __cdecl SoftSprite_RotateSub(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x0047fb50 */ void __cdecl SoftSprite_RotateSubFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x00480680 */ int __cdecl Gfx_SoftCeil(float x);
/* 0x004806a0 */ void __cdecl Sprite_DrawSoft8(const DrawParams *dp, SpriteSet *set, int sprite);
/* 0x00480a00 */ void __cdecl SoftSprite_Blit(int y, int x, SpriteSet *set, int sprite);
/* 0x00480bd0 */ void __cdecl SoftSprite_BlitOpaque(int y, int x, SpriteSet *set, int sprite);
/* 0x00480da0 */ void __cdecl SoftSprite_BlitH(int y, SpriteSet *set, int sprite, int x);
/* 0x00480f80 */ void __cdecl SoftSprite_BlitV(int x, SpriteSet *set, int sprite, int y);
/* 0x00481160 */ void __cdecl SoftSprite_BlitHV(SpriteSet *set, int sprite, int x, int y);
/* 0x00481360 */ void __cdecl SoftSprite_DrawScaled(SpriteSet *set, int sprite, float x, float y, float sx, float sy);
/* 0x004814c0 */ void __cdecl SoftSprite_DrawScaledOpaque(SpriteSet *set, int sprite, float x, float y, float sx, float sy);
/* 0x00481620 */ void __cdecl SoftSprite_Scale(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00481c10 */ void __cdecl SoftSprite_ScaleOpaque(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x004821e0 */ void __cdecl SoftSprite_ScaleH(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x004827e0 */ void __cdecl SoftSprite_ScaleV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00482dd0 */ void __cdecl SoftSprite_ScaleHV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x004833e0 */ void __cdecl SoftSprite_Rotate(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x00483ed0 */ void __cdecl SoftSprite_RotateFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x00485780 */ void __cdecl Gfx_SoftClear(unsigned int color);
/* 0x00485860 */ void __cdecl Gfx_SoftPutPixel(int x, int y, unsigned int color);
/* 0x00485960 */ void __cdecl Gfx_SoftAddPixel(int x, int y, unsigned int color);
/* 0x00485aa0 */ void __cdecl Gfx_SoftInvertPixel(int x, int y);
/* 0x00485b20 */ void __cdecl Gfx_SoftDrawLine(float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00485c10 */ void __cdecl Gfx_SoftDrawLineRaster(const int *rows, unsigned char *pixels, float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00486000 */ float __cdecl Gfx_LineEdgeParam(float x, float y, float dx, float dy, float nx, float ny, float px, float py);
/* 0x00486070 */ void __cdecl Gfx_SoftDrawLineClipped(unsigned char *pixels, const int *rows, float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00486630 */ void __cdecl Gfx_SoftHLine(int x1, int x2, int y, unsigned int color);
/* 0x004866d0 */ void __cdecl Gfx_SoftVLine(int y1, int y2, int x, unsigned int color);
/* 0x00486780 */ void __cdecl Gfx_SoftDrawLineAdd(float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00486810 */ void __cdecl Gfx_SoftHLineAdd(unsigned int color, int x1, int x2, int y);
/* 0x00486930 */ void __cdecl Gfx_SoftHLineHalfAdd(unsigned int color, int x1, int x2, int y);
/* 0x00486a60 */ void __cdecl Gfx_SoftVLineAdd(unsigned int color, int y1, int y2, int x);
/* 0x00486b90 */ void __cdecl Gfx_SoftVLineHalfAdd(unsigned int color, int y1, int y2, int x);
/* 0x00486cd0 */ void __cdecl Gfx_SoftHLineSub(unsigned int color, int x1, int x2, int y);
/* 0x00486de0 */ void __cdecl Gfx_SoftVLineSub(unsigned int color, int y1, int y2, int x);
/* 0x00486f00 */ void __cdecl Gfx_SoftDrawRect(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00487040 */ void __cdecl Gfx_SoftDrawRectAdd(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x004870a0 */ void __cdecl Gfx_SoftDrawRectBlend(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst);
/* 0x00487120 */ void __cdecl Gfx_SoftDrawRectSub(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00487180 */ void __cdecl Gfx_SoftFillRect(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x004872d0 */ void __cdecl Gfx_SoftFillRectAdd(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00487470 */ void __cdecl Gfx_SoftBlendRect(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst);
/* 0x004874c0 */ void __cdecl Gfx_SoftFillRectHalfAdd(unsigned int color, int x1, int x2, int y1, int y2);
/* 0x00487660 */ void __cdecl Gfx_SoftFillRectSub(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x004877e0 */ void __cdecl Gfx_GlClear(unsigned int color);
/* 0x004878c0 */ void __cdecl Gfx_GlPutPixel(int x, int y, unsigned int color);
/* 0x004879b0 */ void __cdecl Gfx_GlAddPixel(int x, int y, unsigned int color);
/* 0x00487ae0 */ void __cdecl Gfx_GlInvertPixel(int x, int y);
/* 0x00487b90 */ void __cdecl Gfx_GlDrawLine(float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00487c90 */ void __cdecl Gfx_GlDrawLineAdd(float x1, float y1, float x2, float y2, unsigned int color);
/* 0x00487de0 */ void __cdecl Gfx_GlDrawRect(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00487e10 */ void __cdecl Gfx_GlDrawRectAdd(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00487e40 */ void __cdecl Gfx_GlDrawRectBlend(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst);
/* 0x00488060 */ void __cdecl Gfx_GlDrawRectSub(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00488260 */ void __cdecl Gfx_GlFillRect(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x00488290 */ void __cdecl Gfx_GlFillRectAdd(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x004882c0 */ void __cdecl Gfx_GlBlendRect(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst);
/* 0x004884e0 */ void __cdecl Gfx_GlFillRectSub(int x1, int y1, int x2, int y2, unsigned int color);
/* 0x004886e0 */ int __cdecl Wav_FrameSize(int format, int channels);
/* 0x00488730 */ int __cdecl Wav_ReadFmt(Stream *s, WavInfo *info, unsigned int size);
/* 0x00488890 */ int __cdecl Wav_ReadInfo(Stream *s, WavInfo *info);
/* 0x00488af0 */ WavReader *__cdecl WavReader_Open(const WavInfo *info, Stream *s, int *status);
/* 0x00488bb0 */ unsigned int __cdecl WavReader_Read(WavReader *r, unsigned int frames, void *buf);
/* 0x00488c20 */ const char *__cdecl Wav_StrError(int status);
/* 0x00488c90 */ int __cdecl Audio_FrameSize(int format, int channels);
/* 0x00488cd0 */ ResamplerSession *__cdecl Resampler_Open(const AudioFormat *out, const AudioFormat *in, int chunk, const int *quality);
/* 0x00489020 */ void __cdecl Resampler_Close(ResamplerSession *sess);
/* 0x00489080 */ unsigned int __cdecl Resampler_InputNeeded(ResamplerSession *sess);
/* 0x004890a0 */ int __cdecl Resampler_State(ResamplerSession *sess);
/* 0x004890d0 */ unsigned int __cdecl Resampler_Write(ResamplerSession *sess, const void *in, unsigned int frames, int last);
/* 0x00489280 */ unsigned int __cdecl Resampler_Read(ResamplerSession *sess, void *out, unsigned int frames);
/* 0x00489330 */ Resampler *__cdecl Resampler_Create(const AudioFormat *out, int chunk, const int *quality);
/* 0x004893d0 */ ResamplerSession *__cdecl Resampler_Begin(Resampler *r, const WavInfo *in);
/* 0x004893f0 */ const AudioFormat *__cdecl Resampler_InFormat(ResamplerSession *sess);
/* 0x00489400 */ const AudioFormat *__cdecl Resampler_OutFormat(ResamplerSession *sess);
/* 0x00489410 */ void __cdecl Resampler_Reset(ResamplerSession *sess);
/* 0x00489430 */ unsigned int __cdecl Resampler_OutBlock(ResamplerSession *sess);
/* 0x00489440 */ unsigned int __cdecl Resampler_InBlock(ResamplerSession *sess);
/* 0x00489450 */ int __cdecl BgmThread_Init(const AudioFormat *format, int chunk);
/* 0x00489650 */ void *__cdecl BgmThread_Main(void *arg);
/* 0x00489a40 */ void __cdecl BgmThread_Cleanup(void);
/* 0x00489b10 */ int __cdecl Bgm_QueueCommand(int time, const intptr_t *data, int cmd, int owns_data);
/* 0x00489b80 */ void __cdecl BgmQueue_Drop(int time);
/* 0x00489bf0 */ int __cdecl BgmQueue_Load(int time, const BgmInfo *info);
/* 0x00489c50 */ int __cdecl BgmQueue_Play(int time, const BgmInfo *info);
/* 0x00489cc0 */ int __cdecl BgmQueue_Unload(int time);
/* 0x00489d10 */ int __cdecl BgmThread_Shutdown(void);
/* 0x00489dc0 */ int __cdecl Bgm_QueueMute(int muted, int time);
/* 0x00489df0 */ int __cdecl BgmThread_Load(const BgmInfo *info);
/* 0x00489f00 */ int __cdecl BgmThread_Play(const BgmInfo *info);
/* 0x0048a070 */ int __cdecl BgmThread_Unload(void);
/* 0x0048a0a0 */ int __cdecl BgmThread_Start(void);
/* 0x0048a130 */ int __cdecl BgmThread_Prebuffer(void);
/* 0x0048a1b0 */ int __cdecl BgmThread_Stop(void);
/* 0x0048a260 */ int __cdecl BgmThread_Hold(unsigned int flags);
/* 0x0048a2e0 */ int __cdecl BgmThread_Unhold(unsigned int flags);
/* 0x0048a350 */ void __cdecl BgmRing_Reset(void);
/* 0x0048a390 */ void __cdecl BgmThread_Silence(void *dst, unsigned int size);
/* 0x0048a3c0 */ int __cdecl BgmThread_FillBlock(void *dst, unsigned int size);
/* 0x0048a4e0 */ void __cdecl BgmThread_PostEnd(void);
/* 0x0048a530 */ void __cdecl BgmThread_Unhook(void);
/* 0x0048a560 */ void __cdecl BgmThread_UpdateHook(void);
/* 0x0048a5c0 */ void __cdecl BgmThread_MixCallback(void *udata, Uint8 *stream, int len);
/* 0x0048a730 */ BgmSettings *__cdecl BgmSettings_New(void);
/* 0x0048a820 */ const char *__cdecl BgmSettings_Get(const BgmSettings *s, const char *key);
/* 0x0048a890 */ int __cdecl BgmSettings_Set(BgmSettings *s, const char *key, const char *value);
/* 0x0048ab10 */ void __cdecl BgmSettings_Clear(BgmSettings *s);
/* 0x0048ab60 */ int __cdecl Mpg123_LoadSymbols(void);
/* 0x0048add0 */ unsigned int __cdecl Bgm_BuiltinVersion(void);
/* 0x0048ade0 */ const char *__cdecl Mpg123_Extensions(void);
/* 0x0048adf0 */ int __cdecl Mpg123_SetupFormat(void *mh);
/* 0x0048ae80 */ int __cdecl Mpg123_NewHandle(Mpg123Instance *inst);
/* 0x0048aee0 */ void *__cdecl Mpg123_Create(void);
/* 0x0048af30 */ void __cdecl Mpg123_Destroy(void *instance);
/* 0x0048af80 */ int __cdecl Mpg123_FeedRead(Mpg123Instance *inst, void *buf, unsigned int size, unsigned int *got);
/* 0x0048b070 */ int __cdecl Mpg123_Init(const BgmPluginConfig *config, const BgmHostCallbacks *host, BgmSettingFn get_setting);
/* 0x0048b230 */ void __cdecl Mpg123_Deinit(void);
/* 0x0048b260 */ void __cdecl Mpg123_SetLoop(Mpg123Instance *inst, int start, int end);
/* 0x0048b320 */ int __cdecl Mpg123_GetFormatOf(void *instance, AudioFormat *format);
/* 0x0048b340 */ int __cdecl Mpg123_GetFormat(AudioFormat *out, const Mpg123Instance *inst);
/* 0x0048b3c0 */ int __cdecl Mpg123_UpdateFormat(Mpg123Instance *inst);
/* 0x0048b440 */ int __cdecl Mpg123_Open(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out);
/* 0x0048b540 */ int __cdecl Mpg123_Close(void *instance);
/* 0x0048b580 */ int __cdecl Mpg123_Start(void *instance);
/* 0x0048b5b0 */ int __cdecl Mpg123_Stop(void *instance);
/* 0x0048b5d0 */ int __cdecl Mpg123_Rewind(void *instance);
/* 0x0048b5e0 */ int __cdecl Mpg123_FeedSeek(Mpg123Instance *inst, int frame);
/* 0x0048b610 */ int __cdecl Mpg123_Seek(void *instance, int pos);
/* 0x0048b680 */ int __cdecl Mpg123_IsPlaying(void *instance);
/* 0x0048b690 */ double __cdecl Mpg123_GetVolume(void *instance);
/* 0x0048b6a0 */ int __cdecl Mpg123_SetVolume(void *instance, double volume);
/* 0x0048b700 */ int __cdecl Mpg123_GetLoopFlag(void *instance);
/* 0x0048b710 */ int __cdecl Mpg123_SetLoopFlag(void *instance, int loop);
/* 0x0048b730 */ int __cdecl Mpg123_SetLoopPoints(void *instance, int start, int end);
/* 0x0048b760 */ int __cdecl Mpg123_GetLoopPoints(void *instance, int *start, int *end);
/* 0x0048b790 */ void __cdecl Mpg123_Silence(int format, void *dst, unsigned int size);
/* 0x0048b7c0 */ int __cdecl Mpg123_Read(void *instance, void *buf, unsigned int size, unsigned int *got, int *event);
/* 0x0048ba10 */ void __cdecl Mpg123_SetEndCallback(void *instance, BgmEndFn fn, void *arg);
/* 0x0048ba30 */ const char *__cdecl Mpg123_StrError(void *instance, int err);
/* 0x0048ba70 */ const char *__cdecl Sdlmix_Extensions(void);
/* 0x0048ba80 */ int __cdecl Sdlmix_Classify(const char *ext);
/* 0x0048bc60 */ int __cdecl Sdlmix_Init(const BgmPluginConfig *config, const BgmHostCallbacks *host, BgmSettingFn get_setting);
/* 0x0048be70 */ int __cdecl Stub_ReturnZero(void);
/* 0x0048be80 */ void *__cdecl Sdlmix_Create(void);
/* 0x0048beb0 */ void __cdecl Sdlmix_Destroy(void *instance);
/* 0x0048bef0 */ int __cdecl Sdlmix_Open(void *instance, const BgmPluginIo *io, BgmInfo *info, BgmOpenResult *out);
/* 0x0048bfe0 */ int __cdecl Sdlmix_Close(void *instance);
/* 0x0048c020 */ void __cdecl Sdlmix_OnFinished(void);
/* 0x0048c040 */ int __cdecl Sdlmix_Start(void *instance);
/* 0x0048c0a0 */ int __cdecl Sdlmix_Pause(void *instance);
/* 0x0048c0b0 */ int __cdecl Sdlmix_Resume(void *instance);
/* 0x0048c0d0 */ int __cdecl Sdlmix_Stop(void *instance);
/* 0x0048c100 */ int __cdecl Sdlmix_Rewind(void *instance);
/* 0x0048c110 */ int __cdecl Sdlmix_Seek(void *instance, int pos);
/* 0x0048c190 */ int __cdecl Sdlmix_IsPlaying(void *instance);
/* 0x0048c1c0 */ int __cdecl Sdlmix_IsPaused(void *instance);
/* 0x0048c1e0 */ double __cdecl Sdlmix_GetVolume(void *instance);
/* 0x0048c1f0 */ int __cdecl Sdlmix_SetVolume(void *instance, double volume);
/* 0x0048c260 */ int __cdecl Sdlmix_GetLoopFlag(void *instance);
/* 0x0048c270 */ int __cdecl Sdlmix_SetLoopFlag(void *instance, int loop);
/* 0x0048c2a0 */ int __cdecl Sdlmix_GetLoopPoints(void *instance, int *start, int *end);
/* 0x0048c2d0 */ int __cdecl Sdlmix_SetLoopPoints(void *instance, int start, int end);
/* 0x0048c2f0 */ int __cdecl Sdlmix_Read(void *instance, void *buf, unsigned int size, unsigned int *got, int *event);
/* 0x0048c310 */ void __cdecl Sdlmix_SetEndCallback(void *instance, BgmEndFn fn, void *arg);
/* 0x0048c330 */ const char *__cdecl Sdlmix_StrError(void *instance, int err);
/* 0x0048c350 */ int __cdecl Sdlmix_SetMute(void *instance, int muted);
/* 0x0048c3a0 */ int __cdecl Sdlmix_GetMute(void *instance);
/* 0x0048c3b0 */ int __cdecl Sdlmix_Poll(void *instance);
/* 0x0048c3e0 */ GrowBuf *__cdecl GrowBuf_Create(int size);
/* 0x0048c450 */ void __cdecl GrowBuf_Free(GrowBuf *buf);
/* 0x0048c480 */ int __cdecl GrowBuf_Resize(GrowBuf *buf, int size);
/* 0x0048c510 */ int __cdecl GrowBufWriter_PutByte(GrowBufWriter *w, int value);
/* 0x0048c550 */ unsigned char *__cdecl Rle8_Encode(const void *pixels, int w, int h, int *size_out);
/* 0x0048c6d0 */ int __cdecl Rle8_Decode(const void *data, size_t size, void *out);
/* 0x0048c730 */ int __cdecl Rle8_PutRun(int count, GrowBufWriter *w, int value);
/* 0x0048c7b0 */ DecompBuffer *__cdecl DecompBuffer_Create(unsigned int size);
/* 0x0048c830 */ int __cdecl DecompBuffer_Reset(DecompBuffer *buf);
/* 0x0048c890 */ void *__cdecl DecompBuffer_Get(const DecompRequest *req, int *found);
/* 0x0048c910 */ void *__cdecl DecompBuffer_Alloc(const DecompRequest *req);
/* 0x0048caa0 */ DecompBlock *__cdecl DecompBuffer_FreeBlock(DecompBlock *blk, DecompBuffer *buf, DecompBlock *prev);
/* 0x0048cb10 */ unsigned char *__cdecl Lz5_Encode(const void *pixels, int w, int h, int *size_out);
/* 0x0048cc00 */ int __cdecl Lz5_Decode(const void *data, size_t size, void *out);
/* 0x0048cc20 */ int __cdecl Lz5_EncodeBlock(const unsigned char *last, GrowBufWriter *w, const unsigned char *src);
/* 0x0048cf90 */ Lz5Node *__cdecl Lz5_Parse(int n, const unsigned char *src);
/* 0x0048d420 */ short *__cdecl Lz5_RunsForward(int n, const unsigned char *src);
/* 0x0048d480 */ short *__cdecl Lz5_RunsBackward(const unsigned char *src, int n);
/* 0x0048d4d0 */ int __cdecl Lz5_DecodeBlock(const unsigned char *end, const unsigned char *src, unsigned char *out);
/* 0x0048d610 */ unsigned char *__cdecl Rle5_Encode(const void *pixels, int w, int h, int *size_out);
/* 0x0048d700 */ int __cdecl Rle5_Decode(const void *data, size_t size, void *out);
/* 0x0048d720 */ int __cdecl Rle5_EncodeBlock(const unsigned char *src, GrowBufWriter *w, const unsigned char *last);
/* 0x0048d880 */ int __cdecl Rle5_DecodeBlock(const unsigned char *src, const unsigned char *end, unsigned char *out);
/* 0x0048d920 */ int __cdecl Rle5_PutPixels(const unsigned char *src, GrowBufWriter *w, const unsigned char *last);
/* 0x0048d9b0 */ unsigned int __cdecl Hash_Lookup3(const void *key, size_t length);
/* 0x0048ded0 */ void __cdecl GlDetect_Clear(void);
/* 0x0048df40 */ int __cdecl GlDetect_TestLinesEven(void);
/* 0x0048e0e0 */ int __cdecl GlDetect_TestLinesOdd(void);
/* 0x0048e280 */ int __cdecl GlDetect_PrimitivesOffset(void);
/* 0x0048e330 */ int __cdecl GlDetect_TestSprite(int w, int h, int x, int y, int flip);
/* 0x0048e600 */ int __cdecl GlDetect_TestSprites(void);
/* 0x0048e6f0 */ int __cdecl GlDetect_SpriteOffset(void);
/* 0x0048e7c0 */ int __cdecl GlDetect_TestIndexShader(const char *frag_src);
/* 0x0048eb00 */ void __cdecl GlDetect_IndexCorrection(void);
/* 0x0048eb60 */ void __cdecl Gl_DetectOffsets(void);
/* 0x0048ebf0 */ void __cdecl GlDetect_DrawSprite(SpriteSet *set, const DrawParams *dp, unsigned int prog);
/* 0x0048ecf0 */ void __cdecl GlDetect_DrawQuad(unsigned int prog, const GlTex *tex, const GlTex *pal_tex, float x0, float y0, float x1, float y1);
/* 0x0048ee70 */ void __cdecl SoftSprite_BlitAdd(int y, int x, SpriteSet *set, int sprite);
/* 0x0048f0b0 */ void __cdecl SoftSprite_BlitAddH(int y, SpriteSet *set, int sprite, int x);
/* 0x0048f310 */ void __cdecl SoftSprite_BlitAddV(int x, SpriteSet *set, int sprite, int y);
/* 0x0048f560 */ void __cdecl SoftSprite_BlitAddHV(SpriteSet *set, int sprite, int x, int y);
/* 0x0048f7d0 */ void __cdecl SoftSprite_BlitAlpha(SpriteSet *set, int x, int sprite, int y, int alpha_src, int alpha_dst);
/* 0x0048ff90 */ void __cdecl SoftSprite_BlitAlphaH(int alpha_src, int y, SpriteSet *set, int sprite, int x, int alpha_dst);
/* 0x00490790 */ void __cdecl SoftSprite_BlitAlphaV(int x, SpriteSet *set, int sprite, int y, int alpha_src, int alpha_dst);
/* 0x00490f60 */ void __cdecl SoftSprite_BlitAlphaHV(SpriteSet *set, int sprite, int x, int y, int alpha_src, int alpha_dst);
/* 0x00491770 */ void __cdecl SoftSprite_ScaleAdd(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00491de0 */ void __cdecl SoftSprite_ScaleAddH(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00492450 */ void __cdecl SoftSprite_ScaleAddV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00492ad0 */ void __cdecl SoftSprite_ScaleAddHV(SpriteSet *set, int sprite, float x, float y, float w, float h);
/* 0x00493170 */ void __cdecl SoftSprite_DrawScaledAdd(SpriteSet *set, int sprite, int alpha_dst, float x, float y, float sx, float sy, int alpha_src);
/* 0x004932e0 */ void __cdecl SoftSprite_ScaleAlpha(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src, int alpha_dst);
/* 0x00493f60 */ void __cdecl SoftSprite_ScaleAlphaH(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src, int alpha_dst);
/* 0x00494c20 */ void __cdecl SoftSprite_ScaleAlphaV(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src, int alpha_dst);
/* 0x004958e0 */ void __cdecl SoftSprite_ScaleAlphaHV(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src, int alpha_dst);
/* 0x004965b0 */ void __cdecl SoftSprite_RotateAddFull(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x00497100 */ void __cdecl SoftSprite_RotateAddFullFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y);
/* 0x00497c40 */ void __cdecl SoftSprite_RotateAdd(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y, int alpha_src, int alpha_dst);
/* 0x00499350 */ void __cdecl SoftSprite_RotateAddFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y, float scale2_x, float scale2_y, int alpha_src, int alpha_dst);
/* 0x0049aa50 */ int __cdecl Audio_FloatToS16(float x);
/* 0x0049aaa0 */ unsigned char __cdecl Audio_FloatToU8(float x);
/* 0x0049ab10 */ AudioConvertFn __cdecl Audio_GetConverter(const AudioFormat *src, const AudioFormat *dst);
/* 0x0049abb0 */ void __cdecl Audio_ConvertS16ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049ac60 */ void __cdecl Audio_ConvertF32ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049ad00 */ void __cdecl Audio_ConvertU8ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049adb0 */ void __cdecl Audio_ConvertS16ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049b190 */ void __cdecl Audio_ConvertU8ToF32(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049b580 */ void __cdecl Audio_ConvertF32ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049b6b0 */ void __cdecl Audio_ConvertU8ToS16(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049b7f0 */ void __cdecl Audio_ConvertF32ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049b910 */ void __cdecl Audio_ConvertS16ToU8(void *dst, const void *src, unsigned int frames, int dst_channels, int src_channels);
/* 0x0049ba00 */ BgmRing *__cdecl BgmRing_Create(int block);
/* 0x0049ba90 */ int __cdecl Bgm_StreamStart(BgmStream *s, unsigned int flags);
/* 0x0049bb60 */ int __cdecl Bgm_StreamSetMute(BgmStream *s, int muted);
/* 0x0049bba0 */ void __cdecl Bgm_StreamSetVolume(double volume);
/* 0x0049bc10 */ int __cdecl Bgm_StreamSetupResampler(const AudioFormat *in, BgmStream *s, int fallback);
/* 0x0049bd30 */ int __cdecl Bgm_StreamOpen(const BgmInfo *info);
/* 0x0049bf40 */ int __cdecl Bgm_StreamCompare(const BgmInfo *info);
/* 0x0049bf90 */ void __cdecl Bgm_StreamStop(void);
/* 0x0049bff0 */ void __cdecl Bgm_StreamClose(BgmStream *s);
/* 0x0049c070 */ BgmStream *__cdecl Bgm_StreamCreate(void);
/* 0x0049c0e0 */ void __cdecl Bgm_StreamFree(void);
/* 0x0049c140 */ unsigned int __cdecl Bgm_StreamRead(void *dst, int *status);

/* ---- functions of the statically linked libraries (third_party/) ---- */
/* 0x0049c3c0 */ png_uint_32 __cdecl png_get_IHDR(png_structp png_ptr, png_infop info_ptr, png_uint_32 *width, png_uint_32 *height, int *bit_depth, int *color_type, int *interlace_method, int *compression_method, int *filter_method);
/* 0x0049c520 */ png_structp __cdecl png_create_read_struct(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
/* 0x0049c8c0 */ void __cdecl png_read_info(png_structp png_ptr, png_infop info_ptr);
/* 0x0049d110 */ void __cdecl png_read_update_info(png_structp png_ptr, png_infop info_ptr);
/* 0x0049d8e0 */ void __cdecl png_read_image(png_structp png_ptr, png_bytepp image);
/* 0x0049d980 */ void __cdecl png_destroy_read_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr);
/* 0x0049e170 */ void __cdecl png_write_info(png_structp png_ptr, png_infop info_ptr);
/* 0x0049e620 */ void __cdecl png_write_end(png_structp png_ptr, png_infop info_ptr);
/* 0x0049e880 */ png_structp __cdecl png_create_write_struct(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
/* 0x0049eb40 */ void __cdecl png_write_image(png_structp png_ptr, png_bytepp image);
/* 0x0049f0e0 */ void __cdecl png_destroy_write_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr);
/* 0x0049f880 */ void __cdecl png_set_write_fn(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn);
/* 0x0049f910 */ void __cdecl png_set_packing(png_structp png_ptr);
/* 0x004a0310 */ void __cdecl png_set_sig_bytes(png_structp png_ptr, int num_bytes);
/* 0x004a0350 */ int __cdecl png_sig_cmp(png_bytep sig, png_size_t start, png_size_t num_to_check);
/* 0x004a0530 */ png_infop __cdecl png_create_info_struct(png_structp png_ptr);
/* 0x004a0c80 */ png_voidp __cdecl png_get_io_ptr(png_structp png_ptr);
/* 0x004a0ca0 */ void __cdecl png_init_io(png_structp png_ptr, png_FILE_p fp);
/* 0x004a1080 */ void __cdecl png_set_read_fn(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn);
/* 0x004a1660 */ void __cdecl png_set_IHDR(png_structp png_ptr, png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth, int color_type, int interlace_method, int compression_method, int filter_method);
/* 0x004a1c60 */ void __cdecl png_set_PLTE(png_structp png_ptr, png_infop info_ptr, png_colorp palette, int num_palette);
/* 0x004c4dc0 */ unsigned int __cdecl glewInit(void);
/* 0x004c4f70 */ LuaCFunction __cdecl lua_atpanic(struct lua_State *L, LuaCFunction panicf);
/* 0x004c4fd0 */ int __cdecl lua_gettop(struct lua_State *L);
/* 0x004c4fe0 */ void __cdecl lua_settop(struct lua_State *L, int idx);
/* 0x004c5030 */ void __cdecl lua_remove(struct lua_State *L, int idx);
/* 0x004c5190 */ void __cdecl lua_pushvalue(struct lua_State *L, int idx);
/* 0x004c51c0 */ int __cdecl lua_type(struct lua_State *L, int idx);
/* 0x004c5230 */ int __cdecl lua_isnumber(struct lua_State *L, int idx);
/* 0x004c5270 */ int __cdecl lua_isstring(struct lua_State *L, int idx);
/* 0x004c5320 */ double __cdecl lua_tonumber(struct lua_State *L, int idx);
/* 0x004c5360 */ int __cdecl lua_tointeger(struct lua_State *L, int idx);
/* 0x004c53a0 */ int __cdecl lua_toboolean(struct lua_State *L, int idx);
/* 0x004c53d0 */ const char *__cdecl lua_tolstring(struct lua_State *L, int idx, size_t *len);
/* 0x004c54e0 */ void *__cdecl lua_touserdata(struct lua_State *L, int idx);
/* 0x004c55a0 */ void __cdecl lua_pushnil(struct lua_State *L);
/* 0x004c55c0 */ void __cdecl lua_pushnumber(struct lua_State *L, double n);
/* 0x004c55e0 */ void __cdecl lua_pushinteger(struct lua_State *L, int n);
/* 0x004c5640 */ void __cdecl lua_pushstring(struct lua_State *L, const char *s);
/* 0x004c56e0 */ const char *__cdecl lua_pushfstring(struct lua_State *L, const char *fmt, ...);
/* 0x004c5710 */ void __cdecl lua_pushcclosure(struct lua_State *L, LuaCFunction fn, int n);
/* 0x004c57b0 */ void __cdecl lua_pushboolean(struct lua_State *L, int b);
/* 0x004c5820 */ void __cdecl lua_gettable(struct lua_State *L, int idx);
/* 0x004c5850 */ void __cdecl lua_getfield(struct lua_State *L, int idx, const char *k);
/* 0x004c58f0 */ void __cdecl lua_rawgeti(struct lua_State *L, int idx, int n);
/* 0x004c5930 */ void __cdecl lua_createtable(struct lua_State *L, int narr, int nrec);
/* 0x004c5a40 */ void __cdecl lua_settable(struct lua_State *L, int idx);
/* 0x004c5a70 */ void __cdecl lua_setfield(struct lua_State *L, int idx, const char *k);
/* 0x004c5b40 */ void __cdecl lua_rawseti(struct lua_State *L, int idx, int n);
/* 0x004c5bb0 */ int __cdecl lua_setmetatable(struct lua_State *L, int idx);
/* 0x004c5cf0 */ void __cdecl lua_call(struct lua_State *L, int nargs, int nresults);
/* 0x004c5d50 */ int __cdecl lua_pcall(struct lua_State *L, int nargs, int nresults, int errfunc);
/* 0x004c6020 */ void *__cdecl lua_newuserdata(struct lua_State *L, size_t size);
/* 0x004c6250 */ int __cdecl luaL_error(struct lua_State *L, const char *fmt, ...);
/* 0x004c6280 */ int __cdecl luaL_newmetatable(struct lua_State *L, const char *tname);
/* 0x004c62e0 */ void __cdecl luaL_checkstack(struct lua_State *L, int sz, const char *msg);
/* 0x004c66b0 */ int __cdecl luaL_ref(struct lua_State *L, int t);
/* 0x004c6760 */ void __cdecl luaL_unref(struct lua_State *L, int t, int ref);
/* 0x004c6860 */ int __cdecl luaL_loadfile(struct lua_State *L, const char *filename);
/* 0x004c6a80 */ int __cdecl luaL_loadbuffer(struct lua_State *L, const char *buff, size_t sz, const char *name);
/* 0x004c6b10 */ struct lua_State *__cdecl luaL_newstate(void);
/* 0x004c6b40 */ int __cdecl luaL_argerror(struct lua_State *L, int narg, const char *extramsg);
/* 0x004c6c40 */ void *__cdecl luaL_checkudata(struct lua_State *L, int narg, const char *tname);
/* 0x004c6d50 */ const char *__cdecl luaL_checklstring(struct lua_State *L, int narg, size_t *len);
/* 0x004c6ec0 */ int __cdecl luaL_checkinteger(struct lua_State *L, int narg);
/* 0x004c7280 */ void __cdecl luaL_register(struct lua_State *L, const char *libname, const LuaReg *l);
/* 0x004c72a0 */ void __cdecl luaL_openlibs(struct lua_State *L);
/* 0x004c76c0 */ void __cdecl lua_close(struct lua_State *L);

#endif /* MUGEN_SYMBOLS_H */
