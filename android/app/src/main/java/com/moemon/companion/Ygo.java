package com.moemon.companion;

/**
 * YU-GI-OH! with the game's own engine (EDOPro's ocgcore with every card's script and the game's bridge, src/ygo,
 * through cpp/ygo_jni.c): the catalog and sets, deck checks and power ratings, the GBA games' duelists, and duels by
 * handle. The engine asks (query: one question at a time, its items); the answer is picks (item indices) and, for a
 * card put on the field from the hand, a zone. A duel is its decks, seed and answers: friends exchange only answers.
 * setPak gives the engine its pack (assets game/ygo_edo.pak) once, before anything else.
 */
public final class Ygo {
    static { System.loadLibrary("moemonlink"); }

    public static native boolean setPak(byte[] pak);
    public static native String catalog();      // [{id, n, t, atk, def, f, a, r, lv, p, ab, lim, ra, ok, s}]
    public static native String extraCard(int id);
    public static native String sets();         // [{code, name, short, starter, ok, cards [[id, rarity, copies]]}]
    public static native String validate(int[] main, int[] extra, int[] owned);   // why not, or null
    public static native String useBanlist(String name);
    public static native int deckRating(int[] main);
    public static native int levelRating(int level);
    public static native int chance(int mine, int theirs);
    public static native String opponent(int target, int quarter, int pick);   // {index, name, game, rating, main, extra}
    public static native String setDeck(int set);   // {main, extra}

    public static native long start(int[] main0, int[] extra0, int[] main1, int[] extra1, int seed, int first);
    public static native void end(long duel);
    public static native String state(long duel, int seat);
    public static native String query(long duel);
    public static native int[] names(long duel);
    public static native String answer(long duel, int[] picks, int zone);   // {"ok", "error"}
    public static native boolean check(long duel, int[] picks, int zone);
    public static native String ai(long duel, int level);   // {"p": [...], "z"}
    public static native boolean concede(long duel, int player);
    public static native int hash(long duel);
    public static native int answers(long duel);
    public static native String events(long duel, int from, int seat);

    // questions (YgoQueryType)
    public static final int Q_NONE = 0, Q_IDLE = 1, Q_BATTLE = 2, Q_CHAIN = 3, Q_YESNO = 4, Q_CARDS = 5, Q_OPTION = 6,
            Q_POSITION = 7, Q_ZONE = 8, Q_SUM = 9, Q_SORT = 10, Q_COUNTER = 11, Q_NAME = 12;
    // Main / Battle Phase actions (YgoActionKind)
    public static final int ACT_SUMMON = 0, ACT_SET_MONSTER = 1, ACT_FLIP_SUMMON = 2, ACT_POSITION = 3, ACT_ACTIVATE = 4,
            ACT_SET_SPELL = 5, ACT_SPECIAL = 6, ACT_TO_BATTLE = 7, ACT_TO_MAIN2 = 8, ACT_END_TURN = 9, ACT_ATTACK = 10,
            ACT_CONCEDE = 11;
    public static final int POS_ATTACK = 1, POS_FACEDOWN_ATTACK = 2, POS_DEFENSE = 4, POS_FACEDOWN_DEFENSE = 8;
    public static final int IF_PICKED = 1, IF_MUST = 2;
    public static final int LOC_NONE = 0, LOC_DECK = 1, LOC_HAND = 2, LOC_MZONE = 3, LOC_SZONE = 4, LOC_GRAVE = 5,
            LOC_BANISHED = 6, LOC_EXTRA = 7;
    public static final int FRAME_NORMAL = 0, FRAME_EFFECT = 1, FRAME_FUSION = 2, FRAME_RITUAL = 3, FRAME_SPELL = 4,
            FRAME_TRAP = 5, FRAME_TOKEN = 6;
    public static final String[] PHASES = {"DRAW", "STANDBY", "MAIN 1", "BATTLE", "MAIN 2", "END"};
    // events (YgoEventType)
    public static final int EV_TURN = 1, EV_PHASE = 2, EV_DRAW = 3, EV_SUMMON = 4, EV_SET = 5, EV_FLIP = 6,
            EV_POSITION = 7, EV_ACTIVATE = 8, EV_RESOLVE = 9, EV_NEGATE = 10, EV_ATTACK = 11, EV_BATTLE = 12,
            EV_DESTROY = 13, EV_TO_GRAVE = 14, EV_TO_HAND = 15, EV_TO_DECK = 16, EV_BANISH = 17, EV_DAMAGE = 18,
            EV_RECOVER = 19, EV_STAT = 20, EV_EQUIP = 21, EV_COIN = 22, EV_DICE = 23, EV_SHUFFLE = 24, EV_REVEAL = 25,
            EV_TARGET = 26, EV_CONTROL = 27, EV_LOOK = 28, EV_WIN = 29, EV_MOVE = 30, EV_DECLARE = 31, EV_COUNTER = 32;
    public static final int DECK_MIN = 40, DECK_MAX = 60, EXTRA_MAX = 15;

    private Ygo() {}
}
