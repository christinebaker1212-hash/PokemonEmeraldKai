/*
 * Storyboards (cutscenes): the intro / ending storyboards of the motif and
 * the characters, and -storyboard on the command line.
 *
 * Two original modules live here:
 *  - the storyboard player (0x428740-0x428AEF, strings 0x4F56CC-0x4F574B),
 *    between the debug console and the title screen: Storyboard_PlayFile
 *    fits the storyboard's [Info] localcoord to the motif coordinate space
 *    and runs it through the screen loop (Screen_Run), whose update callback
 *    is Storyboard_Update;
 *  - the storyboard engine (0x474CD0-0x476C8F, strings 0x4FC3EC-0x4FC5DF),
 *    between the joystick module and the timer: loading the .def
 *    ([SceneDef], [Begin Action] animations, [Scene ...] groups) and the
 *    Storyboard_Tick / Storyboard_ApplyFade / Storyboard_Draw calls that
 *    Screen_Run makes for ScreenLoop.cutscene (a Cutscene).
 *
 * The engine inlines its own copies of the BGM command senders (Bgm_Stop,
 * Bgm_Play, with the BGM queue code), of the sound slot functions, of
 * Ini_NextGroup and of KeyedArray_Get / _First / _Next / _Add / _Free and
 * Font_Free; they behave exactly like the functions they were inlined from,
 * which are called here instead (except for one iteration that must not
 * touch the array's iteration cursor).
 */
#include "mugen.h"

/* Storyboard kind: "<kind>def" is the definition group, "[<kind> ...]" / "[<kind>]" the scenes. */
#define STORYBOARD_KIND "scene"

/* ------------------------------------------------------------------ player */

/*
 * 0x00428740: update callback of the storyboard screen. A button (or the
 * screen's done flag) starts a 5-tick palette fade out; during the fade
 * out escape or a quit request ends it at once.
 */
int Storyboard_Update(ScreenLoop *screen, void *state)
{
    GameVars *g;
    int pressed = 0;

    if (screen->fade_out != 0) {
        if (screen->fade_ticks > 0) {
            if (screen->key == SDLK_ESCAPE || g_quitRequested != 0) {
                screen->fade_ticks = 0;
                screen->fade_out = 1;
            }
            return 1;
        }
        return 0;
    }
    g = g_game;
    if (g->defocus_paused != 0) {
        return 0;
    }
    /* 0xFF0: the buttons (SCREEN_IN_BUTTONS) and bit 0x800. */
    if (screen->input & 0xFF0) {
        pressed = 1;
    }
    if (screen->done != 0 || pressed) {
        screen->fade_out = 1;
        screen->fade_ticks = 5;
        g->fade.type = FADE_PALETTE;
        g->fade.ticks = 0;
        g->fade.time = 5;
        g->fade.out = 1;
    }
    return 0;
}

/* 0x004287D0: the storyboard screen loop for cs; only the update callback is set. */
void Storyboard_SetupScreen(ScreenLoop *screen, Cutscene *cs)
{
    memset(screen, 0, sizeof(*screen));
    screen->scale = 1.0f;
    screen->fadein_time = 10;
    screen->fadeout_time = 10;
    screen->kind = SCREEN_KIND_STORYBOARD;
    screen->fps = 60;
    screen->reload_motif = 1;
    screen->init = NULL;
    screen->end = NULL;
    screen->tick = NULL;
    screen->draw = NULL;
    screen->overlay = NULL;
    /* Quirk: the 10s set above are cleared again. */
    screen->fadein_time = 0;
    screen->fadeout_time = 0;
    screen->start_frame = g_game->frames;
    screen->update = (ScreenFn)Storyboard_Update;
    screen->cutscene = (intptr_t)cs;
}

/*
 * 0x00428850: plays the storyboard .def at path (found with File_Find) to
 * its end. Its [Info] localcoord (default 320x240, at least 1x1) is scaled
 * to fit the motif coordinate space, centred, and the clip rectangle is set
 * to it.
 *
 * Quirks kept: "Storyboard file not found" has no newline; the clip
 * rectangle is not restored afterwards.
 */
