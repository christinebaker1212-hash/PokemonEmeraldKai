/*
 * Animations (original module 0x4635F0-0x46509B): the AIR parser, animation
 * playback, the sprite draw queue and sets of one-shot animations.
 *
 * Animations are read from "[Begin Action n]" groups of a parsed text file
 * (Ini), so the same parser serves .air files and the animations embedded in
 * system.def, fight.def and stage files. Each action is a KeyedArray of
 * AnimElem keyed by element index; the Anim keeps the actions keyed by action
 * number.
 *
 * Errors are reported in g_libraryErrorText (shown by Error_Exit), warnings
 * in the match log.
 */
#include "mugen.h"

#define ANIM_GROUP_PREFIX "begin action "

/* The action an animation state is playing. */
static AnimAction *Anim_CurAction(const AnimState *st)
{
    KeyedArray *actions = st->anim->actions;

    return (AnimAction *)(actions->data + actions->elem_size * st->action);
}

static AnimElem *Anim_ElemAt(const AnimAction *action, int index)
{
    return (AnimElem *)(action->elems->data + action->elems->elem_size * index);
}

/* KeyedArray of sprites of a sprite set (its first field). */
static KeyedArray *Anim_SpriteList(struct SpriteSet *sprites)
{
    return *(KeyedArray **)sprites;
}

static int Anim_IsElemStart(char c)
{
    return (c >= '0' && c <= '9') || c == '-';
}

/*
 * 0x004635F0: parses a "trans" value. alpha_src/alpha_dst get the blending
 * factors (256 = 1.0) for the types that imply them.
 */
int Trans_Parse(const char *s, int *alpha_dst, int *alpha_src)
{
    *alpha_src = 256;
    *alpha_dst = 0;
    if (_stricmp(s, "none") == 0) {
        return TRANS_NONE;
    }
    if (_stricmp(s, "add") == 0) {
        *alpha_src = 256;
        *alpha_dst = 256;
        return TRANS_ADD;
    }
    if (_stricmp(s, "addalpha") == 0) {
        return TRANS_ADD;
    }
    if (_stricmp(s, "add1") == 0) {
        *alpha_src = 256;
        *alpha_dst = 128;
        return TRANS_ADD;
    }
    if (_stricmp(s, "sub") == 0) {
        *alpha_src = 256;
        *alpha_dst = 256;
        return TRANS_SUB;
    }
    if (s[0] != '\0' && _stricmp(s, "default") != 0) {
        return TRANS_INVALID;
    }
    return TRANS_DEFAULT;
}

/*
 * 0x004636C0: fills g_sinTable with sin() * 1024 in 1/8 degree steps over
 * 0..450 degrees (so cosines are 720 entries further). The angle factor is
 * pi/180 rounded to float, as in the original.
 */
int SinTable_Init(void)
{
    int *table = g_sinTable;
    int i;

    if (table == NULL) {
        table = malloc(SIN_TABLE_SIZE * sizeof(int));
        g_sinTable = table;
        if (table == NULL) {
            return 0;
        }
    }
    for (i = 0; i < SIN_TABLE_SIZE; i++) {
        table[i] = (int)(sin(i * 0.01745329238474369 * 0.125) * 1024.0);
    }
    table[0] = 0;
    table[720] = 1024;
    table[1440] = 0;
    table[2160] = -1024;
    table[2880] = 0;
    return 1;
}

/* 0x00463760: loads an AIR file. */
Anim *Anim_Load(const char *path)
{
    Stream *s = Stream_OpenFile(path, "rt");

    if (s != NULL) {
        Ini *ini = Ini_Parse(s, INI_MODE_READ);
        Stream_Close(s);
        if (ini != NULL) {
            Anim *anim = Anim_Parse(ini);
            if (anim == NULL) {
                strcat(g_libraryErrorText, "\nAnim error reading ");
                strcat(g_libraryErrorText, path);
            }
            Ini_Free(ini);
            return anim;
        }
    }
    strcat(g_libraryErrorText, "\nTXTPARSE error opening ");
    strcat(g_libraryErrorText, path);
    return NULL;
}

