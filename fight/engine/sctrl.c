/*
 * State controllers (original module 0x444AA0-0x45380F; its string literals
 * run from "type" to "unhittable", right after the result screen's).
 *
 * The module starts with the sound helpers the controllers and the player
 * runtime use (Sound_PlaySystem / Sound_PlayPanned, also used by the menus)
 * and small helpers, then:
 * - StateCtrl_ParseParams reads the "type" of a [State n, ...] group and its
 *   parameters, into the six Exprs of the StateCtrl or into a parameter block
 *   (sctrl.h) parsed by the per-type helpers further down;
 * - StateCtrl_Parse reads the triggers, persistent and ignorehitpause of the
 *   group and adds the controller to its statedef;
 * - StateCtrl_Run / StateCtrl_Apply run a controller on a player;
 * - StateCtrl_Free and the per-type free helpers.
 *
 * Quirks kept from the original:
 * - The type name is copied to a 256-byte buffer without a length check.
 * - Some failures leak what was allocated (PalFX, Explod blocks; a
 *   ReversalDef block without a valid reversal.attr).
 * - AfterImage palbright reads its first two values into the same Expr
 *   (palbright[1]); palbright[0] keeps its default.
 * - A VarRandom range with one value moves the Expr to the upper bound by a
 *   plain copy and resets the lower bound (freeing the tokens the copy still
 *   points to when the value is not a constant).
 * - VarSet/VarAdd with neither value nor v/fv is accepted and sets nothing.
 * - A missing VarRangeSet value is fatal (Error_Exit).
 * - SuperPause compiles its time twice (Pause parameters, then its own).
 * - Running: the parsed Tag type (550) is fatal ("PlayerSCtrlApplyElem not
 *   recognised"); HitFallSet stores the x velocity as the y velocity too;
 *   EnvColor time -2 becomes -1 (forever) without a warning; the clipboard
 *   controllers print the text pieces as formats and pass the result to the
 *   clipboard as a format again; RemapPal with source -1 clears the remaps
 *   and still adds the entry -1,0 -> dest; SndPan pans without checking the
 *   stereo setting; PlaySnd evaluates volume without using it; BindToTarget
 *   evaluates its postype twice; TargetDrop removes targets while iterating;
 *   projremanim / projcancelanim below -1 only warn; the Zoom clamp warnings
 *   name every parameter "zoom.<name>".
 */
#include "mugen.h"

/* ------------------------------------------------------------ sound helpers */

/* 0x00444AA0: a system sound (menus) at the volume of the [Sound] settings. */
void Sound_PlaySystem(Snd *snd, int group, int channel, int number, double volume)
{
    GameVars *g = g_game;
    double v = g->master_volume / 100.0 * g->wav_volume / 100.0 * volume;

    if (v < 0.0) {
        v = 0.0;
    } else if (v > 100.0) {
        v = 100.0;
    }
    if (!g->sound_enabled || snd == NULL || g->no_sounds != 0 || group == -1) {
        return;
    }
    Sound_Play(channel, snd, group, number, v);
}

/* 0x00444BF0: stereo pan (-127..127) of a sound at screen x (game units), by [Sound] PanningWidth. */
int Sound_Pan(int x)
{
    GameVars *g = g_game;
    int width = (int)g->game_coord[0];
    int pos = x * 320 / width;
    int range;
    int pan;

    if (pos < -40) {
        pos += 40;
    } else if (pos > 40) {
        pos -= 40;
    } else {
        pos = 0;
    }
    range = g->panning_width - 20;
    if (range <= 0) {
        return 0;
    }
    pan = (int)(pos * 128.0 / range);
    if (pan < -127) {
        return -127;
    }
    if (pan > 127) {
        return 127;
    }
    return pan;
}

/* 0x00444B20: a sound panned by its screen position x, at a volume truncated to an int. */
void Sound_PlayPanned(Snd *snd, int group, int x, int number, double volume)
{
    GameVars *g = g_game;
    int slot, pan, v;

    if (!g->sound_enabled || snd == NULL || g->no_sounds != 0 || group == -1) {
        return;
    }
    slot = Sound_Prepare(-1, snd, group, number);
    if (slot == -1) {
        return;
    }
    g = g_game;
    pan = g->stereo_effects ? Sound_Pan(x) : 0;
    v = (int)(g->master_volume / 100.0 * g->wav_volume / 100.0 * volume);
    Sound_SetSlotVolume(slot, (double)v);
    Sound_SetSlotPan(slot, pan);
    Sound_SetSlotLoop(slot, 0);
    Sound_PlaySlot(slot);
}

/* 0x00444C80: sets g_sndUnusedVolume (never read) from a volume (percent, clamped to 0..100); -1 leaves it. */
void Sound_SetVolume(double volume)
{
    int v;

    if (volume == -1.0) {
        return;
    }
    if (volume < 0.0) {
        volume = 0.0;
    } else if (volume > 100.0) {
        volume = 100.0;
    }
    if (!g_game->sound_enabled) {
        return;
    }
    v = (int)volume;
    if (v < -1) {
        g_sndUnusedVolume = -1;
    } else {
        g_sndUnusedVolume = v > 0xFF ? 0xFF : v;
    }
}

/* 0x00444D00: the sound slot of channel for player p (-1 stays -1). */
int Sound_PlayerChannel(int channel, Player *p)
{
    if (channel == -1) {
        return -1;
    }
    return g_game->player_slots * channel + p->index + 1000;
}

/* 0x00444D30: "group, number" of a sound in key (group -1 when not given). */
int Sound_ReadRef(int *group, int *number, Ini *ini, const char *key)
{
    *group = -1;
    return Ini_Scanf(ini, key, "%i,%i", group, number);
}

/* 0x00444D50 */
int Player_IsActive(Player *p)
{
    return p != NULL && p->active != 0;
}

/* 0x00444D70: whether a controller type is available to a character of mugenversion version. */
int StateCtrl_VersionOk(int type, unsigned int version)
{
    unsigned int need = 0;
    int i;

    for (i = 0; g_sctrlsV10[i] != -1; i++) {
        if (type == g_sctrlsV10[i]) {
            need = 0x01000000;
            break;
        }
    }
    for (i = 0; g_sctrlsV11[i] != -1; i++) {
        if (type == g_sctrlsV11[i]) {
            return version >= 0x01010000;
        }
    }
    return version >= need;
}

/* ------------------------------------------------------------ parsing */

/* Compiles the value of key (if any) into e. */
static int compile_key(Ini *ini, const char *key, Expr *e, CharData *owner, int want)
{
    return Expr_Compile(Ini_GetValue(ini, key), e, owner, NULL, want);
}

/* Compiles the value of key into e; 0 when it is missing. */
static int compile_required(Ini *ini, const char *key, Expr *e, CharData *owner, int want)
{
    const char *s = Ini_GetValue(ini, key);

    if (s == NULL) {
        return 0;
    }
    return Expr_Compile(s, e, owner, NULL, want);
}

/*
 * The "var(n) = value" form of VarSet/VarAdd (sys: also sysvar/sysfvar) and
 * ParentVarSet/ParentVarAdd: the first line of the group that starts with one
 * of the names gives the kind (arg[2]), the index (arg[1]) and the value
 * (arg[0]). The kind is taken from the case-sensitive first letters while the
 * names match in any case ("Var(1)" reads as a sysfvar).
 */
static int parse_var_line(Ini *ini, StateCtrl *sc, CharData *owner, int sys)
{
    const char *s = Ini_FirstLine(ini);
    const char *end;
    int kind, want;

    if (s == NULL) {
        return 0;
    }
    while (!Str_PrefixMatchI(s, "var", 3) && !Str_PrefixMatchI(s, "fvar", 4) &&
           (!sys || (!Str_PrefixMatchI(s, "sysvar", 6) && !Str_PrefixMatchI(s, "sysfvar", 7)))) {
        s = Ini_NextLine(ini);
        if (s == NULL) {
            return 0;
        }
    }
    if (sys) {
        if (s[0] == 'v') {
            kind = SCTRL_VAR;
        } else if (s[0] == 'f') {
            kind = SCTRL_FVAR;
        } else {
            kind = (s[3] != 'v') + SCTRL_SYSVAR;
        }
    } else {
        kind = s[0] != 'v';
    }
    s += kind + 3;
    Expr_SetInt(&sc->arg[2], kind);
    while (Char_IsSpace(*s)) {
        s++;
    }
    if (*s != '(') {
        return 0;
    }
    if (!Expr_Compile(s + 1, &sc->arg[1], owner, &end, EXPR_WANT_INT)) {
        return 0;
    }
    if (*end != ')') {
        return 0;
    }
    s = end + 1;
    while (*s != '=') {
        if (*s == '\0') {
            return 0;
        }
        s++;
    }
    if (sys) {
        want = (kind == SCTRL_FVAR || kind == SCTRL_SYSFVAR) ? EXPR_WANT_FLOAT : EXPR_WANT_INT;
    } else {
        want = (kind != SCTRL_VAR) + 1;
    }
    return Expr_Compile(s + 1, &sc->arg[0], owner, NULL, want);
}

/*
 * The "value = x" form: value in arg[0] with the index in v (arg[1]), or a
 * float value with the index in fv (arg[2] set to 1). Neither v nor fv: the
 * value is not even compiled.
 */