void Storyboard_PlayFile(const char *path)
{
    char found[0x200];
    char dir[0x200];
    ScreenLoop screen;
    int local_w = 320;
    int local_h = 240;
    int screen_w = (int)g_game->motif_coord[0];
    int screen_h = (int)g_game->motif_coord[1];
    float scale;
    int x, y;
    Ini *ini;
    Cutscene *cs;

    if (path == NULL || path[0] == '\0') {
        return;
    }
    Log_Printf("Playing storyboard %s: %s\n", path, STORYBOARD_KIND);
    if (!File_Find(path, found, dir, NULL)) {
        Log_Printf("Storyboard file not found: %s", path);
        return;
    }
    ini = Ini_Open(found);
    if (ini == NULL) {
        Error_Printf("Error loading storyboard: %s\n", path);
        return;
    }
    if (Ini_SelectGroup(ini, "Info") != -1) {
        Ini_Scanf(ini, "localcoord", "%i,%i", &local_w, &local_h);
        if (local_w < 1) {
            local_w = 1;
        }
        if (local_h < 1) {
            local_h = 1;
        }
    }
    if (screen_h * local_w > screen_w * local_h) {
        scale = (float)((double)screen_w / local_w);
    } else {
        scale = (float)((double)screen_h / local_h);
    }
    x = -((int)((double)local_w * scale - screen_w) / 2);
    y = -((int)((double)local_h * scale - screen_h) / 2);
    Ini_Free(ini);

    strncpy(g_storyboardFontDir, "font/", 0x1FF);
    cs = Storyboard_Load(found);
    if (cs == NULL) {
        Error_Printf("Error loading storyboard: %s\n", path);
        return;
    }
    if (cs->type == CUTSCENE_STORYBOARD) {
        float s = (float)(scale * g_game->motif_scale);

        Storyboard_SetScale(cs->storyboard, s, s);
        if (cs->type == CUTSCENE_STORYBOARD) {
            cs->storyboard->pos_x = (float)x;
            cs->storyboard->pos_y = (float)y;
        }
    }
    Gfx_SetClipRect(x, y, (int)((double)local_w * scale + x), (int)((double)local_h * scale + y));
    Storyboard_SetupScreen(&screen, cs);
    Screen_Run(&screen);
    Storyboard_Close(cs);
    Log_Printf("Finished playing storyboard %s: %s\n", path, STORYBOARD_KIND);
}

/* ------------------------------------------------------------------ loading */

/*
 * 0x00474CD0: loads the storyboard .def at path. NULL on errors, with a
 * message appended to (or written into) g_libraryErrorText.
 *
 * Quirks kept: the messages are appended without a bounds check; a path
 * longer than the directory buffer overflows it.
 */
Cutscene *Storyboard_Load(const char *path)
{
    char dir[0x204];
    Stream *s;
    Ini *ini = NULL;
    Cutscene *cs;

    s = Stream_OpenFile(path, "rt");
    if (s != NULL) {
        ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
    }
    if (ini == NULL) {
        strcat(g_libraryErrorText, "\nTXTPARSE error opening ");
        strcat(g_libraryErrorText, path);
        return NULL;
    }
    /* The directory part of path, with its last '/' or '\'. */
    if (path[0] == '\0') {
        dir[0] = '\0';
    } else {
        int n = (int)strlen(path) - 1;

        while (n >= 0 && path[n] != '\\' && path[n] != '/') {
            n--;
        }
        n++;
        if (n > 0) {
            strncpy(dir, path, n);
        }
        dir[n] = '\0';
    }
    cs = Storyboard_Create(dir, ini);
    if (cs == NULL) {
        strcat(g_libraryErrorText, "\nCutscene error reading ");
        strcat(g_libraryErrorText, path);
    }
    Ini_Free(ini);
    return cs;
}

