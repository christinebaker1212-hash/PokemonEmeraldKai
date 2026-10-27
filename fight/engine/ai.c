/*
 * The computer players (original functions 0x42EF50-0x433DFF): learned move
 * data and "ElecbyteAIB" files, the recording of the moves a player does
 * (Ai_RecordMove ... Ai_FinishMove), the messages between computer players
 * and the decision code that feeds the player's input buffer (Ai_Tick).
 */
#include <float.h>
#include "mugen.h"

/* PlayerInput_Push codes: a key number, plus this for its release. */
#define INPUT_CODE_RELEASE 0x10

/* Weights of a merged statistic: old value, new sample. */
#define AI_KEEP 0.95
#define AI_NEW  0.05

/* Basic states whose Clsn2 ranges are kept in AiData.bounds: stand, walk, crouch, jump. */
static const int ai_bound_states[4] = { 0, 20, 11, 50 };

/* abs() as cdq / xor / sub: 0x80000000 stays negative. */
static int AiAbs(int v)
{
    unsigned int s = (unsigned int)(v >> 31);

    return (int)(((unsigned int)v ^ s) - s);
}

/*
 * The generator of mugen.random (GameVars.match_rng = match_rng * match_rng_mul
 * % match_rng_mod), inlined by the decision code: returns the low dword.
 */
static int AiRand(void)
{
    GameVars *g = g_game;
    unsigned __int64 s = g->match_rng * (unsigned __int64)g->match_rng_mul;

    s %= (unsigned __int64)g->match_rng_mod;
    g->match_rng = s;
    return (int)(unsigned int)s;
}

static void AiStat_Set(AiStat *s, int v)
{
    s->avg = (float)v;
    s->min = v;
    s->max = v;
    s->dev = 0.0f;
}

/* Adds sample statistic s to d: weighted average and spread, wider range. */
static void AiStat_Merge(AiStat *d, const AiStat *s)
{
    float s_avg = s->avg;
    float d_avg = d->avg;
    float d_dev = d->dev;
    int mn = s->min < d->min ? s->min : d->min;
    int mx = s->max > d->max ? s->max : d->max;
    float avg;
    float dev;
    int diff;

    avg = (float)(d_avg * AI_KEEP + s_avg * AI_NEW);
    diff = AiAbs(Ftol((double)s_avg - (double)d_avg));
    dev = (float)(diff * AI_NEW + d_dev * AI_KEEP);
    d->avg = avg;
    d->min = mn;
    d->max = mx;
    d->dev = dev;
}

/* The learned moves of a player's state type: stand and crouch moves, air moves. */
static KeyedArray *Ai_TableFor(const AiData *ai, int statetype)
{
    if (statetype > 0 && statetype <= STATETYPE_CROUCH) {
        return ai->tables[0];
    }
    if (statetype == STATETYPE_AIR) {
        return ai->tables[1];
    }
    return NULL;
}

/* Switches the decision mode (the previous one is kept). */
static void Ai_SetMode(AiState *ai, int mode)
{
    ai->prev_mode = ai->mode;
    ai->mode = mode;
    ai->mode_ticks = 0;
}

/* 0x0042EF50: gives the player empty learned data and the default AI level. */
int Ai_InitPlayer(Player *p)
{
    p->ai_learn = 1;
    if (!Ai_Alloc(&p->ai_data)) {
        return 0;
    }
    p->last_proj_id = -1;
    p->ai_level = 4;
    return 1;
}

/*
 * 0x0042EF90: loads an "ElecbyteAIB" file into ai (freed first). The file
 * must come from this MUGEN build (2013-08-11). Returns 0 on errors.
 */
