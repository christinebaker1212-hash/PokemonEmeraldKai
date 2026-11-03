package com.moemon.companion;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * A POKéMON card duel with a friend as this phone sees it (Companion.tcgGames: g, peer, pn, seat, seed, prizes, decks
 * [seat 0's, seat 1's], moves [actions in order, both sides']): replayed on the game's engine with the inviter as the
 * engine's player 0 on both phones, to say whose move it is and how it ended.
 */
public final class TcgFriendGame {
    public static final int DAYS_TO_FORFEIT = 3;

    private TcgFriendGame() {}

    static int[] deck(JSONObject g, int seat) {
        JSONArray d = g.optJSONArray("decks") == null ? null : g.optJSONArray("decks").optJSONArray(seat);
        if (d == null || d.length() != Tcg.DECK) {
            return null;
        }
        int[] out = new int[Tcg.DECK];
        for (int i = 0; i < Tcg.DECK; i++) {
            out[i] = d.optInt(i);
        }
        return out;
    }

    static JSONArray moves(JSONObject g) {
        JSONArray m = g.optJSONArray("moves");
        return m == null ? new JSONArray() : m;
    }

    /** The engine with every move so far (0 until both decks are known): end it with Tcg.end */
    public static long replay(JSONObject g) {
        int[] d0 = deck(g, 0), d1 = deck(g, 1);
        if (d0 == null || d1 == null) {
            return 0;
        }
        long d = Tcg.start(d0, d1, g.optInt("seed"), g.optInt("prizes", 6));
        JSONArray moves = moves(g);
        for (int i = 0; d != 0 && i < moves.length(); i++) {
            Tcg.apply(d, moves.optJSONObject(i).toString());
        }
        return d;
    }

    /** "invited" (waiting for their answer), "declined", "yourturn", "theirturn", "won", "lost" */
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
            int seat = g.optInt("seat"), actor = Tcg.actor(d);
            if (actor < 0) {
                return new JSONObject(Tcg.state(d, seat)).optInt("winner") == seat ? "won" : "lost";
            }
            return actor == seat ? "yourturn" : "theirturn";
        } catch (Exception e) {
            return "theirturn";
        } finally {
            Tcg.end(d);
        }
    }

    /** They haven't moved in 3 days: the win can be claimed */
    public static boolean canClaim(JSONObject g) {
        return "theirturn".equals(status(g)) && System.currentTimeMillis() / 1000 - g.optLong("last") > DAYS_TO_FORFEIT * 86400L;
    }
}
