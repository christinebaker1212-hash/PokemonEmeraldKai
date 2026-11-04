package com.moemon.companion;

/**
 * The POKéMON card game with the game's own engine (src/tcg through cpp/tcg_jni.c): the catalog, deck checks and
 * CARD SCALING's ratings, duels (a handle each: state, what can be done, events as JSON; the AI's moves) and flows
 * (src/tcg/tcg_flow.c, the duel board's choices: begin a move, answer its questions, apply the action it builds).
 * Actions are small JSON objects {"t","h","s","a","g","o","c","x":[...]} that also travel to friends.
 */
public final class Tcg {
    static { System.loadLibrary("moemonlink"); }

    // the tables
    public static native String catalog();   // [{id, n, k, t, ra, set, set2, h, d, st, hp, lv, rc, w, r, sp, ef, a:[{n, c[7], dmg, cat, txt}], txt}]
    public static native String packs();     // [[card ids a booster can hold]...]
    public static native int attackSound(int card, int attack);   // an SE_ number (the game board's sound)
    public static native String validate(int[] deck, int[] owned);   // why not, or null
    public static native int rateDeck(int[] deck);                   // slow: off the main thread
    public static native int levelRating(int level);
    public static native int expected(int rating, int other);        // the chance in 1/1000
    public static native String opponent(int mine, int badges, int pick);   // {index, name, rating, level, deck[60]}
    public static native int[] namedDeck(String name);

    // duels
    public static native long start(int[] deck0, int[] deck1, int seed, int prizes);   // 0 if the decks can't play
    public static native void end(long duel);
    public static native long copy(long duel);
    public static native int actor(long duel);                     // whose move (-1: over)
    public static native String state(long duel, int seat);
    public static native String events(long duel, int from, int seat);   // [[seq, type, player, slot, flags, value, card]]
    public static native String options(long duel, int seat);
    public static native String apply(long duel, String action);   // {"ok", "error"}
    public static native String ai(long duel, int level);           // an action
    public static native int cardOf(long duel, int player, int physical);
    public static native int hash(long duel);

    // flows
    public static native long flowNew();
    public static native void flowEnd(long flow);
    public static native String flowBegin(long flow, long duel, int me, int kind, int index);
    public static native String flowSlot(long flow, long duel, int slot);
    public static native String flowPick(long flow, long duel, int[] chosen);
    public static native String flowMenu(long flow, long duel, int value);

    public static final int POKEMON = 0, ENERGY = 1, TRAINER = 2;
    public static final int FIRE = 0, GRASS = 1, LIGHTNING = 2, WATER = 3, FIGHTING = 4, PSYCHIC = 5, COLORLESS = 6,
            NONE = 255;
    public static final int SETUP = 0, TURN = 1, PROMOTION = 2, FINISHED = 3;
    public static final int HEALTHY = 0, ASLEEP = 1, PARALYZED = 2, CONFUSED = 3;
    // flows
    public static final int FLOW_HAND = 0, FLOW_ATTACK = 1, FLOW_RETREAT = 2, FLOW_POWER = 3, FLOW_PROMOTE = 4,
            FLOW_DISCARD = 5, FLOW_PASS = 6, FLOW_READY = 7, FLOW_CONCEDE = 8;
    // actions
    public static final int A_PLAY_BASIC = 0, A_EVOLVE = 1, A_ATTACH = 2, A_ATTACK = 3, A_RETREAT = 4, A_TRAINER = 5,
            A_POWER = 6, A_PASS = 7, A_PROMOTE = 8, A_SETUP_DONE = 9, A_CONCEDE = 10, A_DISCARD_DOLL = 11;
    // events
    public static final int EV_TURN = 1, EV_DRAW = 2, EV_ATTACK = 3, EV_COIN = 4, EV_DAMAGE = 5, EV_HEAL = 6,
            EV_STATUS = 7, EV_KO = 8, EV_PRIZE = 9, EV_TRAINER = 10, EV_PLAY = 11, EV_EVOLVE = 12, EV_ATTACH = 13,
            EV_RETREAT = 14, EV_SWITCH = 15, EV_PROMOTE = 16, EV_NO_DAMAGE = 17, EV_SETUP_DONE = 18,
            EV_SUDDEN_DEATH = 19, EV_WIN = 20, EV_RETREAT_FAIL = 21, EV_POWER = 22, EV_PEEK = 23, EV_DEVOLVE = 24,
            EV_TO_HAND = 25, EV_MORPH = 26, EV_DECK_CHANGED = 27;
    public static final int DMG_WEAK = 1, DMG_RESIST = 2, DMG_POISON = 4, DMG_SELF = 8;
    public static final int PACKS = 11, DECKS = 8, DECK = 60;
    public static final String[] PACK_NAMES = {"COLOSSEUM", "EVOLUTION", "MYSTERY", "LABORATORY", "BEGINNING POKéMON",
            "LEGENDARY POWER", "ISLAND OF FOSSIL", "PSYCHIC BATTLE", "SKY-FLYING POKéMON", "WE ARE TEAM ROCKET",
            "TEAM ROCKET'S AMBITION"};
    public static final String[] TYPE_NAMES = {"FIRE", "GRASS", "LIGHTNING", "WATER", "FIGHTING", "PSYCHIC", "COLORLESS"};

    private Tcg() {}
}
