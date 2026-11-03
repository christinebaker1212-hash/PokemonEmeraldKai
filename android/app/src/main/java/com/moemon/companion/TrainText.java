package com.moemon.companion;

import org.json.JSONArray;

/** The training facilities' words (docs/COMPANION_APP.md "Training facilities"; the game's mp_phone_walk.c rules) */
public final class TrainText {
    public static final String[] KINDS = {"STAT", "LEVEL", "REST"};
    public static final String[] STATS = {"HP", "ATTACK", "DEFENSE", "SPEED", "SP. ATK", "SP. DEF"};

    private TrainText() {}

    /** What a training does, in a few words */
    public static String what(int kind, int stat) {
        switch (kind) {
            case 0: return "STAT: " + STATS[Math.max(0, Math.min(5, stat))] + " EVs";
            case 1: return "LEVEL: EXP to the BADGE cap";
            default: return "REST: Friendship and PP";
        }
    }

    /** About how much it has gained since SINCE (seconds; a day at most), as the game will work it out */
    public static String estimate(int kind, long secs) {
        long mins = Math.min(secs, 86400) / 60;
        switch (kind) {
            case 0: return "+" + mins / 5 + " EVs so far";
            case 1: return "+" + mins * 10 + " EXP so far";
            default: return "+" + mins / 30 + " Friendship so far";
        }
    }

    /** A collection's answer ("gains": [p, o, kind, amount]) in words */
    public static String gains(GameArt art, JSONArray gains) {
        if (gains == null || gains.length() == 0) {
            return "Nothing to collect yet.";
        }
        StringBuilder s = new StringBuilder("Collected: ");
        for (int i = 0; i < gains.length(); i++) {
            JSONArray g = gains.optJSONArray(i);
            int kind = g.optInt(2), n = g.optInt(3);
            s.append(i > 0 ? ", " : "").append("+").append(n).append(kind == 0 ? " EVs" : kind == 1 ? " EXP" : " Friendship");
        }
        return s.append(".").toString();
    }
}
