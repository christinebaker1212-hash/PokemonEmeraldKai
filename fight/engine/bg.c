/*
 * Backgrounds (original module 0x470CA0-0x4745EF): stage, motif and
 * storyboard backgrounds.
 *
 * A background is read from the groups of a parsed text file named after it
 * (Bg_Load(name)): "[<name>def]" (sprite file, clear colour, the file's
 * animations), one "[<name> ...]" group per element (normal, parallax, anim
 * or dummy, with tiling, windows, velocity, sine waves...) and the
 * "[<name>CtrlDef ...]" / "[<name>Ctrl ...]" controllers that change the
 * elements over time (BgCtrl_*).
 *
 * Every tick the owner calls Bg_Tick (controllers, animations, velocities),
 * then Bg_SetCamera / Bg_SetScale (screen position, zoom and window of every
 * element) and Bg_Draw for the back (layer 0) and front (layer 1) elements.
 * Parallax elements are drawn as a sheared sprite by the OpenGL renderers and
 * row by row by the software renderer.
 *
 * Floating point follows the x87 code of the original: intermediate results
 * stay in double (the FPU runs with 53-bit precision) and are rounded to
 * float exactly where the original stores them. sin() and fmod() go through
 * MSVCR90's _CIsin / _CIfmod via mugen.exe's import thunks, as in the
 * original; float to int conversions are those of __ftol2_sse (Ftol), except
 * where the original inlines a 64-bit fistp (Trunc64).
 *
 * Quirks kept from the original:
 * - Bg_Load: with "xscale" on an animated parallax element that also has a
 *   "spriteno" missing from the sprite file, the widths are stale stack values
 *   in the original (0 here);
 * - BgElem_AddAnim leaks the animation state when the action does not exist;
 *   elements with "actionno" crash when the background has no animations;
 * - BgElem_DrawParallaxGL skips animated parallax elements of action 0;
 * - BgElem_DrawParallaxSoftMasked loops forever on a negative row length
 *   (negative scale); the unmasked version stops instead;
 * - BgElem_DrawParallaxSoft(Masked) divide by zero when a tiled row is less
 *   than a pixel wide;
 * - Bg_SetCamera: elements with "positionlink" follow the last unlinked
 *   element processed (or 0, 0 when there is none yet).
 */
#include "mugen.h"

#define BG_PI           3.141592653589793
#define BG_FIXED_ONE    65536.0             /* 16.16 fixed point steps of the software parallax */

#define Bg_Sin(x)        sin(x)
#define Bg_CIfmod(x, y)  fmod(x, y)

static Sprite *Bg_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *list = set->sprites;

    return (Sprite *)(list->data + list->elem_size * index);
}

/* Group header "[<name> ...]" or "[<name>]": header starts with name followed by a blank or ']'. */
static int Bg_IsGroup(const char *header, const char *name, int len)
{
    return Str_StartsWithI(header + 1, name) && (Char_IsSpace(header[len + 1]) || header[len + 1] == ']');
}

/* Defaults of a controller (and of every "[<name>CtrlDef]"). */
static void BgCtrl_InitDefault(BgCtrl *def)
{
    memset(def, 0, sizeof(*def));
    def->type = BGCTRL_NULL;
    def->looptime = -1;
    def->ids[0] = -1;
    def->event_id = -1;
}

/*
 * 0x00470CA0: loads background `name` from ini. Without "spr" in
 * [<name>def] the elements use `sprites`; without `anim` the animations are
 * the file's "[Begin Action]" groups. dir is the directory of the file.
 * Returns NULL (with a match log message) on errors.
 */