static int parse_var_value(Ini *ini, const char *value, StateCtrl *sc, CharData *owner)
{
    const char *s = Ini_GetValue(ini, "v");

    if (s != NULL) {
        if (!Expr_Compile(value, &sc->arg[0], owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        return Expr_Compile(s, &sc->arg[1], owner, NULL, EXPR_WANT_INT);
    }
    s = Ini_GetValue(ini, "fv");
    if (s == NULL) {
        return 1;
    }
    if (!Expr_Compile(s, &sc->arg[1], owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Expr_Compile(value, &sc->arg[0], owner, NULL, EXPR_WANT_FLOAT)) {
        return 0;
    }
    Expr_SetInt(&sc->arg[2], SCTRL_FVAR);
    return 1;
}

/* "x, y" (ParseValues "ff") into two Exprs; a missing value is fine, trailing text is not. */
static int parse_pair(Ini *ini, const char *key, const char *fmt, Expr *a, Expr *b, CharData *owner)
{
    const char *s = Ini_GetValue(ini, key);
    const char *end;

    Cns_ParseValues(s, fmt, owner, &end, a, b);
    return s == NULL || *end == '\0';
}

/* "x, y" into two Exprs: a missing value is fine; a value must give at least one Expr and end there. */
static int parse_pair_checked(Ini *ini, const char *key, const char *fmt, Expr *a, Expr *b, CharData *owner)
{
    const char *s = Ini_GetValue(ini, key);
    const char *end;

    if (s == NULL) {
        return 1;
    }
    if (!Cns_ParseValues(s, fmt, owner, &end, a, b)) {
        return 0;
    }
    return *end == '\0';
}

/* As parse_pair_checked, testing the end of the text before the count. */
static int parse_pair_end_first(Ini *ini, const char *key, const char *fmt, Expr *a, Expr *b, CharData *owner)
{
    const char *s = Ini_GetValue(ini, key);
    const char *end;
    int n;

    if (s == NULL) {
        return 1;
    }
    n = Cns_ParseValues(s, fmt, owner, &end, a, b);
    if (*end != '\0') {
        return 0;
    }
    return n != 0;
}

/* x, y and z of PosSet/PosAdd/VelSet/VelAdd/VelMul/Offset: arg[0..2] flag the given ones, arg[3..5] the values. */
static int parse_xyz(Ini *ini, StateCtrl *sc, CharData *owner, int has_z)
{
    static const char *const keys[3] = { "x", "y", "z" };
    const char *s;
    int i;

    for (i = 0; i < (has_z ? 3 : 2); i++) {
        s = Ini_GetValue(ini, keys[i]);
        if (s != NULL) {
            Expr_SetInt(&sc->arg[i], 1);
            if (!Expr_Compile(s, &sc->arg[3 + i], owner, NULL, EXPR_WANT_FLOAT)) {
                return 0;
            }
        }
    }
    return 1;
}

/* The S/C/L/A first letter of a state type, or 0. */
static int statetype_of(int c)
{
    switch (c) {
    case 'a':
        return STATETYPE_AIR;
    case 'c':
        return STATETYPE_CROUCH;
    case 'l':
        return STATETYPE_LIEDOWN;
    case 's':
        return STATETYPE_STAND;
    default:
        return 0;
    }
}

/*
 * 0x00444DD0: the type of the controller in the selected [State] group of ini
 * and its parameters (sc->arg, sc->params). Prints why on most errors.
 */
int StateCtrl_ParseParams(Ini *ini, StateCtrl *sc, CharData *owner)
{
    char name[0x100];
    const char *s;
    const char *end;
    int kind, index, n, i, ok;

    s = Ini_GetValue(ini, "type");
    if (s == NULL) {
        return 0;
    }
    strcpy(name, s);
    _strlwr(name);
    for (i = 0; i < 6; i++) {
        Expr_SetInt(&sc->arg[i], 0);
    }

    if (strcmp(name, "changestate") == 0 || strcmp(name, "selfstate") == 0) {
        sc->type = name[0] == 'c' ? SCTRL_CHANGESTATE : SCTRL_SELFSTATE;
        if (!compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[1], -1);
        Expr_SetInt(&sc->arg[2], -1);
        if (!compile_key(ini, "ctrl", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "anim", &sc->arg[2], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "tag") == 0) {
        sc->type = SCTRL_TAG;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], -1);
        if (!compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "partnerstateno", &sc->arg[1], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "ctrlset") == 0) {
        sc->type = SCTRL_CTRLSET;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "turn") == 0) {
        sc->type = SCTRL_TURN;
        ok = 1;
    } else if (strcmp(name, "changeanim") == 0 || strcmp(name, "changeanim2") == 0) {
        sc->type = name[10] == '\0' ? SCTRL_CHANGEANIM : SCTRL_CHANGEANIM2;
        if (!compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "elem", &sc->arg[1], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "playsnd") == 0) {
        PlaySndParams *ps = Mem_Calloc(1, sizeof(PlaySndParams));
        if (ps == NULL) {
            return 0;
        }
        sc->type = SCTRL_PLAYSND;
        if (!PlaySnd_ParseParams(ps, ini, owner)) {
            free(ps);
            Error_Printf("Error parsing PlaySnd\n");
            return 0;
        }
        sc->params = ps;
        ok = 1;
    } else if (strcmp(name, "stopsnd") == 0 || strcmp(name, "sndpan") == 0) {
        sc->type = name[1] == 't' ? SCTRL_STOPSND : SCTRL_SNDPAN;
        s = Ini_GetValue(ini, "channel");
        if (s == NULL) {
            Error_Printf("Missing channel parameter\n");
            return 0;
        }
        ok = Expr_Compile(s, &sc->arg[0], owner, NULL, EXPR_WANT_INT);
        if (ok && sc->type == SCTRL_SNDPAN) {
            s = Ini_GetValue(ini, "abspan");
            if (s != NULL) {
                Expr_SetInt(&sc->arg[2], 1);
                if (!Expr_Compile(s, &sc->arg[1], owner, NULL, EXPR_WANT_INT)) {
                    return 0;
                }
            }
            ok = compile_key(ini, "pan", &sc->arg[1], owner, EXPR_WANT_INT);
        }
    } else if (strcmp(name, "lifeset") == 0) {
        sc->type = SCTRL_LIFESET;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "lifeadd") == 0) {
        sc->type = SCTRL_LIFEADD;
        if (!compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[1], 1);
        if (!compile_key(ini, "kill", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "absolute", &sc->arg[2], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "powerset") == 0) {
        sc->type = SCTRL_POWERSET;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "poweradd") == 0) {
        sc->type = SCTRL_POWERADD;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "posset") == 0 || strcmp(name, "posadd") == 0) {
        sc->type = name[3] == 's' ? SCTRL_POSSET : SCTRL_POSADD;
        ok = parse_xyz(ini, sc, owner, 1);
    } else if (strcmp(name, "posfreeze") == 0) {
        sc->type = SCTRL_POSFREEZE;
        Expr_SetInt(&sc->arg[0], 1);
        ok = compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "velset") == 0 || strcmp(name, "veladd") == 0 || strcmp(name, "velmul") == 0) {
        sc->type = name[3] == 's' ? SCTRL_VELSET : name[3] == 'a' ? SCTRL_VELADD : SCTRL_VELMUL;
        ok = parse_xyz(ini, sc, owner, 1);
    } else if (strcmp(name, "gravity") == 0) {
        sc->type = SCTRL_GRAVITY;
        ok = 1;
    } else if (strcmp(name, "hitdef") == 0) {
        HitDefParams *hd = Mem_Calloc(1, sizeof(HitDefParams));
        if (hd == NULL) {
            return 0;
        }
        sc->type = SCTRL_HITDEF;
        if (!HitDef_ParseParams(ini, hd, owner)) {
            free(hd);
            Error_Printf("Error parsing Hitdef\n");
            return 0;
        }
        sc->params = hd;
        ok = 1;
    } else if (strcmp(name, "reversaldef") == 0) {
        HitDefParams *hd = Mem_Calloc(1, sizeof(HitDefParams));
        if (hd == NULL) {
            return 0;
        }
        sc->type = SCTRL_REVERSALDEF;
        s = Ini_GetValue(ini, "reversal.attr");
        if (s == NULL) {
            Error_Printf("Missing reversal.attr parameter\n");
            return 0;
        }
        hd->reversal_attr = HitDefAttr_Parse(s, NULL);
        if (hd->reversal_attr == 0xFFFFFFFFu) {
            Error_Printf("Invalid reversal.attr: %s\n", s);
            return 0;
        }
        if (!HitDef_ParseParams(ini, hd, owner)) {
            free(hd);
            Error_Printf("Error parsing Reversaldef\n");
            return 0;
        }
        sc->params = hd;
        ok = 1;
    } else if (strcmp(name, "hitby") == 0 || strcmp(name, "nothitby") == 0) {
        int attr;
        sc->type = SCTRL_HITBY;
        s = Ini_GetValue(ini, "value");
        if (s != NULL) {
            index = 0;
        } else {
            s = Ini_GetValue(ini, "value2");
            if (s == NULL) {
                return 0;
            }
            index = 1;
        }
        Expr_SetInt(&sc->arg[2], index);
        attr = HitDefAttr_Parse(s, NULL);
        if (attr == -1) {
            return 0;
        }
        Expr_SetInt(&sc->arg[0], name[0] == 'h' ? attr : ~attr & 0xFFF);
        Expr_SetInt(&sc->arg[1], 1);
        ok = compile_key(ini, "time", &sc->arg[1], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "hitvelset") == 0) {
        sc->type = SCTRL_HITVELSET;
        if (!compile_key(ini, "x", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "y", &sc->arg[1], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "hitfallset") == 0) {
        sc->type = SCTRL_HITFALLSET;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetFloat(&sc->arg[3], -FLT_MAX);
        Expr_SetFloat(&sc->arg[4], -FLT_MAX);
        if (!compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = parse_pair(ini, "velocity", "ff", &sc->arg[3], &sc->arg[4], owner);
    } else if (strcmp(name, "hitfalldamage") == 0) {
        sc->type = SCTRL_HITFALLDAMAGE;
        ok = 1;
    } else if (strcmp(name, "hitfallvel") == 0) {
        sc->type = SCTRL_HITFALLVEL;
        ok = 1;
    } else if (strcmp(name, "hitadd") == 0) {
        sc->type = SCTRL_HITADD;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "attackmulset") == 0 || strcmp(name, "defencemulset") == 0) {
        sc->type = name[0] == 'a' ? SCTRL_ATTACKMULSET : SCTRL_DEFENCEMULSET;
        ok = compile_required(ini, "value", &sc->arg[3], owner, EXPR_WANT_FLOAT);
    } else if (strcmp(name, "statetypeset") == 0) {
        sc->type = SCTRL_STATETYPESET;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], -1);
        Expr_SetInt(&sc->arg[2], -1);
        s = Ini_GetValue(ini, "statetype");
        if (s == NULL) {
            s = Ini_GetValue(ini, "value");
        }
        if (s != NULL) {
            n = statetype_of(Char_ToLower(*s));
            if (n == 0) {
                Error_Printf("State type \"%s\" not valid\n", s);
                return 0;
            }
            Expr_SetInt(&sc->arg[0], n);
        }
        s = Ini_GetValue(ini, "movetype");
        if (s != NULL) {
            switch (Char_ToLower(*s)) {
            case 'a':
                n = MOVETYPE_ATTACK;
                break;
            case 'h':
                n = MOVETYPE_HIT;
                break;
            case 'i':
                n = MOVETYPE_IDLE;
                break;
            default:
                Error_Printf("State movetype \"%s\" not valid\n", s);
                return 0;
            }
            Expr_SetInt(&sc->arg[1], n);
        }
        s = Ini_GetValue(ini, "physics");
        if (s != NULL) {
            switch (Char_ToLower(*s)) {
            case 'a':
                n = PHYSICS_AIR;
                break;
            case 'c':
                n = PHYSICS_CROUCH;
                break;
            case 'n':
                n = PHYSICS_NONE;
                break;
            case 's':
                n = PHYSICS_STAND;
                break;
            default:
                Error_Printf("State physics \"%s\" not valid\n", s);
                return 0;
            }
            Expr_SetInt(&sc->arg[2], n);
        }
        ok = 1;
    } else if (strcmp(name, "hitoverride") == 0) {
        HitOverrideParams *ho = Mem_Calloc(1, sizeof(HitOverrideParams));
        if (ho == NULL) {
            return 0;
        }
        sc->type = SCTRL_HITOVERRIDE;
        if (!HitOverride_ParseParams(ini, owner, ho)) {
            free(ho);
            Error_Printf("Error parsing HitOverride\n");
            return 0;
        }
        if (!compile_key(ini, "slot", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        sc->params = ho;
        ok = 1;
    } else if (strcmp(name, "movehitreset") == 0) {
        sc->type = SCTRL_MOVEHITRESET;
        ok = 1;
    } else if (strcmp(name, "victoryquote") == 0) {
        sc->type = SCTRL_VICTORYQUOTE;
        Expr_SetInt(&sc->arg[0], -1);
        ok = compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "assertspecial") == 0) {
        static const char *const flag_keys[3] = { "flag", "flag2", "flag3" };
        sc->type = SCTRL_ASSERTSPECIAL;
        s = Ini_GetValue(ini, "flag");
        if (s == NULL) {
            Error_Printf("AssertSpecial flag parameter missing\n");
            return 0;
        }
        for (i = 0; i < 3; i++) {
            if (i > 0) {
                s = Ini_GetValue(ini, flag_keys[i]);
                if (s == NULL) {
                    continue;
                }
            }
            if (!AssertSpecial_ParseFlag(s, &kind, &index)) {
                if (i == 0) {
                    Error_Printf("AssertSpecial flag \"%s\" not valid\n", s);
                } else if (i == 1) {
                    Error_Printf("AssertSpecial flag2 \"%s\" not valid\n", s);
                } else {
                    Error_Printf("AssertSpecial flag3 \"%s\" not valid\n", s);
                }
                return 0;
            }
            Expr_SetInt(&sc->arg[i], index);
            Expr_SetInt(&sc->arg[3 + i], kind);
            if (i == 0) {
                Expr_SetInt(&sc->arg[1], -1);
                Expr_SetInt(&sc->arg[2], -1);
            }
        }
        ok = 1;
    } else if (strcmp(name, "zoom") == 0) {
        sc->type = SCTRL_ZOOM;
        Expr_SetFloat(&sc->arg[0], 1.0f);
        Expr_SetFloat(&sc->arg[1], 1.0f);
        Expr_SetFloat(&sc->arg[3], 1.0f);
        Expr_SetFloat(&sc->arg[4], 0.0f);
        if (!parse_pair_checked(ini, "pos", "ff", &sc->arg[0], &sc->arg[1], owner)) {
            return 0;
        }
        if (!compile_key(ini, "scale", &sc->arg[3], owner, EXPR_WANT_FLOAT)) {
            return 0;
        }
        ok = compile_key(ini, "lag", &sc->arg[4], owner, EXPR_WANT_FLOAT);
    } else if (strcmp(name, "projectile") == 0) {
        ProjectileParams *pp = Mem_Calloc(1, sizeof(ProjectileParams));
        if (pp == NULL) {
            return 0;
        }
        sc->type = SCTRL_PROJECTILE;
        if (!Projectile_ParseParams(pp, owner, ini)) {
            free(pp);
            Error_Printf("Error parsing Projectile\n");
            return 0;
        }
        sc->params = pp;
        ok = 1;
    } else if (strcmp(name, "width") == 0) {
        int given;
        sc->type = SCTRL_WIDTH;
        Expr_SetInt(&sc->arg[3], 0);
        Expr_SetInt(&sc->arg[0], 0);
        Expr_SetInt(&sc->arg[1], 0);
        Expr_SetInt(&sc->arg[2], 0);
        s = Ini_GetValue(ini, "edge");
        given = s != NULL;
        Cns_ParseValues(s, "ii", owner, &end, &sc->arg[0], &sc->arg[1]);
        if (s != NULL && *end != '\0') {
            return 0;
        }
        s = Ini_GetValue(ini, "player");
        given |= s != NULL;
        Cns_ParseValues(s, "ii", owner, &end, &sc->arg[2], &sc->arg[3]);
        if (s != NULL && *end != '\0') {
            return 0;
        }
        if (!given) {
            s = Ini_GetValue(ini, "value");
            if (!Cns_ParseValues(s, "ii", owner, &end, &sc->arg[0], &sc->arg[1])) {
                return 0;
            }
            if (*end != '\0') {
                return 0;
            }
            Expr_Clone(&sc->arg[2], &sc->arg[0]);
            Expr_Clone(&sc->arg[3], &sc->arg[1]);
        }
        ok = 1;
    } else if (strcmp(name, "sprpriority") == 0) {
        sc->type = SCTRL_SPRPRIORITY;
        ok = compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "angledraw") == 0) {
        sc->type = SCTRL_ANGLEDRAW;
        Expr_SetFloat(&sc->arg[4], 1.0f);
        Expr_SetFloat(&sc->arg[5], 1.0f);
        if (!Cns_CompileFloat(ini, "value", &sc->arg[3], owner, -FLT_MAX)) {
            return 0;
        }
        ok = parse_pair(ini, "scale", "ff", &sc->arg[4], &sc->arg[5], owner);
    } else if (strcmp(name, "angleset") == 0 || strcmp(name, "angleadd") == 0 || strcmp(name, "anglemul") == 0) {
        sc->type = name[5] == 's' ? SCTRL_ANGLESET : name[5] == 'a' ? SCTRL_ANGLEADD : SCTRL_ANGLEMUL;
        ok = compile_key(ini, "value", &sc->arg[3], owner, EXPR_WANT_FLOAT);
    } else if (strcmp(name, "trans") == 0) {
        int alpha_dst, alpha_src, trans;
        sc->type = SCTRL_TRANS;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], 0x100);
        Expr_SetInt(&sc->arg[2], 0);
        s = Ini_GetValue(ini, "trans");
        if (s != NULL) {
            trans = Trans_Parse(s, &alpha_dst, &alpha_src);
            if (trans == -2) {
                Error_Printf("Invalid trans type: %s\n", s);
                return 0;
            }
            Expr_SetInt(&sc->arg[0], trans);
            Expr_SetInt(&sc->arg[1], alpha_src);
            Expr_SetInt(&sc->arg[2], alpha_dst);
        }
        s = Ini_GetValue(ini, "alpha");
        if (s != NULL) {
            Cns_ParseValues(s, "ii", owner, &end, &sc->arg[1], &sc->arg[2]);
            if (*end != '\0') {
                return 0;
            }
        }
        ok = 1;
    } else if (strcmp(name, "screenbound") == 0) {
        sc->type = SCTRL_SCREENBOUND;
        Expr_SetInt(&sc->arg[0], 0);
        if (!compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[1], 0);
        Expr_SetInt(&sc->arg[2], 0);
        ok = parse_pair(ini, "movecamera", "ii", &sc->arg[1], &sc->arg[2], owner);
    } else if (strcmp(name, "targetbind") == 0) {
        sc->type = SCTRL_TARGETBIND;
        Expr_SetInt(&sc->arg[0], 1);
        Expr_SetInt(&sc->arg[2], -1);
        Expr_SetFloat(&sc->arg[3], 0.0f);
        Expr_SetFloat(&sc->arg[4], 0.0f);
        Expr_SetFloat(&sc->arg[5], 0.0f);
        if (!compile_key(ini, "time", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!compile_key(ini, "id", &sc->arg[2], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = parse_pair(ini, "pos", "ff", &sc->arg[3], &sc->arg[4], owner);
    } else if (strcmp(name, "bindtotarget") == 0) {
        sc->type = SCTRL_BINDTOTARGET;
        Expr_SetInt(&sc->arg[0], 1);
        Expr_SetInt(&sc->arg[2], -1);
        Expr_SetFloat(&sc->arg[3], -FLT_MAX);
        Expr_SetFloat(&sc->arg[4], -FLT_MAX);
        Expr_SetFloat(&sc->arg[5], 0.0f);
        Expr_SetInt(&sc->arg[1], 0);
        if (!compile_key(ini, "time", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!compile_key(ini, "id", &sc->arg[2], owner, EXPR_WANT_INT)) {
            return 0;
        }
        s = Ini_GetValue(ini, "pos");
        if (s != NULL) {
            n = Cns_ParseValues(s, "ff:,", owner, &end, &sc->arg[3], &sc->arg[4]);
            if (*end != '\0') {
                if (*end != ',') {
                    return 0;
                }
                if (n < 2) {
                    return 0;
                }
                end++;
            }
            while (Char_IsSpace(*end)) {
                end++;
            }
            switch (Char_ToLower(*end)) {
            case 0:
                break;
            case 'f':
                Expr_SetInt(&sc->arg[1], 0);
                break;
            case 'h':
                Expr_SetInt(&sc->arg[1], 2);
                break;
            case 'm':
                Expr_SetInt(&sc->arg[1], 1);
                break;
            default:
                Error_Printf("Invalid bind position: %s\n", end);
                return 0;
            }
        }
        ok = 1;
    } else if (strcmp(name, "targetstate") == 0 || strcmp(name, "targetpoweradd") == 0) {
        sc->type = name[6] == 's' ? SCTRL_TARGETSTATE : SCTRL_TARGETPOWERADD;
        if (!compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = Cns_CompileInt(ini, "id", &sc->arg[2], owner, -1);
    } else if (strcmp(name, "targetlifeadd") == 0) {
        sc->type = SCTRL_TARGETLIFEADD;
        if (!compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[1], 1);
        if (!compile_key(ini, "kill", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!compile_key(ini, "absolute", &sc->arg[2], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = Cns_CompileInt(ini, "id", &sc->arg[5], owner, -1);
    } else if (strcmp(name, "targetvelset") == 0 || strcmp(name, "targetveladd") == 0) {
        sc->type = name[9] == 's' ? SCTRL_TARGETVELSET : SCTRL_TARGETVELADD;
        if (!Cns_CompileFloat(ini, "x", &sc->arg[3], owner, -FLT_MAX)) {
            return 0;
        }
        if (!Cns_CompileFloat(ini, "y", &sc->arg[4], owner, -FLT_MAX)) {
            return 0;
        }
        if (!Cns_CompileFloat(ini, "z", &sc->arg[5], owner, -FLT_MAX)) {
            return 0;
        }
        ok = Cns_CompileInt(ini, "id", &sc->arg[2], owner, -1);
    } else if (strcmp(name, "targetfacing") == 0) {
        sc->type = SCTRL_TARGETFACING;
        if (!compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = Cns_CompileInt(ini, "id", &sc->arg[2], owner, -1);
    } else if (strcmp(name, "targetdrop") == 0) {
        sc->type = SCTRL_TARGETDROP;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], 1);
        if (!compile_key(ini, "excludeID", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "keepone", &sc->arg[1], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "attackdist") == 0) {
        sc->type = SCTRL_ATTACKDIST;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "playerpush") == 0) {
        sc->type = SCTRL_PLAYERPUSH;
        ok = compile_required(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "varset") == 0 || strcmp(name, "varadd") == 0) {
        sc->type = strcmp(name, "varset") == 0 ? SCTRL_VARSET : SCTRL_VARADD;
        s = Ini_GetValue(ini, "value");
        ok = s != NULL ? parse_var_value(ini, s, sc, owner) : parse_var_line(ini, sc, owner, 1);
    } else if (strcmp(name, "varrangeset") == 0) {
        int isfloat;
        sc->type = SCTRL_VARRANGESET;
        s = Ini_GetValue(ini, "value");
        if (s != NULL) {
            isfloat = 0;
        } else {
            s = Ini_GetValue(ini, "fvalue");
            if (s == NULL) {
                Error_Printf("VarRangeSet value not specified.\n");
                Error_Exit();
            }
            isfloat = 1;
        }
        if (!Cns_CompileInt(ini, "first", &sc->arg[0], owner, 0)) {
            return 0;
        }
        if (!Cns_CompileInt(ini, "last", &sc->arg[1], owner, isfloat ? PLAYER_FVAR_COUNT - 1 : PLAYER_VAR_COUNT - 1)) {
            return 0;
        }
        if (!Expr_Compile(s, &sc->arg[2], owner, NULL, isfloat + 1)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[3], isfloat);
        ok = 1;
    } else if (strcmp(name, "varrandom") == 0) {
        sc->type = SCTRL_VARRANDOM;
        if (!compile_required(ini, "v", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_SetInt(&sc->arg[0], 0);
        Expr_SetInt(&sc->arg[2], 1000);
        s = Ini_GetValue(ini, "range");
        if (s != NULL) {
            n = Cns_ParseValues(s, "ii", owner, &end, &sc->arg[0], &sc->arg[2]);
            if (*end != '\0') {
                return 0;
            }
            if (n == 0) {
                return 0;
            }
            if (n == 1) {
                /* plain copy: the tokens of a non-constant value are freed below (original bug) */
                sc->arg[2] = sc->arg[0];
                Expr_SetInt(&sc->arg[0], 0);
            }
        }
        ok = 1;
    } else if (strcmp(name, "offset") == 0) {
        sc->type = SCTRL_OFFSET;
        ok = parse_xyz(ini, sc, owner, 0);
    } else if (strcmp(name, "envshake") == 0) {
        EnvShakeParams *es;
        sc->type = SCTRL_ENVSHAKE;
        es = Mem_Calloc(1, sizeof(EnvShakeParams));
        if (es == NULL) {
            return 0;
        }
        if (!EnvShake_ParseParams("", es, ini, owner)) {
            free(es);
            Error_Printf("Error parsing EnvShake\n");
            return 0;
        }
        sc->params = es;
        ok = 1;
    } else if (strcmp(name, "fallenvshake") == 0) {
        sc->type = SCTRL_FALLENVSHAKE;
        ok = 1;
    } else if (strcmp(name, "envcolor") == 0) {
        sc->type = SCTRL_ENVCOLOR;
        Expr_SetInt(&sc->arg[3], 0xFF);
        Expr_SetInt(&sc->arg[4], 0xFF);
        Expr_SetInt(&sc->arg[5], 0xFF);
        s = Ini_GetValue(ini, "value");
        if (s != NULL) {
            n = Cns_ParseValues(s, "iii", owner, &end, &sc->arg[3], &sc->arg[4], &sc->arg[5]);
            if (*end != '\0') {
                return 0;
            }
            if (n != 3) {
                return 0;
            }
        }
        Expr_SetInt(&sc->arg[1], 1);
        Expr_SetInt(&sc->arg[2], 0);
        if (!compile_key(ini, "time", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = compile_key(ini, "under", &sc->arg[2], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "gamemakeanim") == 0) {
        sc->type = SCTRL_GAMEMAKEANIM;
        Expr_SetInt(&sc->arg[0], 0);
        Expr_SetInt(&sc->arg[1], 0);
        Expr_SetFloat(&sc->arg[3], 0.0f);
        Expr_SetFloat(&sc->arg[4], 0.0f);
        if (!compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!compile_key(ini, "under", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!parse_pair_end_first(ini, "pos", "ff", &sc->arg[3], &sc->arg[4], owner)) {
            return 0;
        }
        ok = compile_key(ini, "random", &sc->arg[2], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "afterimage") == 0) {
        AfterImageParams *ai = Mem_Calloc(1, sizeof(AfterImageParams));
        if (ai == NULL) {
            return 0;
        }
        sc->type = SCTRL_AFTERIMAGE;
        if (!AfterImage_ParseParams(ai, ini, owner, "")) {
            free(ai);
            Error_Printf("Error parsing AfterImage\n");
            return 0;
        }
        sc->params = ai;
        ok = 1;
    } else if (strcmp(name, "afterimagetime") == 0) {
        sc->type = SCTRL_AFTERIMAGETIME;
        s = Ini_GetValue(ini, "time");
        if (s != NULL) {
            ok = Expr_Compile(s, &sc->arg[0], owner, NULL, EXPR_WANT_INT);
        } else {
            ok = compile_key(ini, "value", &sc->arg[0], owner, EXPR_WANT_INT);
        }
    } else if (strcmp(name, "pause") == 0) {
        PauseParams *pp = Mem_Calloc(1, sizeof(PauseParams));
        if (pp == NULL) {
            return 0;
        }
        sc->type = SCTRL_PAUSE;
        if (!Pause_ParseParams(pp, ini, owner)) {
            free(pp);
            Error_Printf("Error parsing Pause\n");
            return 0;
        }
        sc->params = pp;
        ok = 1;
    } else if (strcmp(name, "superpause") == 0) {
        SuperPauseParams *sp = Mem_Calloc(1, sizeof(SuperPauseParams));
        if (sp == NULL) {
            return 0;
        }
        sc->type = SCTRL_SUPERPAUSE;
        if (!SuperPause_ParseParams(ini, owner, sp)) {
            free(sp);
            Error_Printf("Error parsing SuperPause\n");
            return 0;
        }
        sc->params = sp;
        ok = 1;
    } else if (strcmp(name, "makedust") == 0) {
        sc->type = SCTRL_MAKEDUST;
        Expr_SetInt(&sc->arg[2], 3);
        if (!parse_pair_end_first(ini, "pos", "ii", &sc->arg[0], &sc->arg[1], owner)) {
            return 0;
        }
        s = Ini_GetValue(ini, "pos2");
        if (s != NULL) {
            n = Cns_ParseValues(s, "ff", owner, &end, &sc->arg[3], &sc->arg[4]);
            if (*end != '\0') {
                return 0;
            }
            if (n == 0) {
                return 0;
            }
            Expr_SetFloat(&sc->arg[5], 1.0f);
        }
        ok = compile_key(ini, "spacing", &sc->arg[2], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "palfx") == 0 || strcmp(name, "bgpalfx") == 0 || strcmp(name, "allpalfx") == 0) {
        PalFxParams *pf;
        sc->type = SCTRL_PALFX;
        pf = Mem_Calloc(1, sizeof(PalFxParams));
        if (pf == NULL) {
            return 0;
        }
        if (!PalFx_ParseParams(pf, ini, owner, 0)) {
            return 0;   /* pf leaks */
        }
        pf->kind = name[0] == 'p' ? PALFX_PLAYER : name[0] == 'b' ? PALFX_BG : PALFX_ALL;
        sc->params = pf;
        ok = 1;
    } else if (strcmp(name, "remappal") == 0) {
        sc->type = SCTRL_REMAPPAL;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], 0);
        Expr_SetInt(&sc->arg[3], -1);
        Expr_SetInt(&sc->arg[4], 0);
        if (!parse_pair(ini, "source", "ii", &sc->arg[0], &sc->arg[1], owner)) {
            return 0;
        }
        s = Ini_GetValue(ini, "dest");
        if (s == NULL) {
            return 0;
        }
        Cns_ParseValues(s, "ii", owner, &end, &sc->arg[3], &sc->arg[4]);
        ok = *end == '\0';
    } else if (strcmp(name, "explod") == 0 || strcmp(name, "modifyexplod") == 0) {
        ExplodParams *ep = Mem_Calloc(1, sizeof(ExplodParams));
        if (ep == NULL) {
            return 0;
        }
        sc->type = name[0] == 'e' ? SCTRL_EXPLOD : SCTRL_MODIFYEXPLOD;
        if (!Explod_ParseParams(ep, ini, owner, name[0] != 'e')) {
            return 0;   /* ep leaks */
        }
        sc->params = ep;
        ok = 1;
    } else if (strcmp(name, "removeexplod") == 0) {
        sc->type = SCTRL_REMOVEEXPLOD;
        Expr_SetInt(&sc->arg[0], -1);
        ok = compile_key(ini, "id", &sc->arg[0], owner, EXPR_WANT_INT);
    } else if (strcmp(name, "explodbindtime") == 0) {
        sc->type = SCTRL_EXPLODBINDTIME;
        Expr_SetInt(&sc->arg[0], -1);
        Expr_SetInt(&sc->arg[1], 1);
        if (!compile_key(ini, "id", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        s = Ini_GetValue(ini, "time");
        if (s == NULL) {
            s = Ini_GetValue(ini, "value");
        }
        ok = Expr_Compile(s, &sc->arg[1], owner, NULL, EXPR_WANT_INT);
    } else if (strcmp(name, "forcefeedback") == 0) {
        sc->type = SCTRL_NULL;
        ok = 1;
    } else if (strcmp(name, "helper") == 0) {
        HelperParams *hp;
        sc->type = SCTRL_HELPER;
        hp = Mem_Calloc(1, sizeof(HelperParams));
        if (hp == NULL) {
            return 0;
        }
        if (!Helper_ParseParams(hp, ini, owner)) {
            free(hp);
            Error_Printf("Error parsing Helper\n");
            return 0;
        }
        sc->params = hp;
        ok = 1;
    } else if (strcmp(name, "destroyself") == 0) {
        sc->type = SCTRL_DESTROYSELF;
        if (!Cns_CompileInt(ini, "recursive", &sc->arg[0], owner, 0)) {
            return 0;
        }
        ok = Cns_CompileInt(ini, "removeexplods", &sc->arg[1], owner, 0);
    } else if (strcmp(name, "parentvarset") == 0 || strcmp(name, "parentvaradd") == 0) {
        sc->type = name[9] == 's' ? SCTRL_PARENTVARSET : SCTRL_PARENTVARADD;
        s = Ini_GetValue(ini, "value");
        ok = s != NULL ? parse_var_value(ini, s, sc, owner) : parse_var_line(ini, sc, owner, 0);
    } else if (strcmp(name, "bindtoparent") == 0 || strcmp(name, "bindtoroot") == 0) {
        sc->type = name[6] == 'p' ? SCTRL_BINDTOPARENT : SCTRL_BINDTOROOT;
        Expr_SetInt(&sc->arg[0], 1);
        if (!compile_key(ini, "time", &sc->arg[0], owner, EXPR_WANT_INT)) {
            return 0;
        }
        if (!compile_key(ini, "facing", &sc->arg[1], owner, EXPR_WANT_INT)) {
            return 0;
        }
        ok = parse_pair_checked(ini, "pos", "ff", &sc->arg[3], &sc->arg[4], owner);
    } else if (strcmp(name, "displaytoclipboard") == 0 || strcmp(name, "appendtoclipboard") == 0) {
        CnsText *text;
        sc->type = (name[0] != 'd') + SCTRL_DISPLAYTOCLIPBOARD;
        s = Ini_GetValue(ini, "text");
        if (s == NULL) {
            Error_Printf("Text not specified for %s.\n", name);
            return 0;
        }
        text = CnsText_Parse(s);
        if (text == NULL) {
            return 0;
        }
        sc->params = text;
        s = Ini_GetValue(ini, "params");
        if (s != NULL) {
            n = Cns_ParseValues(s, "nnnnnn", owner, &end, &sc->arg[0], &sc->arg[1], &sc->arg[2], &sc->arg[3],
                                &sc->arg[4], &sc->arg[5]);
            if (n == 0) {
                return 0;
            }
            if (n != text->count) {
                return 0;
            }
            if (*end != '\0') {
                return 0;
            }
        }
        ok = 1;
    } else if (strcmp(name, "clearclipboard") == 0) {
        sc->type = SCTRL_CLEARCLIPBOARD;
        ok = 1;
    } else if (strcmp(name, "null") == 0) {
        sc->type = SCTRL_NULL;
        ok = 1;
    } else {
        Error_Printf("Not a valid elem type: %s\n", name);
        return 0;
    }
    if (!ok) {
        return 0;
    }
    if (!StateCtrl_VersionOk(sc->type, owner->mugenversion)) {
        Error_Printf("Controller not valid for mugenversion of character: %s\n", name);
        return 0;
    }
    return 1;
}

/*
 * 0x00448A10: the controller of the selected [State] group of ini, added to
 * sd->ctrls under (type, number). The trigger lines are compiled into a
 * keyed array by (group, line): triggerall is group -1, triggerN is N - 1.
 */
int StateCtrl_Parse(Ini *ini, StateDef *sd, int number, CharData *owner)
{
    StateCtrl sc;
    Expr e;
    char key[0x18];
    const char *s;
    int all_count = 0, groups = 0, count = 0;
    int group, line, done_lines, i;

    memset(&sc, 0, sizeof(sc));
    Ini_Rewind(ini);
    if (g_game->keep_sources != 0) {
        StateCtrlSource *src = calloc(1, sizeof(StateCtrlSource));
        if (src == NULL) {
            g_outOfMemory = 1;
        }
        sc.source = src;
        if (src == NULL) {
            Error_OutOfMemory();
        }
        sc.source->statedef = sd;
        sc.source->line = Ini_SourceLine(ini->cursor, ini);
    }
    if (Ini_GetValue(ini, "triggerall") != NULL) {
        all_count = 1;
        while (Ini_GetNextValue(ini, "triggerall") != NULL) {
            all_count++;
        }
    }
    strcpy(key, "trigger1");
    Ini_Rewind(ini);
    if (Ini_GetValue(ini, key) == NULL) {
        Error_Printf("Need at least 1 trigger\n");
        return 0;
    }
    do {
        do {
            count++;
        } while (Ini_GetNextValue(ini, key) != NULL);
        groups++;
        sprintf(key, "trigger%i", groups + 1);
    } while (Ini_GetValue(ini, key) != NULL);
    if (count == 0) {
        Error_Printf("Need at least 1 trigger\n");
        return 0;
    }
    sc.triggers = KeyedArray_Create(all_count + count, sizeof(Expr));
    if (sc.triggers == NULL) {
        return 0;
    }
    sc.trigger_groups = groups;
    Ini_Rewind(ini);
    memset(&e, 0, sizeof(e));
    Expr_SetInt(&e, 0);
    for (group = 0, done_lines = 0; done_lines < count; group++) {
        sprintf(key, "trigger%i", group + 1);
        s = Ini_GetValue(ini, key);
        line = 0;
        do {
            if (!Expr_Compile(s, &e, owner, NULL, EXPR_WANT_ANY)) {
                Error_Printf("Error parsing %s, %i\n", key, line + 1);
                return 0;
            }
            owner->trigger_expr_count++;
            KeyedArray_Add(sc.triggers, &e, group, line);
            line++;
            done_lines++;
            s = Ini_GetNextValue(ini, key);
        } while (s != NULL);
    }
    if (all_count > 0) {
        Ini_Rewind(ini);
        s = Ini_GetValue(ini, "triggerall");
        for (i = 0; s != NULL; i++) {
            if (!Expr_Compile(s, &e, owner, NULL, EXPR_WANT_ANY)) {
                Error_Printf("Error parsing triggerall\n");
                return 0;
            }
            owner->trigger_expr_count++;
            KeyedArray_Add(sc.triggers, &e, -1, i);
            s = Ini_GetNextValue(ini, "triggerall");
        }
    }
    sc.persistent = 1;
    s = Ini_GetValue(ini, "persistent");
    if (s != NULL) {
        sc.persistent = atoi(s);
    }
    sc.ignorehitpause = 0;
    s = Ini_GetValue(ini, "ignorehitpause");
    if (s != NULL) {
        sc.ignorehitpause = atoi(s);
    }
    if (!StateCtrl_ParseParams(ini, &sc, owner)) {
        return 0;
    }
    return KeyedArray_Add(sd->ctrls, &sc, sc.type, number) != -1;
}

/* ------------------------------------------------------------ running */

/* game units per unit of the character whose states run (the custom state owner's, else the player's) */
static double states_scale(Player *p)
{
    GameVars *g = g_game;
    Player *owner;

    if (p->custom_states != NULL && p->custom_owner != -1) {
        owner = g->players[p->custom_owner];
        if (owner != NULL) {
            return g_gameCoord[0] / owner->data->localcoord[0];
        }
    }
    return g_gameCoord[0] / p->data->localcoord[0];
}

/* A target entry the Target* controllers act on: not guarding, hit by a HitDef of the ID (-1: any). */
static int target_matches(const TargetEntry *e, int id)
{
    return e->player->in_guard_dist == 0 && (id == -1 || id == e->player->gethit_id);
}

/*
 * 0x00448F50: runs a controller of p's states (StateCtrl_Apply), returning its
 * STATECTRL_* bits. The state, state type, move type and control from before
 * are kept; when a controller of the player's own states changes the state of
 * an active player that used commands this tick, the AI records the move.
 */
int StateCtrl_Run(Player *p, StateCtrl *sc, StateRunCtx *ctx)
{
    int own = p->custom_states == NULL;
    int r;
    int n;

    p->ctrl_state_no = p->state_no;
    p->ctrl_statetype = p->statetype;
    p->ctrl_movetype = p->movetype;
    p->ctrl_ctrl = p->ctrl;
    r = StateCtrl_Apply(p, sc, ctx);
    if (p->active && (r & STATECTRL_CHANGED) && own && p->custom_states == NULL) {
        for (n = 0; p->ai_cmds[n] != -1; n++) {
        }
        if (n > 0) {
            Ai_CopyCommands(p->ai_cmds, p->ai_state_cmds);
            p->ai_state_frame = g_game->frames;
            Ai_RecordMove(p);
            Ai_LearnMove(p);
        }
    }
    return r;
}

/* VarSet / VarAdd (and the sys variants): var(arg[1]) of kind arg[2] set to / increased by arg[0]. */
static int run_var(Player *p, StateCtrl *sc, int add)
{
    int kind = Expr_EvalInt(p, &sc->arg[2], 0);
    int idx = Expr_EvalInt(p, &sc->arg[1], 1);
    float *sysfvar = (float *)&p->sysfvar[0];

    if (kind <= 1) {
        if (idx < 0 || idx > (kind != 0 ? PLAYER_FVAR_COUNT - 1 : PLAYER_VAR_COUNT - 1)) {
            Player_WarnAt(p, sc, "set var out of range (%i)", idx);
            return 0;
        }
    } else if ((unsigned int)idx > 4) {
        Player_WarnAt(p, sc, "set sysvar out of range (%i)", idx);
        return 0;
    }
    switch (kind) {
    case SCTRL_VAR:
        if (add) {
            p->var[idx] += Expr_EvalInt(p, &sc->arg[0], 1);
        } else {
            p->var[idx] = Expr_EvalInt(p, &sc->arg[0], 1);
        }
        break;
    case SCTRL_FVAR:
        if (add) {
            p->fvar[idx] = Expr_EvalFloat(p, &sc->arg[0]) + p->fvar[idx];
        } else {
            p->fvar[idx] = Expr_EvalFloat(p, &sc->arg[0]);
        }
        break;
    case SCTRL_SYSVAR:
        if (add) {
            p->sysvar[idx] += Expr_EvalInt(p, &sc->arg[0], 1);
        } else {
            p->sysvar[idx] = Expr_EvalInt(p, &sc->arg[0], 1);
        }
        break;
    case SCTRL_SYSFVAR:
        if (add) {
            sysfvar[idx] = Expr_EvalFloat(p, &sc->arg[0]) + sysfvar[idx];
        } else {
            sysfvar[idx] = Expr_EvalFloat(p, &sc->arg[0]);
        }
        break;
    }
    return 0;
}

/* ParentVarSet / ParentVarAdd: as VarSet / VarAdd on the parent (no sysvars). */
static int run_parent_var(Player *p, StateCtrl *sc, Player *parent, int add)
{
    int isf, idx;

    if (p->helper != 1) {
        return 0;
    }
    if (parent == NULL) {
        Player_WarnAt(p, sc, "%s", add ? "no parent for parentvaradd" : "no parent for parentvarset");
        return 0;
    }
    isf = Expr_EvalInt(p, &sc->arg[2], 0);
    idx = Expr_EvalInt(p, &sc->arg[1], 1);
    if (idx < 0 || idx > (isf != 0 ? PLAYER_FVAR_COUNT - 1 : PLAYER_VAR_COUNT - 1)) {
        Player_WarnAt(p, sc, "%s", "set parentvar out of range");
        return 0;
    }
    if (isf) {
        if (add) {
            parent->fvar[idx] = Expr_EvalFloat(p, &sc->arg[0]) + parent->fvar[idx];
        } else {
            parent->fvar[idx] = Expr_EvalFloat(p, &sc->arg[0]);
        }
    } else if (add) {
        parent->var[idx] += Expr_EvalInt(p, &sc->arg[0], 1);
    } else {
        parent->var[idx] = Expr_EvalInt(p, &sc->arg[0], 1);
    }
    return 0;
}

/* ChangeState / SelfState after the state change: ctrl (arg[1]) and anim (arg[2]) unless -1. */
static void run_ctrl_anim(Player *p, StateCtrl *sc)
{
    int v = Expr_EvalInt(p, &sc->arg[1], 1);

    if (v != -1) {
        p->ctrl = v != 0;
    }
    v = Expr_EvalInt(p, &sc->arg[2], 1);
    if (v != -1) {
        if (v < 0) {
            Player_WarnAt(p, sc, "%s", "attempted change to negative anim");
            v = 0;
        }
        Player_ChangeAnimSelf(p, v);
    }
}

/* ChangeAnim / ChangeAnim2: action arg[0] from element arg[1]. */
static void run_change_anim(Player *p, StateCtrl *sc, int anim2)
{
    int elem = Expr_EvalInt(p, &sc->arg[1], 1);
    int anim = Expr_EvalInt(p, &sc->arg[0], 1);

    if (elem < 0) {
        Player_WarnAt(p, sc, "%s", "changed to negative animelem");
        elem = 0;
    }
    if (anim < 0) {
        Player_WarnAt(p, sc, "%s", "attempted change to negative anim");
        anim = 0;
    }
    if (anim2) {
        if (elem == 0) {
            Player_ChangeAnim2(p, anim);
        } else {
            Player_ChangeAnim2Elem(p, anim, elem);
        }
    } else if (elem == 0) {
        Player_ChangeAnimSelf(p, anim);
    } else {
        Player_ChangeAnimSelfElem(p, anim, elem);
    }
}

/* One MakeDust cloud: fightfx action 120 at x, y (stage units) as an explod. */
static void make_dust(int x, int y)
{
    GameVars *g = g_game;
    int idx = Explods_Add(g->fightfx_sff, g->explods, 120, g->fightfx_anim);
    SysDrawItem *e;
    KeyedArray *actions;
    Coord c;
    double s;

    if (idx == -1) {
        return;
    }
    e = &g->explods->items[idx];
    e->pos = *Coord_MakeUnit(&c, x, y);
    e->space = 0;
    actions = e->anim->anim->actions;
    e->time = ((AnimAction *)(actions->data + actions->elem_size * e->anim->action))->total_time;
    e->anim->layer = 10;
    e->anim->queue = g->sprite_layer1;
    s = g->fightfx_scale * g->motif_scale;
    e->scale = *Coord_Make(&c, s, s);
}

/*
 * DisplayToClipboard / AppendToClipboard: the text with each parameter
 * printed by its format specifier into a 4 KB buffer, which then goes to the
 * player's clipboard (Player.warnings) as a format string itself. A "%d" or
 * "%i" given a float prints it with "%g"; a "bottom" value prints as
 * "(bottom)". The text pieces between the specifiers are printed as formats
 * too, without arguments.
 */
static void run_clipboard(Player *p, StateCtrl *sc)
{
    CnsText *t = sc->params;
    char buf[0x1000];
    unsigned int len = 0;
    int start = 0;
    int i, n;

    for (i = 0; i < t->count; i++) {
        char *mark = &t->text[t->pos[i]];
        char saved = *mark;
        int ival;
        float fval;
        int type;

        *mark = '\0';
        n = _snprintf(buf + len, 0x1000 - len, t->text + start);
        *mark = saved;
        start = t->pos[i];
        if (n < 0) {
            goto done;
        }
        len += n;
        if (len >= 0xFFF) {
            goto done;
        }
        type = Expr_Eval(p, &sc->arg[i], &ival, &fval);
        mark = &t->text[t->pos[i] + t->len[i]];
        saved = *mark;
        *mark = '\0';
        if (type == 3) {
            n = _snprintf(buf + len, 0x1000 - len, "(bottom)");
        } else if (type == 2) {
            if (t->types[i] == 1) {
                n = _snprintf(buf + len, 0x1000 - len, "%g", (double)fval);
            } else {
                n = _snprintf(buf + len, 0x1000 - len, t->text + start, (double)fval);
            }
        } else if (t->types[i] == 2) {
            n = _snprintf(buf + len, 0x1000 - len, t->text + start, (double)ival);
        } else {
            n = _snprintf(buf + len, 0x1000 - len, t->text + start, ival);
        }
        *mark = saved;
        start += t->len[i];
        if (n < 0) {
            goto done;
        }
        len += n;
        if (len >= 0xFFF) {
            goto done;
        }
    }
    if (len < 0xFFF) {
        _snprintf(buf + len, 0x1000 - len, t->text + start);
    }
done:
    buf[0xFFF] = '\0';
    if (sc->type == SCTRL_APPENDTOCLIPBOARD) {
        TextRing_Printf(p->warnings, buf);
    } else {
        TextRing_SetPrintf(p->warnings, buf);
    }
}

/* BindToParent / BindToRoot (helpers only). */
static int run_bind(Player *p, StateCtrl *sc, Player *to)
{
    if (p->helper != 1) {
        return 0;
    }
    if (to == NULL) {
        Player_WarnAt(p, sc, "%s", sc->type == SCTRL_BINDTOPARENT ? "No parent for bindtoparent" : "No root for bindtoroot");
        return 0;
    }
    if (!PlayerRef_Set(&p->bind_to, to, 0)) {
        Player_WarnAt(p, sc, "%s", "Unable to set bind");
        return 0;
    }
    p->bind_time = Expr_EvalInt(p, &sc->arg[0], 1);
    if (p->bind_time < -1) {
        p->bind_time = 1;
        Player_WarnAt(p, sc, "%s", "illegal helper bindtime");
    }
    p->bind_facing = Expr_EvalInt(p, &sc->arg[1], 1);
    if (p->bind_facing < 0) {
        if (p->bind_facing != -1) {
            p->bind_facing = -1;
            Player_WarnAt(p, sc, "%s", "bind's facing param not +/-1 or 0");
        }
    } else if (p->bind_facing > 0 && p->bind_facing != 1) {
        p->bind_facing = 1;
        Player_WarnAt(p, sc, "%s", "bind's facing param not +/-1 or 0");
    }
    p->bind_x = (float)Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[3]));
    p->bind_y = (float)Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4]));
    return 0;
}

/* SuperPause (Pause_Apply for the SuperPause kind, then darken, spark, p2defmul, poweradd, unhittable, sound). */
static int run_superpause(Player *p, StateCtrl *sc, StateRunCtx *ctx)
{
    GameVars *g = g_game;
    SuperPauseParams *sp = sc->params;
    TargetEntry *e;
    Player *leader;
    float defmul;
    int anim, v, group, number;

    Pause_Apply(1, p, &sp->pause, sc);
    if (Expr_EvalInt(p, &sp->darken, 1)) {
        g->pauses[1][0] = (int)p->pal_groups[0];
        g->pauses[1][1] = (int)~p->pal_groups[1];
        g->pauses[1][2] = (int)p->pal_groups[2];
        g->pauses[1][3] = (int)~p->pal_groups[3];
        g->pauses[1][4] = (int)p->pal_groups[4];
        g->pauses[1][5] = 1;
    } else {
        g->pauses[1][5] = 0;
    }
    if (p->pause_movetime[1] < 0 || p->pause_movetime[1] > g->pauses[1][6]) {
        Player_WarnAt(p, sc, "%s", "set illegal supermovetime");
        p->pause_movetime[1] = 0;
    }
    anim = Expr_EvalInt(p, &sp->anim, 1);
    if (anim < -1) {
        Player_WarnAt(p, sc, "%s", "negative spark animno ignored");
    } else if (anim != -1) {
        int x = Ftol((Player_FromLocalCoord(p, Expr_EvalFloat(p, &sp->pos[0])) - p->draw_offset.x) * p->facing + p->pos.x);
        int y = Ftol(Player_FromLocalCoord(p, Expr_EvalFloat(p, &sp->pos[1])) - p->draw_offset.y + p->pos.y);

        Spark_Create(anim, p, x, y, sp->anim_fight == 0, p->facing != 1, 0);
    }
    defmul = Expr_EvalFloat(p, &sp->p2defmul);
    if (defmul < 0.0f) {
        Player_WarnAt(p, sc, "%s", "negative p2defmul for SuperPause");
        defmul = g->super_target_defence_mul;
    } else if (defmul == 0.0f) {
        defmul = g->super_target_defence_mul;
    }
    if (defmul != 1.0f) {
        for (e = KeyedArray_First(p->targets); e != NULL; e = KeyedArray_Next(p->targets)) {
            if (e->player->in_guard_dist == 0) {
                e->player->defence_mul = e->player->defence_mul / defmul;
            }
        }
    }
    v = Expr_EvalInt(p, &sp->poweradd, 1);
    leader = ctx->leader;
    Player_SetPower(leader, *leader->power_ref + v);
    if (Expr_EvalInt(p, &sp->unhittable, 0)) {
        int t = g->pauses[1][6] + 1;

        if (p->nohit_time < t) {
            p->nohit_time = t;
        }
    }
    group = Expr_EvalInt(p, &sp->sound[0], 1);
    number = Expr_EvalInt(p, &sp->sound[1], 1);
    if (!sp->sound_fight) {
        Sound_PlayPanned(p->data->snd, group, Ftol(p->draw.x), number, p->data->volumescale);
    } else {
        Sound_PlayPanned(g->fight.common_snd, group, Ftol(p->draw.x), number, 100.0);
    }
    return 0;
}

/* MakeDust: a cloud at pos (ints) every spacing ticks of the round state, and one at pos2 (floats) unless its z is 0. */
static int run_makedust(Player *p, StateCtrl *sc)
{
    int spacing = Expr_EvalInt(p, &sc->arg[2], 1);
    double t;
    int x, y;

    if (spacing < 1) {
        Player_WarnAt(p, sc, "%s", "makedust spacing <= 0");
        spacing = 1;
    }
    if (g_game->round_state_time % spacing != 0) {
        return 0;
    }
    Player_UpdateDrawParams(p, 0);
    t = Player_FromLocalCoord(p, Expr_EvalInt(p, &sc->arg[0], 1)) * p->facing + p->pos.x;
    x = Ftol(Match_Random() % 3 + t - 1.0);
    t = Player_FromLocalCoord(p, Expr_EvalInt(p, &sc->arg[1], 1)) + p->pos.y;
    y = Ftol(Match_Random() % 3 + t - 1.0);
    make_dust(x, y);
    if (Expr_EvalFloat(p, &sc->arg[5]) == 0.0f) {
        return 0;
    }
    t = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[3])) * p->facing + p->pos.x;
    x = Ftol(Match_Random() % 3 + t - 1.0);
    t = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4])) + p->pos.y;
    y = Ftol(Match_Random() % 3 + t - 1.0);
    make_dust(x, y);
    return 0;
}

/* GameMakeAnim: a fightfx animation at pos (from the draw position) with a random offset. */
static int run_gamemakeanim(Player *p, StateCtrl *sc)
{
    GameVars *g = g_game;
    KeyedArray *list;
    AnimInstance *inst;
    double t;
    float scale;
    int anim, x, y, rnd, idx, under;

    Player_UpdateDrawParams(p, 0);
    anim = Expr_EvalInt(p, &sc->arg[0], 1);
    if (anim < 0) {
        anim = 0;
        Player_WarnAt(p, sc, "%s", "made negative gameanim");
    }
    t = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[3])) * p->facing + p->draw.x;
    x = Ftol(Camera_GetRawX() + t);
    t = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4])) + p->draw.y;
    y = Ftol(Camera_GetRawY() + t);
    rnd = Expr_EvalInt(p, &sc->arg[2], 1);
    if (rnd > 0) {
        int half = rnd >> 1;

        x += (Match_Random() % rnd - half) * p->facing;
        y += Match_Random() % rnd - half;
    } else if (rnd < 0) {
        Player_WarnAt(p, sc, "%s", "negative random param for GameMakeAnim");
    }
    idx = AnimSet_Add(x, y, p->facing != 1, g->anims, g->fightfx_anim, g->fightfx_sff, anim);
    under = Expr_EvalInt(p, &sc->arg[1], 0);
    if (idx == -1) {
        return 0;
    }
    list = g->anims->list;
    list->slots[idx].key1 = (under != 0) + 1;
    scale = (float)(g->fightfx_scale * g->motif_scale);
    inst = (AnimInstance *)(list->data + list->elem_size * idx);
    if (inst == NULL) {
        return 0;
    }
    inst->scale_x = scale;
    inst->scale_y = scale;
    return 0;
}

