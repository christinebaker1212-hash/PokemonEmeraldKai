package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.List;

/**
 * TRIPLE TRIAD with a friend, a turn at a time through the co-op server (CardDuels, kind "tt"): the invite carries the
 * inviter's hand and the rules ({"kind":"tt", g, from, n, hand, rules, seed, first, at}); the friend's answer is their
 * hand, then each move {seq, slot, cell}. Both hands are open (OPEN is always on) and only rules both phones replay the
 * same way are offered (no SWAP, CHAOS or SUDDEN DEATH). Android-free.
 */
public final class TtFriends {
    public static final int DAYS = CardDuels.DAYS;

    private TtFriends() {}

    private static JSONArray hand(int[] hand) {
        JSONArray h = new JSONArray();
        for (int id : hand) {
            h.put(id);
        }
        return h;
    }

    /** Invites FRIEND with HAND under RULES: the invite into their inbox; the answer is this phone's record of the game */
    public static JSONObject invite(Crypto c, String server, JSONObject friendsSnap, JSONObject friend, String myNameHex,
                                    int[] hand, int rules) throws IOException {
        try {
            JSONArray h = hand(hand);
            JSONObject g = CardDuels.invite(c, server, friendsSnap, friend, myNameHex, "tt",
                    new JSONObject().put("hand", h).put("rules", rules));
            return g.put("rules", rules).put("hands", new JSONArray().put(h).put(JSONObject.NULL));
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** Friends' invites in this save's inbox (each with "friend": its friends list entry), newest first */
    public static List<JSONObject> invites(Crypto c, String server, JSONObject friendsSnap) throws IOException {
        return CardDuels.invites(c, server, friendsSnap, "tt");
    }

    /** Accepts INVITE with HAND: the hand into the game's record; the answer is this phone's record of the game */
    public static JSONObject accept(Crypto c, String server, JSONObject invite, int[] hand) throws IOException {
        JSONObject friend = invite.optJSONObject("friend");
        long now = System.currentTimeMillis() / 1000;
        try {
            JSONArray h = hand(hand);
            CardDuels.accept(c, server, invite, "tt", new JSONObject().put("hand", h));
            return new JSONObject().put("g", invite.optString("g")).put("peer", friend.optString("id"))
                    .put("pn", invite.optString("n")).put("seat", 1).put("rules", invite.optInt("rules"))
                    .put("seed", invite.optInt("seed")).put("first", invite.optInt("first"))
                    .put("hands", new JSONArray().put(invite.optJSONArray("hand")).put(h)).put("moves", new JSONArray())
                    .put("at", invite.optLong("at")).put("last", now).put("srv", server);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    public static boolean decline(Crypto c, String server, JSONObject invite) throws IOException {
        return CardDuels.decline(c, server, invite, "tt");
    }

    /** This side's move number SEQ (1 the game's first) */
    public static boolean move(Crypto c, String server, JSONObject friend, String g, int seq, int slot, int cell)
            throws IOException {
        try {
            return CardDuels.move(c, server, friend, "tt", g, seq, new JSONObject().put("slot", slot).put("cell", cell));
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /**
     * The game's record as the server has it: {"hand": the friend's (accepted) or null, "declined": 0/1, "moves":
     * [[slot, cell]...] in order (the first of each number; a gap stops the list)}
     */
    public static JSONObject read(Crypto c, String server, JSONObject friend, String g) throws IOException {
        JSONObject r = CardDuels.read(c, server, friend, "tt", g);
        try {
            JSONObject zero = r.optJSONObject("zero");
            JSONArray moves = new JSONArray(), raw = r.optJSONArray("moves");
            for (int i = 0; raw != null && i < raw.length(); i++) {
                moves.put(new JSONArray().put(raw.optJSONObject(i).optInt("slot")).put(raw.optJSONObject(i).optInt("cell")));
            }
            return new JSONObject().put("hand", zero == null || !zero.has("hand") ? JSONObject.NULL : zero.optJSONArray("hand"))
                    .put("declined", r.optInt("declined")).put("moves", moves);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }
}
