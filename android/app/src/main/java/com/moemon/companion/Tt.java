package com.moemon.companion;

/**
 * TRIPLE TRIAD with the game's own engine (src/tt through cpp/tt_jni.c): the catalog, the rules' words, duels (a
 * handle each: start, play, the AI, state as JSON), opponents fitted as the game's CARD SCALING does, the daily puzzle.
 */
public final class Tt {
    static { System.loadLibrary("moemonlink"); }

    public static native String catalog();   // [{id, n, s:[up,right,down,left], st, ty, o, r, t}]
    public static native String words();     // {rules:[[name, help]], trades, types, packs, regions:[[name, rules, trade]]}
    public static native int handPower(int[] hand);
    public static native int chance(int mine, int theirs);
    public static native String opponent(int region, int hash, int tier, int day, int mine, int rules);
    public static native long start(int rules, int trade, int seed, int[] hand0, int[] hand1, int first);
    public static native void end(long duel);
    public static native long copy(long duel);
    public static native String play(long duel, int slot, int cell);   // its events, null if it can't be played
    public static native int playable(long duel);
    public static native String state(long duel, int seat);
    public static native int[] ai(long duel, int level, int salt);
    public static native String puzzle(int day);
    public static native int[] best(long duel);

    public static final int RULE_OPEN = 0, RULE_SAME = 1, RULE_PLUS = 2, RULE_RANDOM = 3, RULE_SUDDEN_DEATH = 4,
            RULE_SAME_WALL = 5, RULE_ELEMENTAL = 6, RULE_THREE_OPEN = 7, RULE_REVERSE = 8, RULE_FALLEN_ACE = 9,
            RULE_ASCENSION = 10, RULE_DESCENSION = 11, RULE_ORDER = 12, RULE_CHAOS = 13, RULE_SWAP = 14;
    public static final int EV_PLACE = 0, EV_RULE = 1, EV_FLIP = 2, EV_SHIFT = 3, EV_ROUND = 4, EV_END = 5;

    public static int bit(int rule) {
        return 1 << rule;
    }

    private Tt() {}
}