int Ai_Load(const char *path, AiData *ai)
{
    AiFileHeader header;
    AiMove move;
    FILE *f;
    int count;
    int t;
    int i;

    Ai_Free(ai);
    if (!Ai_Alloc(ai)) {
        return 0;
    }
    f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    if (fread(&header, sizeof(header), 1, f) < 1) {
        Error_Printf("Error reading file header\n");
        fclose(f);
        return 0;
    }
    if (strcmp(header.signature, "ElecbyteAIB") != 0) {
        Error_Printf("Not an AI file\n");
        fclose(f);
        return 0;
    }
    if (header.version != AI_FILE_VERSION) {
        Error_Printf("Incorrect AI version\n");
        fclose(f);
        return 0;
    }
    if (header.year != 2013 || header.month != 8 || header.day != 11) {
        Error_Printf("Different MUGEN version for AI hints: %i.%02i.%02i\n",
                     header.year, header.month, header.day);
        fclose(f);
        return 0;
    }
    for (t = 0; t < 2; t++) {
        KeyedArray *list = ai->tables[t];
        if (fread(&count, sizeof(count), 1, f) < 1) {
            fclose(f);
            return 0;
        }
        for (i = 0; i < count; i++) {
            if (fread(&move, sizeof(move), 1, f) < 1) {
                fclose(f);
                return 0;
            }
            if (KeyedArray_Add(list, &move, 0, move.state_no) == -1) {
                Error_Printf("Out of AI learning space\n");
                fclose(f);
                return 0;
            }
        }
    }
    if (fread(ai->bounds, sizeof(ai->bounds), 1, f) < 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

/* 0x0042F1F0: writes ai to an "ElecbyteAIB" file. Returns 0 on errors. */
int Ai_Save(const char *path, const AiData *ai)
{
    AiFileHeader header;
    FILE *f;
    int count;
    int t;

    for (t = 0; t < 2; t++) {
        if (ai->tables[t] == NULL) {
            return 0;
        }
    }
    f = fopen(path, "wb");
    if (f == NULL) {
        return 0;
    }
    memset(&header, 0, sizeof(header));
    memcpy(header.signature, "ElecbyteAIB", 12);
    header.version = AI_FILE_VERSION;
    header.year = 2013;
    header.month = 8;
    header.day = 11;
    if (fwrite(&header, sizeof(header), 1, f) < 1) {
        fclose(f);
        return 0;
    }
    for (t = 0; t < 2; t++) {
        KeyedArray *list = ai->tables[t];
        AiMove *m;
        count = list->count;
        if (fwrite(&count, sizeof(count), 1, f) < 1) {
            fclose(f);
            return 0;
        }
        if (list->count == 0) {
            continue;
        }
        for (m = KeyedArray_First(list); m != NULL; m = KeyedArray_Next(list)) {
            if (fwrite(m, list->elem_size, 1, f) < 1) {
                fclose(f);
                return 0;
            }
        }
    }
    if (fwrite(ai->bounds, sizeof(ai->bounds), 1, f) < 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

/*
 * 0x0042F360: creates the two move tables (80 stand / crouch moves, 40 air
 * moves) and marks the basic state ranges unknown. On failure the tables
 * created so far are kept.
 */
int Ai_Alloc(AiData *ai)
{
    static const int capacity[2] = { 80, 40 };
    int t;

    for (t = 0; t < 2; t++) {
        ai->tables[t] = KeyedArray_Create(capacity[t], sizeof(AiMove));
        if (ai->tables[t] == NULL) {
            return 0;
        }
    }
    ai->bounds[0][0].avg = -FLT_MAX;
    ai->bounds[1][0].avg = -FLT_MAX;
    ai->bounds[2][0].avg = -FLT_MAX;
    ai->bounds[3][0].avg = -FLT_MAX;
    return 1;
}

/* 0x0042F3C0: frees the move tables and clears ai. */
void Ai_Free(AiData *ai)
{
    int t;

    for (t = 0; t < 2; t++) {
        KeyedArray *list = ai->tables[t];
        if (list != NULL) {
            if (list->data != NULL) {
                free(list->data);
            }
            if (list->slots != NULL) {
                free(list->slots);
            }
            free(list);
        }
    }
    memset(ai, 0, sizeof(*ai));
}

/*
 * 0x0042F420: empties the move tables (the moves stay in memory, the slots
 * become unused). The basic state ranges are kept.
 */
void Ai_Reset(AiData *ai)
{
    int t;
    int i;

    for (t = 0; t < 2; t++) {
        KeyedArray *list = ai->tables[t];
        if (list == NULL) {
            continue;
        }
        memset(list->slots, 0, list->capacity * sizeof(KeyedArraySlot));
        list->count = 0;
        list->last = 0;
        list->first = 0;
        list->iter = 0;
        for (i = 0; i < list->cache_size; i++) {
            list->cache[i].key1 = 0x7FFFFFFF;
            list->cache[i].key2 = (int)0x80000000u;
            list->cache[i].index = -1;
        }
        list->cache_pos = 0;
    }
}

/*
 * 0x0042F4A0: adds the moves of src to dst: new moves are copied, known ones
 * merged (Ai_MergeRecord). Missing tables of dst are created; returns 0 when
 * that fails. Moves that do not fit are dropped.
 */
int Ai_Merge(AiData *dst, const AiData *src)
{
    static const int capacity[2] = { 80, 40 };
    int t;

    for (t = 0; t < 2; t++) {
        KeyedArray *from = src->tables[t];
        KeyedArray *to;
        AiMove *m;
        if (from == NULL) {
            continue;
        }
        if (dst->tables[t] == NULL) {
            dst->tables[t] = KeyedArray_Create(capacity[t], sizeof(AiMove));
            if (dst->tables[t] == NULL) {
                return 0;
            }
        }
        to = dst->tables[t];
        if (from->count == 0) {
            continue;
        }
        for (m = KeyedArray_First(from); m != NULL; m = KeyedArray_Next(from)) {
            int index = KeyedArray_Find(to, 0, m->state_no);
            if (index == -1) {
                KeyedArray_Add(to, m, 0, m->state_no);
            } else {
                Ai_MergeRecord((AiMove *)(to->data + to->elem_size * index), m, -1);
            }
        }
    }
    return 1;
}

/*
 * 0x0042F5C0: clears the decision state of the player (the AI block up to
 * Player.enemy) except the learned data, the move being recorded, the
 * controller history and the AI level.
 */
void Ai_ResetPlayer(Player *p)
{
    unsigned char keep[0x2424 - 0x1DD8];
    int level = p->ai_level;

    memcpy(keep, &p->ai_learn, sizeof(keep));
    memset(&p->ai, 0, offsetof(Player, enemy) - offsetof(Player, ai));   /* Moemon Fight: the same fields on 64-bit */
    memcpy(&p->ai_learn, keep, sizeof(keep));
    (p)->ai_cmds_out[0] = -1;
    (&(p)->ai_state)->msg_count = 0;
    p->ai_level = level;
}

/*
 * 0x0042F640: one tick of a computer player: during the fight it fixes
 * impossible key combinations, plays a queued input or decides (training
 * dummy settings in training mode, else Ai_Think); otherwise it lets go of
 * every key.
 */
void Ai_Tick(Player *p)
{
    InputBuf *in = &p->input;
    Player *enemy = p->enemy;
    unsigned int code;

    if (!p->ai || enemy == NULL || !enemy->active) {
        return;
    }
    (p)->ai_cmds_out[0] = -1;
    if (g_game->round_state != 2) {
        Ai_ReleaseAll(p);
        return;
    }
    if (in->keys[KEY_RIGHT] && in->keys[KEY_LEFT]) {
        PlayerInput_Push(in, p->in_guard_dist ? INPUT_CODE_RELEASE | KEY_RIGHT : INPUT_CODE_RELEASE | KEY_LEFT);
    }
    if (in->keys[KEY_JUMP] && in->keys[KEY_CROUCH]) {
        PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
    }
    code = Input_Pop((&(p)->ai_queue));
    if (code != 0) {
        PlayerInput_Push(in, (int)code);
        return;
    }
    (p)->ai_threatened = Ai_AnyInGuardDist(p);
    if (g_game->team_variant == 3 && p->dummy_control == 0) {
        Ai_TrainingDummy(enemy, p);
    } else {
        Ai_Think(p, p->ai_level);
    }
}

/*
 * 0x0042F750: activates the commands the AI chose this tick (of the root
 * player for helpers that share its commands).
 */
void Ai_UpdatePlayer(Player *p)
{
    Player *t = p->root != NULL ? p->root : p;
    const int *list;

    if (!t->ai) {
        return;
    }
    for (list = (t)->ai_cmds_out; *list != -1; list++) {
        p->cmd_active[*list] = 1;
    }
}

/* 0x0042F7A0: appends label to a -1 terminated list of up to 9 labels; 0 when full. */
int Ai_AddCommand(int *list, int label)
{
    int n = 0;

    if (list[0] != -1) {
        do {
            n++;
        } while (list[n] != -1);
        if (n >= 9) {
            return 0;
        }
    }
    list[n] = label;
    list[n + 1] = -1;
    return 1;
}

/* 0x0042F7D0: copies a -1 terminated label list. */
void Ai_CopyCommands(const int *src, int *dst)
{
    int n = 0;

    while (src[n] != -1) {
        dst[n] = src[n];
        n++;
    }
    dst[n] = -1;
}

/*
 * 0x0042F800: the player changed state: looks up the learned move of the new
 * state (AiState.current) and, if the player learns, starts recording it
 * from the state the last controller ran in.
 */
int Ai_RecordMove(Player *p)
{
    AiLearn *learn = (&(p)->ai_record);
    KeyedArray *list = Ai_TableFor(&p->ai_data, p->statetype);

    if (list != NULL) {
        int index = KeyedArray_Find(list, 0, p->state_no);
        (&(p)->ai_state)->current = index != -1 ? (AiMove *)(list->data + list->elem_size * index) : NULL;
    } else {
        (&(p)->ai_state)->current = NULL;
    }
    if (!p->ai_learn) {
        return 0;
    }
    if (learn->active && p->ai_learn) {
        memset(learn, 0, sizeof(*learn));
        learn->active = 0;
    }
    memset(learn, 0, sizeof(*learn));
    learn->active = 1;
    learn->statetype = p->statetype;
    learn->facing = p->facing;
    learn->start_x = Ftol(p->pos.x);
    learn->start_y = Ftol(p->pos.y);
    learn->contact = 0;
    learn->move.state_no = p->state_no;
    learn->move.statetype = p->statetype;
    learn->move.steps[0].state_no = p->ctrl_state_no;
    learn->move.steps[0].statetype = p->ctrl_statetype;
    learn->move.steps[0].movetype = p->ctrl_movetype;
    learn->move.steps[0].ctrl = p->ctrl_ctrl;
    learn->move.step_count = 1;
    Ai_CopyCommands(p->ai_state_cmds, learn->move.cmds);
    learn->move.bounds[0][0].avg = -FLT_MAX;
    learn->move.bounds[1][0].avg = -FLT_MAX;
    return 1;
}

/* Position of the player relative to where the recorded move started (x towards the start facing). */
static int Ai_LearnDx(const Player *p, const AiLearn *learn)
{
    return Ftol((p->pos.x - learn->start_x) * learn->facing);
}

static int Ai_LearnDy(const Player *p, const AiLearn *learn)
{
    return Ftol(p->pos.y - learn->start_y);
}

/*
 * 0x0042F9C0: one tick of the move being recorded: the move ends when the
 * player is idle with control, is hit or runs custom states (Ai_FinishMove;
 * stored only if it hit something); else the timing and position of its
 * first contact (HitDef with Clsn1, or the projectile it fired), of the
 * contact by kind and of the first tick with control are noted. tick: the
 * game advanced this tick.
 */
void Ai_UpdateLearn(Player *p, int tick)
{
    AnimState *anim = p->anim;
    AiLearn *learn = (&(p)->ai_record);
    AiMove *move = &learn->move;
    HitDef *hd = &p->hitdef;
    int ticks = learn->ticks;
    int recording = 1;
    int store = 0;
    int end = 0;
    int contact;

    if (anim->elem == NULL || !p->ai_learn || !learn->active) {
        recording = 0;
    }
    if (p->movetype == MOVETYPE_IDLE && p->ctrl) {
        if (recording) {
            store = learn->active >= 2;
        }
        end = 1;
    }
    if (p->custom_states != NULL || p->movetype == MOVETYPE_HIT) {
        end = 1;
        (&(p)->ai_state)->current = NULL;
    } else if (end) {
        (&(p)->ai_state)->current = NULL;
    }
    if (p->ai_learn) {
        Ai_TrackBounds(p);
    }
    if (!recording) {
        return;
    }
    if (end) {
        Ai_FinishMove(p, store);
        return;
    }

    switch (p->move_contact) {
    case 0:
        contact = 0;
        break;
    case 1:
        contact = 1;
        break;
    case 2:
        contact = 2;
        break;
    default:
        contact = 0;
        break;
    }
    if (learn->contact == 0) {
        learn->contact = contact;
    } else {
        contact = learn->contact;
    }
    if (tick && !p->paused && g_game->pause_kind == -1) {
        Ai_TrackBox(p, move);
    }

    if (learn->active == 2) {
        if ((hd->active && anim->elem->clsn1 != NULL)
            || Projectiles_CountId((ProjectileList *)p->projectiles, learn->proj_id) != 0) {
            AiStat_Set(&move->contact_at[contact][0], ticks);
            AiStat_Set(&move->contact_at[contact][1], Ai_LearnDx(p, learn));
            AiStat_Set(&move->contact_at[contact][2], Ai_LearnDy(p, learn));
            if (hd->serial != learn->hit_serial) {
                AiStat_Set(&move->damage, Ftol(move->damage.avg) + hd->damage);
                learn->hit_serial = hd->serial;
            }
        }
        if (0.0f == move->ctrl_at[contact][0].avg && p->ctrl) {
            AiStat_Set(&move->ctrl_at[contact][0], ticks);
            AiStat_Set(&move->ctrl_at[contact][1], Ai_LearnDx(p, learn));
            AiStat_Set(&move->ctrl_at[contact][2], Ai_LearnDy(p, learn));
        }
    } else if (learn->active == 1) {
        if (hd->active && anim->elem->clsn1 != NULL) {
            int dx, dy;
            AiStat_Set(&move->hit_at[0], ticks);
            move->contact_at[contact][0] = move->hit_at[0];
            dx = Ai_LearnDx(p, learn);
            AiStat_Set(&move->hit_at[1], dx);
            move->contact_at[contact][1] = move->hit_at[1];
            dy = Ai_LearnDy(p, learn);
            AiStat_Set(&move->hit_at[2], dy);
            move->contact_at[contact][2] = move->hit_at[2];
            move->attr[0] = hd->hitflag;
            move->attr[1] = hd->guardflag;
            move->attr[2] = hd->attr;
            move->attr[3] = hd->reversal_attr;
            learn->active = 2;
        }
        if (p->last_proj_id != -1) {
            AiStat_Set(&move->hit_at[0], ticks);
            move->contact_at[contact][0] = move->hit_at[0];
            AiStat_Set(&move->hit_at[1], Ai_LearnDx(p, learn));
            move->contact_at[contact][1] = move->hit_at[1];
            AiStat_Set(&move->hit_at[2], Ai_LearnDy(p, learn));
            move->contact_at[contact][2] = move->hit_at[2];
            learn->proj_id = p->last_proj_id;
            p->last_proj_id = -1;
            learn->active = 2;
        }
    }
    if (tick && !p->paused && g_game->pause_kind == -1) {
        learn->ticks++;
    }
}

/*
 * 0x00430020: widens the Clsn1 / Clsn2 ranges of move (relative to the start
 * of the move) by the boxes of the player's current element. The ranges
 * start at the first box seen; the stored values are rounded through int.
 */
void Ai_TrackBox(Player *p, AiMove *move)
{
    AiLearn *learn = (&(p)->ai_record);
    int dx = Ftol((p->pos.x - learn->start_x) * learn->facing);
    int dy = Ftol(p->pos.y - learn->start_y);
    int k;
    int i;

    for (k = 0; k < 2; k++) {
        AnimElem *elem = p->anim->elem;
        const Clsn *clsn;
        AiStat *b;
        if (elem == NULL) {
            continue;
        }
        clsn = k == 0 ? elem->clsn1 : elem->clsn2;
        if (clsn == NULL) {
            continue;
        }
        b = move->bounds[k];
        if (b[0].avg == -FLT_MAX) {
            b[0].avg = (float)(clsn->boxes[0].x1 + dx);
            b[1].avg = (float)(clsn->boxes[0].y1 + dy);
            b[2].avg = (float)(clsn->boxes[0].x2 + dx);
            b[3].avg = (float)(clsn->boxes[0].y2 + dy);
        }
        for (i = 0; i < clsn->capacity; i++) {
            const ClsnBox *box = &clsn->boxes[i];
            int v;
            v = Ftol(b[0].avg);
            if (v > box->x1 + dx) {
                v = box->x1 + dx;
            }
            b[0].avg = (float)v;
            v = Ftol(b[1].avg);
            if (v > box->y1 + dy) {
                v = box->y1 + dy;
            }
            b[1].avg = (float)v;
            v = Ftol(b[2].avg);
            if (v < box->x2 + dx) {
                v = box->x2 + dx;
            }
            b[2].avg = (float)v;
            v = Ftol(b[3].avg);
            if (v < box->y2 + dy) {
                v = box->y2 + dy;
            }
            b[3].avg = (float)v;
        }
    }
}

/* 0x004301E0: widens the Clsn2 range of the basic state the player is in (AiData.bounds). */
void Ai_TrackBounds(Player *p)
{
    AnimElem *elem = p->anim->elem;
    const Clsn *clsn;
    AiStat *b;
    int k;
    int i;

    if (elem == NULL) {
        return;
    }
    for (k = 0; k < 4; k++) {
        if (p->state_no == ai_bound_states[k]) {
            break;
        }
    }
    if (k == 4) {
        return;
    }
    clsn = elem->clsn2;
    if (clsn == NULL) {
        return;
    }
    b = p->ai_data.bounds[k];
    if (b[0].avg == -FLT_MAX) {
        b[0].avg = (float)clsn->boxes[0].x1;
        b[1].avg = (float)clsn->boxes[0].y1;
        b[2].avg = (float)clsn->boxes[0].x2;
        b[3].avg = (float)clsn->boxes[0].y2;
    }
    for (i = 0; i < clsn->capacity; i++) {
        const ClsnBox *box = &clsn->boxes[i];
        int v;
        v = Ftol(b[0].avg);
        if (v > box->x1) {
            v = box->x1;
        }
        b[0].avg = (float)v;
        v = Ftol(b[1].avg);
        if (v > box->y1) {
            v = box->y1;
        }
        b[1].avg = (float)v;
        v = Ftol(b[2].avg);
        if (v < box->x2) {
            v = box->x2;
        }
        b[2].avg = (float)v;
        v = Ftol(b[3].avg);
        if (v < box->y2) {
            v = box->y2;
        }
        b[3].avg = (float)v;
    }
}

/*
 * 0x00430330: ends the move being recorded. With store, its final timing is
 * noted and it is added to (or merged into) the learned moves of its state
 * type. Returns 1 when it was stored.
 */
int Ai_FinishMove(Player *p, int store)
{
    AiLearn *learn = (&(p)->ai_record);
    AiMove *move = &learn->move;
    KeyedArray *list;
    int contact;
    int ticks;
    int index;

    if (!p->ai_learn) {
        return 0;
    }
    if (!store) {
        memset(learn, 0, sizeof(*learn));
        learn->active = 0;
        return 0;
    }
    ticks = learn->ticks;
    contact = learn->contact;
    AiStat_Set(&move->end_at[contact][0], ticks);
    AiStat_Set(&move->end_at[contact][1], Ftol((p->pos.x - learn->start_x) * learn->facing));
    AiStat_Set(&move->end_at[contact][2], Ftol(p->pos.y - learn->start_y));
    if (0.0f == move->ctrl_at[contact][0].avg && p->ctrl) {
        AiStat_Set(&move->ctrl_at[contact][0], ticks);
        AiStat_Set(&move->ctrl_at[contact][1], Ftol((p->pos.x - learn->start_x) * learn->facing));
        AiStat_Set(&move->ctrl_at[contact][2], Ftol(p->pos.y - learn->start_y));
    }

    list = Ai_TableFor(&p->ai_data, learn->statetype);
    if (list == NULL) {
        memset(learn, 0, sizeof(*learn));
        learn->active = 0;
        return 0;
    }
    index = KeyedArray_Find(list, 0, move->state_no);
    if (index == -1) {
        if (KeyedArray_Add(list, move, 0, move->state_no) == -1) {
            Ai_ClearLearn(&p->ai_learn);
            return 0;
        }
        KeyedArray_Get(list, 0, move->state_no);
    } else {
        Ai_MergeRecord((AiMove *)(list->data + list->elem_size * index), move, contact);
    }
    memset(learn, 0, sizeof(*learn));
    learn->active = 0;
    return 1;
}

/* 0x004305F0: tells the other computer players that p changed state. */
void Ai_LearnMove(Player *p)
{
    KeyedArray *list = Ai_TableFor(&p->ai_data, p->statetype);
    int value = list != NULL ? p->state_no : 0;

    (&(p)->ai_state)->state_msg_sent = 1;
    Ai_Broadcast(p, NULL, 0, value, list);
}

/*
 * 0x00430660: queues a message from sender (the from field of the message
 * is replaced by it) to every other active computer player with room for
 * it. Returns the number of receivers.
 */
int Ai_Broadcast(Player *sender, Player *from, int type, int value, KeyedArray *table)
{
    GameVars *g = g_game;
    int count = 0;
    int i;

    (void)from;
    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        AiState *ai;
        AiMsg *m;
        if (t == NULL || !t->active || t == sender || !t->ai) {
            continue;
        }
        ai = (&(t)->ai_state);
        if (ai->msg_count >= 10) {
            continue;
        }
        m = &ai->msgs[ai->msg_count];
        m->from = sender;
        m->type = type;
        m->value = value;
        m->table = table;
        ai->msg_count++;
        count++;
    }
    return count;
}

/* Index of the first message of type (from sender, or anyone when NULL), or -1. */
static int Ai_MsgIndex(const Player *p, const Player *sender, int type)
{
    const AiState *ai = (&(p)->ai_state);
    int i;

    for (i = 0; i < ai->msg_count; i++) {
        if (ai->msgs[i].type == type && (sender == NULL || sender == ai->msgs[i].from)) {
            return i;
        }
    }
    return -1;
}

/* 0x00430710: copies the first message of type from sender (NULL: anyone) to out; type -1 if none. */
AiMsg *Ai_FindMsg(AiMsg *out, Player *p, Player *sender, int type)
{
    int i = Ai_MsgIndex(p, sender, type);

    out->from = NULL;
    out->value = 0;
    out->table = NULL;
    out->type = -1;
    if (i != -1) {
        *out = (&(p)->ai_state)->msgs[i];
    }
    return out;
}

/* 0x00430780: as Ai_FindMsg, and marks the message taken (type 2). */
AiMsg *Ai_TakeMsg(AiMsg *out, Player *p, Player *sender, int type)
{
    int i = Ai_MsgIndex(p, sender, type);

    out->from = NULL;
    out->value = 0;
    out->table = NULL;
    out->type = -1;
    if (i != -1) {
        *out = (&(p)->ai_state)->msgs[i];
        (&(p)->ai_state)->msgs[i].type = 2;
    }
    return out;
}

/*
 * 0x00430800: the decisions of a tick: Ai_Decide and the handler of the
 * current mode run until neither changes the mode (at most 100 times).
 */
void Ai_Think(Player *p, int level)
{
    AiState *ai = (&(p)->ai_state);
    int n = 100;
    int changed;
    int r;

    do {
        changed = Ai_Decide(p, level);
        r = 0;
        switch (ai->mode) {
        case AI_MODE_IDLE:
            r = Ai_ModeIdle(p);
            break;
        case AI_MODE_MOVE:
            r = Ai_ModeMove(p);
            break;
        case AI_MODE_ATTACK:
            r = Ai_ModeAttack(p);
            break;
        case AI_MODE_GUARD:
            r = Ai_ModeGuard(p);
            break;
        case AI_MODE_FIGHT:
            r = Ai_ModeFight(p->enemy, p, level);
            break;
        }
    } while ((changed || r) && --n != 0);

    ai->mode_ticks++;
    if (ai->cooldown > 0) {
        ai->cooldown--;
    }
    if (ai->threat_valid) {
        if (ai->threat_owner != NULL && !ai->threat_owner->paused && g_game->pause_kind == -1) {
            ai->threat_ticks++;
        }
        ai->no_threat_ticks = 0;
    } else if (ai->threat_owner != NULL && ai->threat_owner->movetype == MOVETYPE_ATTACK) {
        ai->no_threat_ticks = 0;
    } else {
        ai->no_threat_ticks++;
    }
}

/* The chosen counter move: a random candidate of Ai_FindCounters. */
static int Ai_StartCounter(Player *p)
{
    AiState *ai = (&(p)->ai_state);

    ai->move = ai->candidates[Match_Random() % ai->candidate_count];
    ai->move_statetype = 0;
    ai->move_started = 0;
    Ai_SetMode(ai, AI_MODE_ATTACK);
    return 1;
}

/*
 * 0x00430900: chooses the mode: walk to the enemy while it holds Start, stop
 * at the end of a walk, start fighting freely now and then, copy the move
 * the enemy started (from its state message) as the threat, guard it when it
 * would hit, and answer it with a learned counter move. Returns 1 when the
 * mode changed.
 */
int Ai_Decide(Player *p, int level)
{
    AiState *ai = (&(p)->ai_state);
    Player *enemy = p->enemy;
    Player *owner = ai->threat_owner;
    int dist = AiAbs(Ftol(p->pos.x - enemy->pos.x));
    int mode;
    int tt;
    int contact;
    int late;
    AiMsg msg;

    if (enemy == NULL) {
        return 0;
    }
    mode = ai->mode;
    if (mode == AI_MODE_IDLE && enemy->input.keys[KEY_START]) {
        int y = Ftol(enemy->pos.y);
        int x = Ftol(enemy->pos.x - (double)(p->facing * 40));
        Ai_MakePath(p, y, x);
        Ai_SetMode(ai, AI_MODE_MOVE);
        return 1;
    }
    if (mode == AI_MODE_MOVE
        && (ai->path_index >= 1 || ai->path[ai->path_index + 1].kind == 0)
        && Ai_StepDone(p, ai->path_index)) {
        Ai_ReleaseDirs(p);
        Ai_SetMode(ai, AI_MODE_IDLE);
        return 1;
    }

    Ai_FindMsg(&msg, p, enemy, 1);
    if (msg.type != -1 || enemy->movetype == MOVETYPE_IDLE || enemy->movetype == MOVETYPE_HIT) {
        ai->threat_valid = 0;
        ai->threat_ticks = 0;
    }

    if (mode == AI_MODE_IDLE && level != 9) {
        int r = Match_Random() % 60;
        int t = ai->mode_ticks;
        if (t > 150) {
            r /= 2;
        }
        if (dist < 100) {
            r /= 2;
        }
        if (dist < 50) {
            r /= 2;
        }
        if (p->movetype == MOVETYPE_HIT) {
            if (p->gethit_hitcount > 2) {
                r /= 2;
            }
            if (p->statetype == STATETYPE_LIEDOWN) {
                r /= 2;
            }
        }
        if (r == 0 || (t > 0 && p->hitcount != 0)) {
            Ai_SetMode(ai, AI_MODE_FIGHT);
            return 1;
        }
    }

    Ai_TakeMsg(&msg, p, p->enemy, 0);
    if (msg.type != -1) {
        if (Match_Random() % 12 < level + 3 && msg.table != NULL && (&(p->enemy)->ai_state)->current != NULL) {
            ai->threat = *(&(p->enemy)->ai_state)->current;
            ai->threat_valid = 1;
            ai->threat_ticks = 0;
            ai->threat_owner = p->enemy;
        }
    }

    mode = ai->mode;
    if ((mode == AI_MODE_IDLE || mode == AI_MODE_MOVE || mode == AI_MODE_FIGHT)
        && ai->threat_valid && ai->cooldown == 0 && mode != AI_MODE_GUARD
        && !(mode == AI_MODE_ATTACK && ai->mode_ticks <= 2)) {
        tt = ai->threat_ticks;
        if (p->ctrl
            && ((double)ai->threat.contact_at[0][0].avg > (double)tt || 0.0f == ai->threat.contact_at[0][0].avg)
            && Ai_HitTest(p, p->enemy, &ai->threat) > 0) {
            ai->cooldown = 5;
            if ((unsigned int)(level - 1) <= 2) {
                ai->cooldown += Match_Random() % (4 - level);
            }
            if (Match_Random() % 20 <= level + 11) {
                Ai_SetMode(ai, AI_MODE_GUARD);
                return 1;
            }
        }
    }

    if (ai->mode != AI_MODE_GUARD) {
        return 0;
    }
    if (!ai->threat_valid || owner == NULL || !owner->active) {
        ai->prev_mode = AI_MODE_GUARD;
        ai->mode = AI_MODE_IDLE;
        ai->mode_ticks = 0;
        return 1;
    }
    tt = ai->threat_ticks;
    contact = (&(owner)->ai_record)->contact;
    if (contact == 0 && Ftol(ai->threat.contact_at[0][0].avg) == 0) {
        contact = 1;
    }
    if (contact == 1 && Ftol(ai->threat.contact_at[1][0].avg) == 0) {
        contact = 2;
    }

    Ai_FindMsg(&msg, p, p->enemy, 0);
    if (msg.type != -1) {
        ai->mode = AI_MODE_IDLE;
        ai->mode_ticks = 0;
        ai->prev_mode = AI_MODE_GUARD;
        return 1;
    }
    if (tt > 0 && tt < Ftol(ai->threat.hit_at[0].avg)) {
        int k = (tt >= 3 ? 2 : 5) + tt / 4;
        int chance = Ftol(k - 6.0 / (ai->threat.hit_at[0].avg - (double)tt));
        if (level == 9) {
            chance = 0;
        }
        if (Match_Random() % 100 < chance && Ai_FindCounters(p) > 0) {
            return Ai_StartCounter(p);
        }
    }
    if (ai->mode_ticks == 0 && ai->threat.attr[1] != (unsigned int)-1) {
        unsigned int gf = ai->threat.attr[1];
        if ((gf & 0xC) == 0xC) {
            ai->guard_crouch = p->statetype == STATETYPE_CROUCH;
        } else {
            ai->guard_crouch = (gf >> 3) & 1;
        }
    }
    late = (double)ai->threat.contact_at[contact][0].avg <= (double)tt;
    Ai_TakeMsg(&msg, p, p->enemy, 1);
    if (msg.type == -1 && !late) {
        return 0;
    }
    if (!p->ctrl) {
        return 0;
    }
    {
        int chance = level * 5;
        if (dist > 200) {
            chance /= 2;
        }
        if (dist > 150) {
            chance /= 2;
        }
        if (dist < 100) {
            chance += 10;
        }
        if (dist < 75) {
            chance += 10;
        }
        if (dist < 50) {
            chance += 10;
        }
        if ((Match_Random() % 100 < chance || level == 9) && Ai_FindCounters(p) > 0) {
            return Ai_StartCounter(p);
        }
    }
    Ai_SetMode(ai, AI_MODE_IDLE);
    return 1;
}

/*
 * 0x00430FB0: plans the way to (x, y): a walk when y is 0, else a jump,
 * preceded by a walk to the take-off point when the target is beyond the
 * jump distance.
 */
int Ai_MakePath(Player *p, int y, int x)
{
    AiState *ai = (&(p)->ai_state);
    int d = Ftol((x - p->pos.x) * p->facing);
    int jump;
    int height;

    ai->path[1].kind = 0;
    ai->path_index = 0;
    if (y == 0) {
        ai->path[0].y = 0;
        ai->path[0].x = x;
        ai->path[0].kind = 1;
        return 1;
    }
    if (d >= 0) {
        jump = Ftol(p->consts.jump_fwd_x * -2.0 * p->consts.jump_y / p->consts.yaccel);
    } else {
        jump = Ftol(p->consts.jump_back_x * -2.0 * p->consts.jump_y / p->consts.yaccel);
    }
    height = Ftol(-p->consts.jump_y * (double)p->consts.jump_y / ((double)p->consts.yaccel + p->consts.yaccel));
    if (d >= 0 ? d < jump : d > jump) {
        ai->path[0].x = x;
        ai->path[0].y = y;
        ai->path[0].kind = 2;
        return 1;
    }
    if (y > height) {
        ai->path[0].x = x - jump * y / height * p->facing;
    } else {
        ai->path[0].x = Ftol(x - jump * 0.5 * p->facing);
    }
    ai->path[0].y = 0;
    ai->path[0].kind = 1;
    ai->path[1].x = x;
    ai->path[1].y = y;
    ai->path[1].kind = 2;
    return 1;
}

/*
 * 0x00431110: the player reached the end of path step index (within a pixel
 * of x on the way from AiState.start_x, at rest or past y when jumping).
 */
int Ai_StepDone(Player *p, int index)
{
    AiState *ai = (&(p)->ai_state);
    int tx = ai->path[index].x;
    int ty = ai->path[index].y;
    int sx = ai->start_x;
    int reached;

    if (sx <= tx) {
        if ((double)(tx - 1) <= p->pos.x) {
            reached = 1;
        } else if (sx < tx) {
            reached = 0;
        } else {
            reached = (double)(tx + 1) >= p->pos.x;
        }
    } else {
        reached = (double)(tx + 1) >= p->pos.x;
    }
    if (reached) {
        if (!(0.0 == p->vel.x) || ai->path_kind != 2) {
            return 1;
        }
    }
    /* Rising: done once at or above y. */
    if (0.0 == p->vel.x && 0.0 > p->vel.y && (double)ty >= p->pos.y) {
        return 1;
    }
    if (!(0.0 == p->vel.x)) {
        return 0;
    }
    if (!(0.0 < p->vel.y)) {
        return 0;
    }
    return (double)ty <= p->pos.y;
}

/* 0x00431200: idle: crouches when asked to, lets go of the keys of an ended fight / attack. */
int Ai_ModeIdle(Player *p)
{
    AiState *ai = (&(p)->ai_state);
    InputBuf *in = &p->input;
    int keep = 0;

    if (p->enemy == NULL) {
        return 0;
    }
    if (ai->crouch_idle == 1) {
        keep = KEY_CROUCH;
    }
    if (ai->mode_ticks == 0) {
        ai->idle_ticks = 0;
        if (ai->prev_mode == AI_MODE_FIGHT) {
            Ai_ReleaseKeysExcept(p, keep);
        }
    }
    if (ai->crouch_idle == 1 && !in->keys[KEY_CROUCH]) {
        PlayerInput_Push(in, KEY_CROUCH);
    }
    if (ai->mode_ticks >= 1 && (ai->prev_mode == AI_MODE_GUARD || ai->prev_mode == AI_MODE_ATTACK)
        && !(p)->ai_threatened) {
        Ai_ReleaseDirsExcept(p, keep);
    }
    ai->idle_ticks++;
    return 0;
}

/* Holds dir (KEY_RIGHT / KEY_LEFT, -1 none) and lets go of the other directions (except skip). */
static void Ai_HoldDir(InputBuf *in, int dir, int skip)
{
    int i;

    for (i = 1; i <= 4; i++) {
        if (in->keys[i] && i != dir && i != skip) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
    if (dir != -1 && !in->keys[dir]) {
        PlayerInput_Push(in, dir);
    }
}

/* 0x004312A0: walks / jumps along the planned path. */
int Ai_ModeMove(Player *p)
{
    AiState *ai = (&(p)->ai_state);
    InputBuf *in = &p->input;
    int dir = -1;
    int index;
    int dx, dy;

    if (p->enemy == NULL) {
        return 0;
    }
    if (ai->mode_ticks == 0) {
        ai->step_ticks = 0;
    }
    for (;;) {
        index = ai->path_index;
        dx = Ftol((ai->path[index].x - p->pos.x) * p->facing);
        dy = Ftol(ai->path[index].y - p->pos.y);
        if (ai->step_ticks == 0) {
            ai->start_x = Ftol(p->pos.x);
            ai->start_y = Ftol(p->pos.y);
            ai->path_kind = ai->path[index].kind;
            break;
        }
        if (index < 1 && ai->path[index + 1].kind != 0 && Ai_StepDone(p, index)) {
            ai->path_index = index + 1;
            ai->step_ticks = 0;
            continue;
        }
        break;
    }

    if (ai->path_kind == 1) {
        if (dx > 0) {
            dir = KEY_RIGHT;
        } else if (dx < 0) {
            dir = KEY_LEFT;
        }
        Ai_HoldDir(in, dir, -1);
    } else if (ai->path_kind == 2 && p->statetype != STATETYPE_AIR) {
        if (dx > 0) {
            if ((double)p->consts.jump_fwd_x * dy > (double)p->consts.jump_y * dx) {
                dir = KEY_RIGHT;
            }
        } else if (dx < 0) {
            if ((double)p->consts.jump_back_x * dy < (double)p->consts.jump_y * dx) {
                dir = KEY_LEFT;
            }
        }
        Ai_HoldDir(in, dir, KEY_JUMP);
        if (in->keys[KEY_JUMP] == 0) {
            PlayerInput_Push(in, KEY_JUMP);
        }
    }
    ai->step_ticks++;
    return 0;
}

/*
 * 0x004314F0: does AiState.move: gets into a state type the move starts
 * from, then gives its first command; back to idle when the player is idle
 * again afterwards.
 */
int Ai_ModeAttack(Player *p)
{
    AiState *ai = (&(p)->ai_state);
    AiMove *move = ai->move;
    InputBuf *in = &p->input;
    int st;
    int i;

    if (move == NULL) {
        Ai_SetMode(ai, AI_MODE_IDLE);
        return 1;
    }
    if (ai->mode_ticks == 0) {
        ai->move_statetype = 0;
        for (i = 0; i < move->step_count; i++) {
            st = move->steps[i].statetype;
            if (st == p->statetype) {
                ai->move_statetype = st;
                break;
            }
            if (ai->move_statetype == 0) {
                ai->move_statetype = st;
            }
        }
    }
    if (!ai->move_started) {
        st = ai->move_statetype;
        if (st != 0 && p->statetype != st) {
            if (st == STATETYPE_CROUCH) {
                if (in->keys[KEY_JUMP]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
                }
                if (!in->keys[KEY_CROUCH]) {
                    PlayerInput_Push(in, KEY_CROUCH);
                }
            } else if (st == STATETYPE_STAND) {
                if (in->keys[KEY_JUMP]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
                }
                if (in->keys[KEY_CROUCH]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
                }
            } else if (st == STATETYPE_AIR) {
                if (in->keys[KEY_CROUCH]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
                }
                if (!in->keys[KEY_JUMP]) {
                    PlayerInput_Push(in, KEY_JUMP);
                }
            }
        }
        if (p->statetype == st || ai->mode_ticks > 4) {
            Ai_AddCommand((p)->ai_cmds_out, move->cmds[0]);
            ai->move_started = 1;
        }
    }
    if (p->movetype == MOVETYPE_IDLE && ai->move_started && ai->mode_ticks > 0) {
        Ai_SetMode(ai, AI_MODE_IDLE);
        return 1;
    }
    return 0;
}

/* 0x00431680: guards (holds back, crouching for low attacks) while an enemy attack is near. */
int Ai_ModeGuard(Player *p)
{
    AiState *ai = (&(p)->ai_state);
    InputBuf *in = &p->input;

    if (p->enemy == NULL) {
        Ai_SetMode(ai, AI_MODE_IDLE);
        return 1;
    }
    if ((p)->ai_threatened && p->movetype == MOVETYPE_IDLE && p->ctrl) {
        if (in->keys[KEY_JUMP]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
        }
        if (!in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, KEY_LEFT);
        }
        if (ai->guard_crouch) {
            if (!in->keys[KEY_CROUCH]) {
                PlayerInput_Push(in, KEY_CROUCH);
            }
        } else if (in->keys[KEY_CROUCH]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
        }
    }
    return 0;
}

/*
 * 0x00431740: whether the attack boxes of enemy hit the hurt boxes of p,
 * using learned box ranges where known: p's from its current learned move
 * (or its basic state ranges), enemy's from move (or its current learned
 * move). Returns 100 on a hit, else 0.
 */
int Ai_HitTest(Player *p, Player *enemy, AiMove *move)
{
    int flip_p = p->facing != 1;
    int flip_e = enemy->facing != 1;
    int dx = Ftol(enemy->pos.x - p->pos.x);
    int dy = Ftol(enemy->pos.y - p->pos.y);
    AnimElem *pe = p->anim->elem;
    AnimElem *ee;
    const Clsn *a;
    const Clsn *b;
    Clsn fake[2];
    ClsnBox boxes[2];
    int i;

    if (pe == NULL) {
        return 0;
    }
    ee = enemy->anim->elem;
    if (ee == NULL) {
        return 0;
    }
    a = pe->clsn2;
    b = ee->clsn1;
    memset(fake, 0, sizeof(fake));
    for (i = 0; i < 2; i++) {
        const Player *who;
        AiMove *m;
        AiStat *r = NULL;
        if (i == 0) {
            m = (&(p)->ai_state)->current;
            who = p;
        } else {
            m = (&(enemy)->ai_state)->current;
            who = enemy;
            if (move != NULL) {
                m = move;
            }
        }
        if (m != NULL) {
            r = m->bounds[0];
        }
        if (i == 0) {
            int k;
            for (k = 0; k < 4; k++) {
                if (who->state_no == ai_bound_states[k]) {
                    break;
                }
            }
            if (k < 4) {
                r = (AiStat *)who->ai_data.bounds[k];
            }
        }
        if (r != NULL && r[0].avg != -FLT_MAX) {
            boxes[i].hidden = 0;
            boxes[i].x1 = Ftol(r[0].avg);
            boxes[i].y1 = Ftol(r[1].avg);
            boxes[i].x2 = Ftol(r[2].avg);
            boxes[i].y2 = Ftol(r[3].avg);
            fake[i].boxes = &boxes[i];
            fake[i].capacity = 1;
            fake[i].valid = 1;
            if (i == 0) {
                a = &fake[0];
            } else {
                b = &fake[1];
            }
        }
    }
    if (a == NULL || b == NULL || a->valid == 0 || b->valid == 0) {
        return 0;
    }
    return Clsn_Collide(flip_p, flip_e, a, b, dx, dy) ? 100 : 0;
}

/*
 * 0x00431970: collects the learned moves of p's state type that would hit
 * the threat's owner in time (before the threat hits, or before its owner
 * recovers) into AiState.candidates. Returns their number.
 */
int Ai_FindCounters(Player *p)
{
    AiState *ai = (&(p)->ai_state);
    Player *owner = ai->threat_owner;
    KeyedArray *list;
    AiMove *m;
    int before_hit = 0;
    int remaining;
    int contact;
    int n = 0;
    float f;

    ai->candidate_count = 0;
    if (!ai->threat_valid) {
        return 0;
    }
    list = Ai_TableFor(&p->ai_data, p->statetype);
    if (list == NULL || owner == NULL || !owner->active) {
        return 0;
    }
    contact = (&(owner)->ai_record)->contact;
    f = (float)ai->threat_ticks;
    if (ai->threat.hit_at[0].avg > f) {
        remaining = Ftol((double)ai->threat.hit_at[0].avg - (double)f);
        before_hit = 1;
    } else {
        if (0.0f == ai->threat.ctrl_at[contact][0].avg) {
            return 0;
        }
        remaining = Ftol((double)ai->threat.ctrl_at[contact][0].avg - (double)f);
    }

    if (list->count != 0) {
        for (m = KeyedArray_First(list); m != NULL; m = KeyedArray_Next(list)) {
            int score = 100;
            unsigned int rev;
            if (n >= 49) {
                break;
            }
            if ((double)remaining <= (double)m->hit_at[0].avg) {
                score = 0;
            }
            if (Ai_HitTest(owner, p, m) == 0) {
                score -= 100;
            }
            rev = m->attr[3];
            if (rev != 0) {
                int useless = 1;
                if (before_hit) {
                    unsigned int both;
                    if (Ftol(m->contact_at[0][0].avg) < remaining) {
                        score -= 100;
                    }
                    both = ai->threat.attr[2] & rev;
                    if ((both & 7) && (both & ~7u)) {
                        useless = 0;
                    }
                }
                if (useless) {
                    score -= 100;
                }
            }
            rev = ai->threat.attr[3];
            if (rev != 0) {
                unsigned int both = m->attr[2] & rev;
                if (before_hit && (both & 7) && (both & ~7u)) {
                    score -= 100;
                }
            }
            if (!p->ctrl) {
                score -= 100;
            }
            if (score > 0) {
                ai->candidates[n++] = m;
            }
        }
    }
    ai->candidates[n] = NULL;
    ai->candidate_count = n;
    return n;
}

/* Lets go of the jump and forward keys, holds back, and crouches or stands for the guard flags of hd. */
static void Ai_GuardFor(InputBuf *in, const HitDef *hd)
{
    if ((hd->guardflag & 8) && !(hd->guardflag & 4) && !in->keys[KEY_CROUCH]) {
        PlayerInput_Push(in, KEY_CROUCH);
    }
    if (!(hd->guardflag & 8) && (hd->guardflag & 4) && in->keys[KEY_CROUCH]) {
        PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
    }
}

static void Ai_ReleaseDirKeys(InputBuf *in)
{
    int i;

    for (i = 1; i <= 4; i++) {
        if (in->keys[i]) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
}

/*
 * 0x00431BA0: fighting freely: guards when attacked, walks and jumps at
 * random, presses buttons (or, with AI.Cheat, gives random commands) more
 * often near the enemy, and recovers from falls.
 */
int Ai_ModeFight(Player *enemy, Player *p, int level)
{
    AiState *ai = (&(p)->ai_state);
    InputBuf *in = &p->input;
    InputBuf *queue = (&(p)->ai_queue);
    GameVars *g;
    int dist = AiAbs(Ftol(p->pos.x - enemy->pos.x));
    int button;
    int button2;
    AiMsg msg;

    g = g_game;
    button = AiRand() % 6 + 5;
    button2 = AiRand() % 6 + 5;
    if (level == 9) {
        Ai_TakeMsg(&msg, p, enemy, 1);
        if (msg.type != -1) {
            Ai_SetMode(ai, AI_MODE_IDLE);
            return 1;
        }
    }
    Ai_FindMsg(&msg, p, enemy, 0);
    if (msg.type != -1) {
        Ai_SetMode(ai, AI_MODE_IDLE);
        return 1;
    }
    if (AiRand() % 100 == 0) {
        button = KEY_START;
    }
    if (button2 == button || Match_Random() % 8 != 0) {
        button2 = 0;
    }

    if (((p)->ai_threatened && p->movetype == MOVETYPE_IDLE && p->ctrl) || p->movetype == MOVETYPE_HIT) {
        int t = p->state_time;
        int chance = 10;
        if (t < 2) {
            chance = level / 2 + 26;
        }
        if (t < 5) {
            chance += 5;
        }
        if (t > 10) {
            chance -= 5;
        }
        if (dist < 30) {
            chance = level / 2 + chance + 6;
            if (t < 2) {
                chance += 10;
            }
        }
        if (dist > 160) {
            chance /= 2;
        }
        if (!Player_InGuardDist(enemy, p)) {
            chance /= 5;
        }
        if (Match_Random() % 100 < (level + 6) * chance / 14) {
            if (!p->in_guard_dist) {
                int crouch = enemy->statetype != STATETYPE_CROUCH ? 25 : 75;
                if (in->keys[KEY_JUMP]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
                }
                if (in->keys[KEY_RIGHT]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
                }
                if (!in->keys[KEY_LEFT]) {
                    PlayerInput_Push(in, KEY_LEFT);
                }
                if (Match_Random() % 100 < crouch) {
                    if (!in->keys[KEY_CROUCH]) {
                        PlayerInput_Push(in, KEY_CROUCH);
                    }
                } else if (in->keys[KEY_CROUCH]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
                }
                return 0;
            }
            if (Match_Random() % 100 < 30) {
                Ai_GuardFor(in, &enemy->hitdef);
            }
            return 0;
        }
    }

    if (!(p)->ai_threatened && in->keys[KEY_LEFT]) {
        PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
    }
    if (AiRand() % 100 < 58) {
        int chance;
        int movetype;
        if (Match_Random() % 1000 < 62 && dist > 125 && !in->keys[KEY_RIGHT]) {
            Ai_ReleaseDirKeys(in);
            if (Match_Random() % 1000 < 150) {
                PlayerInput_Push(in, KEY_JUMP);
            }
            if (Match_Random() % 1000 < 750) {
                PlayerInput_Push(in, KEY_RIGHT);
            }
        }
        if (Match_Random() % 1000 < 62) {
            int k;
            Ai_ReleaseDirKeys(in);
            k = AiRand() % 19;
            if (k >= 0) {
                if (k == 0) {
                    PlayerInput_Push(in, KEY_JUMP);
                    PlayerInput_Push(in, KEY_LEFT);
                } else if (k == 1) {
                    PlayerInput_Push(in, KEY_JUMP);
                } else if (k <= 4) {
                    PlayerInput_Push(in, KEY_JUMP);
                    PlayerInput_Push(in, KEY_RIGHT);
                } else if (k <= 10) {
                    PlayerInput_Push(in, KEY_RIGHT);
                } else if (k <= 14) {
                    PlayerInput_Push(in, KEY_LEFT);
                } else if (k == 15) {
                    PlayerInput_Push(in, KEY_CROUCH);
                    PlayerInput_Push(in, KEY_LEFT);
                } else if (k <= 17) {
                    PlayerInput_Push(in, KEY_CROUCH);
                } else {
                    PlayerInput_Push(in, KEY_CROUCH);
                    PlayerInput_Push(in, KEY_RIGHT);
                }
            }
        }
        movetype = p->movetype;
        chance = 100;
        if (movetype == MOVETYPE_ATTACK) {
            chance = 150;
            if (p->move_contact == 2) {
                chance = (level + 12) * 350 / 20 + 150;
            }
        }
        if (dist < 150) {
            chance += 50;
        }
        if (dist < 100) {
            chance += 75;
        }
        if (dist < 60) {
            chance += 5 * level + 105;
        }
        if (dist < 45) {
            chance += 3 * level + 120;
        }
        if (dist < 30) {
            chance += 3 * level + 120;
        }
        chance = (level + 6) * chance / 14;
        if (movetype == MOVETYPE_HIT) {
            chance /= 10;
        }
        if (AiRand() % 1000 < chance) {
            int single;
            if (p->move_contact != 2) {
                single = 750;
            } else if (p->hitdef.attr & 0x6D8) {
                single = 250;
            } else {
                single = 500;
            }
            if (AiRand() % 1000 < single) {
                int b = Match_Random() % 6 + 5;
                PlayerInput_Push(in, in->keys[b] ? b | INPUT_CODE_RELEASE : b);
            } else {
                Ai_ReleaseDirKeys(in);
                if (AiRand() % 1000 < level * 30 + 550 && g->ai_cheat) {
                    int r = AiRand();
                    Ai_AddCommand((p)->ai_cmds_out, r % p->cmds->label_count);
                }
                PlayerInput_Push(queue, button2);
                PlayerInput_Push(queue, button2 | INPUT_CODE_RELEASE);
            }
        }
    }

    if (p->movetype == MOVETYPE_HIT && p->gethit_fall) {
        int chance = 2;
        if (-20.0 < p->pos.y) {
            chance = 20;
        }
        if (-1.0 < p->vel.y && 3.0 > p->vel.y) {
            chance += 8;
        }
        if (AiRand() % 100 < chance) {
            if (Match_Random() % 100 < 50) {
                PlayerInput_Push(queue, KEY_A);
                PlayerInput_Push(queue, KEY_B);
                return 0;
            }
            PlayerInput_Push(queue, KEY_X);
            PlayerInput_Push(queue, KEY_Y);
        }
    }
    return 0;
}

/*
 * 0x004325B0: a training dummy with cooperative control: guards (Guard =
 * auto), keeps its distance to the enemy, stands / crouches / jumps and jams
 * a button as set in the training menu. enemy is p->enemy.
 */
void Ai_TrainingDummy(Player *enemy, Player *p)
{
    InputBuf *in = &p->input;
    const HitDef *hd = &enemy->hitdef;
    int code;

    if ((p)->dummy_timer != 0) {
        (p)->dummy_timer--;
        return;
    }
    if ((p)->ai_threatened && p->movetype == MOVETYPE_IDLE && p->ctrl) {
        if (p->dummy_guard != 0) {
            if (!p->in_guard_dist) {
                if (in->keys[KEY_JUMP]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
                }
                if (in->keys[KEY_RIGHT]) {
                    PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
                }
                Ai_GuardFor(in, hd);
                if (!in->keys[KEY_LEFT]) {
                    PlayerInput_Push(in, KEY_LEFT);
                }
                return;
            }
        } else if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
    }
    if (p->in_guard_dist) {
        if (!(p)->ai_threatened && in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
        return;
    }

    if (p->dummy_dist_max != 0) {
        double dx = p->pos.x - enemy->pos.x;
        float d = (float)AiAbs(Ftol(dx));
        float dmax = (float)p->dummy_dist_max;
        int gw = Ftol(g_game->game_coord[0]);
        float lim = (float)((double)gw * dmax / 320.0);
        if (lim < d) {
            if (p->dummy_mode == 1 && (in->keys[KEY_JUMP] || in->keys[KEY_CROUCH] || in->keys[KEY_LEFT])) {
                (p)->dummy_timer = 5;
            }
            if (in->keys[KEY_JUMP]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
            }
            if (in->keys[KEY_CROUCH]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
            }
            if (in->keys[KEY_LEFT]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
            }
            if ((p)->dummy_timer == 0 && !in->keys[KEY_RIGHT]) {
                PlayerInput_Push(in, KEY_RIGHT);
            }
            return;
        }
        {
            double room = (double)Units_From320(dmax) - (double)d;
            double approach = (double)(-20 * enemy->facing) * enemy->vel.x;
            if (!(approach <= room) && !(p->vel.x * dx > 0.0)) {
                return;
            }
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
            return;
        }
    }

    if (p->dummy_dist_min != 0 && !Player_AtScreenEdge(p)) {
        double dx = p->pos.x - enemy->pos.x;
        int d = AiAbs(Ftol(dx));
        float fd = (float)d;
        float u = Units_From320((float)p->dummy_dist_min);
        if (u > fd) {
            if (p->dummy_mode == 1 && (in->keys[KEY_JUMP] || in->keys[KEY_CROUCH] || in->keys[KEY_RIGHT])) {
                (p)->dummy_timer = 5;
            }
            if (in->keys[KEY_JUMP]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
            }
            if (in->keys[KEY_CROUCH]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
            }
            if (in->keys[KEY_RIGHT]) {
                PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
            }
            if ((p)->dummy_timer == 0 && !in->keys[KEY_LEFT]) {
                PlayerInput_Push(in, KEY_LEFT);
            }
            return;
        }
        {
            double room = (double)fd - (double)u;
            double approach = (double)(20 * enemy->facing) * enemy->vel.x;
            if (!(approach <= room) && !(p->vel.x * dx < 0.0)) {
                return;
            }
        }
        if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
            return;
        }
    }

    switch (p->dummy_mode) {
    case 0:
        if (in->keys[KEY_JUMP]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
        }
        if (in->keys[KEY_CROUCH]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
        }
        if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
        }
        break;
    case 1:
        if (in->keys[KEY_JUMP]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
        }
        if (!in->keys[KEY_CROUCH]) {
            PlayerInput_Push(in, KEY_CROUCH);
        }
        if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
        }
        break;
    case 2:
        if (in->keys[KEY_CROUCH]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
        }
        if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
        }
        if (0.0 == p->pos.y) {
            if (!in->keys[KEY_JUMP]) {
                PlayerInput_Push(in, KEY_JUMP);
            }
        } else if (in->keys[KEY_JUMP]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
        }
        break;
    case 3:
        if (in->keys[KEY_CROUCH]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_CROUCH);
        }
        if (in->keys[KEY_LEFT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_LEFT);
        }
        if (in->keys[KEY_RIGHT]) {
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_RIGHT);
        }
        if (0.0 == p->pos.y || 0.0 < p->vel.y) {
            /* On the ground, or falling: jump (again). */
            if (!in->keys[KEY_JUMP]) {
                PlayerInput_Push(in, KEY_JUMP);
            }
        } else {
            /* Rising: let go (even when not held). */
            PlayerInput_Push(in, INPUT_CODE_RELEASE | KEY_JUMP);
        }
        break;
    }

    if (p->dummy_button_jam == 0) {
        return;
    }
    switch (p->dummy_button_jam) {
    case 1:
        code = KEY_A;
        break;
    case 2:
        code = KEY_B;
        break;
    case 3:
        code = KEY_C;
        break;
    case 4:
        code = KEY_X;
        break;
    case 5:
        code = KEY_Y;
        break;
    case 6:
        code = KEY_Z;
        break;
    case 7:
        code = KEY_START;
        break;
    default:
        code = p->dummy_button_jam;
        break;
    }
    if ((p)->dummy_timer != 0) {
        (p)->dummy_timer--;
        return;
    }
    if (!in->keys[code]) {
        (p)->dummy_timer = 5;
        PlayerInput_Push(in, code);
    } else {
        PlayerInput_Push(in, code | INPUT_CODE_RELEASE);
        (p)->dummy_timer = 5;
    }
}

