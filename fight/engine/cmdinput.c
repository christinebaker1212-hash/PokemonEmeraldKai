/*
 * Command recognition (original block 0x478610-0x478CBA, no strings): the
 * input history of a player (CmdState) as CMD_KEY_* entries with the frame
 * they happened at and how long a released key was held, and the matching
 * of the .cmd commands (CmdSet) against it.
 */
#include "mugen.h"

#define CMD_HISTORY_SIZE    128

/* The key bits of an entry. */
#define CMD_KEY_MASK        0xFFFF
/* Entry made by the game (input code bit 0x20: resynchronisation, facing change); never a press. */
#define CMD_SYNTHETIC       0x100000

/*
 * 0x004789A0: the CMD_KEY_* of an input code (KEY_* in bits 0-3): with
 * CMD_RELEASE for a release (0x10) and CMD_SYNTHETIC for a code made by the
 * game (0x20); 0 for no key.
 */
unsigned int Cmd_KeyFromInput(unsigned int code)
{
    static const unsigned int keys[11] = {
        CMD_KEY_U, CMD_KEY_F, CMD_KEY_D, CMD_KEY_B_DIR, CMD_KEY_A, CMD_KEY_B, CMD_KEY_C, CMD_KEY_X, CMD_KEY_Y,
        CMD_KEY_Z, CMD_KEY_S
    };
    unsigned int r = 0;
    unsigned int k;

    if (code & 0x10) {
        r = CMD_RELEASE;
    }
    if (code & 0x20) {
        r |= CMD_SYNTHETIC;
    }
    k = (code & 0xF) - 1;
    if (k > 10) {
        return r;
    }
    return r | keys[k];
}

/*
 * 0x00478C50: adds an entry to the input history (the oldest one is
 * dropped when it is full).
 */
void CmdState_Push(CmdState *st, unsigned int keys, unsigned int frame, int charge)
{
    st->keys[st->write] = keys;
    st->frames[st->write] = frame;
    st->charge[st->write] = charge;
    st->write++;
    if (st->write >= CMD_HISTORY_SIZE) {
        st->write = 0;
    }
    if (st->write == st->first) {
        st->first++;
        if (st->first >= CMD_HISTORY_SIZE) {
            st->first = 0;
        }
    }
}

/*
 * 0x00478610: adds an input code (or 0 to flush) to the history at frame.
 * A direction change becomes an entry with every direction held (the
 * previous combination is released first); buttons pressed without a
 * direction change wait in `pending` and are added together with the next
 * flush. Releases carry how long the key was held. Returns 1 when an entry
 * was added.
 */
int CmdState_AddInput(unsigned int key, CmdState *st, InputBuf *buf, unsigned int frame)
{
    unsigned int code = 0;
    unsigned int game_made, release;
    int charge = 0;
    int added = 0;
    int i;

    if (key != 0) {
        code = Cmd_KeyFromInput(key);
        if (code == 0) {
            return 0;
        }
    }
    game_made = code >> 20 & 1;
    release = code >> 16 & 1;
    for (i = 0; i < 16; i++) {
        if (code & (1u << i)) {
            if (!release) {
                st->press_frame[i] = frame;
            } else {
                charge = (int)(frame - st->press_frame[i]);
            }
            break;
        }
    }
    if (code != 0) {
        unsigned int low = code & CMD_KEY_MASK;
        if (low == CMD_KEY_U || low == CMD_KEY_F || low == CMD_KEY_D || low == CMD_KEY_B_DIR) {
            unsigned int dirs = 0;
            unsigned int prev;
            if (buf->keys[KEY_JUMP]) {
                dirs = CMD_KEY_U;
            }
            if (buf->keys[KEY_RIGHT]) {
                dirs |= CMD_KEY_F;
            }
            if (buf->keys[KEY_LEFT]) {
                dirs |= CMD_KEY_B_DIR;
            }
            if (buf->keys[KEY_CROUCH]) {
                dirs |= CMD_KEY_D;
            }
            prev = st->dirs;
            if (dirs != prev) {
                if (!release) {
                    code = dirs;
                    if (prev != 0) {
                        CmdState_Push(st, prev | CMD_RELEASE, frame, charge);
                        added = 1;
                    }
                } else {
                    if (prev != 0) {
                        CmdState_Push(st, prev | CMD_RELEASE, frame, charge);
                        added = 1;
                    }
                    code = dirs;
                    if (game_made) {
                        code |= CMD_SYNTHETIC;
                    }
                }
                st->dirs = dirs;
            }
        }
    }
    if ((code & 0x9F000) == 0) {
        unsigned int pending = st->pending;
        if (code != 0) {
            st->pending = pending == 0 ? code : (pending | code);
            code = 0;
        } else if (pending != 0) {
            code = pending;
            st->pending = 0;
        }
    }
    if (code & CMD_KEY_MASK) {
        CmdState_Push(st, code, frame, charge);
        added = 1;
    }
    return added;
}

/*
 * 0x00478A50: history entry i matches one step's keys: held keys ("/")
 * against CmdState.held, else the entry itself, with the charge time
 * ("~30") and the 4-way ("$") rule for directions.
 */
