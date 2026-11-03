package com.moemon.companion;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * A duel with a friend as this phone sees it (Companion.ttGames: g, peer, pn, seat, rules, seed, first, hands [seat 0's,
 * seat 1's], moves): replayed on the game's engine with this phone's side as seat 0 (the rules offered play the same
 * either way round), to say whose turn it is and how it ended.
 */
public final class TtFriendGame {
    public static final int DAYS_TO_FORFEIT = 3;

    private TtFriendGame() {}

    static int[] hand(JSONObject g, int seat) {
        JSONArray h = g.optJSONArray("hands").optJSONArray(seat);
        int[] out = new int[5];
        for (int i = 0; h != null && i < 5; i++) {
            out[i] = h.optInt(i);
        }
        return out;
    }

    /** This phone's side first: the engine's seat 0 is always this phone's */
    public static int localFirst(JSONObject g) {
        return g.optInt("first") == g.optInt("seat") ? 0 : 1;
    }

    /** The engine with every move so far (null until both hands are known): end it with Tt.end */
    public static long replay(JSONObject g) {
        if (g.optJSONArray("hands").isNull(1 - g.optInt("seat")) || g.optJSONArray("hands").isNull(g.optInt("seat"))) {
            return 0;
        }
        long d = Tt.start(g.optInt("rules"), 0, g.optInt("seed"), hand(g, g.optInt("seat")), hand(g, 1 - g.optInt("seat")),
                localFirst(g));
        JSONArray moves = g.optJSONArray("moves");
        for (int i = 0; moves != null && i < moves.length(); i++) {
            Tt.play(d, moves.optJSONArray(i).optInt(0), moves.optJSONArray(i).optInt(1));
        }
        return d;
    }

    /** "invited" (waiting for their answer), "declined", "yourturn", "theirturn", "won", "lost", "draw" */
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
            JSONObject st = new JSONObject(Tt.state(d, 0));
            if (st.optInt("over") != 0) {
                int w = st.optInt("winner");
                return w == 0 ? "won" : w == 1 ? "lost" : "draw";
            }
            return st.optInt("toMove") == 0 ? "yourturn" : "theirturn";
        } catch (Exception e) {
            return "theirturn";
        } finally {
            Tt.end(d);
        }
    }

    /** They haven't moved in 3 days: the win can be claimed */
    public static boolean canClaim(JSONObject g) {
        return "theirturn".equals(status(g)) && System.currentTimeMillis() / 1000 - g.optLong("last") > DAYS_TO_FORFEIT * 86400L;
    }
}
