package com.moemon.companion.core;

/**
 * A Moemon's POKé BALL as the game keeps it (docs/PHONE_LINK.md "The POKé BALL page"): the ball style (mp_ballstyle.c:
 * a color for a POKé BALL, a decoration, or a special ball) and the entrance / recall effect (mp_ballfx.c
 * MP_BallFx_Pack: SEND-OUT, PARTICLES, ENERGY color, RETURN in 24 bits, the SOUND apart). Android-free, so the
 * desktop tests use it too.
 */
public final class BallFx {
    public static final int KIND_COLOR = 0, KIND_FORM = 1, KIND_PART = 2, KIND_RECALL = 3;   // MPFX_PIECE_*
    public static final int SPECIAL_COUNT = 13;

    public int form, part, color, recall, sound;

    public BallFx() {
    }

    public BallFx(BallFx o) {
        form = o.form;
        part = o.part;
        color = o.color;
        recall = o.recall;
        sound = o.sound;
    }

    public static BallFx unpack(long v, int sound) {
        BallFx f = new BallFx();
        int ext = (int) ((v >> 16) & 0xFF);
        f.form = (int) (v & 0xF) | ((ext & 1) << 4);
        f.part = (int) ((v >> 4) & 0xF) | (((ext >> 1) & 1) << 4);
        f.color = (int) ((v >> 8) & 0xF) | (((ext >> 2) & 3) << 4);
        f.recall = (int) ((v >> 12) & 0x7) | (((ext >> 4) & 7) << 3);
        f.sound = sound & 7;
        return f;
    }

    public long pack() {
        long base = (form & 0xF) | ((part & 0xF) << 4) | ((color & 0xF) << 8) | ((recall & 0x7) << 12);
        long ext = ((form >> 4) & 1) | (((part >> 4) & 1) << 1) | (((color >> 4) & 3) << 2) | (((recall >> 3) & 7) << 4);
        return base | (ext << 16);
    }

    public int get(int kind) {
        return kind == KIND_COLOR ? color : kind == KIND_FORM ? form : kind == KIND_PART ? part : recall;
    }

    public void set(int kind, int v) {
        if (kind == KIND_COLOR) {
            color = v;
        } else if (kind == KIND_FORM) {
            form = v;
        } else if (kind == KIND_PART) {
            part = v;
        } else {
            recall = v;
        }
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof BallFx && ((BallFx) o).pack() == pack() && ((BallFx) o).sound == sound;
    }

    @Override
    public int hashCode() {
        return (int) pack() * 31 + sound;
    }

    // ------------------------------------------------------------------------------- the ball style (16 bits)

    /** The color of a POKé BALL (MP_BallStyle_Color) */
    public static int styleColor(int style) {
        int c = (style & 0xF) | (((style >> 11) & 1) << 4) | (((style >> 12) == 0xF ? 1 : 0) << 5);
        return c < 41 ? c : 0;
    }

    public static int styleDeco(int style) {
        return (style >> 4) & 0x7F;
    }

    /** Its special ball (1-13), or 0 */
    public static int styleSpecial(int style) {
        int s = style >> 12;
        return s <= SPECIAL_COUNT ? s : 0;
    }

    /** MP_BallStyle_Make | MP_BallStyle_ForItem */
    public static int makeStyle(int color, int deco, int special) {
        return (color & 0xF) | ((deco & 0x7F) << 4) | (((color >> 4) & 1) << 11)
                | (((color >> 5) & 1) != 0 ? 0xF << 12 : 0) | (special << 12);
    }

    /** Whether the bit for piece i is set in the snapshot's "fxu" hex for a kind (the first piece in the first bit) */
    public static boolean unlocked(String hex, int i) {
        if (hex == null || i / 4 >= hex.length()) {
            return i == 0;
        }
        return ((Character.digit(hex.charAt(i / 4), 16) >> (i % 4)) & 1) != 0;
    }
}