Bg *Bg_Load(const char *name, Ini *ini, const char *dir, SpriteSet *sprites, Anim *anim)
{
    char key_space[40];
    char key_close[40];
    char group[0x200];
    int name_len = (int)strlen(name);
    float width_top = 0.0f, width_bottom = 0.0f;   /* kept across elements, as in the original */
    float xscale_top, xscale_bottom;
    const char *header;
    const char *s;
    int count, debugbg, i;
    Bg *bg;

    sprintf(key_space, "%s ", name);
    sprintf(key_close, "%s]", name);
    count = Ini_CountGroups(ini, key_close);
    count += Ini_CountGroups(ini, key_space);
    if (count == 0) {
        MatchLog_Printf(3, "Need at least one [%s] type bg\n", name);
        return NULL;
    }
    bg = Bg_Create(count);
    if (bg == NULL) {
        return NULL;
    }
    strcpy(group, name);
    strcat(group, "def");
    if (Ini_SelectGroup(ini, group) == -1) {
        MatchLog_Printf(3, "Can't access group %s\n", group);
        Bg_Free(bg);
        return NULL;
    }
    s = Ini_GetValue(ini, "spr");
    if (s != NULL) {
        Path_Join(s, dir, group);
        bg->sprites = Sff_Load(group);
        if (bg->sprites == NULL) {
            MatchLog_Printf(3, "Error reading sprite %s\n", s);
            Bg_Free(bg);
            return NULL;
        }
        bg->owns_sprites = 1;
    } else {
        if (sprites == NULL) {
            MatchLog_Printf(3, "Error: sff file must be specified\n");
            Bg_Free(bg);
            return NULL;
        }
        bg->sprites = sprites;
        bg->owns_sprites = 0;
    }
    if (Ini_GetInt(ini, "debugbg", &debugbg)) {
        if (debugbg != 0) {
            bg->clear_color[2] = 0xFF;
            bg->clear_color[0] = 0xFF;
            bg->clear_color[1] = 0;
        }
    } else {
        Ini_Scanf(ini, "bgclearcolor", "%i,%i,%i", &bg->clear_color[0], &bg->clear_color[1], &bg->clear_color[2]);
    }
    if (anim != NULL) {
        bg->anim = anim;
        bg->owns_anim = 0;
    } else {
        ini->group = -1;
        header = Ini_NextGroup(ini);
        while (!Str_StartsWithI(header + 1, "begin action ")) {
            header = Ini_NextGroup(ini);
            if (header == NULL) {
                break;
            }
        }
        if (header != NULL) {
            bg->anim = Anim_Parse(ini);
            if (bg->anim == NULL) {
                MatchLog_Printf(3, "Error loading anim\n");
                Bg_Free(bg);
                return NULL;
            }
            bg->owns_anim = 1;
        }
    }

    ini->group = -1;
    header = Ini_NextGroup(ini);
    for (i = 0; i < count; i++) {
        float start_x = 0.0f, start_y = 0.0f;
        float delta_x = 1.0f, delta_y = 1.0f;
        float scalestart_x = 1.0f, scalestart_y = 1.0f;
        float scaledelta_x = 0.0f, scaledelta_y = 0.0f;
        float zoomdelta = 1.0f;
        float velocity_x = 0.0f, velocity_y = 0.0f;
        int group_no = -1, number = -1, action = -1;
        int trans = TRANS_DEFAULT, alpha_src = 256, alpha_dst = 0, mask = 0;
        int tile_x = 0, tile_y = 0;
        int layer = 0;
        int type, n, slot, bad;
        BgElem *e;

        while (ini->group != ini->group_count) {
            if (Bg_IsGroup(header, name, name_len)) {
                break;
            }
            header = Ini_NextGroup(ini);
        }
        s = Ini_GetValue(ini, "layerno");
        if (s != NULL) {
            layer = atoi(s);
        }
        s = Ini_GetValue(ini, "type");
        if (s == NULL) {
            MatchLog_Printf(3, "Missing type in %s\n", header);
            Bg_Free(bg);
            return NULL;
        }
        switch (Char_ToLower(s[0])) {
        case 'a':
            type = BG_TYPE_ANIM;
            break;
        case 'd':
            type = BG_TYPE_DUMMY;
            break;
        case 'h':
            type = BG_TYPE_H;
            break;
        case 'n':
            type = BG_TYPE_NORMAL;
            break;
        case 'p':
            type = BG_TYPE_PARALLAX;
            break;
        default:
            MatchLog_Printf(3, "Invalid type: %s in %s\n", s, header);
            Bg_Free(bg);
            return NULL;
        }
        Ini_Scanf(ini, "start", "%f,%f", &start_x, &start_y);
        Ini_Scanf(ini, "delta", "%f,%f", &delta_x, &delta_y);
        Ini_Scanf(ini, "scalestart", "%f,%f", &scalestart_x, &scalestart_y);
        Ini_Scanf(ini, "scaledelta", "%f,%f", &scaledelta_x, &scaledelta_y);
        Ini_Scanf(ini, "zoomdelta", "%f", &zoomdelta);
        delta_x = (float)(delta_x * -1.0);
        delta_y = (float)(delta_y * -1.0);
        scaledelta_x = (float)(scaledelta_x * -1.0);
        scaledelta_y = (float)(-1.0 * scaledelta_y);
        Ini_Scanf(ini, "velocity", "%f,%f", &velocity_x, &velocity_y);
        n = Ini_Scanf(ini, "spriteno", "%i,%i", &group_no, &number);
        if (n != 0 && n != 2) {
            MatchLog_Printf(3, "Invalid number of args for spriteno in %s\n", header);
            Bg_Free(bg);
            return NULL;
        }
        Ini_GetInt(ini, "actionno", &action);

        /* A normal element without a sprite is an animation and the other way round. */
        bad = 0;
        if (type == BG_TYPE_NORMAL) {
            if (group_no == -1) {
                if (action == -1) {
                    bad = 1;
                } else {
                    type = BG_TYPE_ANIM;
                }
            }
        } else if (type == BG_TYPE_ANIM) {
            if (action == -1) {
                if (group_no == -1) {
                    bad = 1;
                } else {
                    type = BG_TYPE_NORMAL;
                }
            }
        } else if (type == BG_TYPE_PARALLAX) {
            if (group_no == -1 && action == -1) {
                bad = 1;
            }
        }
        if (bad) {
            MatchLog_Printf(3, "Either spriteno or actionno must be specified in %s\n", header);
            Bg_Free(bg);
            return NULL;
        }

        s = Ini_GetValue(ini, "trans");
        if (s != NULL) {
            trans = Trans_Parse(s, &alpha_dst, &alpha_src);
            if (trans == TRANS_INVALID) {
                MatchLog_Printf(3, "Invalid argument: trans = %s in %s", s, header);
                Bg_Free(bg);
                return NULL;
            }
            Ini_Scanf(ini, "alpha", "%i,%i", &alpha_src, &alpha_dst);
            if (trans == TRANS_ADD || trans == TRANS_SUB) {
                mask = 1;
            }
        }
        Ini_GetInt(ini, "mask", &mask);

        if (type == BG_TYPE_DUMMY) {
            group_no = -1;
            action = -1;
        } else if (type == BG_TYPE_ANIM) {
            group_no = -1;
        } else if (type == BG_TYPE_NORMAL && group_no != -1) {
            action = -1;
        }
        if (action == -1) {
            slot = BgElem_AddSprite(type, bg, i, start_x, start_y, delta_x, delta_y, trans, alpha_src, alpha_dst, mask,
                                    group_no, number);
            if (slot == -1) {
                MatchLog_Printf(3, "Error adding BG sprite %i,%i for %s\n", group_no, number, header);
                Bg_Free(bg);
                return NULL;
            }
        } else {
            slot = BgElem_AddAnim(bg, i, type, start_x, start_y, delta_x, delta_y, trans, alpha_src, alpha_dst,
                                  action);
            if (slot == -1) {
                MatchLog_Printf(3, "Error adding BG anim %i for %s\n", action, header);
                Bg_Free(bg);
                return NULL;
            }
        }

        e = KeyedArray_Get(bg->elems, 0, i);
        e->layer = layer;
        e->velocity_x = velocity_x;
        e->velocity_y = velocity_y;
        e->scalestart_x = scalestart_x;
        e->scalestart_y = scalestart_y;
        e->scaledelta_x = scaledelta_x;
        e->scaledelta_y = scaledelta_y;
        e->zoomdelta = zoomdelta;
        Ini_GetInt(ini, "id", &e->id);
        Ini_Scanf(ini, "tile", "%i,%i", &tile_x, &tile_y);
        e->tile_x = tile_x;
        e->tile_y = tile_y;
        Ini_Scanf(ini, "tilespacing", "%f,%f", &e->tilespacing_x, &e->tilespacing_y);
        if (action != -1) {
            /* Animations only tile with at least a pixel between the tiles. */
            if (e->tilespacing_x < 1.0f) {
                e->tile_x = 0;
            }
            if (e->tilespacing_y < 1.0f) {
                e->tile_y = 0;
            }
        }
        e->window = 0;
        if (Ini_Scanf(ini, "window", "%i,%i,%i,%i", &e->window_x1, &e->window_y1, &e->window_x2, &e->window_y2) >= 4) {
            int t;
            e->window = 1;
            e->window_unshifted = 1;
            if (e->window_x2 < e->window_x1) {
                t = e->window_x1;
                e->window_x1 = e->window_x2;
                e->window_x2 = t;
            }
            e->window_x2 += 1;
            if (e->window_y2 < e->window_y1) {
                t = e->window_y1;
                e->window_y1 = e->window_y2;
                e->window_y2 = t;
            }
            e->window_y2 += 1;
        }
        if (Ini_Scanf(ini, "maskwindow", "%i,%i,%i,%i", &e->window_x1, &e->window_y1, &e->window_x2,
                      &e->window_y2) >= 4) {
            e->window = 1;
        }
        Ini_Scanf(ini, "windowdelta", "%f,%f", &e->windowdelta_x, &e->windowdelta_y);
        Ini_GetInt(ini, "positionlink", &e->positionlink);
        Ini_Scanf(ini, "sin.x", "%f, %f, %f", &e->sinx_amp, &e->sinx_period, &e->sinx_offset);
        Ini_Scanf(ini, "sin.y", "%f, %f, %f", &e->siny_amp, &e->siny_period, &e->siny_offset);

        if (type == BG_TYPE_PARALLAX) {
            float yscale_start = 100.0f;
            float yscale_delta = 0.0f;
            int centered = 1;
            int by_width = 0;
            int tile;

            if (Ini_Scanf(ini, "paralwidth", "%f,%f", &width_top, &width_bottom) == 2) {
                by_width = 1;
                centered = 0;
            } else if (Ini_Scanf(ini, "width", "%f,%f", &width_top, &width_bottom) == 2) {
                by_width = 1;
            } else if (Ini_Scanf(ini, "xscale", "%f,%f", &xscale_top, &xscale_bottom) == 2) {
                if (group_no != -1) {
                    Sprite *spr = SpriteSet_FindSprite(bg->sprites, group_no, number);
                    if (spr != NULL) {
                        float w = (float)spr->w;
                        width_top = (float)(w * xscale_top);
                        width_bottom = (float)(w * xscale_bottom);
                    }
                } else if (action != -1) {
                    MatchLog_Printf(3, "Animated parallax not supported when using xscale parameter in %s.\n", header);
                    Bg_Free(bg);
                    return NULL;
                }
            } else {
                MatchLog_Printf(3, "Missing or invalid width or xscale for parallax in %s.\n", header);
                Bg_Free(bg);
                return NULL;
            }
            Ini_GetFloat(ini, "yscalestart", &yscale_start);
            yscale_start = (float)(yscale_start * 0.01);
            Ini_GetFloat(ini, "yscaledelta", &yscale_delta);
            yscale_delta = (float)(yscale_delta * 0.01);
            tile = (tile_y != 0 ? BG_TILE_Y : 0) | (tile_x != 0 ? BG_TILE_X : 0);
            if (!BgElem_InitParallax(tile, e, bg, width_top, width_bottom, yscale_start, yscale_delta, by_width)) {
                MatchLog_Printf(3, "Can't init parallax in %s.\n", header);
                Bg_Free(bg);
                return NULL;
            }
            e->centered = centered;
        }
        header = Ini_NextGroup(ini);
    }
    if (!BgCtrl_Load(name, bg, ini)) {
        MatchLog_Printf(3, "Error reading bg control data\n");
        Bg_Free(bg);
        return NULL;
    }
    return bg;
}