int CmdStep_Match(unsigned int keys, const CmdState *st, int i)
{
    unsigned int e, rel, d, h;

    if (keys & CMD_ANY_DIR) {
        d = keys & CMD_KEY_DIRS;
        if (d == 0) {
            if ((keys & 0xFFF) == 0) {
                return 0;
            }
            return (st->held & keys & 0xFFF) != 0;
        }
        h = st->held & CMD_KEY_DIRS;
        if ((d & CMD_KEY_N) && h == 0) {
            return 1;
        }
        if (keys & CMD_4WAY) {
            h &= d;
        }
        return h == d;
    }
    e = st->keys[i];
    if (st->charge[i] < ((int)keys >> CMD_CHARGE_SHIFT & ~3)) {
        return 0;
    }
    rel = e & CMD_RELEASE;
    if (rel == 0 && (e & CMD_SYNTHETIC)) {
        return 0;
    }
    if (keys & CMD_KEY_DIRS) {
        d = keys & 0x1FFFF;
        if (!(keys & CMD_4WAY)) {
            return e == d;
        }
        if ((e & d) != d) {
            return 0;
        }
        return rel == (d & CMD_RELEASE);
    }
    if ((keys & 0xFFF) == 0) {
        return 0;
    }
    if ((keys & CMD_RELEASE) != rel) {
        return 0;
    }
    d = keys & CMD_KEY_MASK;
    return (e & d) == d;
}

/*
 * 0x00478B30: the command was entered up to the history position
 * CmdState.pos: its steps found backwards within cmd->time frames of the
 * last one (held steps may span more), strict steps ('>') with no other
 * entry in between. *frame_out gets the frame of the first step.
 */
int Cmd_Match(const CmdEntry *cmd, const CmdState *st, unsigned int *frame_out)
{
    const CmdStep *steps = cmd->steps;
    int s = cmd->step_count - 1;
    int time = cmd->time;
    unsigned int strict = 0;
    int pos = st->pos;
    unsigned int last;

    if (pos > 0) {
        last = st->frames[pos - 1];
        if (!CmdStep_Match(steps[s].keys, st, pos - 1)) {
            return 0;
        }
    } else {
        last = st->frames[CMD_HISTORY_SIZE - 1];
        if (!CmdStep_Match(steps[s].keys, st, CMD_HISTORY_SIZE - 1)) {
            return 0;
        }
    }
    if (pos == st->first) {
        return 0;
    }
    for (;;) {
        int diff;
        pos--;
        if (pos < 0) {
            pos = CMD_HISTORY_SIZE - 1;
        }
        diff = (int)(last - st->frames[pos]);
        if (diff > time && !(steps[s].keys & CMD_ANY_DIR)) {
            return 0;
        }
        if (CmdStep_Match(steps[s].keys, st, pos)) {
            s--;
            strict = steps[s + 1].flags;
            if (s < 0) {
                if (frame_out != NULL) {
                    *frame_out = st->frames[pos];
                }
                return 1;
            }
        } else {
            if (strict & CMD_STEP_STRICT) {
                return 0;
            }
            strict = 0;
        }
        if (diff > time && (steps[s].keys & CMD_ANY_DIR)) {
            return 0;
        }
        if (pos == st->first) {
            return 0;
        }
    }
}

/*
 * 0x004787E0: matches the commands of the given types (CMD_TYPE_*) at every
 * history position not seen yet: a match newer than the last one of its
 * label activates it for its buffer.time (active: Player.cmd_active, the
 * frame of the last activation by label follows it at +0x80). Commands of a
 * single held step are checked once more at the current position.
 * Returns 0.
 */
int CmdState_Update(const CmdSet *cmds, unsigned char *active, CmdState *st, unsigned int frame,
                    unsigned int type_mask)
{
    int *active_frame = (int *)(active + CMD_MAX_LABELS);
    unsigned int matched = 0;
    int i;

    while (st->pos != st->write) {
        st->pos++;
        if (st->pos >= CMD_HISTORY_SIZE) {
            st->pos = 0;
        }
        for (i = 0; i < cmds->count; i++) {
            const CmdEntry *e = &cmds->entries[i];
            if ((e->type & type_mask) && Cmd_Match(e, st, &matched) && (int)matched > active_frame[e->label]) {
                active[e->label] = (unsigned char)e->buffer_time;
                active_frame[e->label] = (int)frame;
            }
        }
    }
    for (i = 0; i < cmds->count; i++) {
        const CmdEntry *e = &cmds->entries[i];
        if ((e->type & type_mask) && (e->steps[0].keys & CMD_ANY_DIR) && e->step_count == 1 &&
            Cmd_Match(e, st, &matched)) {
            active[e->label] = 1;
            active_frame[e->label] = (int)frame;
        }
    }
    return 0;
}

/* 0x00478900: CMD_KEY_* of the keys held in the input buffer. */
unsigned int Input_GetHeld(const InputBuf *buf)
{
    unsigned int held = 0;

    if (buf->keys[KEY_JUMP]) {
        held = CMD_KEY_U;
    }
    if (buf->keys[KEY_RIGHT]) {
        held |= CMD_KEY_F;
    }
    if (buf->keys[KEY_CROUCH]) {
        held |= CMD_KEY_D;
    }
    if (buf->keys[KEY_LEFT]) {
        held |= CMD_KEY_B_DIR;
    }
    if (buf->keys[KEY_A]) {
        held |= CMD_KEY_A;
    }
    if (buf->keys[KEY_B]) {
        held |= CMD_KEY_B;
    }
    if (buf->keys[KEY_C]) {
        held |= CMD_KEY_C;
    }
    if (buf->keys[KEY_X]) {
        held |= CMD_KEY_X;
    }
    if (buf->keys[KEY_Y]) {
        held |= CMD_KEY_Y;
    }
    if (buf->keys[KEY_Z]) {
        held |= CMD_KEY_Z;
    }
    if (buf->keys[KEY_START]) {
        held |= CMD_KEY_S;
    }
    return held;
}