/*
 * 0x00463890: reads every "[Begin Action n]" group of ini.
 *
 * Quirks kept from the original:
 * - elements are counted as the lines starting with a digit or '-', but an
 *   element line may also start with 'L' + digit;
 * - actions without elements are skipped with a warning, yet still count in
 *   action_count;
 * - total_time becomes -1 at an element lasting forever, but later element
 *   times are still added to it;
 * - on errors the half-built action (and its boxes) is leaked.
 */
Anim *Anim_Parse(Ini *ini)
{
    Anim *anim;
    const char *header;
    int a;

    anim = Mem_Calloc(1, sizeof(Anim));
    if (anim == NULL) {
        return NULL;
    }
    anim->action_count = Ini_CountGroups(ini, ANIM_GROUP_PREFIX);
    if (anim->action_count <= 0) {
        sprintf(g_libraryErrorText, "Need at least one action");
        Anim_Free(anim);
        return NULL;
    }
    anim->actions = KeyedArray_Create(anim->action_count, sizeof(AnimAction));
    if (anim->actions == NULL) {
        sprintf(g_libraryErrorText, "Out of memory.");
        Anim_Free(anim);
        return NULL;
    }
    ini->group = -1;
    header = Ini_NextGroup(ini);
    for (a = 0; a < anim->action_count; a++) {
        AnimAction action;
        Clsn *clsn1_default, *clsn2_default;
        const char *line;
        int action_no, count, i;

        memset(&action, 0, sizeof(action));
        while (!Str_StartsWithI(header + 1, ANIM_GROUP_PREFIX)) {
            header = Ini_NextGroup(ini);
        }
        action_no = atoi(header + 1 + 13);

        count = 0;
        for (line = Ini_FirstLine(ini); line != NULL; line = Ini_NextLine(ini)) {
            if (Anim_IsElemStart(line[0])) {
                count++;
            }
        }
        action.elem_count = count;
        Ini_Rewind(ini);
        if (count == 0) {
            MatchLog_Printf(5, "Action %i must have at least 1 element.", action_no);
            header = Ini_NextGroup(ini);
            continue;
        }

        action.elems = KeyedArray_Create(count, sizeof(AnimElem));
        if (action.elems == NULL) {
            sprintf(g_libraryErrorText, "Out of memory.");
            Anim_Free(anim);
            return NULL;
        }
        action.total_time = 0;
        clsn1_default = NULL;
        clsn2_default = NULL;
        for (i = 0; i < count; i++) {
            AnimElem elem;
            int loopstart;
            int k;

            memset(&elem, 0, sizeof(elem));
            memset(&elem.dp, 0, sizeof(elem.dp));
            elem.dp.width_ratio = 1.0f;
            elem.dp.width_anchor = 0.5f;
            elem.dp.scale_x = 1.0f;
            elem.dp.scale_y = 1.0f;
            elem.dp.scale2_x = 1.0f;
            elem.dp.scale2_y = 1.0f;
            elem.dp.top_stretch = 1.0f;
            elem.dp.bottom_stretch = 1.0f;
            elem.dp.masked = 1;
            elem.dp.alpha_src = 1.0f;
            elem.dp.flags = DRAW_UNUSED_1000;
            loopstart = 0;

            /* Option lines up to the element line itself. */
            for (;;) {
                line = Ini_NextLine(ini);
                if (line == NULL) {
                    goto line_error;
                }
                if (Str_PrefixMatchI(line, "loopstart", 9)) {
                    loopstart = 1;
                } else if (Str_PrefixMatchI(line, "interpolate", 11)) {
                    if (!Anim_ParseInterpolate(&elem, line + 11)) {
                        goto line_error;
                    }
                } else if (Str_PrefixMatchI(line, "clsn1", 5)) {
                    Clsn_Release(elem.clsn1);
                    elem.clsn1 = Clsn_Parse(line, ini);
                    if (elem.clsn1 == NULL) {
                        sprintf(g_libraryErrorText, "Error in clsn1 in %s elem %i", header, i);
                        Anim_Free(anim);
                        return NULL;
                    }
                    if (line[5] == 'd' || line[5] == 'D') {
                        elem.flags |= ANIM_ELEM_CLSN1_DEFAULT;
                        clsn1_default = elem.clsn1;
                    }
                } else if (Str_PrefixMatchI(line, "clsn2", 5)) {
                    Clsn_Release(elem.clsn2);
                    elem.clsn2 = Clsn_Parse(line, ini);
                    if (elem.clsn2 == NULL) {
                        sprintf(g_libraryErrorText, "Error in clsn2 in %s elem %i", header, i);
                        Anim_Free(anim);
                        return NULL;
                    }
                    if (line[5] == 'd' || line[5] == 'D') {
                        elem.flags |= ANIM_ELEM_CLSN2_DEFAULT;
                        clsn2_default = elem.clsn2;
                    }
                }
                if (((line[0] == 'L' || line[0] == 'l') && line[1] >= '0' && line[1] <= '9') ||
                    Anim_IsElemStart(line[0])) {
                    break;
                }
            }

            if (clsn1_default != NULL && elem.clsn1 == NULL) {
                clsn1_default->refs++;
                elem.clsn1 = clsn1_default;
            }
            if (clsn2_default != NULL && elem.clsn2 == NULL) {
                clsn2_default->refs++;
                elem.clsn2 = clsn2_default;
            }
            if (i == 0) {
                elem.start = 0;
            } else {
                AnimElem *prev = (AnimElem *)(action.elems->data + action.elems->elem_size * (i - 1));
                elem.start = prev->time + prev->start;
            }
            if (loopstart) {
                action.loop_time = elem.start;
                action.loop_elem = i;
            }
            for (k = 0; !Anim_IsElemStart(line[k]); k++) {
                if (line[k] == '\0') {
                    sprintf(g_libraryErrorText, "Syntax error in %s elem %i", header, i + 1);
                    Anim_Free(anim);
                    return NULL;
                }
            }
            if (!Anim_ParseElem(&elem, line + k, header, i)) {
                Anim_Free(anim);
                return NULL;
            }
            if (elem.time == -1) {
                action.total_time = -1;
            } else {
                action.total_time += elem.time;
            }
            KeyedArray_Add(action.elems, &elem, 0, i);
            continue;

        line_error:
            sprintf(g_libraryErrorText, "Error reading line in %s elem %i", header, i + 1);
            Anim_Free(anim);
            return NULL;
        }
        if (action.loop_elem == -1) {
            action.loop_elem = 0;
        }
        KeyedArray_Add(anim->actions, &action, 0, action_no);
        header = Ini_NextGroup(ini);
    }
    return anim;
}