/*
 * 0x00471980: reads the "[<name>Ctrl ...]" controllers. "[<name>CtrlDef]"
 * groups before them set the defaults (looptime, eventID, ctrlid) of the
 * controllers that follow. Returns 0 on errors.
 */
int BgCtrl_Load(const char *name, Bg *bg, Ini *ini)
{
    char key_ctrl[40];
    char key_def[40];
    BgCtrl def;
    const char *header;
    const char *s;
    int len, count, i, n;

    sprintf(key_ctrl, "%sctrl", name);
    sprintf(key_def, "%sctrldef", name);
    len = (int)strlen(key_ctrl);
    n = Ini_CountGroups(ini, key_def);
    count = Ini_CountGroups(ini, key_ctrl) - n;
    if (count == 0) {
        return 1;
    }
    bg->ctrl_count = count;
    bg->ctrls = calloc(count, sizeof(BgCtrl));
    if (bg->ctrls == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    BgCtrl_InitDefault(&def);
    ini->group = -1;
    header = Ini_NextGroup(ini);
    for (i = 0; i < count; i++) {
        BgCtrl *c;

        while (ini->group != ini->group_count) {
            if (Str_PrefixMatchI(header + 1, key_def, len + 3) &&
                (Char_IsSpace(header[len + 4]) || header[len + 4] == ']')) {
                BgCtrl_InitDefault(&def);
                s = Ini_GetValue(ini, "looptime");
                if (s != NULL) {
                    def.looptime = atoi(s);
                }
                s = Ini_GetValue(ini, "eventID");
                if (s != NULL) {
                    def.event_id = atoi(s);
                }
                n = Ini_Scanf(ini, "ctrlid", "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i", &def.ids[0], &def.ids[1], &def.ids[2],
                              &def.ids[3], &def.ids[4], &def.ids[5], &def.ids[6], &def.ids[7], &def.ids[8],
                              &def.ids[9]);
                def.ids[n] = -1;
            }
            if (Bg_IsGroup(header, key_ctrl, len)) {
                break;
            }
            header = Ini_NextGroup(ini);
        }
        c = &bg->ctrls[i];
        *c = def;
        n = Ini_Scanf(ini, "ctrlid", "%i,%i,%i,%i,%i,%i,%i,%i,%i,%i", &c->ids[0], &c->ids[1], &c->ids[2], &c->ids[3],
                      &c->ids[4], &c->ids[5], &c->ids[6], &c->ids[7], &c->ids[8], &c->ids[9]);
        c->ids[n] = -1;
        if (c->ids[0] == -1) {
            memcpy(c->ids, def.ids, sizeof(c->ids));
        }
        s = Ini_GetValue(ini, "type");
        if (s == NULL) {
            MatchLog_Printf(3, "%i/%i %s: Missing \"type\" parameter.\n", i, count, header);
            return 0;
        }
        if (_stricmp(s, "visible") == 0) {
            c->type = BGCTRL_VISIBLE;
        } else if (_stricmp(s, "enable") == 0) {
            c->type = BGCTRL_ENABLE;
        } else if (_stricmp(s, "velset") == 0) {
            c->type = BGCTRL_VELSET;
        } else if (_stricmp(s, "veladd") == 0) {
            c->type = BGCTRL_VELADD;
        } else if (_stricmp(s, "posset") == 0) {
            c->type = BGCTRL_POSSET;
        } else if (_stricmp(s, "posadd") == 0) {
            c->type = BGCTRL_POSADD;
        } else if (_stricmp(s, "anim") == 0) {
            c->type = BGCTRL_ANIM;
        } else if (_stricmp(s, "sinx") == 0) {
            c->type = BGCTRL_SINX;
        } else if (_stricmp(s, "siny") == 0) {
            c->type = BGCTRL_SINY;
        } else if (_stricmp(s, "null") == 0) {
            c->type = BGCTRL_NULL;
        } else if (_stricmp(s, "dummy") == 0) {
            c->type = BGCTRL_DUMMY;
        } else {
            MatchLog_Printf(3, "Invalid ctrl type: %s\n", s);
            return 0;
        }
        if (Ini_Scanf(ini, "time", "%i,%i,%i", &c->start, &c->end, &c->looptime) < 2) {
            c->end = c->start;
        }
        switch (c->type) {
        case BGCTRL_VISIBLE:
        case BGCTRL_ENABLE:
        case BGCTRL_ANIM:
            Ini_GetInt(ini, "value", &c->value);
            break;
        case BGCTRL_POSSET:
        case BGCTRL_VELSET:
        case BGCTRL_POSADD:
        case BGCTRL_VELADD:
            c->y = (float)BGCTRL_UNSET;
            c->x = (float)BGCTRL_UNSET;
            Ini_GetFloat(ini, "x", &c->x);
            Ini_GetFloat(ini, "y", &c->y);
            break;
        case BGCTRL_SINX:
        case BGCTRL_SINY:
            Ini_Scanf(ini, "value", "%f,%f,%f", &c->x, &c->y, &c->z);
            break;
        }
        header = Ini_NextGroup(ini);
    }
    return 1;
}

/* 0x00471F30: an empty background with room for count elements. */
Bg *Bg_Create(int count)
{
    Bg *bg = calloc(1, sizeof(Bg));

    if (bg == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    bg->elems = KeyedArray_Create(count, sizeof(BgElem));
    if (bg->elems == NULL) {
        Bg_Free(bg);
        return NULL;
    }
    bg->zoom_center_x = 0.0f;
    bg->clear_cur[0] = -1;
    bg->zoom_center_y = 0.0f;
    bg->clear_color[0] = -1;
    bg->sprites = NULL;
    bg->zoom = 1.0f;
    return bg;
}

/* Frees the first rows of a parallax row table (as many as the element is high). */
static void BgElem_FreeRows(const BgElem *e, unsigned char **rows)
{
    int k;

    for (k = 0; (double)k < e->height; k++) {
        if (rows[k] != NULL) {
            free(rows[k]);
        }
    }
}

/* 0x00471FA0 */
void Bg_Free(Bg *bg)
{
    KeyedArray *list;
    BgElem *e;

    if (bg == NULL) {
        return;
    }
    list = bg->elems;
    if (list != NULL) {
        for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
            if (e->type == BG_TYPE_PARALLAX) {
                if (e->row_widths != NULL) {
                    free(e->row_widths);
                }
                if (e->rows != NULL) {
                    if (e->rows_filled != 0) {
                        BgElem_FreeRows(e, e->rows);
                    }
                    free(e->rows);
                }
                if (e->rows2 != NULL) {
                    if (e->rows2_filled != 0) {
                        BgElem_FreeRows(e, e->rows2);
                    }
                    free(e->rows2);
                }
                if (e->rows3 != NULL) {
                    BgElem_FreeRows(e, e->rows3);
                    free(e->rows3);
                }
            }
            if (e->anim != NULL) {
                if (e->anim->owns_sprites != 0) {
                    SpriteSet_Free(e->anim->sprites);
                }
                free(e->anim);
            }
        }
    }
    KeyedArray_Free(bg->elems);
    if (bg->owns_anim) {
        Anim_Free(bg->anim);
    }
    if (bg->owns_sprites) {
        SpriteSet_Free(bg->sprites);
    }
    if (bg->ctrls != NULL) {
        free(bg->ctrls);
    }
    free(bg);
}

/*
 * 0x004721E0: moves the "window" of every element dx pixels left, once (the
 * stage loader passes half the stage width, to go from stage to screen
 * coordinates). "maskwindow" windows are not moved.
 */
void Bg_ShiftWindows(Bg *bg, int dx)
{
    KeyedArray *list;
    BgElem *e;

    if (bg == NULL) {
        return;
    }
    list = bg->elems;
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(bg->elems)) {
        if (e->window && e->window_unshifted) {
            e->window_x1 -= dx;
            e->window_x2 -= dx;
            e->window_unshifted = 0;
        }
    }
}

