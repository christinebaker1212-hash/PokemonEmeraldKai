#ifndef MUGEN_STORYBOARD_H
#define MUGEN_STORYBOARD_H

/*
 * Storyboards (cutscenes, storyboard.c): a .def file with a [SceneDef] group
 * (sprites, sounds, fonts, start scene), optional animations ([Begin Action
 * n]) and [Scene ...] groups played one after the other. Each scene has a
 * clear colour, a clipping window, a background, fades, a BGM, up to 100
 * layers (an animation or a text) and up to 100 sounds.
 */

#define STORYBOARD_FONTS    10      /* font0 .. font9 (Fonts_Load) */
#define SCENE_LAYERS        100     /* layer0 .. layer99 */
#define SCENE_SOUNDS        100     /* sound0 .. sound99 */
#define STORYBOARD_TRACKS   2       /* playback states; only the first one is ever active */

/* SceneLayer.type */
enum {
    SCENE_LAYER_ANIM = 0,           /* layerN.anim (also an unused layer: anim -1) */
    SCENE_LAYER_TEXT = 1            /* layerN.text */
};

typedef union SceneLayerValue {
    int anim;                       /* SCENE_LAYER_ANIM: action number, -1 for none */
    char *text;                     /* SCENE_LAYER_TEXT: malloc'ed, unquoted */
} SceneLayerValue;

/* layerN.* of a scene (0x34 bytes). */
typedef struct SceneLayer {
    int type;                       /* 0x00 SCENE_LAYER_* */
    int offset_x;                   /* 0x04 layerN.offset */
    int offset_y;                   /* 0x08 */
    int start_time;                 /* 0x0C layerN.starttime */
    int end_time;                   /* 0x10 layerN.endtime, -1: until the scene ends */
    SceneLayerValue value;          /* 0x14 */
    int font;                       /* 0x18 layerN.font: font index 0..9, -1 for none */
    int bank;                       /* 0x1C */
    int align;                      /* 0x20 FONT_ALIGN_* */
    int r;                          /* 0x24 default 255 */
    int g;                          /* 0x28 */
    int b;                          /* 0x2C */
    int text_delay;                 /* 0x30 layerN.textdelay: ticks per character, <= 0 shows all */
} SceneLayer;
MUGEN_ASSERT_OFFSET(SceneLayer, value, 0x14);
MUGEN_ASSERT_OFFSET(SceneLayer, text_delay, 0x30);
MUGEN_ASSERT_SIZE(SceneLayer, 0x34);

/* soundN.* of a scene (0x28 bytes, Scene.sounds elements keyed (0, N)). */
typedef struct SceneSound {
    int group;                      /* 0x00 soundN.value */
    int number;                     /* 0x04 */
    double volume;                  /* 0x08 soundN.volumescale, percent (default 100) */
    int unused_10;                  /* 0x10 -1, never read */
    float unused_14;                /* 0x14 1.0, never read */
    int unused_18;                  /* 0x18 0, never read */
    int pan;                        /* 0x1C soundN.pan */
    int start_time;                 /* 0x20 soundN.starttime */
    int unused_24;                  /* 0x24 0, never read */
} SceneSound;
MUGEN_ASSERT_OFFSET(SceneSound, volume, 0x08);
MUGEN_ASSERT_OFFSET(SceneSound, pan, 0x1C);
MUGEN_ASSERT_SIZE(SceneSound, 0x28);

/* A [Scene ...] group (0x16C0 bytes, Storyboard.scenes elements keyed (0, index)). */
typedef struct Scene {
    BgmInfo bgm;                    /* 0x0000 "bgm" and its parameters; path relative to the .def */
    int bgm_change;                 /* 0x0220 "bgm" given (always in the first scene): the BGM is stopped */
    int pad_224;                    /* 0x0224 never accessed by the game */
    int fadein_time;                /* 0x0228 */
    int fadein_col;                 /* 0x022C >= 128: fade from white, else from black */
    int fadeout_time;               /* 0x0230 */
    int fadeout_col;                /* 0x0234 */
    int clear_color[3];             /* 0x0238 clearcolor r, g, b; r = -1: no clear (default of the first scene) */
    int window_on;                  /* 0x0244 "window" given with 4 values (and x1 != -1, x2 >= x1) */
    int window[4];                  /* 0x0248 window x1, y1, x2, y2 */
    struct Bg *bg;                  /* 0x0258 background named by bg.name (Bg_Load), or NULL */
    SceneLayer layers[SCENE_LAYERS];/* 0x025C */
    KeyedArray *sounds;             /* 0x16AC SceneSound, NULL without sounds */
    int layerall_x;                 /* 0x16B0 layerall.pos */
    int layerall_y;                 /* 0x16B4 */
    int end_time;                   /* 0x16B8 end.time (required); -1 never ends */
    int pad_16bc;                   /* 0x16BC never accessed by the game */
} Scene;
MUGEN_ASSERT_OFFSET(Scene, bgm_change, 0x0220);
MUGEN_ASSERT_OFFSET(Scene, fadein_time, 0x0228);
MUGEN_ASSERT_OFFSET(Scene, clear_color, 0x0238);
MUGEN_ASSERT_OFFSET(Scene, window_on, 0x0244);
MUGEN_ASSERT_OFFSET(Scene, bg, 0x0258);
MUGEN_ASSERT_OFFSET(Scene, layers, 0x025C);
MUGEN_ASSERT_OFFSET(Scene, sounds, 0x16AC);
MUGEN_ASSERT_OFFSET(Scene, end_time, 0x16B8);
MUGEN_ASSERT_SIZE(Scene, 0x16C0);