/* RemapPal: source -1 clears every remap (the entry is still added, with source -1,0). */
static int run_remappal(Player *p, StateCtrl *sc)
{
    PalRemapEntry entry;

    entry.src_group = Expr_EvalInt(p, &sc->arg[0], 1);
    entry.src_index = Expr_EvalInt(p, &sc->arg[1], 1);
    entry.dst_group = Expr_EvalInt(p, &sc->arg[3], 1);
    entry.dst_index = Expr_EvalInt(p, &sc->arg[4], 1);
    if (entry.src_group == -1) {
        memset(p->remap, 0, sizeof(PalRemap));
        entry.src_index = 0;
    } else if (entry.dst_group != -1 && !SpriteSet_FindPalette(p->sprites, entry.src_group, entry.src_index)) {
        Player_WarnAt(p, sc, "RemapPal player does not have a source palette %i,%i", entry.src_group, entry.src_index);
    }
    if (entry.dst_group != -1 && !SpriteSet_FindPalette(p->sprites, entry.dst_group, entry.dst_index)) {
        Player_WarnAt(p, sc, "RemapPal player does not have a dest palette %i,%i", entry.dst_group, entry.dst_index);
    }
    if (!PalRemap_Set(p->remap, &entry)) {
        Player_WarnAt(p, sc, "RemapPal player exceeded max remaps on %i,%i -> %i,%i",
                      entry.src_group, entry.src_index, entry.dst_group, entry.dst_index);
    }
    return 0;
}

/* AssertSpecial flag i (0..2): index arg[i] (-1: none), kind arg[3 + i] (0 global, else the player's). */
static int assert_flag(Player *p, StateCtrl *sc, int i)
{
    int idx = Expr_EvalInt(p, &sc->arg[i], 0);

    if (idx == -1) {
        return 0;
    }
    if (Expr_EvalInt(p, &sc->arg[3 + i], 0) == 0) {
        ((unsigned char *)g_game + offsetof(GameVars, assert_intro))[idx] = 1;
    } else {
        ((unsigned char *)p + offsetof(Player, nostandguard))[idx] = 1;
    }
    return 1;
}

/* Zoom (mugenversion 1.1). */
static int run_zoom(Player *p, StateCtrl *sc)
{
    GameVars *g = g_game;
    float x = Expr_EvalFloat(p, &sc->arg[0]);
    float y = Expr_EvalFloat(p, &sc->arg[1]);
    float scale = Expr_EvalFloat(p, &sc->arg[3]);
    float lag;

    scale = StateCtrl_Clamp("scale", p, sc, scale, 1.0f, FLT_MAX);
    lag = Expr_EvalFloat(p, &sc->arg[4]);
    lag = StateCtrl_Clamp("lag", p, sc, lag, 0.0f, 1.0f);
    g->zoom_active = 1.0f;
    g->zoom_scale = scale;
    g->zoom_x = (float)Player_FromLocalCoord(p, x);
    g->zoom_y = (float)Player_FromLocalCoord(p, y);
    g->zoom_lag = lag;
    return 0;
}

/* BindToTarget: to the first target matching the ID, at its foot (0), mid (1) or head (2) position plus pos. */
static int run_bindtotarget(Player *p, StateCtrl *sc)
{
    TargetEntry *e = KeyedArray_First(p->targets);
    int id = Expr_EvalInt(p, &sc->arg[2], 1);
    Player *t;
    int ox, oy;
    float x, y;
    double off;

    for (; e != NULL; e = KeyedArray_Next(p->targets)) {
        if (target_matches(e, id)) {
            break;
        }
    }
    if (e == NULL) {
        return 0;
    }
    t = e->player;
    ox = 0;
    oy = 0;
    if (Expr_EvalInt(p, &sc->arg[1], 0) == 1) {
        double r = g_gameCoord[0] / t->data->localcoord[0];

        ox = Ftol(t->consts.mid_pos[0] * r);
        oy = Ftol(r * t->consts.mid_pos[1]);
    } else if (Expr_EvalInt(p, &sc->arg[1], 0) == 2) {
        double r = g_gameCoord[0] / t->data->localcoord[0];

        ox = Ftol(t->consts.head_pos[0] * r);
        oy = Ftol(r * t->consts.head_pos[1]);
    }
    e->bind_time = Expr_EvalInt(p, &sc->arg[0], 1);
    if (e->bind_time < -1) {
        Player_WarnAt(p, sc, "%s", "set illegal target bindtime");
        e->bind_time = 1;
    }
    x = Expr_EvalFloat(p, &sc->arg[3]);
    if (x != -FLT_MAX) {
        off = ox;
        p->pos.x = (Player_FromLocalCoord(p, x) + off) * t->facing + t->pos.x;
        e->bind_x = Ftol(-((Player_FromLocalCoord(p, x) + off) * t->facing * p->facing));
    }
    y = Expr_EvalFloat(p, &sc->arg[4]);
    if (y == -FLT_MAX) {
        return 0;
    }
    off = oy;
    p->pos.y = Player_FromLocalCoord(p, y) + off + t->pos.y;
    e->bind_y = Ftol(-(Player_FromLocalCoord(p, y) + off));
    return 0;
}