/*
 * 0x00472250: adds an element showing sprite (group, number) of the
 * background's sprites (none for BG_TYPE_DUMMY) at slot key (0, index).
 * Returns the slot or -1 (animations, missing sprite).
 */
int BgElem_AddSprite(int type, Bg *bg, int index, float start_x, float start_y, float delta_x, float delta_y,
                     int trans, int alpha_src, int alpha_dst, int mask, int group, int number)
{
    BgElem e;

    if (type == BG_TYPE_ANIM) {
        return -1;
    }
    memset(&e, 0, sizeof(e));
    e.start_x = start_x;
    e.start_y = start_y;
    e.trans = trans;
    e.delta_x = delta_x;
    e.delta_y = delta_y;
    e.type = type;
    e.alpha_src = alpha_src;
    e.scale_y = 1.0f;
    e.alpha_dst = alpha_dst;
    e.scale_x = 1.0f;
    e.mask = mask;
    if (type != BG_TYPE_DUMMY) {
        Sprite *spr;
        e.sprite = KeyedArray_Find(bg->sprites->sprites, group, number);
        if (e.sprite == -1) {
            MatchLog_Printf(3, "Sprite doesn't exist: %i,%i\n", group, number);
            return -1;
        }
        spr = Bg_SpriteAt(bg->sprites, e.sprite);
        e.width = (float)spr->w;
        e.height = (float)spr->h;
    }
    e.enabled = 1;
    e.visible = 1;
    return KeyedArray_Add(bg->elems, &e, 0, index);
}

/*
 * 0x004723A0: adds an element playing action `action` of the background's
 * animations at slot key (0, index). Returns the slot or -1.
 */
int BgElem_AddAnim(Bg *bg, int index, int type, float start_x, float start_y, float delta_x, float delta_y, int trans,
                   int alpha_src, int alpha_dst, int action)
{
    SpriteSet *sprites;
    Anim *anim;
    AnimState *st;
    BgElem e;

    memset(&e, 0, sizeof(e));
    e.start_x = start_x;
    e.type = type;
    e.start_y = start_y;
    sprites = bg->sprites;
    anim = bg->anim;
    e.delta_x = delta_x;
    e.delta_y = delta_y;
    e.scale_y = 1.0f;
    e.scale_x = 1.0f;
    e.trans = trans;
    e.alpha_src = alpha_src;
    e.alpha_dst = alpha_dst;
    st = calloc(1, sizeof(AnimState));
    if (st == NULL) {
        g_outOfMemory = 1;
    } else {
        st->sprites = sprites;
        st->anim = anim;
    }
    e.anim = st;
    if (st == NULL) {
        return -1;
    }
    if (!Anim_SetAction(st, action)) {
        return -1;
    }
    e.action = action;
    e.sprite = -1;
    e.enabled = 1;
    e.visible = 1;
    return KeyedArray_Add(bg->elems, &e, 0, index);
}

/* 0x004724E0: the scale every element is drawn with. */
void Bg_SetScale(Bg *bg, float scale_x, float scale_y)
{
    KeyedArray *list = bg->elems;
    BgElem *e;

    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        e->scale_x = scale_x;
        e->scale_y = scale_y;
    }
}

/*
 * 0x00472540: places the elements for camera position (x, y) and zoom
 * around (center_x, center_y), then moves them by (offset_x, offset_y):
 * screen position, scale, window and parallax y scale of every element.
 */
void Bg_SetCamera(Bg *bg, float x, float y, float offset_x, float offset_y, float zoom, float center_x,
                  float center_y)
{
    KeyedArray *list = bg->elems;
    float link_x = 0.0f;
    float link_y = 0.0f;
    double zoom_m1;
    BgElem *e;

    bg->cam_x = x;
    bg->cam_y = y;
    bg->zoom = zoom;
    bg->zoom_center_x = center_x;
    bg->zoom_center_y = center_y;
    e = KeyedArray_First(list);
    if (e == NULL) {
        return;
    }
    zoom_m1 = (double)zoom - 1.0;
    for (; e != NULL; e = KeyedArray_Next(list)) {
        float z = (float)(e->zoomdelta * zoom_m1 + 1.0);
        float wave_x = 0.0f;
        float wave_y = 0.0f;
        float px, py;       /* position before the start offset and zoom */
        double ys;

        if (e->sinx_period != 0.0f) {
            double a = ((double)e->sinx_time + e->sinx_offset) / e->sinx_period * BG_PI;
            wave_x = (float)(Bg_Sin(a + a) * e->sinx_amp);
        }
        if (e->siny_period != 0.0f) {
            double a = ((double)e->siny_time + e->siny_offset) / e->siny_period * BG_PI;
            wave_y = (float)(Bg_Sin(a + a) * e->siny_amp);
        }
        if (!e->positionlink) {
            float t = (float)bg->ticks;
            px = (float)((double)e->velocity_x * t + (double)e->delta_x * x + wave_x);
            py = (float)((double)t * e->velocity_y + (double)e->delta_y * y + wave_y);
            link_x = (float)(px + (double)e->start_x);
            link_y = (float)((double)e->start_y + py);
            px = (float)(px + (double)e->pos_x);
            py = (float)(py + (double)e->pos_y);
            e->x = (float)(((double)px + e->start_x - center_x) * z + center_x);
            ys = (double)e->start_y + py;
        } else {
            px = (float)((double)e->pos_x + e->start_x + wave_x);
            py = (float)((double)e->pos_y + e->start_y + wave_y);
            e->x = (float)(((double)px + link_x - center_x) * z + center_x);
            ys = (double)py + link_y;
        }
        e->y = (float)((ys - center_y) * z + center_y);
        e->x = (float)(e->x + (double)offset_x);
        e->y = (float)(e->y + (double)offset_y);
        if (e->window) {
            double wx = (double)e->windowdelta_x * x;
            double wy = (double)e->windowdelta_y * y;
            e->clip_x1 = (float)((((double)e->window_x1 - wx) - center_x) * z + center_x + offset_x);
            e->clip_y1 = (float)((((double)e->window_y1 - wy) - center_y) * z + center_y + offset_y);
            e->clip_x2 = (float)((((double)e->window_x2 - wx) - center_x) * z + center_x + offset_x);
            e->clip_y2 = (float)((((double)e->window_y2 - wy) - center_y) * z + center_y + offset_y);
        }
        e->draw_scale_x = (float)(((double)px * e->scaledelta_x + e->scalestart_x) * z);
        e->draw_scale_y = (float)(((double)e->scaledelta_y * py + e->scalestart_y) * z);
        if (e->type == BG_TYPE_PARALLAX) {
            double v = (double)e->yscale_start - (double)e->yscale_delta * y;
            e->yscale = v > 0.0 ? (float)(1.0 / v) : 1000000.0f;
        }
    }
}