/* 0x00474E40: a Cutscene for the storyboard in ini (files relative to dir), or NULL. */
Cutscene *Storyboard_Create(const char *dir, Ini *ini)
{
    char group[0x100];
    Cutscene *cs;

    strcpy(group, STORYBOARD_KIND "def");
    if (Ini_SelectGroup(ini, group) == -1) {
        sprintf(g_libraryErrorText, "Can't access group %s", group);
        return NULL;
    }
    cs = calloc(1, sizeof(Cutscene));
    if (cs == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    cs->type = CUTSCENE_STORYBOARD;
    cs->storyboard = Storyboard_Parse(dir, ini);
    if (cs->storyboard == NULL) {
        free(cs);
        return NULL;
    }
    return cs;
}

/*
 * 0x00474F50: reads the storyboard in ini: [SceneDef] (spr, snd, fonts,
 * startscene), the [Begin Action] animations and every [Scene ...] group,
 * then prepares playback (Storyboard_Reset). NULL on errors (message in
 * g_libraryErrorText).
 *
 * Scenes inherit clearcolor, window and layerall.pos from the previous
 * scene; the first one has no clear colour and always stops the BGM.
 *
 * Quirks kept: two scenes with the same group name both read the first
 * one; the scene name buffer may be cleared one byte before its start
 * (empty name) and filled one byte past its end; errors after the scene
 * list was made leak it, and a scene that fails leaks its texts, sounds
 * and background; the sound list of a scene is used without a NULL check.
 */
Storyboard *Storyboard_Parse(const char *dir, Ini *ini)
{
    struct {
        char path[0x208];           /* file paths, keys */
        char name[0x100];           /* "<kind> ", then the scene's group name */
        char name2[0x100];          /* "<kind>]" */
    } buf;
    const char *dirs[3];
    Scene cur;
    Scene prev;
    SceneSound snd;
    Storyboard *sb;
    DynArray *lines;
    const char *value;
    const char *line;
    int kind_len;
    int count;
    int i, j, k;

    dirs[0] = NULL;
    dirs[1] = NULL;
    dirs[2] = NULL;
    strcpy(buf.path, STORYBOARD_KIND "def");
    if (Ini_SelectGroup(ini, buf.path) == -1) {
        sprintf(g_libraryErrorText, "Can't access group %s", buf.path);
        return NULL;
    }
    sb = calloc(1, sizeof(Storyboard));
    if (sb == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    sb->scale_y = 1.0f;
    sb->scale_x = 1.0f;
    strcpy(sb->dir, dir);
    if (Ini_SelectGroup(ini, buf.path) == -1) {
        sprintf(g_libraryErrorText, "Can't access group %s", buf.path);
        goto fail;
    }

    /* [SceneDef] */
    value = Ini_GetValue(ini, "spr");
    if (value == NULL) {
        strcpy(g_libraryErrorText, "Error: sff file must be specified");
        goto fail;
    }
    Path_Join(value, dir, buf.path);
    sb->sprites = Sff_Load(buf.path);
    if (sb->sprites == NULL) {
        sprintf(g_libraryErrorText, "Error reading sprite %s", value);
        goto fail;
    }
    sb->owns_sprites = 1;
    value = Ini_GetValue(ini, "snd");
    if (value != NULL) {
        Path_Join(value, dir, buf.path);
        sb->snd = Snd_Load(buf.path);
        if (sb->snd == NULL) {
            sprintf(g_libraryErrorText, "Error reading sound %s", value);
            goto fail;
        }
        sb->owns_snd = 1;
    }
    dirs[0] = dir;
    dirs[1] = g_storyboardFontDir;
    Fonts_Load(ini, sb->fonts, dirs);
    for (i = 0; i < STORYBOARD_FONTS; i++) {
        if (sb->fonts[i] != NULL) {
            sb->owns_fonts[i] = 1;
        }
    }
    value = Ini_GetValue(ini, "startscene");
    if (value != NULL) {
        sb->start_scene = atoi(value);
    }

    /* The first "[Begin Action n]" group, if any, starts the animations. */
    ini->group = -1;
    line = Ini_NextGroup(ini);
    while (!Str_StartsWithI(line + 1, "begin action ")) {     /* no NULL check before the first test */
        line = Ini_NextGroup(ini);
        if (line == NULL) {
            break;
        }
    }
    if (line != NULL) {
        sb->anims = Anim_Parse(ini);
        if (sb->anims == NULL) {
            strcat(g_libraryErrorText, "\nError loading anim.");
            goto fail;
        }
        sb->owns_anims = 1;
    }

    /* The scene groups: "[<kind>]" and "[<kind> ...]", in file order. */
    kind_len = (int)strlen(STORYBOARD_KIND);
    sprintf(buf.name, "%s ", STORYBOARD_KIND);
    sprintf(buf.name2, "%s]", STORYBOARD_KIND);
    count = Ini_CountGroups(ini, buf.name2);
    count += Ini_CountGroups(ini, buf.name);
    if (count == 0) {
        sprintf(g_libraryErrorText, "Need at least one [%s] type act", STORYBOARD_KIND);
        goto fail;
    }
    sb->scene_count = count;
    sb->scenes = KeyedArray_Create(count, sizeof(Scene));
    if (sb->scenes == NULL) {
        goto fail;
    }
    lines = DynArray_Create(count, sizeof(const char *));
    if (lines == NULL) {
        goto fail;
    }
    ini->group = -1;
    line = Ini_NextGroup(ini);
    for (i = count; i > 0; i--) {
        while (ini->group != ini->group_count) {
            if (Str_StartsWithI(line + 1, STORYBOARD_KIND) &&
                (line[kind_len + 1] == ' ' || line[kind_len + 1] == ']')) {
                break;
            }
            line = Ini_NextGroup(ini);
        }
        DynArray_Push(lines, &line);
        line = Ini_NextGroup(ini);
    }

    for (i = 0; i < count; i++) {
        line = ((const char **)lines->data)[i];
        /* The group name: the header without its brackets. */
        buf.name[0] = '\0';
        strncat(buf.name, line + 1, 0x100);
        (buf.name - 1)[strlen(buf.name)] = '\0';
        Ini_SelectGroup(ini, buf.name);

        if (i > 0) {
            memcpy(&prev, &cur, sizeof(Scene));
        }
        memset(&cur, 0, sizeof(Scene));
        for (j = 0; j < SCENE_LAYERS; j++) {
            cur.layers[j].value.anim = -1;
        }
        if (i == 0) {
            cur.clear_color[0] = -1;
            cur.bgm_change = 1;
        } else {
            cur.clear_color[0] = prev.clear_color[0];
            cur.clear_color[1] = prev.clear_color[1];
            cur.clear_color[2] = prev.clear_color[2];
            cur.window[0] = prev.window[0];
            cur.window[1] = prev.window[1];
            cur.window[2] = prev.window[2];
            cur.window[3] = prev.window[3];
            cur.window_on = prev.window_on;
            cur.layerall_x = prev.layerall_x;
            cur.layerall_y = prev.layerall_y;
        }

        if (Ini_GetValue(ini, "bgm") != NULL) {
            cur.bgm_change = 1;
            cur.bgm.path[0] = '\0';
            if (Bgm_ReadInfo(ini, "bgm", &cur.bgm)) {
                strcpy(buf.path, cur.bgm.path);
                Path_Join(buf.path, dir, cur.bgm.path);
            }
        }
        value = Ini_GetValue(ini, "fadein.time");
        if (value != NULL) {
            cur.fadein_time = atoi(value);
        }
        value = Ini_GetValue(ini, "fadein.col");
        if (value != NULL) {
            cur.fadein_col = atoi(value);
        }
        value = Ini_GetValue(ini, "fadeout.time");
        if (value != NULL) {
            cur.fadeout_time = atoi(value);
        }
        value = Ini_GetValue(ini, "fadeout.col");
        if (value != NULL) {
            cur.fadeout_col = atoi(value);
        }
        Ini_Scanf(ini, "clearcolor", "%i,%i,%i", &cur.clear_color[0], &cur.clear_color[1], &cur.clear_color[2]);
        if (Ini_Scanf(ini, "window", "%i,%i,%i,%i", &cur.window[0], &cur.window[1], &cur.window[2],
                      &cur.window[3]) == 4) {
            cur.window_on = 1;
        }
        if (cur.window[0] == -1 || cur.window[2] < cur.window[0]) {
            cur.window_on = 0;
        }
        value = Ini_GetValue(ini, "bg.name");
        if (value != NULL && value[0] != '\0') {
            int group = ini->group;

            cur.bg = Bg_Load(value, ini, dir, sb->sprites, sb->anims);
            if (cur.bg == NULL) {
                goto fail;
            }
            Ini_SelectGroupIndex(ini, group);
        }
        Ini_Scanf(ini, "layerall.pos", "%i,%i", &cur.layerall_x, &cur.layerall_y);

        for (j = 0; j < SCENE_LAYERS; j++) {
            SceneLayer *l = &cur.layers[j];

            l->end_time = -1;
            sprintf(buf.path, "layer%i.anim", j);
            value = Ini_GetValue(ini, buf.path);
            if (value != NULL) {
                l->value.anim = atoi(value);
                l->type = SCENE_LAYER_ANIM;
            }
            sprintf(buf.path, "layer%i.text", j);
            value = Ini_GetValue(ini, buf.path);
            if (value != NULL) {
                char *text = malloc(strlen(value) + 1);

                if (text != NULL) {
                    strcpy(text, value);
                    Str_Unquote(text);
                    l->value.text = text;
                    l->type = SCENE_LAYER_TEXT;
                    l->b = 0xFF;
                    l->g = 0xFF;
                    l->r = 0xFF;
                    l->font = -1;
                    sprintf(buf.path, "layer%i.font", j);
                    Ini_Scanf(ini, buf.path, "%i,%i,%i,%i,%i,%i", &l->font, &l->bank, &l->align, &l->r, &l->g, &l->b);
                    sprintf(buf.path, "layer%i.textdelay", j);
                    value = Ini_GetValue(ini, buf.path);
                    if (value != NULL) {
                        l->text_delay = atoi(value);
                    }
                    if (l->font < 0 || l->font >= STORYBOARD_FONTS) {
                        l->font = -1;
                    }
                }
            }
            sprintf(buf.path, "layer%i.offset", j);
            Ini_Scanf(ini, buf.path, "%i,%i", &l->offset_x, &l->offset_y);
            sprintf(buf.path, "layer%i.starttime", j);
            value = Ini_GetValue(ini, buf.path);
            if (value != NULL) {
                l->start_time = atoi(value);
            }
            sprintf(buf.path, "layer%i.endtime", j);
            value = Ini_GetValue(ini, buf.path);
            if (value != NULL) {
                l->end_time = atoi(value);
            }
        }

        value = Ini_GetValue(ini, "end.time");
        if (value == NULL) {
            sprintf(g_libraryErrorText, "\nIn %s: end.time not specified.", line);
            goto fail;
        }
        cur.end_time = atoi(value);

        for (j = 0; j < SCENE_SOUNDS; j++) {
            snd.group = 0;
            snd.number = 0;
            snd.volume = 0.0;
            snd.unused_10 = -1;
            snd.unused_14 = 1.0f;
            snd.unused_18 = 0;
            snd.pan = 0;
            snd.start_time = 0;
            snd.unused_24 = 0;
            sprintf(buf.path, "sound%i.value", j);
            if (Ini_Scanf(ini, buf.path, "%i,%i", &snd.group, &snd.number) == 2) {
                KeyedArray *list;

                sprintf(buf.path, "sound%i.volumescale", j);
                value = Ini_GetValue(ini, buf.path);
                if (value == NULL || sscanf(value, "%lf", &snd.volume) != 1) {
                    snd.volume = 100.0;
                }
                sprintf(buf.path, "sound%i.pan", j);
                value = Ini_GetValue(ini, buf.path);
                if (value != NULL) {
                    snd.pan = atoi(value);
                }
                sprintf(buf.path, "sound%i.starttime", j);
                value = Ini_GetValue(ini, buf.path);
                if (value != NULL) {
                    snd.start_time = atoi(value);
                }
                if (cur.sounds == NULL) {
                    cur.sounds = KeyedArray_Create(10, sizeof(SceneSound));
                    cur.sounds->grow_step = 10;
                }
                list = cur.sounds;
                if (list->count < list->capacity || KeyedArray_Grow(list, list->grow_step + list->capacity)) {
                    KeyedArray_Add(list, &snd, 0, j);
                }
            }
        }
        KeyedArray_Add(sb->scenes, &cur, 0, i);
    }
    free(lines->data);
    free(lines);

    for (k = 0; k < STORYBOARD_TRACKS; k++) {
        for (j = 0; j < SCENE_LAYERS; j++) {
            SpriteSet *sprites = sb->sprites;
            Anim *anims = sb->anims;
            AnimState *st = calloc(1, sizeof(AnimState));

            if (st == NULL) {
                g_outOfMemory = 1;
            } else {
                st->anim = anims;
                st->sprites = sprites;
            }
            sb->tracks[k].anims[j] = st;
            if (st == NULL) {
                goto fail;
            }
        }
    }
    Storyboard_Reset(sb);
    return sb;

fail:
    Storyboard_Free(sb);
    return NULL;
}

/* 0x00475D90: sets the localcoord to screen scale; values <= 0 become 1 (NaN is kept). */
void Storyboard_SetScale(Storyboard *sb, float scale_x, float scale_y)
{
    if (scale_x <= 0.0f) {
        scale_x = 1.0f;
    }
    if (scale_y <= 0.0f) {
        scale_y = 1.0f;
    }
    sb->scale_x = scale_x;
    sb->scale_y = scale_y;
}

/* 0x00475DE0: rewinds playback to the start scene (first track only) and stops the BGM. */
void Storyboard_Reset(Storyboard *sb)
{
    int k;

    for (k = 0; k < STORYBOARD_TRACKS; k++) {
        StoryboardTrack *t = &sb->tracks[k];

        memset(t->anim_on, 0, sizeof(t->anim_on));
        t->frames = 0;
        t->time = 0;
        t->scene = sb->start_scene;
        t->active = 0;
    }
    sb->tracks[0].active = 1;
    Bgm_Stop();
}

/* ------------------------------------------------------------------ playback */

/* Sound s of a scene, through the sound slots (inlined in Storyboard_Tick). */
static void storyboard_play_sound(Snd *snd, const SceneSound *s)
{
    int slot;

    if (g_sndActive != 1) {
        return;
    }
    slot = Sound_FindFreeSlot();
    if (slot == -1) {
        return;
    }
    if (Sound_PrepareSlot(slot, snd, s->group, s->number) == -1) {
        return;
    }
    g_sndSlots[slot].owner = -1;
    Sound_SetSlotVolume(slot, s->volume);
    Sound_SetSlotPan(slot, s->pan);
    Sound_PlaySlot(slot);
}

/*
 * 0x00475EB0: advances the active tracks by one tick: moves to the next
 * scene when the current one has run end.time ticks, then (on a scene's
 * first tick) restarts its layer animations and background and changes the
 * BGM, starts / stops / steps the layer animations, ticks the background
 * and plays the sounds due. Returns 1 when the last scene is over.
 *
 * Quirks kept: a start scene out of range is used through a NULL scene
 * (crash); the sounds play even when an earlier one of the same tick took
 * the last free slot.
 */
int Storyboard_Tick(Storyboard *sb)
{
    int done[STORYBOARD_TRACKS];
    int k, j;

    done[0] = 0;
    done[1] = 0;
    for (;;) {
        if ((sb->tracks[0].active == 0 || done[0] != 0) && (sb->tracks[1].active == 0 || done[1] != 0)) {
            for (k = 0; k < STORYBOARD_TRACKS; k++) {
                sb->tracks[k].frames++;
            }
            return 0;
        }
        for (k = 0; k < STORYBOARD_TRACKS; k++) {
            StoryboardTrack *t = &sb->tracks[k];
            Scene *scene;
            int start = 0;

            if (t->active == 0) {
                continue;
            }
            scene = KeyedArray_Get(sb->scenes, 0, t->scene);
            if (t->time >= scene->end_time && scene->end_time != -1) {
                t->scene++;
                if (t->scene >= sb->scene_count) {
                    return 1;
                }
                scene = KeyedArray_Get(sb->scenes, 0, t->scene);
                t->time = 0;
                start = 1;
            } else if (t->time == 0) {
                start = 1;
            }
            if (start) {
                if (sb->anims != NULL) {
                    memset(t->anim_on, 0, sizeof(t->anim_on));
                }
                if (scene->bg != NULL) {
                    Bg_Reset(scene->bg);
                }
                if (scene->bgm_change != 0) {
                    Bgm_Stop();
                }
                if (scene->bgm.path[0] != '\0') {
                    Bgm_Play(&scene->bgm);
                }
            }

            if (sb->anims != NULL) {
                for (j = 0; j < SCENE_LAYERS; j++) {
                    const SceneLayer *l = &scene->layers[j];

                    if (l->start_time == t->time && l->type == SCENE_LAYER_ANIM && l->value.anim >= 0) {
                        t->anim_on[j] = Anim_SetAction(t->anims[j], l->value.anim);
                    }
                    if (l->end_time == t->time && l->type == SCENE_LAYER_ANIM) {
                        t->anim_on[j] = 0;
                    }
                }
            }
            if (sb->anims != NULL) {
                for (j = 0; j < SCENE_LAYERS; j++) {
                    if (t->anim_on[j] != 0) {
                        Anim_Step(t->anims[j]);
                    }
                }
            }
            if (scene->bg != NULL) {
                Bg_Tick(scene->bg);
            }
            if (scene->sounds != NULL && sb->snd != NULL) {
                /* An iteration of the sound list that leaves its iteration cursor alone. */
                KeyedArray *list = scene->sounds;
                const SceneSound *s = NULL;
                int it = 0;

                if (list->count != 0) {
                    it = list->first;
                    s = (const SceneSound *)(list->data + list->elem_size * it);
                }
                while (s != NULL) {
                    if (t->time == s->start_time) {
                        storyboard_play_sound(sb->snd, s);
                    }
                    if (it >= list->last) {
                        break;
                    }
                    do {
                        it++;
                    } while (!list->slots[it].used);
                    s = (const SceneSound *)(list->data + list->elem_size * it);
                }
            }
            t->time++;
            if (t->time >= scene->end_time && scene->end_time != -1 && t->scene + 1 >= sb->scene_count) {
                return 1;
            }
            done[k] = 1;
        }
    }
}

/*
 * 0x00476440: applies the fade in / fade out of the current scenes to the
 * global PalFX: a factor of 256 at full brightness, multiplied in below 256
 * (fade from black), added as an offset above it (fade from white, colour
 * >= 128). The fade out wins over the fade in.
 */
void Storyboard_ApplyFade(Storyboard *sb)
{
    int level = 2;
    int k;

    for (k = 0; k < STORYBOARD_TRACKS; k++) {
        const StoryboardTrack *t = &sb->tracks[k];
        const Scene *scene;
        int fading = 0;
        int left;

        if (t->active == 0) {
            continue;
        }
        scene = KeyedArray_Get(sb->scenes, 0, t->scene);
        if (t->time - 1 <= scene->fadein_time) {
            if (scene->fadein_time > 0) {
                level = (t->time - 1) * 256 / scene->fadein_time;
                if (scene->fadein_col >= 0x80) {
                    level = 0x200 - level;
                }
            } else {
                level = 0x100;
            }
            fading = 1;
        }
        left = scene->end_time - t->time;
        if (left < scene->fadeout_time && scene->fadeout_time > 0) {
            level = left * 256 / scene->fadeout_time;
            if (scene->fadeout_col >= 0x80) {
                level = 0x200 - level;
            }
        } else if (!fading) {
            continue;
        }
        if (level <= 0x100) {
            PalFx_ScaleGlobalMul(level, level, level);
        } else if (level - 0x100 != 0) {
            int add = level - 0x100;

            g_palFx.add[0] += add;
            g_palFx.add[1] += add;
            g_palFx.add[2] += add;
            g_palFx.serial = g_palFxSerial++;
        }
    }
}

/*
 * 0x00476570: draws the current scenes: clear colour, background (back
 * layers), layer animations and texts (texts with a delay appear one
 * character per textdelay ticks), background front layers, all inside the
 * scene's window when it has one.
 *
 * Quirk kept: the window's y1 is multiplied by the scale, the other
 * coordinates are not.
 */
void Storyboard_Draw(Storyboard *sb)
{
    int clip_x1 = g_clipX1;
    int clip_y1 = g_clipY1;
    int clip_x2 = g_clipX2;
    int clip_y2 = g_clipY2;
    float scale_x = sb->scale_x;
    float scale_y = sb->scale_y;
    int k, j;

    for (k = 0; k < STORYBOARD_TRACKS; k++) {
        StoryboardTrack *t = &sb->tracks[k];
        Scene *scene;

        if (t->active == 0) {
            continue;
        }
        scene = KeyedArray_Get(sb->scenes, 0, t->scene);
        if (scene->clear_color[0] != -1) {
            g_gfxClear(Video_MapRGB(scene->clear_color[0], scene->clear_color[1], scene->clear_color[2]));
        }
        if (scene->bg != NULL) {
            Bg_SetScale(scene->bg, scale_x, scale_y);
            Bg_SetCamera(scene->bg, 0.0f, 0.0f, 0.0f, sb->pos_y / scale_y, 1.0f, 0.0f, 0.0f);
            scene->bg->unused_14 = (int)(sb->pos_x / scale_x);
        }
        if (scene->window_on != 0) {
            Gfx_SetClipRectScaled((float)((double)scene->window[0] + sb->pos_x / scale_x),
                                  (float)((double)scene->window[1] * scale_y + sb->pos_y / scale_y),
                                  (float)((double)scene->window[2] + sb->pos_x / scale_x),
                                  (float)((double)scene->window[3] + sb->pos_y / scale_y), scale_x, scale_y);
        }
        if (scene->bg != NULL) {
            Bg_Draw(scene->bg, 0);
        }
        for (j = 0; j < SCENE_LAYERS; j++) {
            const SceneLayer *l = &scene->layers[j];
            int time = t->time - 1;
            float x = (float)((double)(l->offset_x + scene->layerall_x) * scale_x + sb->pos_x);
            float y = (float)((double)(scene->layerall_y + l->offset_y) * scale_y + sb->pos_y);
            Font *font;

            if (sb->anims != NULL && t->anim_on[j] != 0 && l->type == SCENE_LAYER_ANIM) {
                Anim_DrawAt(t->anims[j], x, y, scale_x, scale_y, 1.0f, 1.0f, 0);
            }
            if (l->type == SCENE_LAYER_TEXT && time >= l->start_time && (time < l->end_time || l->end_time == -1) &&
                l->value.text != NULL && l->font != -1 && (font = sb->fonts[l->font]) != NULL) {
                if (l->text_delay <= 0) {
                    Font_DrawTextScaled(l->value.text, font, x, y, scale_x, scale_y, l->align, l->bank, l->r, l->g,
                                        l->b);
                } else {
                    int len = Storyboard_Utf8Prefix(l->value.text, (time - l->start_time) / l->text_delay + 1);
                    char *s = malloc(len + 1);

                    if (s != NULL) {
                        strncpy(s, l->value.text, len);
                        s[len] = '\0';
                        Font_DrawTextScaled(s, font, x, y, scale_x, scale_y, l->align, l->bank, l->r, l->g, l->b);
                        free(s);
                    }
                }
            }
        }
        if (scene->bg != NULL) {
            Bg_Draw(scene->bg, 1);
        }
        if (scene->window_on != 0) {
            Gfx_SetClipRect(clip_x1, clip_y1, clip_x2, clip_y2);
        }
    }
}

/* ------------------------------------------------------------------ freeing */

/* 0x004769C0: stops the BGM and frees a cutscene (NULL is accepted). */
void Storyboard_Close(Cutscene *cs)
{
    if (cs == NULL) {
        return;
    }
    if (cs->storyboard != NULL) {
        Bgm_Stop();
        Storyboard_Free(cs->storyboard);
    }
    free(cs);
}

/* 0x004769F0: frees a storyboard and what it owns (NULL is accepted). */
void Storyboard_Free(Storyboard *sb)
{
    KeyedArray *list;
    Scene *scene;
    int k, j;

    if (sb == NULL) {
        return;
    }
    list = sb->scenes;
    if (list != NULL) {
        for (scene = KeyedArray_First(list); scene != NULL; scene = KeyedArray_Next(list)) {
            if (scene->bg != NULL) {
                Bg_Free(scene->bg);
            }
            for (j = 0; j < SCENE_LAYERS; j++) {
                if (scene->layers[j].type == SCENE_LAYER_TEXT && scene->layers[j].value.text != NULL) {
                    free(scene->layers[j].value.text);
                }
            }
            if (scene->sounds != NULL) {
                KeyedArray_Free(scene->sounds);
            }
        }
    }
    if (sb->scenes != NULL) {
        KeyedArray_Free(sb->scenes);
    }
    for (k = 0; k < STORYBOARD_TRACKS; k++) {
        for (j = 0; j < SCENE_LAYERS; j++) {
            AnimState *st = sb->tracks[k].anims[j];

            if (st != NULL) {
                if (st->owns_sprites != 0) {
                    SpriteSet_Free(st->sprites);
                }
                free(st);
            }
        }
    }
    if (sb->owns_anims != 0) {
        Anim_Free(sb->anims);
    }
    if (sb->owns_sprites != 0) {
        SpriteSet_Free(sb->sprites);
    }
    if (sb->owns_snd != 0 && sb->snd != NULL) {
        Snd_Free(sb->snd);
    }
    for (j = 0; j < STORYBOARD_FONTS; j++) {
        if (sb->owns_fonts[j] != 0 && sb->fonts[j] != NULL) {
            Font_Free(sb->fonts[j]);
        }
    }
    free(sb);
}

/*
 * 0x00476BF0: byte length of the first chars characters of s, for the text
 * delay: a UTF-8 sequence counts as one character, and the continuation
 * bytes after the last character are included.
 *
 * Quirks kept: a step that starts on a non-ASCII byte, a blank or a control
 * character also takes the continuation bytes after it and the character
 * that follows them (an ideographic space, E3 80 80, is entered first);
 * when the text ends with a continuation byte the walk reads the byte after
 * the terminator (the caller copies with strncpy, so the drawn text is the
 * same).
 */
int Storyboard_Utf8Prefix(const char *s, int chars)
{
    int i = 0;
    char c;

    if (s[0] != '\0') {
        while (chars > 0) {
            c = s[i];
            if (c != '\0') {
                for (;;) {
                    if (c > 0 && !((unsigned int)(c - 1) <= 0x7E && isspace(c)) && !iscntrl(s[i])) {
                        break;
                    }
                    if ((unsigned char)s[i] == 0xE3 && (unsigned char)s[i + 1] == 0x80 &&
                        (unsigned char)s[i + 2] == 0x80) {
                        i++;
                    }
                    if ((unsigned char)s[i] < 0x80 || (unsigned char)s[i] > 0xBF) {
                        break;
                    }
                    c = s[i + 1];
                    i++;
                    if (c == '\0') {
                        break;
                    }
                }
            }
            i++;
            chars--;
            if (s[i] == '\0') {
                break;
            }
        }
    }
    while (s[i] != '\0' && (unsigned char)s[i] >= 0x80 && (unsigned char)s[i] <= 0xBF) {
        i++;
    }
    return i;
}