/*
 * 0x00463F80: reads an element line from its first number:
 * group, number, x, y, time[, flip[, trans[, scale_x[, scale_y[, angle]]]]].
 */
int Anim_ParseElem(AnimElem *elem, const char *s, const char *header, int index)
{
    int field = 0;
    int i = 0;

    if (s[0] != '\0') {
        for (;;) {
            const char *p;

            while (Char_IsSpace(s[i])) {
                i++;
            }
            p = s + i;
            if (*p == '\0') {
                break;
            }
            switch (field) {
            case 0:
                elem->group = atoi(p);
                break;
            case 1:
                elem->number = atoi(p);
                break;
            case 2:
                elem->dp.x = (float)atof(p);
                elem->dp.flags |= DRAW_POS;
                break;
            case 3:
                elem->dp.y = (float)atof(p);
                elem->dp.flags |= DRAW_POS;
                break;
            case 4:
                elem->time = atoi(p);
                break;
            case 5:
                Anim_ParseElemFlip(p, elem);
                break;
            case 6:
                Anim_ParseElemTrans(p, elem);
                break;
            case 7:
                elem->dp.scale_x = (float)atof(p);
                elem->dp.flags |= DRAW_SCALE;
                break;
            case 8:
                elem->dp.scale_y = (float)atof(p);
                elem->dp.flags |= DRAW_SCALE;
                break;
            case 9:
                elem->dp.angle = (float)atof(p);
                elem->dp.flags |= DRAW_ANGLE;
                break;
            }
            field++;
            while (s[i] != ',' && s[i] != '\0') {
                i++;
            }
            if (s[i] == '\0') {
                break;
            }
            i++;
            if (s[i] == '\0') {
                break;
            }
        }
        if (field >= 5) {
            return 1;
        }
    }
    sprintf(g_libraryErrorText, "Too few arguments on element %i of %s:\n%s (%i)", index + 1, header, s, field);
    return 0;
}