/*
 * 0x004728A0: draws the elements of layer `layer` (0 back, 1 front). Layer 0
 * first clears the render target with the clear colour, if there is one.
 */
void Bg_Draw(Bg *bg, int layer)
{
    KeyedArray *list = bg->elems;
    BgElem *e;

    if (bg->clear_cur[0] != -1 && layer == 0) {
        g_gfxClear(Video_MapRGB(bg->clear_cur[0], bg->clear_cur[1], bg->clear_cur[2]));
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->layer == layer) {
            BgElem_Draw(e, bg);
        }
    }
}

/*
 * 0x00472930: one tick: controllers, then animations and sine waves of the
 * enabled elements and the controller velocity of all of them.
 */
void Bg_Tick(Bg *bg)
{
    KeyedArray *list = bg->elems;
    BgElem *e;

    bg->clear_cur[0] = bg->clear_color[0];
    bg->clear_cur[1] = bg->clear_color[1];
    bg->clear_cur[2] = bg->clear_color[2];
    BgCtrl_Run(bg);
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->enabled) {
            if (e->anim != NULL && e->action != -1) {
                Anim_Step(e->anim);
            }
            if (e->sinx_period != 0.0f) {
                e->sinx_time = (float)(e->sinx_time + 1.0);
            }
            if (e->siny_period != 0.0f) {
                e->siny_time = (float)(e->siny_time + 1.0);
            }
        }
        e->pos_x = (float)(e->vel_x + (double)e->pos_x);
        e->pos_y = (float)(e->vel_y + (double)e->pos_y);
    }
    bg->ticks++;
}

/*
 * 0x00472A50: runs the controllers whose time has come (start <= timer <=
 * end, or start -1 on the first tick) on their elements, then advances their
 * timers.
 */
void BgCtrl_Run(Bg *bg)
{
    KeyedArray *list = bg->elems;
    int i;

    if (bg->ctrls == NULL) {
        return;
    }
    for (i = 0; i < bg->ctrl_count; i++) {
        BgCtrl *c = &bg->ctrls[i];

        if (((c->timer >= c->start && c->timer <= c->end) || (c->start == -1 && bg->ticks == 0)) &&
            c->type < BGCTRL_TYPE_LIMIT) {
            BgElem *e;
            for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
                int match = c->ids[0] == -1;
                int k;

                for (k = 0; c->ids[k] != -1; k++) {
                    if (e->id == c->ids[k]) {
                        match = 1;
                    }
                }
                if (!match) {
                    continue;
                }
                switch (c->type) {
                case BGCTRL_VISIBLE:
                    e->visible = c->value;
                    break;
                case BGCTRL_ENABLE:
                    e->enabled = c->value;
                    break;
                case BGCTRL_POSSET:
                    if (c->x != (float)BGCTRL_UNSET) {
                        e->pos_x = c->x;
                    }
                    if (c->y != (float)BGCTRL_UNSET) {
                        e->pos_y = c->y;
                    }
                    break;
                case BGCTRL_VELSET:
                    if (c->x != (float)BGCTRL_UNSET) {
                        e->vel_x = c->x;
                    }
                    if (c->y != (float)BGCTRL_UNSET) {
                        e->vel_y = c->y;
                    }
                    break;
                case BGCTRL_POSADD:
                    if (c->x != (float)BGCTRL_UNSET) {
                        e->pos_x = (float)(e->pos_x + (double)c->x);
                    }
                    if (c->y != (float)BGCTRL_UNSET) {
                        e->pos_y = (float)(c->y + (double)e->pos_y);
                    }
                    break;
                case BGCTRL_VELADD:
                    if (c->x != (float)BGCTRL_UNSET) {
                        e->vel_x = (float)(e->vel_x + (double)c->x);
                    }
                    if (c->y != (float)BGCTRL_UNSET) {
                        e->vel_y = (float)(c->y + (double)e->vel_y);
                    }
                    break;
                case BGCTRL_ANIM:
                    if (e->action != -1 && e->anim != NULL) {
                        Anim_SetAction(e->anim, c->value);
                        Anim_Reset(e->anim);
                    }
                    break;
                case BGCTRL_SINX:
                    e->sinx_amp = c->x;
                    e->sinx_period = c->y;
                    e->sinx_offset = c->z;
                    break;
                case BGCTRL_SINY:
                    e->siny_amp = c->x;
                    e->siny_period = c->y;
                    e->siny_offset = c->z;
                    break;
                }
            }
        }
        c->timer++;
        if (c->timer >= c->looptime && c->looptime != -1) {
            c->timer = 0;
        }
    }
}

/*
 * 0x00472D20: back to the initial state: controller timers, animations,
 * controller position / velocity / visibility, sine waves and the tick count.
 */
void Bg_Reset(Bg *bg)
{
    KeyedArray *list = bg->elems;
    BgElem *e;
    int i;

    bg->clear_cur[0] = bg->clear_color[0];
    bg->clear_cur[1] = bg->clear_color[1];
    bg->clear_cur[2] = bg->clear_color[2];
    for (i = 0; i < bg->ctrl_count; i++) {
        bg->ctrls[i].timer = 0;
    }
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        if (e->action != -1 && e->anim != NULL) {
            Anim_SetAction(e->anim, e->action);
            Anim_Reset(e->anim);
        }
        e->vel_y = 0.0f;
        e->vel_x = 0.0f;
        e->visible = 1;
        e->pos_y = 0.0f;
        e->enabled = 1;
        e->pos_x = 0.0f;
        e->siny_time = 0.0f;
        e->sinx_time = 0.0f;
    }
    bg->ticks = 0;
}

/*
 * 0x00472E40: compatibility with stages made for older versions (the stage
 * loader calls it by the stage's version): changes how anim elements blend
 * and where parallax elements start.
 */
void Bg_SetLegacy(Bg *bg)
{
    KeyedArray *list = bg->elems;
    BgElem *e;

    bg->legacy = 1;
    for (e = KeyedArray_First(list); e != NULL; e = KeyedArray_Next(list)) {
        e->legacy = 1;
    }
}

/* 0x00472EB0: the first element with "id" id, or NULL. */
BgElem *Bg_FindById(Bg *bg, int id)
{
    BgElem *e;

    for (e = KeyedArray_First(bg->elems); e != NULL; e = KeyedArray_Next(bg->elems)) {
        if (e->id == id) {
            return e;
        }
    }
    return NULL;
}

/*
 * 0x00472F10: turns e into a parallax element from width_top to
 * width_bottom ("width" / "paralwidth", or the sprite width times "xscale").
 * For the software renderer the rows of a sprite element are copied: as
 * they are with "xscale", scaled to the width of each row otherwise (8-bit
 * sprites only; other rows stay blank). Only tile & BG_TILE_X is kept.
 * Returns 0 when memory runs out.
 */
