/*
 * Player input buffer (original module 0x477AC0-0x477EB6, no strings): the
 * ring of input codes of a player (Player.input) and the held state of each
 * KEY_*, and the matching of configured keys with input codes.
 *
 * An input code is KEY_* in bits 0-3, 0x10 for a release and 0x20 for a
 * change made by the game itself (resynchronisation, facing changes).
 * Configured keys (GameVars.input_keys) are SDL key codes, with 0x200 for a
 * joystick button; Input_KeyCode turns them into the codes of g_inputState.
 */
#include "mugen.h"

#define INPUT_CODE_RELEASE  0x10
#define INPUT_CODE_AUTO     0x20

/* The input code of a configured key: 0x280 for none, joystick buttons get 0x1000. */
static unsigned int Input_KeyCode(unsigned int key)
{
    unsigned int code = key & ~0x200u;

    if (code == 0x200 || code == 0) {
        return 0x280;
    }
    if (key & 0x200) {
        return code | 0x1000;
    }
    return code;
}

/* Left and right swapped in the KEY_* bits of a code, as when facing left. */
static unsigned int Input_FlipCode(unsigned int code)
{
    if ((code & 0xF) == KEY_LEFT) {
        return (code & ~0xDu) | KEY_RIGHT;
    }
    if ((code & 0xF) == KEY_RIGHT) {
        return (code & ~0xBu) | KEY_LEFT;
    }
    return code;
}

/* 0x00477AC0: releases every key (KEY_JUMP..12) without queuing anything. */
void Input_Clear(InputBuf *buf)
{
    int i;

    for (i = 1; i < KEY_COUNT; i++) {
        buf->keys[i] = 0;
    }
}

/*
 * 0x00477D00: queues an input code (unless locked or without a key) and
 * updates the held state of its key. A full ring drops its oldest entry.
 */
void PlayerInput_Push(InputBuf *in, int code_)
{
    unsigned int code = (unsigned int)code_;

    if ((code & 0xF) == 0 || in->locked) {
        return;
    }
    in->ring[in->write] = code;
    in->write++;
    if (in->write >= PLAYER_INPUT_RING_SIZE) {
        in->write = 0;
    }
    if (in->write == in->read) {
        in->read++;
        if (in->read >= PLAYER_INPUT_RING_SIZE) {
            in->read = 0;
        }
    }
    in->keys[code & 0xF] = ~(code >> 4) & 1;
}

/*
 * 0x00477B10: queues a press or release (with INPUT_CODE_AUTO) for every key
 * whose held state differs from the state of its configured key in
 * g_inputState (keys: the player's GameVars.input_keys row).
 */
void Input_Resync(InputBuf *buf, const int *keys)
{
    unsigned int i;

    for (i = 1; i < KEY_COUNT; i++) {
        unsigned int k = buf->flipped ? Input_FlipCode(i) : i;
        unsigned int code = Input_KeyCode((unsigned int)keys[i]);
        int held = buf->keys[k] != 0;
        if (held != (g_inputState[code] != 0)) {
            unsigned int c = (held ? INPUT_CODE_RELEASE : 0) | i | INPUT_CODE_AUTO;
            if (buf->flipped) {
                c = Input_FlipCode(c);
            }
            PlayerInput_Push(buf, (int)c);
        }
    }
}

/*
 * 0x00477C40: sets the facing flip of the buffer. A held left or right
 * (but not both) is released and pressed again as the other direction so
 * that forward / back stay right.
 */
void Input_SetFlipped(InputBuf *buf, int flipped)
{
    int old = buf->flipped;

    if (flipped == old) {
        return;
    }
    if (!buf->locked && (!buf->keys[KEY_LEFT] || !buf->keys[KEY_RIGHT])) {
        if (buf->keys[old ? KEY_RIGHT : KEY_LEFT]) {
            PlayerInput_Push(buf, old ? 0x32 : 0x34);
            if (buf->flipped) {
                PlayerInput_Push(buf, 0x24);
                buf->flipped = flipped;
                return;
            }
            PlayerInput_Push(buf, 0x22);
        } else if (buf->keys[old ? KEY_LEFT : KEY_RIGHT]) {
            PlayerInput_Push(buf, old ? 0x34 : 0x32);
            PlayerInput_Push(buf, buf->flipped ? 0x22 : 0x24);
        }
    }
    buf->flipped = flipped;
}

/* 0x00477D70: takes the oldest queued code, 0 when the ring is empty. */
unsigned int Input_Pop(InputBuf *buf)
{
    unsigned int code;

    if (buf->read == buf->write) {
        return 0;
    }
    code = buf->ring[buf->read];
    buf->read++;
    if (buf->read >= PLAYER_INPUT_RING_SIZE) {
        buf->read = 0;
    }
    return code;
}

/* 0x00477DA0: code is the input code of one of the configured buttons KEY_A..KEY_Z. */
int Keys_IsAttackCode(const int *keys, unsigned int code)
{
    int i;

    for (i = KEY_A; i <= KEY_Z; i++) {
        if (code == Input_KeyCode((unsigned int)keys[i])) {
            return 1;
        }
    }
    return 0;
}

/* 0x00477DF0: code is the input code of a configured button (KEY_A..KEY_Z or KEY_START). */
int Keys_IsButtonCode(const int *keys, int code)
{
    if (Keys_IsAttackCode(keys, (unsigned int)code)) {
        return 1;
    }
    return (unsigned int)code == Input_KeyCode((unsigned int)keys[KEY_START]);
}

/*
 * 0x00477E50: the KEY_* of a code (bit 0x1000 marks a release, giving
 * INPUT_CODE_RELEASE) among the configured keys; 0 when none matches.
 */
unsigned int Keys_FindCode(unsigned int code, const int *keys)
{
    unsigned int release = (code & 0x1000) ? INPUT_CODE_RELEASE : 0;
    unsigned int i;

    code &= ~0x1000u;
    for (i = 1; i < KEY_COUNT; i++) {
        if (code == Input_KeyCode((unsigned int)keys[i])) {
            return i == 0 ? 0 : (i | release);
        }
    }
    return 0;
}