/* 0x00464110: "H", "V" or "HV" flip field. */
void Anim_ParseElemFlip(const char *s, AnimElem *elem)
{
    unsigned int flip = 0;

    if (*s != '\0') {
        do {
            int c;
            if (*s == ',') {
                break;
            }
            c = Char_ToLower(*s);
            if (c == 'h') {
                flip |= DRAW_FLIP_H;
            } else if (c == 'v') {
                flip |= DRAW_FLIP_V;
            }
            s++;
        } while (*s != '\0');
    }
    elem->dp.flags |= DRAW_FLIP;
    elem->dp.flip = flip;
}

/*
 * 0x00464170: trans field: "A" (add), "A1" (add, half destination), "S"
 * (subtract) or "AS<src>D<dst>" (add with alpha, 256 = 1.0). "AS"/"AD"
 * values that are missing keep the element defaults (1.0, 0.0).
 */
void Anim_ParseElemTrans(const char *s, AnimElem *elem)
{
    int c = Char_ToLower(s[0]);

    if (c == 'a') {
        if (s[1] == '1') {
            elem->dp.flags |= DRAW_BLEND;
            elem->dp.alpha_src = 1.0f;
            elem->dp.blend = BLEND_ADD;
            elem->dp.alpha_dst = 0.5f;
            return;
        }
        c = Char_ToLower(s[1]);
        elem->dp.flags |= DRAW_BLEND;
        elem->dp.blend = BLEND_ADD;
        if (c != 's') {
            elem->dp.alpha_src = 1.0f;
            elem->dp.alpha_dst = 1.0f;
            return;
        }
        {
            const char *src = s + 2;
            int i = 2;

            if (Char_IsDigit(*src)) {
                elem->dp.alpha_src = (float)(atoi(src) * 0.00390625);
            }
            if (*src != '\0') {
                while (Char_IsDigit(s[i])) {
                    i++;
                    if (s[i] == '\0') {
                        break;
                    }
                }
            }
            if (Char_ToLower(s[i]) == 'd' && Char_IsDigit(s[i + 1])) {
                elem->dp.alpha_dst = (float)(atoi(s + i + 1) * 0.00390625);
            }
        }
    } else if (c == 's') {
        elem->dp.flags |= DRAW_BLEND;
        elem->dp.alpha_src = 1.0f;
        elem->dp.blend = BLEND_SUB;
        elem->dp.alpha_dst = 1.0f;
    }
}