/* The Target* controllers that act on every target matching the ID (TargetBind ... TargetDrop). */
static int run_targets(Player *p, StateCtrl *sc)
{
    TargetEntry *e;
    int kept = 0;

    for (e = KeyedArray_First(p->targets); e != NULL; e = KeyedArray_Next(p->targets)) {
        Player *t = e->player;
        float x, y;
        int v, id;

        switch (sc->type) {
        case SCTRL_TARGETBIND:
            id = Expr_EvalInt(p, &sc->arg[2], 1);
            if (!target_matches(e, id)) {
                break;
            }
            e->bind_time = Expr_EvalInt(p, &sc->arg[0], 1);
            if (e->bind_time < -1) {
                Player_WarnAt(p, sc, "%s", "set illegal target bindtime");
                e->bind_time = 1;
            }
            x = Expr_EvalFloat(p, &sc->arg[3]);
            if (x != -FLT_MAX) {
                e->bind_x = Ftol(Player_FromLocalCoord(p, x));
            }
            y = Expr_EvalFloat(p, &sc->arg[4]);
            if (y != -FLT_MAX) {
                e->bind_y = Ftol(Player_FromLocalCoord(p, y));
            }
            break;
        case SCTRL_TARGETSTATE:
            id = Expr_EvalInt(p, &sc->arg[2], 1);
            if (target_matches(e, id)) {
                CustomStateInfo info;

                Player_GetCustomState(p, &info);
                Player_SetCustomState(t, &info);
                v = Expr_EvalInt(p, &sc->arg[0], 1);
                if (v < 0) {
                    Player_WarnAt(p, sc, "%s", "attempted targetstate to negative");
                    v = 0;
                }
                Player_ChangeState(v, t);
            }
            break;
        case SCTRL_TARGETPOWERADD:
            v = Expr_EvalInt(p, &sc->arg[0], 1);
            id = Expr_EvalInt(p, &sc->arg[2], 1);
            if (target_matches(e, id)) {
                Player_SetPower(t, *t->power_ref + v);
            }
            break;
        case SCTRL_TARGETLIFEADD:
            id = Expr_EvalInt(p, &sc->arg[5], 1);
            if (target_matches(e, id)) {
                int absolute = Expr_EvalInt(p, &sc->arg[2], 0);
                int kill = Expr_EvalInt(p, &sc->arg[1], 0);

                v = Expr_EvalInt(p, &sc->arg[0], 1);
                Player_ApplyDamage(t, -v, kill, absolute);
            }
            break;
        case SCTRL_TARGETVELSET:
        case SCTRL_TARGETVELADD:
            id = Expr_EvalInt(p, &sc->arg[2], 1);
            if (!target_matches(e, id)) {
                break;
            }
            x = Expr_EvalFloat(p, &sc->arg[3]);
            if (x != -FLT_MAX) {
                if (sc->type == SCTRL_TARGETVELSET) {
                    t->vel.x = Player_FromLocalCoord(p, (double)x * t->facing);
                } else {
                    t->vel.x = Player_FromLocalCoord(p, (double)x * t->facing) + t->vel.x;
                }
            }
            y = Expr_EvalFloat(p, &sc->arg[4]);
            if (y != -FLT_MAX) {
                if (sc->type == SCTRL_TARGETVELSET) {
                    t->vel.y = Player_FromLocalCoord(p, y);
                } else {
                    t->vel.y = Player_FromLocalCoord(p, y) + t->vel.y;
                }
            }
            break;
        case SCTRL_TARGETFACING:
            v = Expr_EvalInt(p, &sc->arg[0], 1);
            if (v > 0) {
                if (p->facing != t->facing) {
                    Player_Turn(t);
                }
            } else if (v < 0 && p->facing == t->facing) {
                Player_Turn(t);
            }
            break;
        case SCTRL_TARGETDROP: {
            int exclude = Expr_EvalInt(p, &sc->arg[0], 1);
            int keepone = Expr_EvalInt(p, &sc->arg[1], 1);

            if (exclude == t->gethit_id && kept != 1) {
                if (keepone) {
                    kept = 1;
                }
            } else {
                KeyedArray_Remove(p->targets->iter, p->targets);
            }
            break;
        }
        }
    }
    return 0;
}

/* ChangeState of the partner (an unused tag-in type, 551): state arg[0] for p, arg[1] for the partner. */
static int run_tagin(Player *p, StateCtrl *sc, Player *partner)
{
    int v;

    if (partner == NULL) {
        return 5;
    }
    v = Expr_EvalInt(p, &sc->arg[0], 1);
    if (v >= 0) {
        Player_ChangeState(v, p);
    }
    v = Expr_EvalInt(p, &sc->arg[1], 1);
    if (partner->facing != p->facing) {
        Player_Turn(partner);
    }
    if (v < 0) {
        Player_WarnAt(p, sc, "%s", "attempted change to negative anim");
        return 0;
    }
    Player_ChangeState(v, partner);
    partner->standby = 0;
    return 5;
}

/*
 * 0x00449020: applies a controller to p (its triggers already true); returns
 * STATECTRL_* bits: 5 when the state changed, 2 when the helper destroyed
 * itself, else 0. Unknown types (the parsed "tag", 550) are fatal.
 */
int StateCtrl_Apply(Player *p, StateCtrl *sc, StateRunCtx *ctx)
{
    GameVars *g = g_game;
    Player *enemy = ctx->enemy;
    Player *partner = ctx->partner;
    Player *parent = ctx->parent;
    Player *root = ctx->root;
    HitDef *hd = &p->hitdef;
    int v;

    switch (sc->type) {
    case SCTRL_NULL:
        return 0;
    case SCTRL_CHANGESTATE:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if (v < 0) {
            Player_WarnAt(p, sc, "%s", "attempted change to negative state");
            v = 0;
        }
        Player_ChangeState(v, p);
        if (p->helper == 0 && p->ctrl != 0 &&
            (p->statetype == STATETYPE_STAND || p->statetype == STATETYPE_CROUCH) &&
            g->stage.autoturn != 0 && !Screen_IsSpecial() && Player_IsActive(enemy)) {
            Player_FaceTowards(p, enemy);
        }
        run_ctrl_anim(p, sc);
        return 5;
    case SCTRL_SELFSTATE:
        Player_ClearCustomState(p);
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if (v < 0) {
            Player_WarnAt(p, sc, "%s", "attempted to selfstate to negative state");
            v = 0;
        }
        Player_ChangeState(v, p);
        run_ctrl_anim(p, sc);
        return 5;
    case SCTRL_TURN:
        Player_Turn(p);
        return 0;
    case SCTRL_VARSET:
        return run_var(p, sc, 0);
    case SCTRL_VARADD:
        return run_var(p, sc, 1);
    case SCTRL_VARRANDOM: {
        int idx = Expr_EvalInt(p, &sc->arg[1], 1);
        int lo, hi;

        if ((unsigned int)idx > PLAYER_VAR_COUNT - 1) {
            Player_WarnAt(p, sc, "randomized var out of range (%i)", idx);
            return 0;
        }
        lo = Expr_EvalInt(p, &sc->arg[0], 1);
        hi = Expr_EvalInt(p, &sc->arg[2], 1);
        if (hi < lo) {
            int swap = lo;

            lo = hi;
            hi = swap;
        }
        p->var[idx] = Match_Random() % (hi - lo + 1) + lo;
        return 0;
    }
    case SCTRL_VARRANGESET: {
        int isf = Expr_EvalInt(p, &sc->arg[3], 0);
        int first = Expr_EvalInt(p, &sc->arg[0], 1);
        int last = Expr_EvalInt(p, &sc->arg[1], 1);
        int i;

        if (first < 0 || last > (isf != 0 ? PLAYER_FVAR_COUNT - 1 : PLAYER_VAR_COUNT - 1) || first > last) {
            Player_WarnAt(p, sc, "set illegal var range (%i-%i)", first, last);
            return 0;
        }
        if (isf) {
            float f = Expr_EvalFloat(p, &sc->arg[2]);

            if (f == 0.0f) {
                memset(&p->fvar[first], 0, (last - first) * 4 + 4);
            } else {
                for (i = first; i <= last; i++) {
                    p->fvar[i] = f;
                }
            }
        } else {
            v = Expr_EvalInt(p, &sc->arg[2], 1);
            if (v == 0) {
                memset(&p->var[first], 0, (last - first) * 4 + 4);
            } else {
                for (i = first; i <= last; i++) {
                    p->var[i] = v;
                }
            }
        }
        return 0;
    }
    case SCTRL_LIFESET:
        Player_SetLife(p, Expr_EvalInt(p, &sc->arg[0], 1));
        return 0;
    case SCTRL_LIFEADD: {
        int absolute = Expr_EvalInt(p, &sc->arg[2], 0);
        int kill = Expr_EvalInt(p, &sc->arg[1], 0);

        v = Expr_EvalInt(p, &sc->arg[0], 1);
        Player_ApplyDamage(p, -v, kill, absolute);
        return 0;
    }
    case SCTRL_POWERSET:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        Player_SetPower(ctx->leader, v);
        return 0;
    case SCTRL_POWERADD: {
        Player *leader;

        v = Expr_EvalInt(p, &sc->arg[0], 1);
        leader = ctx->leader;
        Player_SetPower(leader, *leader->power_ref + v);
        return 0;
    }
    case SCTRL_CTRLSET:
        p->ctrl = Expr_EvalInt(p, &sc->arg[0], 1) != 0;
        return 0;
    case SCTRL_POSSET:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            p->pos.x = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[3])) + g->camera_offset_x;
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            p->pos.y = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4]));
        }
        return 0;
    case SCTRL_POSADD:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            float x = (float)((double)Expr_EvalFloat(p, &sc->arg[3]) * p->facing);

            p->pos.x = Player_FromLocalCoord(p, x) + p->pos.x;
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            float y = Expr_EvalFloat(p, &sc->arg[4]);

            p->pos.y = Player_FromLocalCoord(p, y) + p->pos.y;
        }
        return 0;
    case SCTRL_POSFREEZE:
        p->pos_freeze = Expr_EvalInt(p, &sc->arg[0], 1) != 0;
        return 0;
    case SCTRL_VELSET:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            p->vel.x = Player_FromLocalCoord(p, (double)Expr_EvalFloat(p, &sc->arg[3]) * p->facing);
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            p->vel.y = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4]));
        }
        return 0;
    case SCTRL_VELADD:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            float x = (float)((double)Expr_EvalFloat(p, &sc->arg[3]) * p->facing);

            p->vel.x = Player_FromLocalCoord(p, x) + p->vel.x;
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            float y = Expr_EvalFloat(p, &sc->arg[4]);

            p->vel.y = Player_FromLocalCoord(p, y) + p->vel.y;
        }
        return 0;
    case SCTRL_VELMUL:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            p->vel.x = Expr_EvalFloat(p, &sc->arg[3]) * p->vel.x;
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            p->vel.y = Expr_EvalFloat(p, &sc->arg[4]) * p->vel.y;
        }
        return 0;
    case SCTRL_GRAVITY:
        p->vel.y = g_gameCoord[0] / p->data->localcoord[0] * p->consts.yaccel + p->vel.y;
        return 0;
    case SCTRL_OFFSET:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            p->draw_offset.x = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[3]));
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            p->draw_offset.y = Player_FromLocalCoord(p, Expr_EvalFloat(p, &sc->arg[4]));
        }
        return 0;
    case SCTRL_HITBY:
        if (Expr_EvalInt(p, &sc->arg[2], 0) == 0) {
            p->hitby_attr = Expr_EvalInt(p, &sc->arg[0], 0);
            p->hitby_time = Expr_EvalInt(p, &sc->arg[1], 0);
        } else {
            p->nothitby_attr = Expr_EvalInt(p, &sc->arg[0], 0);
            p->nothitby_time = Expr_EvalInt(p, &sc->arg[1], 0);
        }
        return 0;
    case SCTRL_CHANGEANIM:
        run_change_anim(p, sc, 0);
        return 0;
    case SCTRL_CHANGEANIM2:
        run_change_anim(p, sc, 1);
        return 0;
    case SCTRL_PLAYSND:
        PlaySnd_Run(sc->params, p, sc);
        return 0;
    case SCTRL_STOPSND:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if ((unsigned int)(v + 1) > 0x20) {
            Player_WarnAt(p, sc, "stopped illegal channel number %i", v);
            return 0;
        }
        Sound_Stop(Sound_PlayerChannel(v, p));
        return 0;
    case SCTRL_SNDPAN: {
        int slot, x;

        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if ((unsigned int)v > 0x1F) {
            Player_WarnAt(p, sc, "attemted to pan illegal channel number %i", v);
            return 0;
        }
        v = Sound_PlayerChannel(v, p);
        if (v == -1) {
            return 0;
        }
        slot = Sound_FindSlot(v);
        if (slot == -1) {
            return 0;
        }
        x = Ftol(Player_FromLocalCoord(p, Expr_EvalInt(p, &sc->arg[1], 1)) * p->facing);
        if (!Expr_EvalInt(p, &sc->arg[2], 1)) {
            x = Ftol(p->pos.x - g->camera_offset_x + x);
        }
        Sound_SetSlotPan(slot, Sound_Pan(x));
        return 0;
    }
    case SCTRL_HITDEF:
    case SCTRL_REVERSALDEF: {
        int dist = hd->guard_dist;
        int juggle = hd->juggle;

        HitDef_SetFromParams(p, sc->params, hd, sc);
        hd->active = 1;
        if (sc->type == SCTRL_REVERSALDEF) {
            hd->hitonce_used = 0;
        }
        hd->juggle = juggle;
        hd->damage = Player_ScaleDamage(p, hd->damage);
        hd->guard_damage = Player_ScaleDamage(p, hd->guard_damage);
        if (hd->guard_dist == -1) {
            hd->guard_dist = dist;
        }
        hd->serial = g->hitdef_serial++;
        return 0;
    }
    case SCTRL_PROJECTILE: {
        Player *owner = root != NULL ? root : p;
        ProjectileList *pl;
        Projectile *proj;

        v = Projectile_Fire(sc->params, p, ctx, owner, sc);
        if (v == -1) {
            return 0;
        }
        pl = owner->projectiles;
        proj = (Projectile *)(pl->list->data + pl->list->elem_size * v);
        if (proj == NULL) {
            return 0;
        }
        p->last_proj_id = proj->id;
        proj->hitdef.damage = Player_ScaleDamage(p, proj->hitdef.damage);
        proj->hitdef.guard_damage = Player_ScaleDamage(p, proj->hitdef.guard_damage);
        return 0;
    }
    case SCTRL_WIDTH:
        p->width_edge_front = Expr_EvalInt(p, &sc->arg[0], 1);
        p->width_edge_back = Expr_EvalInt(p, &sc->arg[1], 1);
        p->width_player_front = Expr_EvalInt(p, &sc->arg[2], 1);
        p->width_player_back = Expr_EvalInt(p, &sc->arg[3], 1);
        return 0;
    case SCTRL_SCREENBOUND:
        p->screenbound = Expr_EvalInt(p, &sc->arg[0], 1) != 0;
        p->movecamera_x = Expr_EvalInt(p, &sc->arg[1], 1) != 0;
        p->movecamera_y = Expr_EvalInt(p, &sc->arg[2], 1) != 0;
        return 0;
    case SCTRL_SPRPRIORITY:
        p->sprpriority = Expr_EvalInt(p, &sc->arg[0], 1);
        return 0;
    case SCTRL_ANGLEDRAW: {
        float a;

        p->draw2.flags |= DRAW_SCALE | DRAW_ANGLE;
        a = Expr_EvalFloat(p, &sc->arg[3]);
        if (a != -FLT_MAX) {
            p->draw2.angle = a;
        }
        p->draw2.scale_x = Expr_EvalFloat(p, &sc->arg[4]) * p->draw2.scale_x;
        p->draw2.scale_y = Expr_EvalFloat(p, &sc->arg[5]) * p->draw2.scale_y;
        return 0;
    }
    case SCTRL_ANGLESET:
        p->draw2.angle = Expr_EvalFloat(p, &sc->arg[3]);
        return 0;
    case SCTRL_ANGLEADD:
        p->draw2.angle = Expr_EvalFloat(p, &sc->arg[3]) + p->draw2.angle;
        return 0;
    case SCTRL_ANGLEMUL:
        p->draw2.angle = Expr_EvalFloat(p, &sc->arg[3]) * p->draw2.angle;
        return 0;
    case SCTRL_TRANS: {
        int mode = Expr_EvalInt(p, &sc->arg[0], 1);
        int src = Expr_EvalInt(p, &sc->arg[1], 1);
        int dst = Expr_EvalInt(p, &sc->arg[2], 1);

        if (src < 0) {
            src = 0;
        } else if (src > 0x100) {
            src = 0x100;
        }
        if (dst < 0) {
            dst = 0;
        } else if (dst > 0x100) {
            dst = 0x100;
        }
        DrawParams_SetBlend(mode, &p->draw2, src, dst);
        return 0;
    }
    case SCTRL_BINDTOTARGET:
        return run_bindtotarget(p, sc);
    case SCTRL_TARGETBIND:
    case SCTRL_TARGETSTATE:
    case SCTRL_TARGETPOWERADD:
    case SCTRL_TARGETLIFEADD:
    case SCTRL_TARGETVELSET:
    case SCTRL_TARGETVELADD:
    case SCTRL_TARGETFACING:
    case SCTRL_TARGETDROP:
        return run_targets(p, sc);
    case SCTRL_ATTACKDIST:
        hd->guard_dist = Expr_EvalInt(p, &sc->arg[0], 1);
        return 0;
    case SCTRL_PLAYERPUSH:
        p->playerpush = Expr_EvalInt(p, &sc->arg[0], 1) != 0;
        return 0;
    case SCTRL_HITFALLSET: {
        float f;

        v = Expr_EvalInt(p, &sc->arg[0], 0);
        if (v != -1) {
            p->gethit_fall = v;
        }
        f = Expr_EvalFloat(p, &sc->arg[3]);
        if (f != -FLT_MAX) {
            p->gethit_fall_xvel = f;
        }
        /* the original reads xvel again for yvel */
        f = Expr_EvalFloat(p, &sc->arg[3]);
        if (f != -FLT_MAX) {
            p->gethit_fall_yvel = f;
        }
        return 0;
    }
    case SCTRL_HITVELSET:
        if (Expr_EvalInt(p, &sc->arg[0], 0)) {
            p->vel.x = p->gethit_xveladd + p->gethit_vel_x;
        }
        if (Expr_EvalInt(p, &sc->arg[1], 0)) {
            p->vel.y = p->gethit_yveladd + p->gethit_vel_y;
        }
        return 0;
    case SCTRL_HITFALLDAMAGE:
        Player_SetLife(p, p->life - p->gethit_fall_damage);
        p->gethit_fall_damage = 0;
        return 0;
    case SCTRL_HITFALLVEL:
        if (p->gethit_fall_xvel != -FLT_MAX) {
            p->vel.x = p->facing * (double)p->gethit_fall_xvel;
        }
        if (p->gethit_fall_yvel != -FLT_MAX) {
            p->vel.y = p->gethit_fall_yvel;
        }
        return 0;
    case SCTRL_HITADD:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        p->combo_hits += v;
        p->hitcount += v;
        p->uniqhitcount += v;
        Combo_AddHits(p->team_side, v);
        return 0;
    case SCTRL_ATTACKMULSET:
        p->attack_mul = (float)(Expr_EvalFloat(p, &sc->arg[3]) * p->consts.attack / 100.0);
        return 0;
    case SCTRL_DEFENCEMULSET:
        p->defence_mul = (float)(Expr_EvalFloat(p, &sc->arg[3]) * 100.0 / p->consts.defence);
        return 0;
    case SCTRL_STATETYPESET:
        v = Expr_EvalInt(p, &sc->arg[0], 0);
        if (v != -1) {
            p->statetype = v;
        }
        v = Expr_EvalInt(p, &sc->arg[1], 0);
        if (v != -1) {
            p->movetype = v;
        }
        v = Expr_EvalInt(p, &sc->arg[2], 0);
        if (v != -1) {
            p->physics = v;
        }
        return 0;
    case SCTRL_HITOVERRIDE:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if (v < 0 || v > 7) {
            Player_WarnAt(p, sc, "%s", "illegal override slot: 0 assumed");
            v = 0;
        }
        HitOverride_Apply(&p->hit_override[v], p, sc->params, sc);
        return 0;
    case SCTRL_MOVEHITRESET:
        p->move_contact = 0;
        return 0;
    case SCTRL_VICTORYQUOTE: {

        if (p->helper != 0) {
            return 0;
        }
        v = Expr_EvalInt(p, &sc->arg[0], 0);
        g->teams[p->team_index].member_data[p->team_member][p->team_slot].victory_quote = v;
        return 0;
    }
    case SCTRL_ASSERTSPECIAL:
        assert_flag(p, sc, 0);
        assert_flag(p, sc, 1);
        assert_flag(p, sc, 2);
        return 0;
    case SCTRL_ZOOM:
        return run_zoom(p, sc);
    case SCTRL_ENVSHAKE:
        EnvShake_Apply((EnvShake *)g->envshake, sc->params, p, sc);
        return 0;
    case SCTRL_FALLENVSHAKE:
        if (p->gethit_fall_envshake_time > 0) {
            memcpy(g->envshake, &p->gethit_fall_envshake_unused, sizeof(g->envshake));
            p->gethit_fall_envshake_time = 0;
        }
        return 0;
    case SCTRL_ENVCOLOR: {
        int r = Expr_EvalInt(p, &sc->arg[3], 1);
        int gr = Expr_EvalInt(p, &sc->arg[4], 1);
        int b = Expr_EvalInt(p, &sc->arg[5], 1);

        g->envcolor_color = Video_MapRGB(r, gr, b);
        v = Expr_EvalInt(p, &sc->arg[1], 1);
        g->envcolor_time = v;
        if (v != -1) {
            g->envcolor_time = v + 1;
        }
        if (g->envcolor_time < -1) {
            Player_WarnAt(p, sc, "%s", "set illegal envcolortime");
            g->envcolor_time = 1;
        }
        g->envcolor_under = Expr_EvalInt(p, &sc->arg[2], 1) != 0;
        return 0;
    }
    case SCTRL_GAMEMAKEANIM:
        return run_gamemakeanim(p, sc);
    case SCTRL_AFTERIMAGE: {
        AfterImage *ai;

        if (p->afterimage == NULL) {
            p->afterimage = AfterImages_FindFree(g->afterimages);
        }
        ai = p->afterimage;
        if (ai == NULL) {
            return 0;
        }
        AfterImage_Apply(p, sc->params, ai, p->anim, sc);
        return 0;
    }
    case SCTRL_AFTERIMAGETIME: {
        AfterImage *ai = p->afterimage;

        if (ai == NULL) {
            return 0;
        }
        ai->ticks = 0;
        ai->settings.time = Expr_EvalInt(p, &sc->arg[0], 1);
        if (ai->settings.time < -1) {
            Player_WarnAt(p, sc, "%s", "set illegal afterimagetime");
            ai->settings.time = 1;
        }
        return 0;
    }
    case SCTRL_PAUSE:
        Pause_Apply(0, p, sc->params, sc);
        return 0;
    case SCTRL_SUPERPAUSE:
        return run_superpause(p, sc, ctx);
    case SCTRL_MAKEDUST:
        return run_makedust(p, sc);
    case SCTRL_PALFX:
        PalFx_Apply(&p->palfx_def, sc->params, p, sc);
        return 0;
    case SCTRL_REMAPPAL:
        return run_remappal(p, sc);
    case SCTRL_EXPLOD:
        Explod_Create(p, sc->params, ctx, sc);
        return 0;
    case SCTRL_MODIFYEXPLOD:
        Explod_Modify(p, sc->params, ctx, sc);
        return 0;
    case SCTRL_REMOVEEXPLOD:
        v = Expr_EvalInt(p, &sc->arg[0], 1);
        if (v < -1) {
            Player_WarnAt(p, sc, "%s", "removal of illegal explod ID");
            v = -1;
        }
        Explods_RemoveOwner(g->explods, p->id, v);
        return 0;
    case SCTRL_EXPLODBINDTIME: {
        int time;

        v = Expr_EvalInt(p, &sc->arg[0], 1);
        time = Expr_EvalInt(p, &sc->arg[1], 1);
        if (v < -1) {
            Player_WarnAt(p, sc, "%s", "illegal ID to explodbindtime");
            v = -1;
        }
        if (time < -1) {
            Player_WarnAt(p, sc, "%s", "illegal explodbindtime");
            time = -1;
        }
        Explods_SetBindTime(g->explods, p->id, v, time);
        return 0;
    }
    case SCTRL_HELPER:
        Helper_Create(p, sc->params, ctx, sc);
        return 0;
    case SCTRL_DESTROYSELF: {
        int recursive = Expr_EvalInt(p, &sc->arg[0], 0);
        int remove_explods = Expr_EvalInt(p, &sc->arg[1], 0);

        if (p->helper == 1) {
            Helper_Remove(p, recursive, remove_explods);
            return 2;
        }
        Player_WarnAt(p, sc, "%s", "destroyself called for non-helper");
        return 0;
    }
    case SCTRL_PARENTVARSET:
        return run_parent_var(p, sc, parent, 0);
    case SCTRL_PARENTVARADD:
        return run_parent_var(p, sc, parent, 1);
    case SCTRL_DISPLAYTOCLIPBOARD:
    case SCTRL_APPENDTOCLIPBOARD:
        run_clipboard(p, sc);
        return 0;
    case SCTRL_CLEARCLIPBOARD: {
        TextRing *ring = p->warnings;

        ring->cur = 0;
        ring->head = 0;
        ring->lines[0][0] = '\0';
        return 0;
    }
    case SCTRL_BINDTOPARENT:
        return run_bind(p, sc, parent);
    case SCTRL_BINDTOROOT:
        return run_bind(p, sc, root);
    case SCTRL_TAGIN:
        return run_tagin(p, sc, partner);
    case SCTRL_TAGOUT:
        p->standby = 1;
        return 5;
    case SCTRL_NOP:
        return 0;
    default:
        Error_Printf("PlayerSCtrlApplyElem not recognised\n");
        Error_Exit();
    }
}