int BgElem_InitParallax(int tile, BgElem *e, Bg *bg, float width_top, float width_bottom, float yscale_start,
                        float yscale_delta, int by_width)
{
    Sprite *spr = NULL;
    int is_texture = 0;
    int is_anim = 0;
    unsigned char *pixels;
    int bytes, i;

    if (e->sprite != -1) {
        spr = Bg_SpriteAt(bg->sprites, e->sprite);
        is_texture = spr->format == SPRITE_FORMAT_TEXTURE;
    } else if (e->action != -1) {
        is_anim = 1;
    }
    e->type = BG_TYPE_PARALLAX;
    e->width = width_top;
    e->yscale_start = yscale_start;
    e->yscale_delta = -yscale_delta;
    e->tile_x = tile & BG_TILE_X;
    e->tile_y = 0;
    e->width_top = width_top;
    e->by_width = by_width;
    e->width_bottom = width_bottom;
    e->width_ratio = (float)((double)width_bottom / width_top);
    if (is_anim || is_texture || spr == NULL) {
        return 1;
    }
    bytes = (spr->bpp + 7) >> 3;
    e->row_widths = calloc((size_t)(__int64)e->height, sizeof(int));
    if (e->row_widths == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    e->rows = calloc((size_t)(__int64)e->height, sizeof(unsigned char *));
    if (e->rows == NULL) {
        g_outOfMemory = 1;
        return 0;
    }
    pixels = Sprite_GetPixels(bg->sprites, e->sprite);
    if (pixels == NULL) {
        return 0;
    }
    for (i = 0; (double)i < e->height; i++) {
        if (e->by_width) {
            e->row_widths[i] = Ftol(((double)width_bottom - width_top) * i / (e->height - 1.0) + width_top);
        } else {
            e->row_widths[i] = spr->w;
        }
        if (e->by_width) {
            int k;
            e->rows[i] = Mem_Calloc(e->row_widths[i] + 4, bytes);
            if (e->rows[i] == NULL) {
                return 0;
            }
            for (k = 0; k < e->row_widths[i]; k++) {
                if (bytes == 1) {
                    e->rows[i][k] = pixels[spr->rows[i] + spr->w * k / e->row_widths[i]];
                }
            }
        } else {
            e->rows[i] = Mem_Calloc(spr->w, bytes);
            if (e->rows[i] == NULL) {
                return 0;
            }
            memcpy(e->rows[i], pixels + spr->rows[i], spr->w * bytes);
        }
        e->rows_filled = 1;
    }
    return 1;
}

/*
 * 0x00473220: draws one element at its Bg_SetCamera place, scaled by its
 * Bg_SetScale scale, inside its window (the clip rectangle is restored
 * afterwards). BG_TYPE_H and dummy elements draw nothing.
 */
void BgElem_Draw(BgElem *e, Bg *bg)
{
    int clip_x2 = g_clipX2;
    int clip_y1 = g_clipY1;
    int clip_y2 = g_clipY2;
    int clip_x1 = g_clipX1;
    float half_w = (float)(g_targetWidth / 2);
    float scale_x = e->scale_x;
    float scale_y = e->scale_y;
    float x = (float)(e->x * scale_x);
    float y = (float)(e->y * scale_y);
    DrawParams dp;

    if (!e->enabled || !e->visible) {
        return;
    }
    if (e->window) {
        /* Windows moved by Bg_ShiftWindows are relative to the middle of the screen. */
        float offset = (float)(g_targetWidth * 0.5 / scale_x);
        if (e->window_unshifted) {
            offset = 0.0f;
        }
        Gfx_SetClipRectScaled((float)(offset + (double)e->clip_x1), e->clip_y1, (float)(e->clip_x2 + (double)offset),
                              e->clip_y2, scale_x, scale_y);
    }
    memset(&dp, 0, sizeof(dp));
    dp.scale2_y = 1.0f;
    dp.scale2_x = 1.0f;
    dp.alpha_src = 1.0f;
    dp.bottom_stretch = 1.0f;
    dp.top_stretch = 1.0f;
    dp.width_ratio = 1.0f;
    dp.width_anchor = 0.5f;
    dp.masked = 1;
    dp.flags = DRAW_UNUSED_1000 | DRAW_SCALE | DRAW_POS;
    dp.x = (float)(x + (double)half_w);
    dp.y = y;
    dp.scale_x = (float)(e->draw_scale_x * (double)scale_x);
    dp.scale_y = (float)(e->draw_scale_y * (double)scale_y);
    DrawParams_SetBlend(e->trans, &dp, e->alpha_src, e->alpha_dst);
    if (e->legacy && e->trans == TRANS_NONE && e->type == BG_TYPE_ANIM) {
        dp.flags &= ~DRAW_BLEND;
    }
    dp.masked = e->type == BG_TYPE_ANIM ? 1 : e->mask;
    switch (e->type) {
    case BG_TYPE_NORMAL:
        if (e->tile_x == 0 && e->tile_y == 0) {
            Sprite_Draw(bg->sprites, e->sprite, &dp);
        } else {
            DrawParams_FoldScales(&dp);
            BgElem_DrawTiled(e, &dp, bg);
        }
        break;
    case BG_TYPE_PARALLAX:
        dp.scale_y = (float)(e->yscale * (double)dp.scale_y);
        BgElem_DrawParallax(e, bg, &dp);
        break;
    case BG_TYPE_ANIM:
        if (e->tile_x == 0 && e->tile_y == 0) {
            Anim_Draw(e->anim, &dp, 1.0f, 1.0f);
        } else {
            DrawParams_FoldScales(&dp);
            BgElem_DrawTiled(e, &dp, bg);
        }
        break;
    }
    Gfx_SetClipRect(clip_x1, clip_y1, clip_x2, clip_y2);
}

/*
 * 0x004734B0: draws a tiled normal or anim element: "tile" 1 repeats it over
 * the whole screen, n > 1 n times (at least once, even off screen).
 */
void BgElem_DrawTiled(BgElem *e, const DrawParams *dp, Bg *bg)
{
    float scale_x = dp->scale_x;
    float scale_y = dp->scale_y;
    DrawParams local = *dp;
    float step_x = (float)(((double)e->width + e->tilespacing_x) * scale_x);
    float step_y = (float)(((double)e->height + e->tilespacing_y) * scale_y);
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    int wrap_x = 0;
    int wrap_y = 0;
    int first_col = 1;
    int first_row = 1;
    float start_x, start_y, end_x, end_y, x, y;
    int count_x, count_y;

    if (scale_x == 0.0f || scale_y == 0.0f) {
        return;
    }
    if (e->tile_x != 0 && (float)fabs(step_x) < 1.0) {
        return;
    }
    if (e->tile_y != 0 && (float)fabs(step_y) < 1.0) {
        return;
    }
    if (e->sprite != -1) {
        const Sprite *spr = Bg_SpriteAt(bg->sprites, e->sprite);
        offset_x = (float)((double)spr->x * scale_x);
        offset_y = (float)((double)scale_y * spr->y);
    }
    start_x = dp->x;
    start_y = dp->y;
    count_x = e->tile_x != 0 ? e->tile_x : 1;
    count_y = e->tile_y != 0 ? e->tile_y : 1;
    if (e->tile_x == 1) {
        start_x = (float)(Bg_Fmod((float)(start_x - (double)offset_x), step_x) + (double)offset_x);
        count_x = -1;
        wrap_x = 1;
    }
    if (e->tile_y == 1) {
        start_y = (float)(Bg_Fmod((float)(start_y - (double)offset_y), step_y) + (double)offset_y);
        count_y = -1;
        wrap_y = 1;
    }
    end_x = (float)((double)g_targetWidth + offset_x);
    end_y = (float)(offset_y + (double)g_targetHeight);
    if (wrap_x > 0) {
        double d = (double)wrap_x * step_x;
        start_x = (float)(start_x - d);
        end_x = (float)(d + end_x);
    }
    if (wrap_y > 0) {
        double d = (double)wrap_y * step_y;
        start_y = (float)(start_y - d);
        end_y = (float)(d + end_y);
    }
    y = start_y;
    while ((y < end_y && count_y != 0) || first_row > 0) {
        int cx = count_x;

        x = start_x;
        while ((x < end_x && cx != 0) || first_col > 0) {
            local.x = x;
            local.y = y;
            if (e->type == BG_TYPE_NORMAL) {
                if (g_renderMode == RENDER_MODE_GL) {
                    Sprite_DrawGL(bg->sprites, e->sprite, &local);
                } else {
                    Sprite_DrawSoft(bg->sprites, e->sprite, &local);
                }
            } else if (e->type == BG_TYPE_ANIM) {
                Anim_Draw(e->anim, &local, 1.0f, 1.0f);
            }
            x = (float)(x + (double)step_x);
            if (cx > 0) {
                cx--;
            }
            if (first_col > 0) {
                first_col--;
            }
        }
        y = (float)(y + (double)step_y);
        if (count_y > 0) {
            count_y--;
        }
        if (first_row > 0) {
            first_row--;
        }
    }
}

/*
 * 0x00473800: draws a parallax element: as a sheared sprite with the OpenGL
 * renderers, row by row onto 15/16-bit targets for 8-bit sprites otherwise
 * (dp is not used then).
 */
void BgElem_DrawParallax(BgElem *e, Bg *bg, const DrawParams *dp)
{
    const Sprite *spr;
    float scale_x, scale_y;

    if (g_renderMode == RENDER_MODE_GL || g_renderMode == RENDER_MODE_GL_ALT) {
        BgElem_DrawParallaxGL(e, bg, dp);
        return;
    }
    if (e->sprite == -1) {
        return;
    }
    scale_x = e->draw_scale_x;
    scale_y = (float)(e->yscale * (double)e->draw_scale_y);
    spr = Bg_SpriteAt(bg->sprites, e->sprite);
    if (scale_x <= 0.0f || scale_y <= 0.0f) {
        return;
    }
    if ((g_targetBpp != 15 && g_targetBpp != 16) || spr->bpp != 8) {
        return;
    }
    if (e->mask == 1) {
        BgElem_DrawParallaxSoftMasked(bg, e, scale_x, scale_y);
    } else {
        BgElem_DrawParallaxSoft(bg, e, scale_x, scale_y);
    }
}

/*
 * 0x004738D0: OpenGL parallax: the sprite (or the current sprite of the
 * animation) is drawn once, sheared so that its bottom moves with the
 * camera faster than its top. With "width" / "paralwidth" and tiling the
 * renderer repeats it across the screen.
 */
void BgElem_DrawParallaxGL(BgElem *e, Bg *bg, const DrawParams *dp)
{
    float px = (float)(e->scale_x * (double)e->x);
    float half_w = (float)(g_targetWidth / 2);
    Sprite *spr = NULL;
    int sprite = e->sprite;
    int from_anim = 0;
    DrawParams combined;
    DrawParams local;
    float shear;
    float sx, sy;
    int saved_x;

    if (sprite != -1) {
        spr = Bg_SpriteAt(bg->sprites, sprite);
    } else {
        /* Quirk: compares the action number with 0 instead of -1. */
        if (e->anim == NULL || e->action == 0) {
            return;
        }
        sprite = e->anim->sprite;
        if (sprite != -1) {
            spr = Bg_SpriteAt(bg->sprites, sprite);
        }
        from_anim = 1;
    }
    if (spr == NULL) {
        return;
    }
    if (from_anim) {
        combined = e->anim->dp;
        DrawParams_ScalePos(dp, &combined);
        local = *DrawParams_Combine(dp, &combined);
    } else {
        local = *dp;
    }
    if (e->by_width && e->tile_x) {
        sx = local.scale_x;
        sy = local.scale_y;
        local.scale_x = 1.0f;
        local.scale_y = 1.0f;
    } else {
        sx = dp->scale_x;
        sy = dp->scale_y;
        if (dp->scale_x != 0.0f) {
            local.scale_x = (float)(local.scale_x / (double)dp->scale_x);
        }
        if (dp->scale_y != 0.0f) {
            local.scale_y = (float)(local.scale_y / (double)dp->scale_y);
        }
    }
    shear = (float)(e->x / (double)e->draw_scale_x);
    if (e->by_width) {
        if (e->tile_x) {
            float fw;
            local.x = 0.0f;
            fw = (float)g_targetWidth;
            local.scale2_x = (float)(fw / (double)spr->w);
            local.scale2_y = sy;
            local.top_stretch = (float)(fw / (double)e->width_top / sx);
            local.bottom_stretch = (float)(fw / (double)e->width_bottom / sx);
            local.top_offset = (float)(local.top_stretch * (double)px / fw);
            local.bottom_offset = local.top_offset;
        } else {
            double ratio_m1;
            float fh, t, k, kx;
            int sign;
            local.width_ratio = (float)((double)e->width_top / e->width_bottom);
            local.width_anchor = e->centered ? 0.5f : (float)((double)spr->x / spr->w);
            sign = (local.flip2 & 1) == 0 ? 1 : -1;
            t = (float)((double)e->width_bottom / spr->w);
            ratio_m1 = (double)e->width_ratio - 1.0;
            fh = (float)spr->h;
            shear = (float)((double)shear * ratio_m1 / fh / t * sign);
            k = (float)(ratio_m1 * (e->legacy ? spr->y + 1 : spr->y) / fh + 1.0);
            kx = (float)(k * (double)px);
            local.x = (float)(kx + (double)half_w);
            local.shear_x = shear;
            local.scale2_y = sy;
            local.scale2_x = (float)(t * (double)sx);
        }
    } else {
        double ratio_m1 = (double)e->width_ratio - 1.0;
        float fh = (float)spr->h;
        float k, kx;
        shear = (float)((double)shear * ratio_m1 / fh);
        k = (float)(ratio_m1 * (e->legacy ? spr->y + 1 : spr->y) / fh + 1.0);
        kx = (float)(k * (double)px);
        local.x = (float)(kx + (double)half_w);
        local.shear_x = shear;
        local.scale2_x = sx;
        local.scale2_y = sy;
    }
    saved_x = spr->x;
    if (e->by_width) {
        if (e->tile_x) {
            spr->x = 0;
        } else if (e->centered) {
            spr->x = spr->w / 2;
        }
    }
    if (g_renderMode == RENDER_MODE_GL) {
        Sprite_DrawGL(bg->sprites, sprite, &local);
    } else {
        Sprite_DrawSoft(bg->sprites, sprite, &local);
    }
    spr->x = saved_x;
}

/*
 * Software parallax: rows of the 8-bit sprite are stretched one by one onto
 * the 16-bit render target, each row starting further from the middle of
 * the screen than the previous one. Masked (colour 0 skipped) and opaque
 * versions of the same code follow.
 */

/* x of the first pixel of source row `row` (see BgElem_DrawParallaxSoftMasked). */
static int BgElem_SoftRowStart(const BgElem *e, int row, float px, float scale_x, int axis_x)
{
    if (e->by_width) {
        float w0 = (float)e->row_widths[0];
        return Ftol((px - (double)w0 * scale_x * 0.5) * e->row_widths[row] / w0 + (double)(g_targetWidth / 2));
    }
    return Ftol((((double)e->width_ratio - 1.0) * (row + 1) / e->height + 1.0) * px +
                   (double)(g_targetWidth / 2 - axis_x));
}

/* Tiled rows: moves the start back to the last tile beginning left of the clip rectangle. */
static int BgElem_SoftWrapStart(const BgElem *e, int row, int xs, float scale_x)
{
    double w = (double)e->row_widths[row] * scale_x;

    if (w - 1.0 + xs < g_clipX1 || xs > g_clipX1) {
        int m = (xs - g_clipX1) % Ftol(w) + g_clipX1;
        xs = m;
        if (m > g_clipX1) {
            xs = Ftol(m - w);
        }
    }
    return xs;
}

/* 0x00473C80: software parallax, colour 0 transparent ("mask = 1"). */
void BgElem_DrawParallaxSoftMasked(Bg *bg, BgElem *e, float scale_x, float scale_y)
{
    const Sprite *spr = Bg_SpriteAt(bg->sprites, e->sprite);
    float px = (float)(e->x * (double)e->scale_x);
    float y_acc = 0.0f;
    const Sprite *target = Bg_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int row = 0;
    int axis_x, axis_y, y;
    float fy;
    unsigned char *pixels;
    unsigned char *target_pixels;
    Palette *pal;

    if (spr == NULL || scale_x <= 0.0f || scale_y <= 0.0f) {
        return;
    }
    scale_x = (float)(scale_x * (double)e->scale_x);
    scale_y = (float)(scale_y * (double)e->scale_y);
    axis_x = Ftol((double)spr->x * scale_x);
    fy = (float)(e->y * (double)e->scale_y);
    axis_y = Ftol((double)spr->y * scale_y);
    fy = (float)(fy - (double)axis_y);
    fy = (float)floor(fy + 0.5);
    y = Ftol(fy);
    pixels = Sprite_GetPixels(bg->sprites, e->sprite);
    if (pixels == NULL) {
        return;
    }
    target_pixels = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = SpriteSet_GetPalette(bg->sprites, spr->pal_index);
    if (pal != NULL) {
        Pal_Convert(pal);
    }
    if (y >= g_clipY2) {
        return;
    }
    while (!(e->height <= (double)row)) {
        if (y >= g_clipY1) {
            int xs = BgElem_SoftRowStart(e, row, px, scale_x, axis_x);
            int xe, x, x_end, len;
            float u;
            unsigned short *dst;

            if (e->tile_x) {
                xs = BgElem_SoftWrapStart(e, row, xs, scale_x);
            }
            xe = Ftol((double)e->row_widths[row] * scale_x - 1.0 + xs);
            if (xs < g_clipX1) {
                u = (float)((double)(g_clipX1 - xs) / scale_x);
                xs = g_clipX1;
            } else {
                u = 0.0f;
            }
            if (xe >= g_clipX2) {
                xe = g_clipX2 - 1;
            }
            dst = (unsigned short *)(target_pixels + target->rows[y] + xs * 2);
            if (xe >= g_clipX1 && xs < g_clipX2) {
                const unsigned char *src_row = e->rows[row];
                len = xe - xs + 1;
                x = xs;
                x_end = e->tile_x ? g_clipX2 - 1 : xe;
                if (x <= x_end) {
                    do {
                        /* Quirk: a negative length never ends the loop. */
                        if (len >= 0) {
                            int next = x + len;
                            int iu, step;
                            unsigned int acc;
                            float frac;
                            const unsigned char *src;
                            if (next > g_clipX2) {
                                len += g_clipX2 - next;
                            }
                            iu = Ftol(u);
                            src = src_row + iu;
                            step = Trunc64(1.0 / scale_x * BG_FIXED_ONE);
                            frac = (float)(u - (double)iu);
                            acc = (unsigned int)(__int64)((double)frac * BG_FIXED_ONE + (double)(unsigned int)step);
                            for (; len != 0; len--) {
                                if (*src != 0) {
                                    *dst = pal->rgb565[*src];
                                }
                                dst++;
                                src += acc >> 16;
                                acc = (acc & 0xFFFF) + step;
                            }
                            u = 0.0f;
                            len = Ftol((double)e->row_widths[row] * scale_x);
                            x = next;
                        }
                    } while (x <= x_end);
                }
            }
        }
        y_acc = (float)(1.0 / scale_y + y_acc);
        {
            int n = Ftol(y_acc);
            y_acc = (float)(y_acc - (double)n);
            y++;
            row += n;
        }
        if (y >= g_clipY2) {
            break;
        }
    }
}

/* 0x00474110: software parallax, every pixel drawn ("mask" other than 1). */
void BgElem_DrawParallaxSoft(Bg *bg, BgElem *e, float scale_x, float scale_y)
{
    const Sprite *spr = Bg_SpriteAt(bg->sprites, e->sprite);
    float px = (float)(e->x * (double)e->scale_x);
    float y_acc = 0.0f;
    const Sprite *target = Bg_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int row = 0;
    int axis_x, axis_y, y;
    float fy;
    unsigned char *pixels;
    unsigned char *target_pixels;
    Palette *pal;

    if (spr == NULL || scale_x <= 0.0f || scale_y <= 0.0f) {
        return;
    }
    scale_x = (float)(scale_x * (double)e->scale_x);
    scale_y = (float)(scale_y * (double)e->scale_y);
    axis_x = Ftol((double)spr->x * scale_x);
    fy = (float)(e->y * (double)e->scale_y);
    axis_y = Ftol((double)spr->y * scale_y);
    fy = (float)(fy - (double)axis_y);
    fy = (float)floor(fy + 0.5);
    y = Ftol(fy);
    pixels = Sprite_GetPixels(bg->sprites, e->sprite);
    if (pixels == NULL) {
        return;
    }
    target_pixels = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = SpriteSet_GetPalette(bg->sprites, spr->pal_index);
    if (pal != NULL) {
        Pal_Convert(pal);
    }
    if (y >= g_clipY2) {
        return;
    }
    while (!(e->height <= (double)row)) {
        if (y >= g_clipY1) {
            int xs = BgElem_SoftRowStart(e, row, px, scale_x, axis_x);
            int xe, x, x_end, len;
            float u;
            unsigned short *dst;

            if (e->tile_x) {
                xs = BgElem_SoftWrapStart(e, row, xs, scale_x);
            }
            xe = Ftol((double)e->row_widths[row] * scale_x - 1.0 + xs);
            if (xs < g_clipX1) {
                u = (float)((double)(g_clipX1 - xs) / scale_x);
                xs = g_clipX1;
            } else {
                u = 0.0f;
            }
            if (xe >= g_clipX2) {
                xe = g_clipX2 - 1;
            }
            dst = (unsigned short *)(target_pixels + target->rows[y] + xs * 2);
            if (xe >= g_clipX1 && xs < g_clipX2) {
                const unsigned char *src_row = e->rows[row];
                len = xe - xs + 1;
                x = xs;
                x_end = e->tile_x ? g_clipX2 - 1 : xe;
                if (x <= x_end) {
                    do {
                        int next = x + len;
                        int iu, step;
                        unsigned int acc;
                        float frac;
                        const unsigned char *src;
                        if (len < 0) {
                            break;
                        }
                        if (next > g_clipX2) {
                            len += g_clipX2 - next;
                        }
                        iu = Ftol(u);
                        src = src_row + iu;
                        step = Trunc64(1.0 / scale_x * BG_FIXED_ONE);
                        frac = (float)(u - (double)iu);
                        acc = (unsigned int)(__int64)((double)frac * BG_FIXED_ONE + (double)(unsigned int)step);
                        for (; len != 0; len--) {
                            *dst = pal->rgb565[*src];
                            dst++;
                            src += acc >> 16;
                            acc = (acc & 0xFFFF) + step;
                        }
                        u = 0.0f;
                        len = Ftol((double)e->row_widths[row] * scale_x);
                        x = next;
                    } while (x <= x_end);
                }
            }
        }
        y_acc = (float)(1.0 / scale_y + y_acc);
        {
            int n = Ftol(y_acc);
            y_acc = (float)(y_acc - (double)n);
            y++;
            row += n;
        }
        if (y >= g_clipY2) {
            break;
        }
    }
}

/* 0x004745A0: fmod(x, y), moved to the period ending at 0 when positive (tile start). */
float Bg_Fmod(float x, float y)
{
    float r = (float)Bg_CIfmod(x, y);

    if (r > 0.0f) {
        float ay = (float)fabs(y);
        r = (float)(r - (double)ay);
    }
    return r;
}