/* 0x004642C0 */
void Anim_Free(Anim *anim)
{
    int i;

    if (anim == NULL) {
        return;
    }
    if (anim->actions != NULL) {
        /* Slots past the inserted actions are zero (skipped actions are never inserted). */
        for (i = 0; i < anim->action_count; i++) {
            AnimAction *action = (AnimAction *)(anim->actions->data + anim->actions->elem_size * i);
            KeyedArray *elems = action->elems;
            AnimElem *elem;

            if (elems == NULL) {
                continue;
            }
            for (elem = KeyedArray_First(elems); elem != NULL; elem = KeyedArray_Next(elems)) {
                Clsn_Release(elem->clsn1);
                Clsn_Release(elem->clsn2);
            }
            KeyedArray_Free(elems);
        }
        KeyedArray_Free(anim->actions);
    }
    free(anim);
}

/*
 * 0x00464410: advances the animation by one tick: looks up the element and
 * sprite of the current time, then moves the time on. Returns 1 when the
 * action reached its end and went back to the loop start.
 */
int Anim_Step(AnimState *st)
{
    AnimAction *action = Anim_CurAction(st);
    int looped = 0;

    Anim_SeekElem(st);
    if (st->time >= action->total_time && action->total_time != -1) {
        st->time = action->loop_time;
        looped = 1;
        st->elem = (AnimElem *)Anim_CurAction(st)->elems->data;
        st->elem_index = 0;
        Anim_SeekElem(st);
    }
    if (st->elem != NULL) {
        AnimElem *elem = st->elem;
        st->sprite = KeyedArray_Find(Anim_SpriteList(st->sprites), elem->group, elem->number);
        st->dp = elem->dp;
        Anim_Interpolate(st);
    }
    st->ticks++;
    st->time++;
    return looped;
}

/*
 * 0x004644B0: blends offset, alpha, scale and angle towards the next element
 * when it asks for it. Products and sums are kept in double precision and
 * only the result is rounded to float, as the x87 code does.
 */
void Anim_Interpolate(AnimState *st)
{
    const AnimElem *cur = st->elem;
    const AnimElem *next = st->next;
    double t, u;

    if (cur == next || cur->time <= 0) {
        return;
    }
    t = (float)((double)(st->time - cur->start) / cur->time);
    u = 1.0 - t;
    if (next->interp_offset == 1) {
        st->dp.x = (float)(next->dp.x * t + cur->dp.x * u);
        st->dp.y = (float)(next->dp.y * t + cur->dp.y * u);
    }
    if (next->interp_blend == 1 && cur->dp.blend == next->dp.blend) {
        st->dp.alpha_src = (float)(next->dp.alpha_src * t + cur->dp.alpha_src * u);
        st->dp.flags |= DRAW_BLEND;
        st->dp.alpha_dst = (float)(next->dp.alpha_dst * t + cur->dp.alpha_dst * u);
    }
    if (next->interp_scale == 1) {
        st->dp.scale_x = (float)(next->dp.scale_x * t + cur->dp.scale_x * u);
        st->dp.flags |= DRAW_SCALE;
        st->dp.scale_y = (float)(next->dp.scale_y * t + cur->dp.scale_y * u);
    }
    if (next->interp_angle == 1) {
        st->dp.flags |= DRAW_ANGLE;
        st->dp.angle = (float)(u * cur->dp.angle + t * next->dp.angle);
    }
}

/*
 * 0x004645B0: moves elem forward to the last element started at the current
 * time and finds the element after it.
 */
void Anim_SeekElem(AnimState *st)
{
    AnimAction *action = Anim_CurAction(st);
    int i;

    for (i = st->elem_index; i < action->elem_count; i++) {
        AnimElem *elem = Anim_ElemAt(action, i);
        if (elem->start > st->time) {
            break;
        }
        st->elem_index = i;
        st->elem = elem;
    }
    if (st->elem == NULL) {
        st->next = NULL;
        return;
    }
    i = st->elem_index + 1;
    if (i >= action->elem_count) {
        i = action->loop_elem;
    }
    st->next = Anim_ElemAt(action, i);
}

/*
 * 0x00464620: restarts the current action. An out of range slot falls back
 * to the first one (the check lets action == action_count through).
 */