/* 0x00432C00: some active player has p in its guard distance. */
int Ai_AnyInGuardDist(Player *p)
{
    GameVars *g = g_game;
    int i;

    for (i = g->players_first; i <= g->players_last; i++) {
        Player *t = g->players[i];
        if (t != NULL && t->active && Player_InGuardDist(t, p)) {
            return 1;
        }
    }
    return 0;
}

/* 0x00432C60: drops the move being recorded (ai_learn: &Player.ai_learn). */
void Ai_ClearLearn(int *ai_learn)
{
    Player *p = (Player *)((char *)ai_learn - offsetof(Player, ai_learn));
    AiLearn *learn = &p->ai_record;

    memset(learn, 0, sizeof(*learn));
    learn->active = 0;
}

/*
 * 0x00432C90: merges the sample move src into the learned move dst. The
 * per-contact statistics are taken from src only for contact kind (-1: all);
 * statistics dst does not have yet are copied.
 */
void Ai_MergeRecord(AiMove *dst, const AiMove *src, int contact)
{
    int i;
    int k;

    for (i = 0; i < src->step_count; i++) {
        int j;
        for (j = 0; j < dst->step_count; j++) {
            if (dst->steps[j].state_no == src->steps[i].state_no) {
                break;
            }
        }
        if (j == dst->step_count && j < 32) {
            dst->steps[j] = src->steps[i];
            dst->step_count++;
        }
    }
    dst->cmds[0] = src->cmds[0];
    dst->cmds[1] = -1;
    AiStat_Merge(&dst->hit_at[0], &src->hit_at[0]);
    AiStat_Merge(&dst->hit_at[1], &src->hit_at[1]);
    AiStat_Merge(&dst->hit_at[2], &src->hit_at[2]);

    for (i = 0; i < 3; i++) {
        if (Ftol(src->contact_at[i][0].avg) == 0 || (contact != -1 && contact != i)) {
            continue;
        }
        if (Ftol(dst->contact_at[i][0].avg) == 0) {
            dst->contact_at[i][0] = src->contact_at[i][0];
            dst->contact_at[i][1] = src->contact_at[i][1];
            dst->contact_at[i][2] = src->contact_at[i][2];
        } else {
            AiStat_Merge(&dst->contact_at[i][0], &src->contact_at[i][0]);
            AiStat_Merge(&dst->contact_at[i][1], &src->contact_at[i][1]);
            AiStat_Merge(&dst->contact_at[i][2], &src->contact_at[i][2]);
        }
    }

    for (k = 0; k < 2; k++) {
        if (src->bounds[k][0].avg == -FLT_MAX) {
            continue;
        }
        if (dst->bounds[k][0].avg == -FLT_MAX) {
            dst->bounds[k][0] = src->bounds[k][0];
            dst->bounds[k][2] = src->bounds[k][2];
            dst->bounds[k][1] = src->bounds[k][1];
            dst->bounds[k][3] = src->bounds[k][3];
        } else {
            AiStat_Merge(&dst->bounds[k][0], &src->bounds[k][0]);
            AiStat_Merge(&dst->bounds[k][2], &src->bounds[k][2]);
            AiStat_Merge(&dst->bounds[k][1], &src->bounds[k][1]);
            AiStat_Merge(&dst->bounds[k][3], &src->bounds[k][3]);
        }
    }

    if (dst->attr[0] != src->attr[0]) {
        dst->attr[0] = (unsigned int)-1;
    }
    if (dst->attr[1] != src->attr[1]) {
        dst->attr[1] = (unsigned int)-1;
    }
    if (dst->attr[2] != src->attr[2] && dst->attr[2] != 0 && src->attr[2] != 0) {
        dst->attr[2] = (unsigned int)-1;
    }
    if (dst->attr[3] != src->attr[3] && dst->attr[3] != 0 && src->attr[3] != 0) {
        dst->attr[3] = (unsigned int)-1;
    }
    AiStat_Merge(&dst->damage, &src->damage);

    for (i = 0; i < 3; i++) {
        if (Ftol(src->ctrl_at[i][0].avg) != 0 && (contact == -1 || contact == i)) {
            if (Ftol(dst->ctrl_at[i][0].avg) == 0) {
                memcpy(dst->ctrl_at[i], src->ctrl_at[i], sizeof(dst->ctrl_at[i]));
            } else {
                AiStat_Merge(&dst->ctrl_at[i][0], &src->ctrl_at[i][0]);
                AiStat_Merge(&dst->ctrl_at[i][1], &src->ctrl_at[i][1]);
                AiStat_Merge(&dst->ctrl_at[i][2], &src->ctrl_at[i][2]);
            }
        }
        if (!(0.0f == src->end_at[i][0].avg)) {
            if (0.0f == dst->end_at[i][0].avg) {
                memcpy(dst->end_at[i], src->end_at[i], sizeof(dst->end_at[i]));
            } else {
                AiStat_Merge(&dst->end_at[i][0], &src->end_at[i][0]);
                AiStat_Merge(&dst->end_at[i][1], &src->end_at[i][1]);
                AiStat_Merge(&dst->end_at[i][2], &src->end_at[i][2]);
            }
        }
    }
}

