package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;

/**
 * TRIPLE TRIAD with a friend, a turn at a time through the co-op server (docs/COMPANION_APP.md "Card games", "Turn by
 * turn"). The game is deterministic, so only the moves travel. The invite goes into the friend's inbox (like a trade
 * offer: {"kind":"tt", g, from, n, hand, rules, seed, first, at}); the game's record is under Derive(pair key,
 * "ttgame" + g), each entry sealed with the pair key: the friend's hand (seq 0, or "declined"), then each move
 * {seq, slot, cell}. Both hands are open (OPEN is always on) and only rules both phones replay the same way are
 * offered (no SWAP, CHAOS or SUDDEN DEATH). Android-free.
 */
public final class TtFriends {
    public static final int DAYS = 7;

    private TtFriends() {}

    private static byte[] pk(JSONObject friend) {
        byte[] pk = Hex.bytes(friend.optString("pk"));
        if (pk == null || pk.length != 32) {
            throw new IllegalStateException("nopair");
        }
        return pk;
    }

    static byte[] gameKey(Crypto c, byte[] pk, String g) {
        return c.derive(pk, "ttgame" + g);
    }

    private static boolean post(Crypto c, String server, byte[] key, byte[] pk, JSONObject entry) throws IOException {
        byte[] sealed = c.seal(pk, 0, entry.toString().getBytes(StandardCharsets.UTF_8));
        try (Server s = new Server(server)) {
            return s.add(key, DAYS * 86400 + 3600, sealed);
        }
    }