void Anim_Reset(AnimState *st)
{
    if (st->action > st->anim->action_count) {
        st->action = 0;
    }
    st->ticks = 0;
    st->time = 0;
    st->elem = (AnimElem *)Anim_CurAction(st)->elems->data;
    st->elem_index = 0;
    Anim_SeekElem(st);
}

/* 0x00464660: switches to an action number; 0 if it does not exist. Setting the current action keeps its time. */
int Anim_SetAction(AnimState *st, int action_no)
{
    int slot;

    if (st->action > st->anim->action_count) {
        st->action = 0;
        st->elem = (AnimElem *)((AnimAction *)st->anim->actions->data)->elems->data;
        st->elem_index = 0;
        Anim_SeekElem(st);
    }
    slot = KeyedArray_Find(st->anim->actions, 0, action_no);
    if (slot == -1) {
        return 0;
    }
    if (st->action != slot) {
        st->action = slot;
        st->ticks = 0;
        st->time = 0;
        st->elem = (AnimElem *)Anim_CurAction(st)->elems->data;
        st->elem_index = 0;
        Anim_SeekElem(st);
    }
    return 1;
}

/*
 * 0x004646E0: restarts the action at element index elem by stepping through
 * it. Returns 0 for an invalid index or when the action loops first.
 */
int Anim_SetElem(AnimState *st, int elem)
{
    Anim_Reset(st);
    if (elem < 0 || elem >= Anim_CurAction(st)->elem_count) {
        return 0;
    }
    if (elem != 0) {
        Anim_SeekElem(st);
        while (st->elem_index != elem) {
            if (Anim_Step(st)) {
                return 0;
            }
        }
        st->time--;
        st->ticks--;
    }
    return 1;
}

/*
 * 0x00464770: draws the elements shown at the previous tick (several when
 * elements of zero time overlap), combined with dp; positions are scaled by
 * sx, sy. With a queue the sprites are queued instead of drawn.
 *
 * Original bug kept: when the current PalFX/remap is the drawing module's
 * static one, the queued item points at its copy inside this function's
 * stack item, which is gone once the item is stored in the queue.
 */
void Anim_Draw(AnimState *st, const DrawParams *dp, float sx, float sy)
{
    AnimAction *action = Anim_CurAction(st);
    int t = st->time - 1;
    int i;

    if (t < 0) {
        t = 0;
    }
    for (i = 0; i < action->elem_count; i++) {
        AnimElem *elem = Anim_ElemAt(action, i);

        if (t >= elem->start && (t < elem->start + elem->time || elem->time == -1)) {
            DrawParams local = st->dp;

            DrawParams_ScalePos(dp, &local);
            local.x = local.x * sx;
            local.y = local.y * sy;
            DrawParams_Combine(dp, &local);
            if (st->sprite != -1) {
                if (st->queue == NULL) {
                    if (g_renderMode == RENDER_MODE_GL) {
                        Sprite_DrawGL(st->sprites, st->sprite, &local);
                    } else {
                        Sprite_DrawSoft(st->sprites, st->sprite, &local);
                    }
                } else {
                    SpriteQueueItem item;

                    memset(&item, 0, sizeof(item));
                    item.layer = st->layer;
                    item.sprites = st->sprites;
                    item.sprite = st->sprite;
                    if (PalFx_CurIsExternal()) {
                        item.palfx = g_palFxCur;
                    } else {
                        memcpy(item.palfx_copy, g_palFxCur, sizeof(item.palfx_copy));
                        item.palfx = (struct PalFx *)item.palfx_copy;
                    }
                    if (PalRemap_CurIsExternal()) {
                        item.remap = g_palRemapCur;
                    } else {
                        memcpy(item.remap_copy, g_palRemapCur, sizeof(item.remap_copy));
                        item.remap = (struct PalRemap *)item.remap_copy;
                    }
                    item.dp = local;
                    SpriteQueue_Add(&item, st->queue);
                }
            }
        }
        if (elem->start > st->time - 1) {
            break;
        }
    }
}