/* Playback state (0x330 bytes). */
typedef struct StoryboardTrack {
    int active;                     /* 0x000 */
    AnimState *anims[SCENE_LAYERS]; /* 0x004 animation of each layer (calloc'ed by Storyboard_Parse) */
    int anim_on[SCENE_LAYERS];      /* 0x194 the layer's animation is running (Anim_SetAction result) */
    int time;                       /* 0x324 ticks into the scene */
    int frames;                     /* 0x328 ticks since Storyboard_Reset (counted, not used) */
    int scene;                      /* 0x32C index of the current scene */
} StoryboardTrack;
MUGEN_ASSERT_OFFSET(StoryboardTrack, anim_on, 0x194);
MUGEN_ASSERT_OFFSET(StoryboardTrack, time, 0x324);
MUGEN_ASSERT_SIZE(StoryboardTrack, 0x330);

/* A loaded storyboard (0x8E4 bytes, calloc'ed by Storyboard_Parse). */
typedef struct Storyboard {
    KeyedArray *scenes;             /* 0x000 Scene by (0, index) */
    int scene_count;                /* 0x004 */
    SpriteSet *sprites;             /* 0x008 [SceneDef] spr */
    Snd *snd;                       /* 0x00C [SceneDef] snd, or NULL */
    Font *fonts[STORYBOARD_FONTS];  /* 0x010 [SceneDef] font0 .. font9 */
    int owns_sprites;               /* 0x038 */
    int owns_snd;                   /* 0x03C */
    int owns_fonts[STORYBOARD_FONTS]; /* 0x040 */
    Anim *anims;                    /* 0x068 the .def's [Begin Action] groups, or NULL */
    int owns_anims;                 /* 0x06C */
    char dir[0x200];                /* 0x070 directory of the .def, ending in '/' or '\' (or empty) */
    StoryboardTrack tracks[STORYBOARD_TRACKS]; /* 0x270 */
    int start_scene;                /* 0x8D0 [SceneDef] startscene (not checked) */
    float pos_x;                    /* 0x8D4 screen position of the localcoord origin */
    float pos_y;                    /* 0x8D8 */
    float scale_x;                  /* 0x8DC localcoord to screen (Storyboard_SetScale) */
    float scale_y;                  /* 0x8E0 */
} Storyboard;
MUGEN_ASSERT_OFFSET(Storyboard, owns_sprites, 0x038);
MUGEN_ASSERT_OFFSET(Storyboard, anims, 0x068);
MUGEN_ASSERT_OFFSET(Storyboard, dir, 0x070);
MUGEN_ASSERT_OFFSET(Storyboard, tracks, 0x270);
MUGEN_ASSERT_OFFSET(Storyboard, start_scene, 0x8D0);
MUGEN_ASSERT_OFFSET(Storyboard, scale_y, 0x8E0);
MUGEN_ASSERT_SIZE(Storyboard, 0x8E4);

/* Cutscene.type */
enum {
    CUTSCENE_STORYBOARD = 0         /* the only kind: the screen loop ticks and draws it only with this type */
};

/* A cutscene (8 bytes): what Storyboard_Load returns and ScreenLoop.cutscene points to. */
typedef struct Cutscene {
    int type;                       /* 0x00 CUTSCENE_* */
    Storyboard *storyboard;         /* 0x04 */
} Cutscene;
MUGEN_ASSERT_SIZE(Cutscene, 0x08);

/* ScreenLoop.kind of the storyboard screen. */
#define SCREEN_KIND_STORYBOARD 10

#endif /* MUGEN_STORYBOARD_H */