/*
 * 0x0044C570: value clamped to min..max with a warning naming the parameter
 * (as "zoom.<name>"; no warning without a name).
 */
float StateCtrl_Clamp(const char *name, Player *p, StateCtrl *sc, float value, float min, float max)
{
    if (min > value) {
        if (name != NULL) {
            Player_WarnAt(p, sc, "clamping value %s.%s %f to minimum allowed %f", "zoom", name, (double)value, (double)min);
        }
        value = min;
    }
    if (max < value) {
        if (name != NULL) {
            Player_WarnAt(p, sc, "clamping value %s.%s %f to maximum allowed %f", "zoom", name, (double)value, (double)max);
        }
        value = max;
    }
    return value;
}

/* ------------------------------------------------------------ parameter blocks */

/* 0x0044C610: the value of prefix + key (at most 0x50 characters). */
char *Ini_GetValuePrefixed(const char *key, const char *prefix, Ini *ini)
{
    char buf[0x50];

    sprintf(buf, "%s%s", prefix, key);
    return Ini_GetValue(ini, buf);
}

/*
 * 0x0044C640: the AfterImage parameters (prefix "" for the controller,
 * "afterimage." for a projectile). The alpha is read only with a valid
 * trans; an invalid trans leaves -2 and is not an error.
 */