    /** Invites FRIEND with HAND under RULES: the invite into their inbox; the answer is this phone's record of the game */
    public static JSONObject invite(Crypto c, String server, JSONObject friendsSnap, JSONObject friend, String myNameHex,
                                    int[] hand, int rules) throws IOException {
        byte[] pk = pk(friend), key = Hex.bytes(friend.optString("key")), me = Hex.bytes(friendsSnap.optString("me"));
        if (key == null || key.length != 32 || me == null || me.length != 8) {
            throw new IllegalStateException("nopair");
        }
        String g = Hex.of(c.random(8));
        byte[] seedBytes = c.random(4);
        int seed = ((seedBytes[0] & 255) | (seedBytes[1] & 255) << 8 | (seedBytes[2] & 255) << 16 | (seedBytes[3] & 127) << 24);
        long now = System.currentTimeMillis() / 1000;
        try {
            JSONArray h = new JSONArray();
            for (int id : hand) {
                h.put(id);
            }
            JSONObject inv = new JSONObject().put("kind", "tt").put("g", g).put("from", Hex.of(me)).put("n", myNameHex)
                    .put("hand", h).put("rules", rules).put("seed", seed).put("first", seed >> 7 & 1).put("at", now);
            byte[] sealed = c.seal(pk, 0, inv.toString().getBytes(StandardCharsets.UTF_8));
            byte[] entry = new byte[8 + sealed.length];
            System.arraycopy(me, 0, entry, 0, 8);
            System.arraycopy(sealed, 0, entry, 8, sealed.length);
            try (Server s = new Server(server)) {
                if (!s.add(c.derive(key, "inbox"), DAYS * 86400 + 3600, entry)) {
                    throw new IOException("the server didn't take the invite");
                }
            }
            return new JSONObject().put("g", g).put("peer", friend.optString("id")).put("pn", friend.optString("n"))
                    .put("seat", 0).put("rules", rules).put("seed", seed).put("first", seed >> 7 & 1)
                    .put("hands", new JSONArray().put(h).put(JSONObject.NULL)).put("moves", new JSONArray())
                    .put("at", now).put("last", now).put("srv", server);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** Friends' invites in this save's inbox (each with "friend": its friends list entry), newest first */
    public static List<JSONObject> invites(Crypto c, String server, JSONObject friendsSnap) throws IOException {
        List<JSONObject> out = new ArrayList<>();
        byte[] inbox = Hex.bytes(friendsSnap.optString("inbox"));
        JSONArray list = friendsSnap.optJSONArray("list");
        if (inbox == null || inbox.length != 16 || list == null) {
            return out;
        }
        try (Server s = new Server(server)) {
            for (byte[] e : s.get(inbox)) {
                if (e.length < 9) {
                    continue;
                }
                String from = Hex.of(java.util.Arrays.copyOf(e, 8));
                for (int i = 0; i < list.length(); i++) {
                    JSONObject f = list.optJSONObject(i);
                    byte[] pk = from.equals(f.optString("id")) ? Hex.bytes(f.optString("pk")) : null;
                    if (pk == null || pk.length != 32) {
                        continue;
                    }
                    byte[] opened = c.open(pk, java.util.Arrays.copyOfRange(e, 8, e.length));
                    if (opened == null || opened.length < 5) {
                        continue;
                    }
                    try {
                        JSONObject o = new JSONObject(new String(opened, 4, opened.length - 4, StandardCharsets.UTF_8));
                        if ("tt".equals(o.optString("kind")) && from.equals(o.optString("from"))) {
                            out.add(0, o.put("friend", f));
                        }
                    } catch (JSONException ignored) {
                    }
                }
            }
        }
        return out;
    }

    /** Accepts INVITE with HAND: the hand into the game's record; the answer is this phone's record of the game */
    public static JSONObject accept(Crypto c, String server, JSONObject invite, int[] hand) throws IOException {
        JSONObject friend = invite.optJSONObject("friend");
        byte[] pk = pk(friend);
        long now = System.currentTimeMillis() / 1000;
        try {
            JSONArray h = new JSONArray();
            for (int id : hand) {
                h.put(id);
            }
            if (!post(c, server, gameKey(c, pk, invite.optString("g")), pk, new JSONObject().put("seq", 0).put("hand", h))) {
                throw new IOException("the server didn't take the answer");
            }
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
        byte[] pk = pk(invite.optJSONObject("friend"));
        try {
            return post(c, server, gameKey(c, pk, invite.optString("g")), pk, new JSONObject().put("seq", 0).put("declined", 1));
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** This side's move number SEQ (1 the game's first) */
    public static boolean move(Crypto c, String server, JSONObject friend, String g, int seq, int slot, int cell)
            throws IOException {
        byte[] pk = pk(friend);
        try {
            return post(c, server, gameKey(c, pk, g), pk, new JSONObject().put("seq", seq).put("slot", slot).put("cell", cell));
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /**
     * The game's record as the server has it: {"hand": the friend's (accepted) or null, "declined": 0/1, "moves":
     * [[slot, cell]...] in order (the first of each number; a gap stops the list)}
     */
    public static JSONObject read(Crypto c, String server, JSONObject friend, String g) throws IOException {
        byte[] pk = pk(friend);
        TreeMap<Integer, JSONObject> bySeq = new TreeMap<>();
        try (Server s = new Server(server)) {
            for (byte[] e : s.get(gameKey(c, pk, g))) {
                byte[] opened = c.open(pk, e);
                if (opened == null || opened.length < 5) {
                    continue;
                }
                try {
                    JSONObject o = new JSONObject(new String(opened, 4, opened.length - 4, StandardCharsets.UTF_8));
                    bySeq.putIfAbsent(o.optInt("seq", -1), o);
                } catch (JSONException ignored) {
                }
            }
        }
        try {
            JSONObject out = new JSONObject();
            JSONObject zero = bySeq.get(0);
            out.put("hand", zero == null || !zero.has("hand") ? JSONObject.NULL : zero.optJSONArray("hand"));
            out.put("declined", zero != null && zero.optInt("declined") == 1 ? 1 : 0);
            JSONArray moves = new JSONArray();
            for (int seq = 1; bySeq.containsKey(seq); seq++) {
                JSONObject m = bySeq.get(seq);
                moves.put(new JSONArray().put(m.optInt("slot")).put(m.optInt("cell")));
            }
            return out.put("moves", moves);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }
}