/* 0x00464970: " Offset", " Blend", " Scale" or " Angle" after "Interpolate". */
int Anim_ParseInterpolate(AnimElem *elem, const char *s)
{
    char name[64];
    char *p;
    int n;

    if (!Char_IsSpace(s[0])) {
        return 0;
    }
    n = Str_ReadIdent(s, name);
    for (p = name; *p != '\0'; p++) {
        *p = (char)tolower(*p);
    }
    if (strcmp(name, "offset") == 0) {
        elem->interp_offset = 1;
    } else if (strcmp(name, "blend") == 0) {
        elem->interp_blend = 1;
    } else if (strcmp(name, "scale") == 0) {
        elem->interp_scale = 1;
    } else if (strcmp(name, "angle") == 0) {
        elem->interp_angle = 1;
    } else {
        MatchLog_Printf(3, "Unrecognized Interpolate type: %s\n", name);
        return 0;
    }
    while (Char_IsSpace(s[n])) {
        n++;
    }
    return s[n] == '\0';
}

/* 0x00464B50: draws at a position with scale and flip. */
void Anim_DrawAt(AnimState *st, float x, float y, float scale_x, float scale_y, float sx, float sy,
                 unsigned int flip)
{
    DrawParams dp;

    memset(&dp, 0, sizeof(dp));
    dp.width_ratio = 1.0f;
    dp.width_anchor = 0.5f;
    dp.x = x;
    dp.y = y;
    dp.scale_x = scale_x;
    dp.scale_y = scale_y;
    dp.flip = flip;
    dp.scale2_x = 1.0f;
    dp.scale2_y = 1.0f;
    dp.top_stretch = 1.0f;
    dp.bottom_stretch = 1.0f;
    dp.masked = 1;
    dp.alpha_src = 1.0f;
    dp.flags = DRAW_POS | DRAW_FLIP | DRAW_SCALE;
    Anim_Draw(st, &dp, sx, sy);
}

/* 0x00464BF0: start time of element index of the current action, -1 if out of range. */
int Anim_GetElemStart(AnimState *st, int index)
{
    AnimAction *action = Anim_CurAction(st);

    if (index < 0 || index >= action->elem_count) {
        return -1;
    }
    return Anim_ElemAt(action, index)->start;
}

/*
 * 0x00464C20: index of the first element starting after time + offset
 * (wrapped into the loop), i.e. the element count when it is the last one.
 * Returns -1 for negative times before the first loop. Like the original,
 * this divides by zero when the loop has no length.
 */
int Anim_ElemAtOffset(AnimState *st, int offset)
{
    AnimAction *action = Anim_CurAction(st);
    int time = st->time;
    int total = action->total_time;
    int span = total - action->loop_time;
    int t = time + offset;
    int i;

    if (t < 0) {
        if (time < total) {
            return -1;
        }
        t = time - offset + span;
    }
    if (t >= total && total != -1) {
        t = (t - action->loop_time) % span + action->loop_time;
    }
    for (i = 0; i < action->elem_count; i++) {
        if (Anim_ElemAt(action, i)->start > t) {
            break;
        }
    }
    return i;
}

/* 0x00464CA0 */
SpriteQueue *SpriteQueue_Create(int capacity)
{
    SpriteQueue *q = Mem_Calloc(1, sizeof(SpriteQueue));

    if (q == NULL) {
        return NULL;
    }
    q->items = KeyedArray_Create(capacity, sizeof(SpriteQueueItem));
    if (q->items == NULL) {
        sprintf(g_libraryErrorText, "Out of memory.");
        SpriteQueue_Free(q);
        return NULL;
    }
    q->order = Mem_Calloc(capacity, sizeof(int));
    if (q->order == NULL) {
        SpriteQueue_Free(q);
        return NULL;
    }
    return q;
}

