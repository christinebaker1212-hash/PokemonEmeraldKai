package com.moemon.companion;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * A YU-GI-OH! duel with a friend as this phone sees it (Companion.ygoGames: g, peer, pn, seat, seed, first, decks
 * [[main, extra] of seat 0, of seat 1], moves [answers {p, z}, or {concede: seat}, in order, both sides']): rebuilt on
 * the game's engine from its decks, seed and answers, the inviter as the engine's player 0 on both phones.
 */
public final class YgoFriendGame {
    public static final int DAYS_TO_FORFEIT = 3;

    private YgoFriendGame() {}

    static int[] deck(JSONObject g, int seat, int part) {
        JSONArray d = g.optJSONArray("decks") == null || g.optJSONArray("decks").isNull(seat) ? null
                : g.optJSONArray("decks").optJSONArray(seat);
        return d == null ? null : Ygo.ints(d.optJSONArray(part));
    }

    static JSONArray moves(JSONObject g) {
        JSONArray m = g.optJSONArray("moves");
        return m == null ? new JSONArray() : m;
    }

    /** The duel with every move so far (0 until both decks are known): end it with Ygo.end */
    public static long replay(JSONObject g) {
        int[] m0 = deck(g, 0, 0), x0 = deck(g, 0, 1), m1 = deck(g, 1, 0), x1 = deck(g, 1, 1);
        if (m0 == null || m1 == null) {
            return 0;
        }
        long d = Ygo.start(m0, x0, m1, x1, g.optInt("seed"), g.optInt("first"));
        JSONArray moves = moves(g);
        for (int i = 0; d != 0 && i < moves.length(); i++) {
            JSONObject m = moves.optJSONObject(i);
            if (m.has("concede")) {
                Ygo.concede(d, m.optInt("concede"));
            } else {
                Ygo.answer(d, Ygo.ints(m.optJSONArray("p")), m.optInt("z"));
            }
        }
        return d;
    }

    /** "invited", "declined", "yourturn", "theirturn", "won", "lost" */
    public static String status(JSONObject g) {
        if (g.optInt("declined") == 1) {
            return "declined";
        }
        if (g.optInt("forfeit") == 1) {
            return "won";
        }
        long d = replay(g);
        if (d == 0) {
            return "invited";
        }
        try {
            int seat = g.optInt("seat");
            JSONObject st = new JSONObject(Ygo.state(d, seat));
            if (st.optInt("over") == 1) {
                return st.optInt("winner") == seat ? "won" : "lost";
            }
            return st.optInt("asks") == seat ? "yourturn" : "theirturn";
        } catch (Exception e) {
            return "theirturn";
        } finally {
            Ygo.end(d);
        }
    }

    public static boolean canClaim(JSONObject g) {
        return "theirturn".equals(status(g)) && System.currentTimeMillis() / 1000 - g.optLong("last") > DAYS_TO_FORFEIT * 86400L;
    }
}