/* 0x00433B90: lets go of the four direction keys. */
void Ai_ReleaseDirs(Player *p)
{
    InputBuf *in = &p->input;
    int i;

    for (i = 1; i <= 4; i++) {
        if (in->keys[i]) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
}

/* 0x00433D10: lets go of the direction keys except keep. */
void Ai_ReleaseDirsExcept(Player *p, int keep)
{
    InputBuf *in = &p->input;
    int i;

    for (i = 1; i <= 4; i++) {
        if (i != keep && in->keys[i]) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
}

/*
 * Held state of key i for i = 1..13: index 13 is one past InputBuf.keys and
 * reads InputBuf.read, as in the original (a nonzero read position is "let
 * go" like a key, and PlayerInput_Push then clears it).
 */
static int Ai_KeyHeld(const InputBuf *in, int i)
{
    return ((const int *)((const char *)in + 0x78))[i];
}

/* 0x00433D40: lets go of every key (and of "key" 13, see Ai_KeyHeld). */
void Ai_ReleaseAll(Player *p)
{
    InputBuf *in = &p->input;
    int i;

    for (i = 1; i <= 13; i++) {
        if (Ai_KeyHeld(in, i)) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
}

/* 0x00433DD0: lets go of every key except keep (and of "key" 13, see Ai_KeyHeld). */
void Ai_ReleaseKeysExcept(Player *p, int keep)
{
    InputBuf *in = &p->input;
    int i;

    for (i = 1; i <= 13; i++) {
        if (i != keep && Ai_KeyHeld(in, i)) {
            PlayerInput_Push(in, i | INPUT_CODE_RELEASE);
        }
    }
}