/* 0x00464D20 */
void SpriteQueue_Free(SpriteQueue *q)
{
    if (q == NULL) {
        return;
    }
    KeyedArray_Free(q->items);
    if (q->order != NULL) {
        free(q->order);
    }
    free(q);
}

/* 0x00464D70: inserts item, keeping order sorted by layer (after equal layers). */
int SpriteQueue_Add(const SpriteQueueItem *item, SpriteQueue *q)
{
    int layer = item->layer;
    KeyedArray *items = q->items;
    int *order = q->order;
    int slot, n, i;

    slot = KeyedArray_Add(items, item, 0, layer);
    if (slot == -1) {
        return 0;
    }
    n = items->count - 1;
    i = 0;
    if (n > 0) {
        while (layer > items->slots[order[i]].key2) {
            i++;
            if (i >= n) {
                order[i] = slot;
                return 1;
            }
        }
        for (; n > i; n--) {
            order[n] = order[n - 1];
        }
    }
    order[i] = slot;
    return 1;
}

/*
 * 0x00464DE0: draws an instance at (x, y) relative to its position, all
 * scaled by scale. The position is truncated to whole pixels.
 */
void AnimSet_DrawInstance(AnimInstance *inst, int x, int y, float scale)
{
    float sx = inst->scale_x * scale;
    float sy = inst->scale_y * scale;
    int py = (int)((double)(inst->y + y) * scale);
    int px = (int)((double)(inst->x + x) * scale);

    Anim_DrawAt(&inst->state, (float)px, (float)py, sx, sy, sx, sy, inst->flip);
}

/* 0x00464E70 */
AnimSet *AnimSet_Create(void)
{
    AnimSet *set = Mem_Calloc(1, sizeof(AnimSet));

    if (set == NULL) {
        return NULL;
    }
    set->list = KeyedArray_Create(0x20, sizeof(AnimInstance));
    if (set->list == NULL) {
        AnimSet_Free(set);
        return NULL;
    }
    return set;
}

/* 0x00464EC0: the animations themselves belong to the caller. */
void AnimSet_Free(AnimSet *set)
{
    if (set == NULL) {
        return;
    }
    KeyedArray_Free(set->list);
    free(set);
}

/* 0x00464F00: starts an animation; returns its slot, or -1. */
int AnimSet_Add(int x, int y, unsigned int flip, AnimSet *set, Anim *anim, struct SpriteSet *sprites, int action)
{
    AnimInstance inst;

    inst.x = x;
    inst.y = y;
    inst.flip = flip;
    inst.scale_x = 1.0f;
    inst.scale_y = 1.0f;
    memset(&inst.state, 0, sizeof(inst.state));
    inst.state.anim = anim;
    inst.state.sprites = sprites;
    if (!Anim_SetAction(&inst.state, action)) {
        return -1;
    }
    Anim_Reset(&inst.state);
    return KeyedArray_Add(set->list, &inst, 1, 0);
}

/* 0x00464FD0: steps every animation and drops the ones that ended. */
void AnimSet_Update(AnimSet *set)
{
    KeyedArray *list = set->list;
    AnimInstance *inst;

    for (inst = KeyedArray_First(list); inst != NULL; inst = KeyedArray_Next(list)) {
        if (Anim_Step(&inst->state)) {
            KeyedArray_Remove(list->iter, set->list);
        }
    }
}

/* 0x00465030: draws the animations whose key1 flags match mask. */
void AnimSet_Draw(AnimSet *set, int x, int y, unsigned int mask, float scale)
{
    KeyedArray *list = set->list;
    AnimInstance *inst;

    for (inst = KeyedArray_First(list); inst != NULL; inst = KeyedArray_Next(list)) {
        if (list->slots[list->iter].key1 & mask) {
            AnimSet_DrawInstance(inst, x, y, scale);
        }
    }
}