int AfterImage_ParseParams(AfterImageParams *ai, Ini *ini, CharData *owner, const char *prefix)
{
    char key[0x40];
    const char *s;
    const char *end;
    int alpha_src, alpha_dst;

    Expr_SetInt(&ai->alpha[0], 0x100);
    Expr_SetInt(&ai->alpha[1], 0x100);
    Expr_SetInt(&ai->length, 20);
    Expr_SetInt(&ai->timegap, 1);
    Expr_SetInt(&ai->framegap, 4);
    Expr_SetInt(&ai->time, 1);
    Expr_SetInt(&ai->palbright[0], 30);
    Expr_SetInt(&ai->palbright[1], 30);
    Expr_SetInt(&ai->palbright[2], 30);
    Expr_SetInt(&ai->palcontrast[0], 120);
    Expr_SetInt(&ai->palcontrast[1], 120);
    Expr_SetInt(&ai->palcontrast[2], 220);
    Expr_SetInt(&ai->palpostbright[0], 0);
    Expr_SetInt(&ai->palpostbright[1], 0);
    Expr_SetInt(&ai->palpostbright[2], 0);
    Expr_SetInt(&ai->paladd[0], 10);
    Expr_SetInt(&ai->paladd[1], 10);
    Expr_SetInt(&ai->paladd[2], 25);
    Expr_SetFloat(&ai->palmul[0], 0.65f);
    Expr_SetFloat(&ai->palmul[1], 0.65f);
    Expr_SetFloat(&ai->palmul[2], 0.75f);
    ai->trans = 0;

    sprintf(key, "%s%s", prefix, "time");
    if (!Expr_Compile(Ini_GetValue(ini, key), &ai->time, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%s%s", prefix, "length");
    if (!Expr_Compile(Ini_GetValue(ini, key), &ai->length, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%s%s", prefix, "palcolor");
    s = Ini_GetValue(ini, key);
    Expr_SetInt(&ai->palcolor, 0x100);
    if (s != NULL && !Expr_Compile(s, &ai->palcolor, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%s%s", prefix, "palinvertall");
    s = Ini_GetValue(ini, key);
    Expr_SetInt(&ai->palinvertall, 0);
    if (s != NULL && !Expr_Compile(s, &ai->palinvertall, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValuePrefixed("palbright", prefix, ini);
    if (s != NULL) {
        /* original bug: the first two values both go to palbright[1] */
        if (!Cns_ParseValues(s, "iii", owner, &end, &ai->palbright[1], &ai->palbright[1], &ai->palbright[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValuePrefixed("palcontrast", prefix, ini);
    if (s != NULL) {
        if (!Cns_ParseValues(s, "iii", owner, &end, &ai->palcontrast[0], &ai->palcontrast[1], &ai->palcontrast[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValuePrefixed("palpostbright", prefix, ini);
    if (s != NULL) {
        if (!Cns_ParseValues(s, "iii", owner, &end, &ai->palpostbright[0], &ai->palpostbright[1],
                             &ai->palpostbright[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValuePrefixed("paladd", prefix, ini);
    if (s != NULL) {
        if (!Cns_ParseValues(s, "iii", owner, &end, &ai->paladd[0], &ai->paladd[1], &ai->paladd[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValuePrefixed("palmul", prefix, ini);
    if (s != NULL) {
        if (!Cns_ParseValues(s, "fff", owner, &end, &ai->palmul[0], &ai->palmul[1], &ai->palmul[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    if (!Expr_Compile(Ini_GetValuePrefixed("timegap", prefix, ini), &ai->timegap, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Expr_Compile(Ini_GetValuePrefixed("framegap", prefix, ini), &ai->framegap, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    ai->trans = 0;
    s = Ini_GetValuePrefixed("trans", prefix, ini);
    if (s == NULL) {
        return 1;
    }
    ai->trans = Trans_Parse(s, &alpha_dst, &alpha_src);
    if (ai->trans == -2) {
        return 1;
    }
    Expr_SetInt(&ai->alpha[0], alpha_src);
    Expr_SetInt(&ai->alpha[1], alpha_dst);
    s = Ini_GetValuePrefixed("alpha", prefix, ini);
    if (s == NULL) {
        return 1;
    }
    if (!Cns_ParseValues(s, "ii", owner, &end, &ai->alpha[0], &ai->alpha[1])) {
        return 0;
    }
    return *end == '\0';
}

/*
 * 0x0044CDF0: starts afterimage buffer ai following anim with the evaluated
 * parameters (time below -1: 1; length 1..ai->max_length, 20 when not
 * positive; timegap at least 1; framegap 4 when below 1).
 */
int AfterImage_Apply(Player *p, AfterImageParams *params, AfterImage *ai, AnimState *anim, StateCtrl *sc)
{
    AfterImageSettings s;
    int i;

    memset(&s, 0, sizeof(s));
    s.time = Expr_EvalInt(p, &params->time, 1);
    if (s.time < -1) {
        s.time = 1;
        Player_WarnAt(p, sc, "%s", "illegal time for afterimage");
    }
    s.length = Expr_EvalInt(p, &params->length, 1);
    if (s.length <= 0) {
        s.length = 20;
        Player_WarnAt(p, sc, "%s", "illegal length for afterimage");
    } else if (s.length > ai->max_length) {
        s.length = ai->max_length;
        Player_WarnAt(p, sc, "length for afterimage exceeds maximum of %i", ai->max_length);
    }
    s.timegap = Expr_EvalInt(p, &params->timegap, 1);
    if (s.timegap < 1) {
        s.timegap = 1;
        Player_WarnAt(p, sc, "%s", "illegal timegap for afterimage");
    }
    s.framegap = Expr_EvalInt(p, &params->framegap, 1);
    if (s.framegap < 1) {
        s.framegap = 4;
        Player_WarnAt(p, sc, "%s", "illegal framegap for afterimage");
    }
    s.palcolor = Expr_EvalInt(p, &params->palcolor, 1);
    s.palinvertall = Expr_EvalInt(p, &params->palinvertall, 1);
    for (i = 0; i < 3; i++) {
        s.palbright[i] = Expr_EvalInt(p, &params->palbright[i], 1);
    }
    for (i = 0; i < 3; i++) {
        s.palcontrast[i] = Expr_EvalInt(p, &params->palcontrast[i], 1);
    }
    for (i = 0; i < 3; i++) {
        s.palpostbright[i] = Expr_EvalInt(p, &params->palpostbright[i], 1);
    }
    for (i = 0; i < 3; i++) {
        s.paladd[i] = Expr_EvalInt(p, &params->paladd[i], 1);
    }
    for (i = 0; i < 3; i++) {
        s.palmul[i] = Expr_EvalFloat(p, &params->palmul[i]);
    }
    s.trans = params->trans;
    s.alpha[0] = Expr_EvalInt(p, &params->alpha[0], 1);
    s.alpha[1] = Expr_EvalInt(p, &params->alpha[1], 1);
    ai->settings = s;
    ai->next = 0;
    ai->oldest = 0;
    ai->ticks = 0;
    ai->running = 1;
    ai->anim = anim;
    return 1;
}

/* 0x0044D0C0 */
void AfterImageParams_Free(AfterImageParams *ai)
{
    int i;

    Expr_Free(&ai->time);
    Expr_Free(&ai->length);
    Expr_Free(&ai->timegap);
    Expr_Free(&ai->framegap);
    Expr_Free(&ai->palcolor);
    Expr_Free(&ai->palinvertall);
    for (i = 0; i < 3; i++) {
        Expr_Free(&ai->palbright[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&ai->palcontrast[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&ai->palpostbright[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&ai->paladd[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&ai->palmul[i]);
    }
    Expr_Free(&ai->alpha[0]);
    Expr_Free(&ai->alpha[1]);
}

/*
 * 0x0044D1C0: the EnvShake parameters (prefix "" for the controller,
 * "envshake." / "fall.envshake." for a HitDef). Always succeeds unless an
 * expression is bad.
 */
int EnvShake_ParseParams(const char *prefix, EnvShakeParams *es, Ini *ini, CharData *owner)
{
    char key[0x40];
    const char *s;

    sprintf(key, "%stime", prefix);
    s = Ini_GetValue(ini, key);
    Expr_SetInt(&es->time, 0);
    if (s != NULL && !Expr_Compile(s, &es->time, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%sdir", prefix);
    s = Ini_GetValue(ini, key);
    Expr_SetInt(&es->dir, 0);
    if (s != NULL && !Expr_Compile(s, &es->dir, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%sampl", prefix);
    s = Ini_GetValue(ini, key);
    Expr_SetInt(&es->ampl, -4);
    if (s != NULL && !Expr_Compile(s, &es->ampl, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    sprintf(key, "%sfreq", prefix);
    s = Ini_GetValue(ini, key);
    Expr_SetFloat(&es->freq, 60.0f);
    if (s != NULL && !Expr_Compile(s, &es->freq, owner, NULL, EXPR_WANT_FLOAT)) {
        return 0;
    }
    sprintf(key, "%sphase", prefix);
    s = Ini_GetValue(ini, key);
    Expr_SetFloat(&es->phase, 0.0f);
    if (s != NULL && !Expr_Compile(s, &es->phase, owner, NULL, EXPR_WANT_FLOAT)) {
        return 0;
    }
    es->no_phase = Ini_GetValue(ini, key) == NULL;
    return 1;
}

/*
 * 0x0044D420: evaluates an EnvShake into out. The amplitude is scaled to game
 * units by the states' owner; a freq of 0 becomes 0.001, a negative one is
 * made positive (with a warning); without a phase, freq 90 or more gives
 * phase 90. dir is never evaluated (0).
 */
int EnvShake_Apply(EnvShake *out, EnvShakeParams *es, Player *p, StateCtrl *sc)
{
    float ampl, freq;

    memset(out, 0, sizeof(*out));
    out->time = Expr_EvalInt(p, &es->time, 1);
    ampl = Expr_EvalFloat(p, &es->ampl);
    out->ampl = (float)(ampl * states_scale(p));
    out->phase = Expr_EvalFloat(p, &es->phase);
    freq = Expr_EvalFloat(p, &es->freq);
    if (freq == 0.0f) {
        freq = 0.001f;
    } else if (freq < 0.0f) {
        freq = (float)(freq * -1.0);
        Player_WarnAt(p, sc, "%s", "negative freq for EnvShake");
    }
    out->freq = freq;
    out->unused_00 = 0;
    if (es->no_phase && freq >= 90.0f) {
        out->phase = 90.0f;
    }
    return 1;
}

/* 0x0044D540 */
void EnvShakeParams_Free(EnvShakeParams *es)
{
    Expr_Free(&es->time);
    Expr_Free(&es->dir);
    Expr_Free(&es->freq);
    Expr_Free(&es->ampl);
    Expr_Free(&es->phase);
}

/*
 * 0x0044D580: the Explod (modify 0) or ModifyExplod (modify 1) parameters.
 * ModifyExplod leaves INT_MIN / -FLT_MAX in the values that are not given.
 */
int Explod_ParseParams(ExplodParams *ep, Ini *ini, CharData *owner, int modify)
{
    const char *s;
    const char *end;
    int alpha_dst, alpha_src;
    int space_def, unset_def, removetime_def, bindtime_def;
    float fdef;

    if (modify) {
        ep->postype = INT_MIN;
    } else {
        ep->postype = owner->mugenversion < g_game->compat_version[2] ? POSTYPE_P1 : POSTYPE_NONE;
    }
    Expr_SetFloat(&ep->scale[0], modify ? -FLT_MAX : 1.0f);
    Expr_SetFloat(&ep->scale[1], modify ? -FLT_MAX : 1.0f);
    s = Ini_GetValue(ini, "id");
    Expr_SetInt(&ep->id, modify ? -1 : 0);
    if (s != NULL && !Expr_Compile(s, &ep->id, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "anim");
    if (s != NULL) {
        if (s[0] == 'f' || s[0] == 'F') {
            if (!Expr_Compile(s + 1, &ep->anim, owner, NULL, EXPR_WANT_INT)) {
                return 0;
            }
            ep->anim_fight = 1;
        } else if (!Expr_Compile(s, &ep->anim, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
    }
    space_def = modify ? INT_MIN : -1;
    ep->space = space_def;
    s = Ini_GetValue(ini, "space");
    if (s != NULL) {
        if (_stricmp(s, "screen") == 0) {
            ep->space = 1;
        } else if (_stricmp(s, "stage") == 0) {
            ep->space = 0;
        } else if (_stricmp(s, "auto") == 0) {
            ep->space = -1;
        }
    }
    if (!parse_pair_checked(ini, "pos", "ff", &ep->pos[0], &ep->pos[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "vel");
    if (s == NULL) {
        s = Ini_GetValue(ini, "velocity");
    }
    if (s != NULL) {
        if (!Cns_ParseValues(s, "ff", owner, &end, &ep->vel[0], &ep->vel[1])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    if (!parse_pair_checked(ini, "accel", "ff", &ep->accel[0], &ep->accel[1], owner)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "random", "ii", &ep->random[0], &ep->random[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "supermove");
    unset_def = modify ? INT_MIN : 0;
    Expr_SetInt(&ep->supermove, unset_def);
    if (s != NULL && !Expr_Compile(s, &ep->supermove, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "supermovetime", &ep->supermovetime, owner, unset_def)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "pausemovetime", &ep->pausemovetime, owner, unset_def)) {
        return 0;
    }
    removetime_def = modify ? INT_MIN : -2;
    if (!Cns_CompileInt(ini, "removetime", &ep->removetime, owner, removetime_def)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "scale", "ff", &ep->scale[0], &ep->scale[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "postype");
    if (s != NULL) {
        ep->postype = Cns_ParsePosType(s);
        if (ep->postype == -2) {
            Error_Printf("Illegal postype: %s\n", s);
            return 0;
        }
    }
    fdef = modify ? -FLT_MAX : 0.0f;
    if (!Cns_CompileFloat(ini, "angle", &ep->angle, owner, fdef)) {
        return 0;
    }
    fdef = modify ? -FLT_MAX : 0.0f;
    if (!Cns_CompileFloat(ini, "yangle", &ep->yangle, owner, fdef)) {
        return 0;
    }
    fdef = modify ? -FLT_MAX : 0.0f;
    if (!Cns_CompileFloat(ini, "xangle", &ep->xangle, owner, fdef)) {
        return 0;
    }
    /* the default of bindid is the one of removetime (-2) */
    if (!Cns_CompileInt(ini, "bindid", &ep->bindid, owner, removetime_def)) {
        return 0;
    }
    bindtime_def = modify ? INT_MIN : 1;
    if (!Cns_CompileInt(ini, "bindtime", &ep->bindtime, owner, bindtime_def)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "sprpriority", &ep->sprpriority, owner, unset_def)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "ontop", &ep->ontop, owner, unset_def)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "facing", &ep->facing, owner, 1)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "vfacing", &ep->vfacing, owner, 1)) {
        return 0;
    }
    if (modify) {
        Expr_SetInt(&ep->shadow[0], INT_MIN);
    }
    s = Ini_GetValue(ini, "shadow");
    if (s != NULL) {
        if (!Cns_ParseValues(s, "iii", owner, &end, &ep->shadow[0], &ep->shadow[1], &ep->shadow[2])) {
            return 0;
        }
        if (*end != '\0') {
            return 0;
        }
    }
    if (!Cns_CompileInt(ini, "ownpal", &ep->ownpal, owner, modify ? INT_MIN : ep->anim_fight != 0)) {
        return 0;
    }
    Expr_SetInt(&ep->remappal[0], space_def);
    if (!parse_pair_checked(ini, "remappal", "ii", &ep->remappal[0], &ep->remappal[1], owner)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "removeongethit", &ep->removeongethit, owner, unset_def)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "ignorehitpause", &ep->ignorehitpause, owner, bindtime_def)) {
        return 0;
    }
    ep->trans = space_def;
    s = Ini_GetValue(ini, "trans");
    if (s == NULL) {
        return 1;
    }
    ep->trans = Trans_Parse(s, &alpha_dst, &alpha_src);
    if (ep->trans == -2) {
        return 1;
    }
    Expr_SetInt(&ep->alpha[0], alpha_src);
    Expr_SetInt(&ep->alpha[1], alpha_dst);
    s = Ini_GetValue(ini, "alpha");
    if (s == NULL) {
        return 1;
    }
    if (!Cns_ParseValues(s, "ii", owner, &end, &ep->alpha[0], &ep->alpha[1])) {
        return 0;
    }
    return *end == '\0';
}

/* 0x0044DD70 */
void ExplodParams_Free(ExplodParams *ep)
{
    Expr_Free(&ep->id);
    Expr_Free(&ep->anim);
    Expr_Free(&ep->pos[0]);
    Expr_Free(&ep->pos[1]);
    Expr_Free(&ep->vel[0]);
    Expr_Free(&ep->vel[1]);
    Expr_Free(&ep->accel[0]);
    Expr_Free(&ep->accel[1]);
    Expr_Free(&ep->random[0]);
    Expr_Free(&ep->random[1]);
    Expr_Free(&ep->supermove);
    Expr_Free(&ep->supermovetime);
    Expr_Free(&ep->pausemovetime);
    Expr_Free(&ep->removetime);
    Expr_Free(&ep->scale[0]);
    Expr_Free(&ep->scale[1]);
    Expr_Free(&ep->angle);
    Expr_Free(&ep->yangle);
    Expr_Free(&ep->xangle);
    Expr_Free(&ep->bindid);
    Expr_Free(&ep->bindtime);
    Expr_Free(&ep->sprpriority);
    Expr_Free(&ep->ontop);
    Expr_Free(&ep->facing);
    Expr_Free(&ep->vfacing);
    Expr_Free(&ep->shadow[0]);
    Expr_Free(&ep->shadow[1]);
    Expr_Free(&ep->shadow[2]);
    Expr_Free(&ep->ownpal);
    Expr_Free(&ep->remappal[0]);
    Expr_Free(&ep->remappal[1]);
    Expr_Free(&ep->removeongethit);
    Expr_Free(&ep->ignorehitpause);
    Expr_Free(&ep->alpha[0]);
    Expr_Free(&ep->alpha[1]);
}

/* 0x0044DF00: PalFX parameters; hitdef selects the "palfx." names of a HitDef. kind is set by the caller. */
int PalFx_ParseParams(PalFxParams *pf, Ini *ini, CharData *owner, int hitdef)
{
    const char *s;
    const char *end;
    int n;

    Expr_SetInt(&pf->mul[0], 0x100);
    Expr_SetInt(&pf->mul[1], 0x100);
    Expr_SetInt(&pf->mul[2], 0x100);
    if (!Expr_Compile(Ini_GetValue(ini, hitdef ? "palfx.time" : "time"), &pf->time, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, hitdef ? "palfx.mul" : "mul");
    if (s != NULL) {
        if (Cns_ParseValues(s, "iii", owner, &end, &pf->mul[0], &pf->mul[1], &pf->mul[2]) != 3 || *end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValue(ini, hitdef ? "palfx.add" : "add");
    if (s != NULL) {
        if (Cns_ParseValues(s, "iii", owner, &end, &pf->add[0], &pf->add[1], &pf->add[2]) != 3 || *end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValue(ini, hitdef ? "palfx.sinadd" : "sinadd");
    if (s != NULL) {
        n = Cns_ParseValues(s, "iiii", owner, &end, &pf->sinadd[0], &pf->sinadd[1], &pf->sinadd[2], &pf->period);
        if ((unsigned int)(n - 3) > 1 || *end != '\0') {
            return 0;
        }
    }
    s = Ini_GetValue(ini, hitdef ? "palfx.invertall" : "invertall");
    Expr_SetInt(&pf->invertall, 0);
    if (s != NULL && !Expr_Compile(s, &pf->invertall, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, hitdef ? "palfx.color" : "color");
    Expr_SetInt(&pf->color, 0x100);
    if (s != NULL && !Expr_Compile(s, &pf->color, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    return 1;
}

/*
 * 0x0044E1A0: evaluates a PalFX into dst (the player's PalFX definition), or
 * into the BGPalFX / AllPalFX definition of GameVars for those kinds. Its
 * "active" field gets the time; invalid colour and mul values only warn.
 */
int PalFx_Apply(PalFxDef *dst, PalFxParams *pf, Player *p, StateCtrl *sc)
{
    int i;

    if (pf->kind == PALFX_BG) {
        dst = &g_game->bg_palfx_def;
    } else if (pf->kind == PALFX_ALL) {
        dst = &g_game->all_palfx_def;
    }
    memset(dst, 0, sizeof(*dst));
    dst->active = Expr_EvalInt(p, &pf->time, 1);
    if (dst->active < -1) {
        dst->active = 0;
        Player_WarnAt(p, sc, "%s", "invalid PalFX time");
    }
    dst->color = Expr_EvalInt(p, &pf->color, 1);
    if (dst->color < 0 || dst->color > 0x100) {
        Player_WarnAt(p, sc, "%s", "invalid PalFX color value");
    }
    for (i = 0; i < 3; i++) {
        dst->add[i] = Expr_EvalInt(p, &pf->add[i], 1);
    }
    for (i = 0; i < 3; i++) {
        dst->mul[i] = Expr_EvalInt(p, &pf->mul[i], 1);
    }
    if (dst->mul[0] < 0 || dst->mul[1] < 0 || dst->mul[2] < 0) {
        Player_WarnAt(p, sc, "%s", "invalid PalFX mul value");
    }
    for (i = 0; i < 3; i++) {
        dst->sinadd[i] = Expr_EvalInt(p, &pf->sinadd[i], 1);
    }
    dst->period = Expr_EvalInt(p, &pf->period, 1);
    dst->invertall = Expr_EvalInt(p, &pf->invertall, 1);
    return 1;
}

/* 0x0044E330 */
void PalFxParams_Free(PalFxParams *pf)
{
    int i;

    Expr_Free(&pf->time);
    Expr_Free(&pf->color);
    for (i = 0; i < 3; i++) {
        Expr_Free(&pf->add[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&pf->mul[i]);
    }
    for (i = 0; i < 3; i++) {
        Expr_Free(&pf->sinadd[i]);
    }
    Expr_Free(&pf->invertall);
    Expr_Free(&pf->period);
}

/* 0x0044E3C0: PlaySnd parameters; the value is required. */
int PlaySnd_ParseParams(PlaySndParams *ps, Ini *ini, CharData *owner)
{
    const char *value = Ini_GetValue(ini, "value");
    const char *s;
    int fight;

    if (value == NULL) {
        return 0;
    }
    Expr_SetInt(&ps->channel, -1);
    Expr_SetFloat(&ps->freqmul, 1.0f);
    Expr_SetFloat(&ps->volumescale, 100.0f);
    fight = 0;
    if (!Cns_ParseSoundRef(value, &fight, &ps->group, &ps->number, owner)) {
        return 0;
    }
    ps->fight = fight;
    if (!compile_key(ini, "volume", &ps->volume, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "volumescale", &ps->volumescale, owner, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_key(ini, "channel", &ps->channel, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "lowpriority", &ps->lowpriority, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "freqmul", &ps->freqmul, owner, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_key(ini, "loop", &ps->loop, owner, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "abspan");
    if (s != NULL) {
        if (!Expr_Compile(s, &ps->pan, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        ps->abspan = 1;
        return 1;
    }
    return compile_key(ini, "pan", &ps->pan, owner, EXPR_WANT_INT);
}

/*
 * 0x0044E5E0: PlaySnd. Every parameter is evaluated first (volume is
 * evaluated but unused; freqmul is only checked). Channel -1 or an illegal
 * channel plays on a free slot; lowpriority skips the sound while its channel
 * plays. The pan is relative to the player (times its facing) unless abspan.
 */
void PlaySnd_Run(PlaySndParams *ps, Player *p, StateCtrl *sc)
{
    GameVars *g;
    Snd *snd;
    int channel, group, number, fight, lowpriority, loop, abspan, pan, id, slot, x;
    double volumescale, volume;
    float freqmul;

    if (ps == NULL) {
        return;
    }
    channel = Expr_EvalInt(p, &ps->channel, 1);
    group = Expr_EvalInt(p, &ps->group, 1);
    number = Expr_EvalInt(p, &ps->number, 1);
    Expr_EvalInt(p, &ps->volume, 1);
    volumescale = Expr_EvalFloat(p, &ps->volumescale);
    fight = ps->fight;
    lowpriority = Expr_EvalInt(p, &ps->lowpriority, 1);
    freqmul = Expr_EvalFloat(p, &ps->freqmul);
    loop = Expr_EvalInt(p, &ps->loop, 1);
    abspan = ps->abspan;
    pan = Expr_EvalInt(p, &ps->pan, 1);
    g = g_game;
    if (!g->sound_enabled || g->no_sounds != 0) {
        return;
    }
    snd = fight ? g->fight.common_snd : p->data->snd;
    if (snd == NULL) {
        return;
    }
    if (channel < -1 || channel > 31) {
        Player_WarnAt(p, sc, "%s", "Illegal playsnd channel.");
        id = -1;
    } else if (channel == -1) {
        id = -1;
    } else {
        id = g->player_slots * channel + p->index + 1000;
    }
    if (lowpriority && id != -1 && Sound_IsPlaying(id)) {
        return;
    }
    if (group < -1) {
        Player_WarnAt(p, sc, "%s", "Illegal playsnd groupno.");
        group = -1;
    }
    if (number < -1) {
        Player_WarnAt(p, sc, "%s", "Illegal playsnd sndno.");
        number = -1;
    }
    slot = Sound_Prepare(id, snd, group, number);
    if (slot == -1 || group == -1) {
        return;
    }
    x = p->facing * pan;
    g = g_game;
    if (!abspan) {
        x = Ftol(p->pos.x - g->camera_offset_x + x);
    }
    pan = g->stereo_effects ? Sound_Pan(x) : 0;
    volume = g->wav_volume * g->master_volume / 100.0 * volumescale / 100.0;
    if (!fight) {
        volume = volume * (p->data->volumescale / 100.0);
    }
    if (!(0.0f < freqmul)) {
        Player_WarnAt(p, sc, "%s", "Non-positive freqmul.");
    }
    Sound_SetSlotVolume(slot, volume);
    Sound_SetSlotPan(slot, pan);
    Sound_SetSlotLoop(slot, loop != 0);
    Sound_PlaySlot(slot);
}

/* 0x0044E890 */
void PlaySndParams_Free(PlaySndParams *ps)
{
    Expr_Free(&ps->channel);
    Expr_Free(&ps->group);
    Expr_Free(&ps->number);
    Expr_Free(&ps->volume);
    Expr_Free(&ps->volumescale);
    Expr_Free(&ps->lowpriority);
    Expr_Free(&ps->freqmul);
    Expr_Free(&ps->loop);
    Expr_Free(&ps->pan);
}

/* 0x0044E8F0: HitOverride parameters (attr stays 0 when not given). */
int HitOverride_ParseParams(Ini *ini, CharData *owner, HitOverrideParams *ho)
{
    const char *s;

    Expr_SetInt(&ho->time, 1);
    Expr_SetInt(&ho->stateno, -1);
    s = Ini_GetValue(ini, "attr");
    if (s != NULL) {
        ho->attr = HitDefAttr_Parse(s, NULL);
        if (ho->attr == 0xFFFFFFFFu) {
            Error_Printf("Invalid attr: %s\n", s);
            return 0;
        }
    }
    if (!compile_key(ini, "stateno", &ho->stateno, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "time", &ho->time, owner, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "forceair");
    Expr_SetInt(&ho->forceair, 0);
    if (s != NULL && !Expr_Compile(s, &ho->forceair, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    return 1;
}

/* 0x0044EA20: sets HitOverride slot ho (stateno below -1: -1; time below -1: 1). */
int HitOverride_Apply(HitOverride *ho, Player *p, HitOverrideParams *params, StateCtrl *sc)
{
    int v;

    ho->active = 1;
    ho->attr = params->attr;
    v = Expr_EvalInt(p, &params->stateno, 1);
    if (v < -1) {
        Player_WarnAt(p, sc, "%s", "Illegal hitoverride stateno.");
        v = -1;
    }
    ho->stateno = v;
    v = Expr_EvalInt(p, &params->time, 1);
    if (v < -1) {
        Player_WarnAt(p, sc, "%s", "Illegal hitoverride time.");
        v = 1;
    }
    ho->time = v;
    ho->forceair = Expr_EvalInt(p, &params->forceair, 1);
    return 1;
}

/* 0x0044EAC0 (part): the animtype letters (light, medium, hard/heavy, back, up, diagup); -1 if invalid. */
static int animtype_of(int c)
{
    switch (c) {
    case 'l':
        return 0;
    case 'm':
        return 1;
    case 'h':
        return 2;
    case 'b':
        return 3;
    case 'u':
        return 4;
    case 'd':
        return 5;
    default:
        return -1;
    }
}

/* The ground.type / air.type letters (none, high, low, trip); -1 if invalid. */
static int hittype_of(int c)
{
    switch (c) {
    case 'n':
        return 0;
    case 'h':
        return 1;
    case 'l':
        return 2;
    case 't':
        return 3;
    default:
        return -1;
    }
}

/* A spark number with an optional S prefix (the character's own sprites). */
static int parse_spark(Ini *ini, const char *key, Expr *e, int *own, CharData *owner)
{
    const char *s = Ini_GetValue(ini, key);

    if (s == NULL) {
        return 1;
    }
    if ((unsigned int)(*s - 1) <= 0x7E ? tolower(*s) == 's' : *s == 's') {
        if (!Expr_Compile(s + 1, e, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        *own = 1;
        return 1;
    }
    return Expr_Compile(s, e, owner, NULL, EXPR_WANT_INT);
}

/* Compiles key into e if given, else e becomes a copy of def. */
static int compile_or_clone(Ini *ini, const char *key, Expr *e, const Expr *def, CharData *owner, int want)
{
    const char *s = Ini_GetValue(ini, key);

    if (s != NULL) {
        return Expr_Compile(s, e, owner, NULL, want);
    }
    Expr_Clone(e, def);
    return 1;
}

/*
 * 0x0044EAC0: the HitDef (or ReversalDef, or projectile HitDef) parameters.
 * Many defaults are copies of other parameters, so the order matters. An
 * invalid attr fails without a message.
 */
int HitDef_ParseParams(Ini *ini, HitDefParams *hd, CharData *owner)
{
    const char *s;
    const char *end;
    int n, c;

    Expr_SetInt(&hd->hitsound[0], -1);
    Expr_SetInt(&hd->guardsound[0], -1);
    hd->guardsound_fight = 1;
    hd->hitsound_fight = 1;
    Expr_SetInt(&hd->guard_dist, -1);
    Expr_SetInt(&hd->chainid, -1);
    Expr_SetInt(&hd->nochainid[0], -1);
    Expr_SetInt(&hd->nochainid[1], -1);
    hd->affectteam = 2;
    s = Ini_GetValue(ini, "attr");
    if (s != NULL) {
        hd->attr = HitDefAttr_Parse(s, NULL);
        if (hd->attr == -1) {
            return 0;
        }
    }
    s = Ini_GetValue(ini, "affectteam");
    if (s != NULL) {
        c = (unsigned int)(*s - 1) <= 0x7E ? tolower(*s) : *s;
        if (c == 'b') {
            hd->affectteam = 3;
        } else if (c == 'e') {
            hd->affectteam = 2;
        } else if (c == 'f') {
            hd->affectteam = 1;
        } else {
            Error_Printf("Invalid affectteam value: %s\n", s);
            return 0;
        }
    }
    Expr_SetInt(&hd->air_juggle, -1);
    Expr_SetInt(&hd->numhits, 1);
    Expr_SetInt(&hd->forcestand, 0);
    Expr_SetInt(&hd->forcenofall, 0);
    if (!compile_key(ini, "id", &hd->id, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "chainid", &hd->chainid, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "nochainid", "ii", &hd->nochainid[0], &hd->nochainid[1], owner)) {
        return 0;
    }
    Expr_SetInt(&hd->sparkno, -2);
    if (!parse_spark(ini, "sparkno", &hd->sparkno, &hd->spark_own, owner)) {
        return 0;
    }
    Expr_SetInt(&hd->guard_sparkno, -2);
    if (!parse_spark(ini, "guard.sparkno", &hd->guard_sparkno, &hd->guard_spark_own, owner)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "sparkxy", "ii", &hd->sparkxy[0], &hd->sparkxy[1], owner)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "pausetime", "ii", &hd->p1_pausetime, &hd->p2_pausetime, owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "guard.pausetime");
    if (s != NULL) {
        n = Cns_ParseValues(s, "ii", owner, &end, &hd->guard_p1_pausetime, &hd->guard_p2_pausetime);
        if (n == 0 || *end != '\0') {
            return 0;
        }
        if (n == 1) {
            Expr_Clone(&hd->guard_p2_pausetime, &hd->p2_pausetime);
        }
    } else {
        Expr_Clone(&hd->guard_p1_pausetime, &hd->p1_pausetime);
        Expr_Clone(&hd->guard_p2_pausetime, &hd->p2_pausetime);
    }
    if (!Cns_CompileInt(ini, "kill", &hd->kill, owner, 1)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "guard.kill", &hd->guard_kill, owner, 1)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "fall.kill", &hd->fall_kill, owner, 1)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "hitonce", &hd->hitonce, owner, (hd->attr & HITATTR_THROWS) != 0)) {
        return 0;
    }
    if (!compile_key(ini, "numhits", &hd->numhits, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "damage", "ii", &hd->damage[0], &hd->damage[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "getpower");
    if (s != NULL) {
        n = Cns_ParseValues(s, "ii", owner, &end, &hd->getpower[0], &hd->getpower[1]);
        if (n == 0 || *end != '\0') {
            return 0;
        }
        if (n == 1) {
            Expr_Clone(&hd->getpower[1], &hd->getpower[0]);
            Expr_Halve(&hd->getpower[1]);
        }
    } else {
        Expr_Clone(&hd->getpower[0], &hd->damage[0]);
        Expr_MulFloat(&hd->getpower[0], g_game->attack_life_to_power_mul);
        Expr_Clone(&hd->getpower[1], &hd->getpower[0]);
        Expr_Halve(&hd->getpower[1]);
    }
    s = Ini_GetValue(ini, "givepower");
    if (s != NULL) {
        n = Cns_ParseValues(s, "ii", owner, &end, &hd->givepower[0], &hd->givepower[1]);
        if (n == 0 || *end != '\0') {
            return 0;
        }
        if (n == 1) {
            Expr_Clone(&hd->givepower[1], &hd->givepower[0]);
            Expr_Halve(&hd->givepower[1]);
        }
    } else {
        Expr_Clone(&hd->givepower[0], &hd->damage[0]);
        Expr_MulFloat(&hd->givepower[0], g_game->gethit_life_to_power_mul);
        Expr_Clone(&hd->givepower[1], &hd->givepower[0]);
        Expr_Halve(&hd->givepower[1]);
    }
    Expr_SetInt(&hd->priority, 4);
    hd->priority_type = 0;
    s = Ini_GetValue(ini, "priority");
    if (s != NULL) {
        if (!Cns_ParseValues(s, "i:,", owner, &end, &hd->priority)) {
            return 0;
        }
        if (*end != '\0') {
            end++;
            while (Char_IsSpace(*end)) {
                end++;
            }
            switch (Char_ToLower(*end)) {
            case 'd':
                hd->priority_type = 2;
                break;
            case 'h':
                hd->priority_type = 0;
                break;
            case 'm':
                hd->priority_type = 1;
                break;
            default:
                return 0;
            }
        }
    }
    if (!Cns_ParseSoundRef(Ini_GetValue(ini, "hitsound"), &hd->hitsound_fight, &hd->hitsound[0], &hd->hitsound[1],
                           owner)) {
        return 0;
    }
    if (!Cns_ParseSoundRef(Ini_GetValue(ini, "guardsound"), &hd->guardsound_fight, &hd->guardsound[0],
                           &hd->guardsound[1], owner)) {
        return 0;
    }
    if (!compile_key(ini, "ground.slidetime", &hd->ground_slidetime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "ground.hittime", &hd->ground_hittime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Cns_ParseVelocity(Ini_GetValue(ini, "ground.velocity"), &hd->ground_velocity[0], &hd->ground_velocity[1],
                           owner)) {
        return 0;
    }
    if (!Cns_ParseVelocity(Ini_GetValue(ini, "air.velocity"), &hd->air_velocity[0], &hd->air_velocity[1], owner)) {
        return 0;
    }
    if (!compile_key(ini, "air.juggle", &hd->air_juggle, owner, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&hd->air_hittime, 20);
    if (!compile_key(ini, "air.hittime", &hd->air_hittime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&hd->mindist[0], -1025);
    Expr_SetInt(&hd->mindist[1], -1025);
    Expr_SetInt(&hd->maxdist[0], -1025);
    Expr_SetInt(&hd->maxdist[1], -1025);
    s = Ini_GetValue(ini, "snap");
    if (s != NULL) {
        /* before mugenversion 1.0 snap had a z value */
        if (owner->mugenversion < g_game->compat_version[1]) {
            n = Cns_ParseValues(s, "iiii", owner, &end, &hd->mindist[0], &hd->mindist[1], NULL, &hd->snap_time);
        } else {
            n = Cns_ParseValues(s, "iii", owner, &end, &hd->mindist[0], &hd->mindist[1], &hd->snap_time);
        }
        if (n == 0 || *end != '\0') {
            return 0;
        }
        Expr_Clone(&hd->maxdist[0], &hd->mindist[0]);
        Expr_Clone(&hd->maxdist[1], &hd->mindist[1]);
    } else {
        s = Ini_GetValue(ini, "mindist");
        if (s != NULL) {
            if (owner->mugenversion < g_game->compat_version[1]) {
                n = Cns_ParseValues(s, "iii", owner, &end, &hd->mindist[0], &hd->mindist[1], NULL);
            } else {
                n = Cns_ParseValues(s, "ii", owner, &end, &hd->mindist[0], &hd->mindist[1]);
            }
            if (n == 0 || *end != '\0') {
                return 0;
            }
        }
        s = Ini_GetValue(ini, "maxdist");
        if (s != NULL) {
            if (owner->mugenversion < g_game->compat_version[1]) {
                n = Cns_ParseValues(s, "iii", owner, &end, &hd->maxdist[0], &hd->maxdist[1], NULL);
            } else {
                n = Cns_ParseValues(s, "ii", owner, &end, &hd->maxdist[0], &hd->maxdist[1]);
            }
            if (n == 0 || *end != '\0') {
                return 0;
            }
        }
    }
    Expr_SetInt(&hd->p1sprpriority, 1);
    s = Ini_GetValue(ini, "p1sprpriority");
    if (s == NULL) {
        s = Ini_GetValue(ini, "sprpriority");
    }
    if (!Expr_Compile(s, &hd->p1sprpriority, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "p2sprpriority", &hd->p2sprpriority, owner, 0)) {
        return 0;
    }
    Expr_SetInt(&hd->p1facing, 0);
    if (!compile_key(ini, "p1facing", &hd->p1facing, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "p1getp2facing", &hd->p1getp2facing, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "p2facing", &hd->p2facing, owner, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&hd->p1stateno, -1);
    Expr_SetInt(&hd->p2stateno, -1);
    Expr_SetInt(&hd->p2getp1state, 0);
    if (!compile_key(ini, "p1stateno", &hd->p1stateno, owner, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "p2stateno");
    if (s != NULL) {
        Expr_SetInt(&hd->p2getp1state, 1);
        if (!Expr_Compile(s, &hd->p2stateno, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
    }
    if (!compile_key(ini, "p2getp1state", &hd->p2getp1state, owner, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "forcestand");
    if (s != NULL) {
        hd->forcestand_given = 1;
        if (!Expr_Compile(s, &hd->forcestand, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
    }
    if (!compile_key(ini, "forcenofall", &hd->forcenofall, owner, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&hd->fall_damage, 0);
    Expr_SetFloat(&hd->fall_xvelocity, -FLT_MAX);
    {
        float yvel = (float)(Units_From320(-4.5f) / (g_gameCoord[0] / owner->localcoord[0]));
        Expr_SetFloat(&hd->fall_yvelocity, yvel);
    }
    Expr_SetInt(&hd->fall_recover, 1);
    Expr_SetInt(&hd->fall_recovertime, 4);
    if (!compile_key(ini, "fall.damage", &hd->fall_damage, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "fall.xvelocity", &hd->fall_xvelocity, owner, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_key(ini, "fall.yvelocity", &hd->fall_yvelocity, owner, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_key(ini, "fall.recover", &hd->fall_recover, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "fall.recovertime", &hd->fall_recovertime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetFloat(&hd->yaccel, -FLT_MAX);
    if (!compile_key(ini, "yaccel", &hd->yaccel, owner, EXPR_WANT_FLOAT)) {
        return 0;
    }
    Expr_SetInt(&hd->down_bounce, 0);
    Expr_SetInt(&hd->down_hittime, 20);
    s = Ini_GetValue(ini, "down.velocity");
    if (s != NULL) {
        n = Cns_ParseValues(s, "ff", owner, &end, &hd->down_velocity[0], &hd->down_velocity[1]);
        if (n == 0 || *end != '\0') {
            return 0;
        }
        if (n == 1) {
            Expr_Clone(&hd->down_velocity[1], &hd->air_velocity[1]);
        }
    } else {
        Expr_Clone(&hd->down_velocity[0], &hd->air_velocity[0]);
        Expr_Clone(&hd->down_velocity[1], &hd->air_velocity[1]);
    }
    if (!compile_key(ini, "down.bounce", &hd->down_bounce, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "down.hittime", &hd->down_hittime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "guard.slidetime", &hd->guard_slidetime, &hd->ground_slidetime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "guard.hittime", &hd->guard_hittime, &hd->ground_hittime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "guard.velocity", &hd->guard_velocity, &hd->ground_velocity[0], owner,
                          EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "guard.ctrltime", &hd->guard_ctrltime, &hd->guard_slidetime, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "guard.dist", &hd->guard_dist, owner, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "ground.cornerpush.veloff");
    if (s != NULL) {
        if (!Expr_Compile(s, &hd->ground_cornerpush, owner, NULL, EXPR_WANT_FLOAT)) {
            return 0;
        }
    } else if (hd->attr & HITATTR_AIR) {
        Expr_SetFloat(&hd->ground_cornerpush, 0.0f);
    } else {
        Expr_Clone(&hd->ground_cornerpush, &hd->guard_velocity);
        Expr_MulFloat(&hd->ground_cornerpush, 1.3f);
    }
    if (!compile_or_clone(ini, "air.cornerpush.veloff", &hd->air_cornerpush, &hd->ground_cornerpush, owner,
                          EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "down.cornerpush.veloff", &hd->down_cornerpush, &hd->ground_cornerpush, owner,
                          EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "guard.cornerpush.veloff", &hd->guard_cornerpush, &hd->ground_cornerpush, owner,
                          EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "airguard.cornerpush.veloff", &hd->airguard_cornerpush, &hd->guard_cornerpush, owner,
                          EXPR_WANT_FLOAT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "airguard.velocity");
    if (s != NULL) {
        n = Cns_ParseValues(s, "ff", owner, &end, &hd->airguard_velocity[0], &hd->airguard_velocity[1]);
        if (n == 0 || *end != '\0') {
            return 0;
        }
        if (n == 1) {
            Expr_Clone(&hd->airguard_velocity[1], &hd->air_velocity[1]);
            Expr_MulFloat(&hd->airguard_velocity[1], 0.5f);
        }
    } else {
        Expr_Clone(&hd->airguard_velocity[0], &hd->air_velocity[0]);
        Expr_MulFloat(&hd->airguard_velocity[0], 1.5f);
        Expr_Clone(&hd->airguard_velocity[1], &hd->air_velocity[1]);
        Expr_MulFloat(&hd->airguard_velocity[1], 0.5f);
    }
    if (!compile_or_clone(ini, "airguard.ctrltime", &hd->airguard_ctrltime, &hd->guard_ctrltime, owner,
                          EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&hd->air_fall, 0);
    s = Ini_GetValue(ini, "fall");
    if (s != NULL) {
        if (!Expr_Compile(s, &hd->fall, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
        Expr_Clone(&hd->air_fall, &hd->fall);
    }
    s = Ini_GetValue(ini, "air.fall");
    if (s != NULL) {
        Expr_Free(&hd->air_fall);
        if (!Expr_Compile(s, &hd->air_fall, owner, NULL, EXPR_WANT_INT)) {
            return 0;
        }
    }
    s = Ini_GetValue(ini, "guardflag");
    if (s != NULL) {
        hd->guardflag |= HitFlags_Parse(s);
    }
    s = Ini_GetValue(ini, "hitflag");
    if (s != NULL) {
        hd->hitflag |= HitFlags_Parse(s);
    } else {
        hd->hitflag |= 0x3E;
    }
    hd->animtype = 0;
    s = Ini_GetValue(ini, "animtype");
    if (s != NULL) {
        n = animtype_of(Char_ToLower(*s));
        if (n == -1) {
            Error_Printf("Invalid animtype: %s\n", s);
            return 0;
        }
        hd->animtype = n;
    }
    hd->air_animtype = hd->animtype;
    s = Ini_GetValue(ini, "air.animtype");
    if (s != NULL) {
        n = animtype_of(Char_ToLower(*s));
        if (n == -1) {
            Error_Printf("Invalid air.animtype: %s\n", s);
            return 0;
        }
        hd->air_animtype = n;
    }
    if (hd->air_animtype == 4) {
        hd->fall_animtype = 4;
    } else if (hd->air_animtype == 5) {
        hd->fall_animtype = 5;
    } else {
        hd->fall_animtype = 3;
    }
    s = Ini_GetValue(ini, "fall.animtype");
    if (s != NULL) {
        n = animtype_of(Char_ToLower(*s));
        if (n == -1) {
            Error_Printf("Invalid fall.animtype %s\n", s);
            return 0;
        }
        hd->fall_animtype = n;
    }
    hd->ground_type = 1;
    s = Ini_GetValue(ini, "ground.type");
    if (s != NULL) {
        n = hittype_of(Char_ToLower(*s));
        if (n == -1) {
            Error_Printf("Invalid ground.type: %s\n", s);
            return 0;
        }
        hd->ground_type = n;
    }
    hd->air_type = hd->ground_type;
    s = Ini_GetValue(ini, "air.type");
    if (s != NULL) {
        n = hittype_of(Char_ToLower(*s));
        if (n == -1) {
            Error_Printf("Invalid air.type: %s\n", s);
            return 0;
        }
        hd->air_type = n;
    }
    if (!PalFx_ParseParams(&hd->palfx, ini, owner, 1)) {
        return 0;
    }
    hd->palfx.kind = PALFX_PLAYER;
    if (!EnvShake_ParseParams("envshake.", &hd->envshake, ini, owner)) {
        return 0;
    }
    return EnvShake_ParseParams("fall.envshake.", &hd->fall_envshake, ini, owner) != 0;
}

/* An int parameter of a HitDef: below min it becomes repl, with the warning msg. */
static int hd_eval(Player *p, StateCtrl *sc, const Expr *e, int min, int repl, const char *msg)
{
    int v = Expr_EvalInt(p, e, 1);

    if (v < min) {
        v = repl;
        Player_WarnAt(p, sc, "%s", msg);
    }
    return v;
}

/*
 * 0x00450300: evaluates a HitDef's parameters into hd (the player's current
 * HitDef or a projectile's); hd->active, hitonce_used and serial are left to the
 * caller. Pre-1.0 characters get one tick less of p1 pause time (not for a
 * ReversalDef) and of guard p1 pause time. A chainID equal to one of the
 * nochainIDs clears all three.
 */
int HitDef_SetFromParams(Player *p, HitDefParams *hp, HitDef *hd, StateCtrl *sc)
{
    float vy;
    int v;

    hd->affectteam = hp->affectteam;
    hd->attr = hp->attr;
    hd->reversal_attr = hp->reversal_attr;
    hd->id = hd_eval(p, sc, &hp->id, 0, 0, "assigned negative hitID");
    hd->chainid = hd_eval(p, sc, &hp->chainid, -1, -1, "specified negative chainID");
    hd->nochainid[0] = hd_eval(p, sc, &hp->nochainid[0], -1, -1, "specified negative nochainID");
    hd->nochainid[1] = hd_eval(p, sc, &hp->nochainid[1], -1, -1, "specified negative nochainID2");
    if ((hd->chainid == hd->nochainid[0] || hd->chainid == hd->nochainid[1]) && hd->chainid != -1) {
        hd->nochainid[1] = -1;
        hd->nochainid[0] = -1;
        hd->chainid = -1;
    }
    hd->kill = Expr_EvalInt(p, &hp->kill, 1) != 0;
    hd->guard_kill = Expr_EvalInt(p, &hp->guard_kill, 1) != 0;
    hd->fall_kill = Expr_EvalInt(p, &hp->fall_kill, 1) != 0;
    hd->hitonce = Expr_EvalInt(p, &hp->hitonce, 1) != 0;
    hd->juggle = hd_eval(p, sc, &hp->air_juggle, -1, -1, "illegal juggle points");
    hd->damage = Expr_EvalInt(p, &hp->damage[0], 1);
    hd->guard_damage = Expr_EvalInt(p, &hp->damage[1], 1);
    hd->getpower = Expr_EvalInt(p, &hp->getpower[0], 0);
    hd->guard_getpower = Expr_EvalInt(p, &hp->getpower[1], 0);
    hd->givepower = Expr_EvalInt(p, &hp->givepower[0], 0);
    hd->guard_givepower = Expr_EvalInt(p, &hp->givepower[1], 0);
    hd->p2_pausetime = hd_eval(p, sc, &hp->p2_pausetime, 0, 0, "specified negative p2shaketime");
    hd->p1_pausetime = hd_eval(p, sc, &hp->p1_pausetime, 0, 0, "specified negative pausetime");
    if (p->data->mugenversion < g_game->compat_version[1] && hd->p1_pausetime > 0 && hd->reversal_attr == 0) {
        hd->p1_pausetime--;
    }
    hd->numhits = Expr_EvalInt(p, &hp->numhits, 1);
    hd->hitsound[0] = hd_eval(p, sc, &hp->hitsound[0], -1, -1, "specified illegal sound group");
    hd->hitsound[1] = hd_eval(p, sc, &hp->hitsound[1], -1, -1, "specified illegal sound no");
    hd->hitsound[2] = hp->hitsound_fight;
    hd->guardsound[0] = hd_eval(p, sc, &hp->guardsound[0], -1, -1, "specified illegal gsound group");
    hd->guardsound[1] = hd_eval(p, sc, &hp->guardsound[1], -1, -1, "specified illegal gsound no");
    hd->guardsound[2] = hp->guardsound_fight;
    hd->guardflag = hp->guardflag;
    hd->hitflag = hp->hitflag;
    hd->priority = hd_eval(p, sc, &hp->priority, 0, 0, "specified negative hit priority");
    hd->priority_type = hp->priority_type;
    hd->p1stateno = hd_eval(p, sc, &hp->p1stateno, -1, -1, "specified negative p1stateno");
    hd->p2stateno = hd_eval(p, sc, &hp->p2stateno, -1, -1, "specified negative p2stateno");
    hd->p2getp1state = Expr_EvalInt(p, &hp->p2getp1state, 1) != 0;
    if (hd->p2getp1state) {
        Player_GetCustomState(p, &hd->custom);
    }
    hd->snap_time = hd_eval(p, sc, &hp->snap_time, -1, 1, "specified illegal p2bindtime");
    hd->p1sprpriority = Expr_EvalInt(p, &hp->p1sprpriority, 1);
    hd->p2sprpriority = Expr_EvalInt(p, &hp->p2sprpriority, 1);
    hd->animtype = hp->animtype;
    hd->forcestand = Expr_EvalInt(p, &hp->forcestand, 1) != 0;
    hd->forcenofall = Expr_EvalInt(p, &hp->forcenofall, 1) != 0;
    if (Expr_EvalInt(p, &hp->fall, 1)) {
        hd->hitflag |= 1;
    }
    hd->fall_damage = Expr_EvalInt(p, &hp->fall_damage, 1);
    hd->fall_animtype = hp->fall_animtype;
    hd->fall_xvel = Expr_EvalFloat(p, &hp->fall_xvelocity);
    hd->fall_yvel = Expr_EvalFloat(p, &hp->fall_yvelocity);
    hd->fall_recover = Expr_EvalInt(p, &hp->fall_recover, 1) != 0;
    hd->fall_recovertime = hd_eval(p, sc, &hp->fall_recovertime, 0, 4, "specified negative recovertime");
    hd->spark_own = hp->spark_own;
    hd->sparkno = hd_eval(p, sc, &hp->sparkno, -2, -2, "illegal spark no");
    hd->guard_spark_own = hp->guard_spark_own;
    hd->guard_sparkno = hd_eval(p, sc, &hp->guard_sparkno, -2, -2, "illegal gspark no");
    hd->sparkxy[0] = Expr_EvalInt(p, &hp->sparkxy[0], 1);
    hd->sparkxy[1] = Expr_EvalInt(p, &hp->sparkxy[1], 1);
    hd->p1facing = Expr_EvalInt(p, &hp->p1facing, 1) < 0;
    v = Expr_EvalInt(p, &hp->p1getp2facing, 1);
    if (v < 0) {
        v = -1;
        hd->p1facing = 0;
    } else if (v > 0) {
        v = 1;
        hd->p1facing = 0;
    }
    hd->p1getp2facing = v;
    hd->mindist[0] = Expr_EvalInt(p, &hp->mindist[0], 1);
    hd->mindist[1] = Expr_EvalInt(p, &hp->mindist[1], 1);
    hd->maxdist[0] = Expr_EvalInt(p, &hp->maxdist[0], 1);
    hd->maxdist[1] = Expr_EvalInt(p, &hp->maxdist[1], 1);
    hd->p2facing = Expr_EvalInt(p, &hp->p2facing, 1);
    if (hd->p2facing < -1 || hd->p2facing > 1) {
        hd->p2facing = 0;
        Player_WarnAt(p, sc, "%s", "Illegal p2facing parameter");
    }
    hd->ground_type = hp->ground_type;
    hd->ground_hittime = hd_eval(p, sc, &hp->ground_hittime, 0, 0, "negative ground.p2hittime");
    hd->ground_slidetime = hd_eval(p, sc, &hp->ground_slidetime, 0, 0, "negative ground.p2slidetime");
    hd->ground_velocity[0] = Expr_EvalFloat(p, &hp->ground_velocity[0]);
    vy = Expr_EvalFloat(p, &hp->ground_velocity[1]);
    hd->ground_velocity[1] = vy;
    if (vy != 0.0f && !hp->forcestand_given) {
        hd->forcestand = 1;
    }
    hd->ground_cornerpush = Expr_EvalFloat(p, &hp->ground_cornerpush);
    hd->air_type = hp->air_type;
    hd->air_animtype = hp->air_animtype;
    hd->air_hittime = hd_eval(p, sc, &hp->air_hittime, 0, 0, "negative air.p2hittime");
    hd->air_fall = Ftol(Expr_EvalFloat(p, &hp->air_fall));
    hd->air_velocity[0] = Expr_EvalFloat(p, &hp->air_velocity[0]);
    hd->air_velocity[1] = Expr_EvalFloat(p, &hp->air_velocity[1]);
    hd->air_cornerpush = Expr_EvalFloat(p, &hp->air_cornerpush);
    hd->down_bounce = Expr_EvalInt(p, &hp->down_bounce, 1) != 0;
    hd->down_hittime = hd_eval(p, sc, &hp->down_hittime, 0, 0, "negative down.p2hittime");
    hd->down_velocity[0] = Expr_EvalFloat(p, &hp->down_velocity[0]);
    hd->down_velocity[1] = Expr_EvalFloat(p, &hp->down_velocity[1]);
    hd->down_cornerpush = Expr_EvalFloat(p, &hp->down_cornerpush);
    hd->guard_velocity = Expr_EvalFloat(p, &hp->guard_velocity);
    hd->guard_hittime = hd_eval(p, sc, &hp->guard_hittime, 0, 0, "negative guard.p2hittime");
    hd->guard_slidetime = hd_eval(p, sc, &hp->guard_slidetime, 0, 0, "negative guard.p2slidetime");
    hd->guard_ctrltime = hd_eval(p, sc, &hp->guard_ctrltime, 0, 0, "negative guard.p2ctrltime");
    hd->guard_dist = Expr_EvalInt(p, &hp->guard_dist, 1);
    hd->guard_p2_pausetime = hd_eval(p, sc, &hp->guard_p2_pausetime, 0, 0, "negative guard.p2shaketime");
    hd->guard_p1_pausetime = hd_eval(p, sc, &hp->guard_p1_pausetime, 0, 0, "negative guard.p1pausetime");
    if (p->data->mugenversion < g_game->compat_version[1] && hd->guard_p1_pausetime > 0) {
        hd->guard_p1_pausetime--;
    }
    hd->guard_cornerpush = Expr_EvalFloat(p, &hp->guard_cornerpush);
    hd->airguard_velocity[0] = Expr_EvalFloat(p, &hp->airguard_velocity[0]);
    hd->airguard_velocity[1] = Expr_EvalFloat(p, &hp->airguard_velocity[1]);
    hd->airguard_ctrltime = hd_eval(p, sc, &hp->airguard_ctrltime, 0, 0, "negative airguard.p2ctrltime");
    hd->airguard_cornerpush = Expr_EvalFloat(p, &hp->airguard_cornerpush);
    hd->yaccel = Expr_EvalFloat(p, &hp->yaccel);
    PalFx_Apply((PalFxDef *)hd->palfx, &hp->palfx, p, sc);
    EnvShake_Apply((EnvShake *)hd->envshake, &hp->envshake, p, sc);
    EnvShake_Apply((EnvShake *)hd->fall_envshake, &hp->fall_envshake, p, sc);
    return 1;
}

/* 0x00450F70 */
void HitDefParams_Free(HitDefParams *hd)
{
    Expr_Free(&hd->id);
    Expr_Free(&hd->chainid);
    Expr_Free(&hd->nochainid[0]);
    Expr_Free(&hd->nochainid[1]);
    Expr_Free(&hd->kill);
    Expr_Free(&hd->guard_kill);
    Expr_Free(&hd->fall_kill);
    Expr_Free(&hd->hitonce);
    Expr_Free(&hd->air_juggle);
    Expr_Free(&hd->damage[0]);
    Expr_Free(&hd->damage[1]);
    Expr_Free(&hd->getpower[0]);
    Expr_Free(&hd->getpower[1]);
    Expr_Free(&hd->givepower[0]);
    Expr_Free(&hd->givepower[1]);
    Expr_Free(&hd->p2_pausetime);
    Expr_Free(&hd->p1_pausetime);
    Expr_Free(&hd->numhits);
    Expr_Free(&hd->hitsound[0]);
    Expr_Free(&hd->hitsound[1]);
    Expr_Free(&hd->guardsound[0]);
    Expr_Free(&hd->guardsound[1]);
    Expr_Free(&hd->priority);
    Expr_Free(&hd->p1stateno);
    Expr_Free(&hd->p2stateno);
    Expr_Free(&hd->p2getp1state);
    Expr_Free(&hd->snap_time);
    Expr_Free(&hd->p1sprpriority);
    Expr_Free(&hd->p2sprpriority);
    Expr_Free(&hd->forcestand);
    Expr_Free(&hd->forcenofall);
    Expr_Free(&hd->fall_damage);
    Expr_Free(&hd->fall_xvelocity);
    Expr_Free(&hd->fall_yvelocity);
    Expr_Free(&hd->fall_recover);
    Expr_Free(&hd->fall_recovertime);
    Expr_Free(&hd->fall);
    Expr_Free(&hd->sparkno);
    Expr_Free(&hd->guard_sparkno);
    Expr_Free(&hd->sparkxy[0]);
    Expr_Free(&hd->sparkxy[1]);
    Expr_Free(&hd->p1facing);
    Expr_Free(&hd->p1getp2facing);
    Expr_Free(&hd->mindist[0]);
    Expr_Free(&hd->mindist[1]);
    Expr_Free(&hd->maxdist[0]);
    Expr_Free(&hd->maxdist[1]);
    Expr_Free(&hd->p2facing);
    Expr_Free(&hd->ground_hittime);
    Expr_Free(&hd->ground_slidetime);
    Expr_Free(&hd->ground_velocity[0]);
    Expr_Free(&hd->ground_velocity[1]);
    Expr_Free(&hd->ground_cornerpush);
    Expr_Free(&hd->air_hittime);
    Expr_Free(&hd->air_fall);
    Expr_Free(&hd->air_velocity[0]);
    Expr_Free(&hd->air_velocity[1]);
    Expr_Free(&hd->air_cornerpush);
    Expr_Free(&hd->down_bounce);
    Expr_Free(&hd->down_hittime);
    Expr_Free(&hd->down_velocity[0]);
    Expr_Free(&hd->down_velocity[1]);
    Expr_Free(&hd->down_cornerpush);
    Expr_Free(&hd->guard_velocity);
    Expr_Free(&hd->guard_hittime);
    Expr_Free(&hd->guard_slidetime);
    Expr_Free(&hd->guard_ctrltime);
    Expr_Free(&hd->guard_cornerpush);
    Expr_Free(&hd->guard_dist);
    Expr_Free(&hd->guard_p2_pausetime);
    Expr_Free(&hd->guard_p1_pausetime);
    Expr_Free(&hd->airguard_velocity[0]);
    Expr_Free(&hd->airguard_velocity[1]);
    Expr_Free(&hd->airguard_ctrltime);
    Expr_Free(&hd->airguard_cornerpush);
    Expr_Free(&hd->yaccel);
    PalFxParams_Free(&hd->palfx);
    EnvShakeParams_Free(&hd->envshake);
    EnvShakeParams_Free(&hd->fall_envshake);
}

/* A parameter with a single value: present means one value and nothing after it. */
static int parse_one(Ini *ini, const char *key, const char *fmt, Expr *e, CharData *owner)
{
    const char *s = Ini_GetValue(ini, key);
    const char *end;

    Cns_ParseValues(s, fmt, owner, &end, e);
    return s == NULL || *end == '\0';
}

/* A projectile bound default: value in 320-wide units, converted to the character's units. */
static int proj_bound(double value320, CharData *owner, int scaled)
{
    int width = (int)g_game->game_coord[0];
    float v = scaled ? (float)(width * value320 / 320.0) : (float)(width / 320.0);

    return (int)(v / (g_gameCoord[0] / owner->localcoord[0]));
}

/* 0x00451370: Projectile parameters, with its HitDef and optional afterimage. */
int Projectile_ParseParams(ProjectileParams *pp, CharData *owner, Ini *ini)
{
    const char *s;
    int v;

    pp->postype = POSTYPE_P1;
    Expr_SetFloat(&pp->velocity[0], 0.0f);
    Expr_SetFloat(&pp->velocity[1], 0.0f);
    Expr_SetInt(&pp->projremovetime, -1);
    Expr_SetInt(&pp->projhits, 1);
    Expr_SetInt(&pp->projpriority, 1);
    Expr_SetInt(&pp->projhitanim, -1);
    Expr_SetInt(&pp->projcancelanim, -1);
    Expr_SetFloat(&pp->velmul[0], 1.0f);
    Expr_SetFloat(&pp->velmul[1], 1.0f);
    Expr_SetFloat(&pp->accel[0], 0.0f);
    Expr_SetFloat(&pp->accel[1], 0.0f);
    Expr_SetFloat(&pp->projscale[0], 1.0f);
    Expr_SetFloat(&pp->projscale[1], 1.0f);
    Expr_SetInt(&pp->projremove, 1);
    v = proj_bound(40.0, owner, 1);
    Expr_SetInt(&pp->projedgebound, v);
    v = proj_bound(40.0, owner, 1);
    Expr_SetInt(&pp->projstagebound, v);
    v = proj_bound(-240.0, owner, 1);
    Expr_SetInt(&pp->projheightbound[0], v);
    v = proj_bound(1.0, owner, 0);
    Expr_SetInt(&pp->projheightbound[1], v);
    Expr_SetInt(&pp->projsprpriority, 3);

    if (!compile_key(ini, "projid", &pp->projid, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "projanim", &pp->projanim, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!parse_pair(ini, "offset", "ii", &pp->offset[0], &pp->offset[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "postype");
    if (s != NULL) {
        pp->postype = Cns_ParsePosType(s);
        if (pp->postype == -2) {
            Error_Printf("Illegal postype: %s\n", s);
            return 0;
        }
    }
    if (!compile_key(ini, "projhitanim", &pp->projhitanim, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "projremanim", &pp->projremanim, &pp->projhitanim, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_or_clone(ini, "projcancelanim", &pp->projcancelanim, &pp->projremanim, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "projsprpriority", &pp->projsprpriority, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "projedgebound", &pp->projedgebound, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!compile_key(ini, "projstagebound", &pp->projstagebound, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!parse_pair(ini, "projheightbound", "ii", &pp->projheightbound[0], &pp->projheightbound[1], owner)) {
        return 0;
    }
    if (!parse_pair(ini, "velocity", "ff", &pp->velocity[0], &pp->velocity[1], owner)) {
        return 0;
    }
    if (!parse_pair(ini, "remvelocity", "ff", &pp->remvelocity[0], &pp->remvelocity[1], owner)) {
        return 0;
    }
    if (!parse_pair(ini, "velmul", "ff", &pp->velmul[0], &pp->velmul[1], owner)) {
        return 0;
    }
    if (!parse_pair(ini, "accel", "ff", &pp->accel[0], &pp->accel[1], owner)) {
        return 0;
    }
    if (!parse_pair(ini, "projscale", "ff", &pp->projscale[0], &pp->projscale[1], owner)) {
        return 0;
    }
    if (!parse_one(ini, "projhits", "i", &pp->projhits, owner)) {
        return 0;
    }
    if (!parse_one(ini, "projpriority", "i", &pp->projpriority, owner)) {
        return 0;
    }
    if (!parse_one(ini, "projremove", "i", &pp->projremove, owner)) {
        return 0;
    }
    if (!parse_one(ini, "projremovetime", "i", &pp->projremovetime, owner)) {
        return 0;
    }
    if (!parse_one(ini, "projmisstime", "i", &pp->projmisstime, owner)) {
        return 0;
    }
    {
        const char *end;
        /* not checked at all */
        Cns_ParseValues(Ini_GetValue(ini, "projshadow"), "iii", owner, &end, &pp->projshadow[0], &pp->projshadow[1],
                        &pp->projshadow[2]);
    }
    if (!Cns_CompileInt(ini, "pausemovetime", &pp->pausemovetime, owner, 0)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "supermovetime", &pp->supermovetime, owner, 0)) {
        return 0;
    }
    if (!HitDef_ParseParams(ini, &pp->hitdef, owner)) {
        Error_Printf("Error parsing Hitdef of projectile\n");
        return 0;
    }
    if (!Cns_CompileInt(ini, "ownpal", &pp->ownpal, owner, 0)) {
        return 0;
    }
    Expr_SetInt(&pp->remappal[0], -1);
    if (!parse_pair_checked(ini, "remappal", "ii", &pp->remappal[0], &pp->remappal[1], owner)) {
        return 0;
    }
    if (Ini_GetValue(ini, "afterimage.time") != NULL) {
        pp->afterimage = Mem_Calloc(1, sizeof(AfterImageParams));
        if (pp->afterimage == NULL) {
            return 0;
        }
        if (!AfterImage_ParseParams(pp->afterimage, ini, owner, "afterimage.")) {
            Error_Printf("Error parsing Afterimage of projectile\n");
            return 0;
        }
    }
    return 1;
}

/*
 * 0x00451DA0: fires a projectile: builds it from pp (velocities, bounds and
 * offset in game units by the states' owner; HitDef_SetFromParams for its
 * HitDef) and adds it to the projectiles of owner (the root of a helper).
 * Returns its slot, or -1.
 *
 * Quirks kept from the original: the projectile Coords are built from a
 * local Coord whose pad_24 is never set (stack garbage is copied); velmul
 * and the draw scale get unit 0; projremanim and projcancelanim below -1
 * only warn.
 */
int Projectile_Fire(ProjectileParams *pp, Player *p, StateRunCtx *ctx, Player *owner, StateCtrl *sc)
{
    GameVars *g;
    Projectile proj;
    Projectile *e;
    ProjectileList *pl;
    Coord c;
    float x, y, f, sb;
    double s, half, scale_y;
    int r, gr, b, v, anim, ownpal, remap_group, remap_index, slot;

    memset(&proj, 0, sizeof(proj));
    c.x = 0.0;
    c.y = 0.0;
    c.z = 0.0;
    c.unit = 1.0;
    c.type = 2;
    proj.pos = c;
    proj.vel = c;
    proj.remvel = c;
    proj.accel = c;
    c.x = 1.0;
    c.y = 1.0;
    c.z = 0.0;
    c.unit = 0.0;
    proj.velmul = c;
    memcpy(&proj.draw_scale_x, &c, sizeof(c));
    proj.id = Expr_EvalInt(p, &pp->projid, 1);
    if (proj.id < 0) {
        proj.id = 0;
        Player_WarnAt(p, sc, "%s", "negative projectile ID");
    }
    proj.team_side = p->team_side;
    proj.removeonhit = Expr_EvalInt(p, &pp->projremove, 1) != 0;
    proj.removetime = Expr_EvalInt(p, &pp->projremovetime, 1);
    if (proj.removetime < -1) {
        proj.removetime = 1;
        Player_WarnAt(p, sc, "%s", "illegal projremovetime");
    }
    r = Expr_EvalInt(p, &pp->projshadow[0], 1);
    gr = Expr_EvalInt(p, &pp->projshadow[1], 1);
    b = Expr_EvalInt(p, &pp->projshadow[2], 1);
    if (r < -1 || gr < -1 || b < -1 || r > 0xFF || gr > 0xFF || b > 0xFF) {
        proj.shadow = -1;
        Player_WarnAt(p, sc, "%s", "illegal shadowcolor");
    } else if (r == -1 || gr == -1 || b == -1) {
        proj.shadow = -1;
    } else {
        proj.shadow = (int)Video_MapRGB(r, gr, b);
    }
    proj.projmisstime = Expr_EvalInt(p, &pp->projmisstime, 1);
    if (proj.projmisstime < 0) {
        proj.projmisstime = 2;
        Player_WarnAt(p, sc, "%s", "negative projectile miss time");
    }
    proj.hits = Expr_EvalInt(p, &pp->projhits, 1);
    if (proj.hits < 0) {
        proj.hits = 1;
        Player_WarnAt(p, sc, "%s", "negative # of projectile hits");
    }
    proj.priority = Expr_EvalInt(p, &pp->projpriority, 1);
    proj.anim_hit = Expr_EvalInt(p, &pp->projhitanim, 1);
    if (proj.anim_hit < -1) {
        proj.anim_hit = -1;
        Player_WarnAt(p, sc, "%s", "illegal projhit actionid");
    }
    proj.anim_remove = Expr_EvalInt(p, &pp->projremanim, 1);
    if (proj.anim_remove < -1) {
        Player_WarnAt(p, sc, "%s", "illegal projremove actionid");
    }
    proj.anim_cancel = Expr_EvalInt(p, &pp->projcancelanim, 1);
    if (proj.anim_cancel < -1) {
        Player_WarnAt(p, sc, "%s", "illegal projcancel actionid");
    }
    f = Expr_EvalFloat(p, &pp->velocity[0]);
    proj.vel.x = f * states_scale(p);
    f = Expr_EvalFloat(p, &pp->velocity[1]);
    proj.vel.y = f * states_scale(p);
    proj.velmul.x = Expr_EvalFloat(p, &pp->velmul[0]);
    proj.velmul.y = Expr_EvalFloat(p, &pp->velmul[1]);
    f = Expr_EvalFloat(p, &pp->remvelocity[0]);
    proj.remvel.x = f * states_scale(p);
    f = Expr_EvalFloat(p, &pp->remvelocity[1]);
    proj.remvel.y = f * states_scale(p);
    f = Expr_EvalFloat(p, &pp->accel[0]);
    proj.accel.x = f * states_scale(p);
    f = Expr_EvalFloat(p, &pp->accel[1]);
    proj.accel.y = f * states_scale(p);
    proj.draw_scale_x = Expr_EvalFloat(p, &pp->projscale[0]);
    scale_y = Expr_EvalFloat(p, &pp->projscale[1]);
    if (p->consts.proj_doscale) {
        double r2 = g_gameCoord[0] / p->data->localcoord[0];

        f = (float)(p->consts.xscale * r2);
        proj.draw_scale_x = f * proj.draw_scale_x;
        f = (float)(r2 * p->consts.yscale);
        proj.draw_scale_y = scale_y * f;
    } else {
        proj.draw_scale_x = states_scale(p) * proj.draw_scale_x;
        proj.draw_scale_y = scale_y * states_scale(p);
    }
    HitDef_SetFromParams(p, &pp->hitdef, &proj.hitdef, sc);
    proj.hitdef.serial = -1;
    if (proj.hitdef.attr != 0) {
        proj.hitdef.active = 1;
    }
    if (proj.hitdef.guard_dist == -1) {
        proj.hitdef.guard_dist = p->consts.proj_attack_dist;
    }
    proj.facing = p->facing;
    proj.hitpause_time = 0;
    proj.time = 0;
    proj.ticks = 0;
    proj.state = PROJ_STATE_LIVE;
    Player_GetPosOrigin(NULL, p, pp->postype, &ctx->enemy, 0, &x, &y, NULL, NULL, NULL);
    if (pp->postype == 1 || pp->postype == 2 || pp->postype == 4 || pp->postype == 3) {
        y = 0.0f;
    }
    v = Expr_EvalInt(p, &pp->offset[0], 0) * p->facing;
    proj.pos.x = v * states_scale(p) + x;
    v = Expr_EvalInt(p, &pp->offset[1], 0);
    proj.pos.y = v * states_scale(p) + y;
    proj.vel.x = proj.vel.x * p->facing;
    proj.remvel.x = proj.remvel.x * p->facing;
    proj.accel.x = proj.accel.x * p->facing;
    proj.sprpriority = Expr_EvalInt(p, &pp->projsprpriority, 1);
    v = Expr_EvalInt(p, &pp->projstagebound, 0);
    sb = (float)(v * states_scale(p));
    g = g_game;
    s = g_gameCoord[0] / g->stage.unit;
    f = (float)((double)Ftol(g->game_coord[0]) / g->stage.zoomout);
    half = f * 0.5;
    proj.bound[1] = Ftol(g->stage.boundleft * s - sb - half);
    proj.bound[3] = Ftol(g->stage.boundright * s + sb + half);
    v = Expr_EvalInt(p, &pp->projedgebound, 0);
    proj.bound[0] = Ftol(v * states_scale(p));
    v = Expr_EvalInt(p, &pp->projheightbound[0], 0);
    proj.bound[2] = Ftol(v * states_scale(p));
    v = Expr_EvalInt(p, &pp->projheightbound[1], 0);
    proj.bound[4] = Ftol(v * states_scale(p));
    anim = Expr_EvalInt(p, &pp->projanim, 1);
    if (anim < -1) {
        anim = 0;
        Player_WarnAt(p, sc, "%s", "illegal projanim");
    }
    proj.pause_movetime[0] = Ftol(Expr_EvalFloat(p, &pp->pausemovetime));
    proj.pause_movetime[1] = Ftol(Expr_EvalFloat(p, &pp->supermovetime));
    ownpal = Expr_EvalInt(p, &pp->ownpal, 0);
    remap_group = Expr_EvalInt(p, &pp->remappal[0], 0);
    remap_index = Expr_EvalInt(p, &pp->remappal[1], 0);
    slot = Projectiles_Add(owner->projectiles, owner, &proj, anim);
    if (slot == -1) {
        return -1;
    }
    pl = owner->projectiles;
    e = (Projectile *)(pl->list->data + pl->list->elem_size * slot);
    if (pp->afterimage != NULL) {
        if (e->afterimage == NULL) {
            e->afterimage = AfterImages_FindFree(g_game->afterimages);
        }
        if (e->afterimage != NULL) {
            AfterImage_Apply(p, pp->afterimage, e->afterimage, &e->anim, sc);
        }
    }
    if (ownpal == 0) {
        e->palfx = owner->palfx;
        e->remap = owner->remap;
        return slot;
    }
    e->palfx = NULL;
    if (remap_group == -1) {
        e->remap_own = *p->remap;
    } else {
        KeyedArray *pals = ((ProjectileList *)owner->projectiles)->sprites->palettes;
        void *pal = NULL;

        if (pals != NULL) {
            int i = KeyedArray_Find(pals, remap_group, remap_index);

            if (i != -1) {
                pal = pals->data + pals->elem_size * i;
            }
        }
        if (pal != NULL) {
            PalRemapEntry entry;

            memset(&e->remap_own, 0, sizeof(e->remap_own));
            entry.src_group = -1;
            entry.src_index = 0;
            entry.dst_group = remap_group;
            entry.dst_index = remap_index;
            PalRemap_Set(&e->remap_own, &entry);
        } else {
            Player_WarnAt(p, sc, "Projectile invalid remappal %i,%i", remap_group, remap_index);
        }
    }
    e->remap = &e->remap_own;
    return slot;
}

/* 0x00452AB0 */
void ProjectileParams_Free(ProjectileParams *pp)
{
    Expr_Free(&pp->projid);
    Expr_Free(&pp->offset[0]);
    Expr_Free(&pp->offset[1]);
    Expr_Free(&pp->projremove);
    Expr_Free(&pp->projremovetime);
    Expr_Free(&pp->projmisstime);
    Expr_Free(&pp->projedgebound);
    Expr_Free(&pp->projstagebound);
    Expr_Free(&pp->projheightbound[0]);
    Expr_Free(&pp->projheightbound[1]);
    Expr_Free(&pp->projhits);
    Expr_Free(&pp->projpriority);
    Expr_Free(&pp->projanim);
    Expr_Free(&pp->projhitanim);
    Expr_Free(&pp->projremanim);
    Expr_Free(&pp->projcancelanim);
    Expr_Free(&pp->projshadow[0]);
    Expr_Free(&pp->projshadow[1]);
    Expr_Free(&pp->projshadow[2]);
    Expr_Free(&pp->projsprpriority);
    Expr_Free(&pp->velocity[0]);
    Expr_Free(&pp->velocity[1]);
    Expr_Free(&pp->velmul[0]);
    Expr_Free(&pp->velmul[1]);
    Expr_Free(&pp->remvelocity[0]);
    Expr_Free(&pp->remvelocity[1]);
    Expr_Free(&pp->accel[0]);
    Expr_Free(&pp->accel[1]);
    Expr_Free(&pp->projscale[0]);
    Expr_Free(&pp->projscale[1]);
    Expr_Free(&pp->pausemovetime);
    Expr_Free(&pp->supermovetime);
    Expr_Free(&pp->ownpal);
    Expr_Free(&pp->remappal[0]);
    Expr_Free(&pp->remappal[1]);
    HitDefParams_Free(&pp->hitdef);
    if (pp->afterimage != NULL) {
        AfterImageParams_Free(pp->afterimage);
        free(pp->afterimage);   /* the pointer is left */
    }
}

/* 0x00452C70: Helper parameters; the size.* values not given stay INT_MIN / -FLT_MAX. */
int Helper_ParseParams(HelperParams *hp, Ini *ini, CharData *owner)
{
    const char *s;

    s = Ini_GetValue(ini, "helpertype");
    if (s != NULL) {
        if (_stricmp(s, "normal") == 0) {
            hp->helpertype = 0;
        } else if (_stricmp(s, "player") == 0) {
            hp->helpertype = 1;
        } else if (Str_PrefixMatchI(s, "proj", 4)) {
            hp->helpertype = 2;
        } else {
            Error_Printf("Illegal helper type: %s\n", s);
            return 0;
        }
    }
    if (Ini_GetValue(ini, "name") != NULL && !Ini_ReadString(ini, "name", sizeof(hp->name) - 1, hp->name)) {
        return 0;
    }
    if (!compile_key(ini, "id", &hp->id, owner, EXPR_WANT_INT)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "pos", "ff", &hp->pos[0], &hp->pos[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "postype");
    if (s != NULL) {
        hp->postype = Cns_ParsePosType(s);
        if (hp->postype == -2) {
            Error_Printf("Illegal postype: %s\n", s);
            return 0;
        }
    }
    s = Ini_GetValue(ini, "facing");
    Expr_SetInt(&hp->facing, 1);
    if (s != NULL && !Expr_Compile(s, &hp->facing, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "stateno");
    Expr_SetInt(&hp->stateno, 0);
    if (s != NULL && !Expr_Compile(s, &hp->stateno, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "keyctrl", &hp->keyctrl, owner, 0)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "ownpal", &hp->ownpal, owner, 0)) {
        return 0;
    }
    Expr_SetInt(&hp->remappal[0], -1);
    if (!parse_pair_checked(ini, "remappal", "ii", &hp->remappal[0], &hp->remappal[1], owner)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "pausemovetime", &hp->pausemovetime, owner, 0)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "supermovetime", &hp->supermovetime, owner, 0)) {
        return 0;
    }
    if (!Cns_CompileFloat(ini, "size.xscale", &hp->xscale, owner, -FLT_MAX)) {
        return 0;
    }
    if (!Cns_CompileFloat(ini, "size.yscale", &hp->yscale, owner, -FLT_MAX)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.ground.back", &hp->ground_back, owner, INT_MIN)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.ground.front", &hp->ground_front, owner, INT_MIN)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.air.back", &hp->air_back, owner, INT_MIN)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.air.front", &hp->air_front, owner, INT_MIN)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.height", &hp->height, owner, INT_MIN)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "size.proj.doscale", &hp->proj_doscale, owner, INT_MIN)) {
        return 0;
    }
    Expr_SetInt(&hp->head_pos[0], INT_MIN);
    Expr_SetInt(&hp->head_pos[1], INT_MIN);
    Expr_SetInt(&hp->mid_pos[0], INT_MIN);
    Expr_SetInt(&hp->mid_pos[1], INT_MIN);
    if (!parse_pair_checked(ini, "size.head.pos", "ii", &hp->head_pos[0], &hp->head_pos[1], owner)) {
        return 0;
    }
    if (!parse_pair_checked(ini, "size.mid.pos", "ii", &hp->mid_pos[0], &hp->mid_pos[1], owner)) {
        return 0;
    }
    return Cns_CompileInt(ini, "size.shadowoffset", &hp->shadowoffset, owner, INT_MIN) != 0;
}

/* 0x00453170 */
void HelperParams_Free(HelperParams *hp)
{
    Expr_Free(&hp->id);
    Expr_Free(&hp->pos[0]);
    Expr_Free(&hp->pos[1]);
    Expr_Free(&hp->facing);
    Expr_Free(&hp->stateno);
    Expr_Free(&hp->keyctrl);
    Expr_Free(&hp->ownpal);
    Expr_Free(&hp->remappal[0]);
    Expr_Free(&hp->remappal[1]);
    Expr_Free(&hp->pausemovetime);
    Expr_Free(&hp->supermovetime);
    Expr_Free(&hp->xscale);
    Expr_Free(&hp->yscale);
    Expr_Free(&hp->ground_back);
    Expr_Free(&hp->ground_front);
    Expr_Free(&hp->air_back);
    Expr_Free(&hp->air_front);
    Expr_Free(&hp->height);
    Expr_Free(&hp->proj_doscale);
    Expr_Free(&hp->head_pos[0]);
    Expr_Free(&hp->head_pos[1]);
    Expr_Free(&hp->mid_pos[0]);
    Expr_Free(&hp->mid_pos[1]);
    Expr_Free(&hp->shadowoffset);
}

/* 0x00453290: Pause parameters. */
int Pause_ParseParams(PauseParams *pp, Ini *ini, CharData *owner)
{
    const char *s;

    s = Ini_GetValue(ini, "time");
    Expr_SetInt(&pp->time, 0);
    if (s != NULL && !Expr_Compile(s, &pp->time, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "movetime");
    Expr_SetInt(&pp->movetime, 0);
    if (s != NULL && !Expr_Compile(s, &pp->movetime, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "endcmdbuftime");
    Expr_SetInt(&pp->endcmdbuftime, 0);
    if (s != NULL && !Expr_Compile(s, &pp->endcmdbuftime, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "pausebg");
    Expr_SetInt(&pp->pausebg, 1);
    if (s != NULL && !Expr_Compile(s, &pp->pausebg, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    return 1;
}

/*
 * 0x00453400: Pause (kind 0) / SuperPause (kind 1): time, the player's
 * movetime (Player.pause_movetime[kind]), pausebg and endcmdbuftime into
 * GameVars.pauses[kind]; illegal values become 0.
 */
int Pause_Apply(int kind, Player *p, PauseParams *pp, StateCtrl *sc)
{
    int *t = g_game->pauses[kind];

    t[6] = Expr_EvalInt(p, &pp->time, 1);
    if (t[6] < 0) {
        Player_WarnAt(p, sc, "%s", "set illegal pausetime");
        t[6] = 0;
    }
    p->pause_movetime[kind] = Expr_EvalInt(p, &pp->movetime, 1);
    if (p->pause_movetime[kind] < 0 || p->pause_movetime[kind] > t[6]) {
        Player_WarnAt(p, sc, "%s", "set illegal pausemovetime");
        p->pause_movetime[kind] = 0;
    }
    t[7] = Expr_EvalInt(p, &pp->pausebg, 1);
    t[8] = Expr_EvalInt(p, &pp->endcmdbuftime, 1);
    if (t[8] < 0 || t[8] > t[6]) {
        Player_WarnAt(p, sc, "%s", "set illegal endcmdbuftime");
        t[8] = 0;
    }
    return 1;
}

/* 0x00453520 */
void PauseParams_Free(PauseParams *pp)
{
    Expr_Free(&pp->time);
    Expr_Free(&pp->movetime);
    Expr_Free(&pp->endcmdbuftime);
    Expr_Free(&pp->pausebg);
}

/* 0x00453550: SuperPause parameters (the Pause ones first, whatever their result). */
int SuperPause_ParseParams(Ini *ini, CharData *owner, SuperPauseParams *sp)
{
    const char *s;

    Pause_ParseParams(&sp->pause, ini, owner);
    s = Ini_GetValue(ini, "time");
    Expr_SetInt(&sp->pause.time, 30);
    if (s != NULL && !Expr_Compile(s, &sp->pause.time, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    Expr_SetInt(&sp->anim, 100);
    sp->anim_fight = 1;
    if (!Cns_ParseAnimRef(Ini_GetValue(ini, "anim"), &sp->anim_fight, &sp->anim, owner)) {
        return 0;
    }
    if (!parse_pair_end_first(ini, "pos", "ff", &sp->pos[0], &sp->pos[1], owner)) {
        return 0;
    }
    s = Ini_GetValue(ini, "darken");
    Expr_SetInt(&sp->darken, 1);
    if (s != NULL && !Expr_Compile(s, &sp->darken, owner, NULL, EXPR_WANT_INT)) {
        return 0;
    }
    s = Ini_GetValue(ini, "p2defmul");
    Expr_SetFloat(&sp->p2defmul, 0.0f);
    if (s != NULL && !Expr_Compile(s, &sp->p2defmul, owner, NULL, EXPR_WANT_FLOAT)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "poweradd", &sp->poweradd, owner, 0)) {
        return 0;
    }
    if (!Cns_CompileInt(ini, "unhittable", &sp->unhittable, owner, 1)) {
        return 0;
    }
    Expr_SetInt(&sp->sound[0], -1);
    sp->sound_fight = 1;
    return Cns_ParseSoundRef(Ini_GetValue(ini, "sound"), &sp->sound_fight, &sp->sound[0], &sp->sound[1], owner) != 0;
}

/* 0x00453790 */
void SuperPauseParams_Free(SuperPauseParams *sp)
{
    Expr_Free(&sp->pause.time);
    Expr_Free(&sp->pause.movetime);
    Expr_Free(&sp->pause.endcmdbuftime);
    Expr_Free(&sp->pause.pausebg);
    Expr_Free(&sp->darken);
    Expr_Free(&sp->p2defmul);
    Expr_Free(&sp->poweradd);
    Expr_Free(&sp->unhittable);
    Expr_Free(&sp->anim);
    Expr_Free(&sp->pos[0]);
    Expr_Free(&sp->pos[1]);
    Expr_Free(&sp->sound[0]);
    Expr_Free(&sp->sound[1]);
}

/* 0x0044C360: frees what a controller owns (not the StateCtrl itself). */
void StateCtrl_Free(StateCtrl *sc)
{
    int i;

    ExprList_Free(sc->triggers);
    for (i = 0; i < 6; i++) {
        Expr_Free(&sc->arg[i]);
    }
    switch (sc->type) {
    case SCTRL_PLAYSND:
        PlaySndParams_Free(sc->params);
        break;
    case SCTRL_HITDEF:
    case SCTRL_REVERSALDEF:
        HitDefParams_Free(sc->params);
        break;
    case SCTRL_ENVSHAKE:
        EnvShakeParams_Free(sc->params);
        break;
    case SCTRL_HITOVERRIDE: {
        HitOverrideParams *ho = sc->params;
        Expr_Free(&ho->stateno);
        Expr_Free(&ho->time);
        Expr_Free(&ho->forceair);
        break;
    }
    case SCTRL_PROJECTILE:
        ProjectileParams_Free(sc->params);
        break;
    case SCTRL_AFTERIMAGE:
        AfterImageParams_Free(sc->params);
        break;
    case SCTRL_PALFX:
        PalFxParams_Free(sc->params);
        break;
    case SCTRL_EXPLOD:
    case SCTRL_MODIFYEXPLOD:
        ExplodParams_Free(sc->params);
        break;
    case SCTRL_PAUSE:
        PauseParams_Free(sc->params);
        break;
    case SCTRL_SUPERPAUSE:
        SuperPauseParams_Free(sc->params);
        break;
    case SCTRL_HELPER:
        HelperParams_Free(sc->params);
        break;
    }
    if (sc->params != NULL) {
        free(sc->params);
    }
    if (sc->source != NULL) {
        free(sc->source);
    }
}
