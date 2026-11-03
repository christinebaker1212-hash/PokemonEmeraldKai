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
 * A card game with a friend, a turn at a time through the co-op server (docs/PHONE_LINK.md "TRIPLE TRIAD on the
 * phone", "The POKéMON card game on the phone"): TRIPLE TRIAD ("tt", TtFriends) and the POKéMON card game ("tcg").
 * Both games are deterministic, so only the moves travel. The invite goes into the friend's inbox (like a trade
 * offer: {"kind", g, from, n, seed, first, at, + the game's own: a hand and rules, a deck and Prize cards}); the
 * game's record is under Derive(pair key, kind + "game" + g), each entry sealed with the pair key: the friend's answer
 * (seq 0: their hand / deck, or "declined"), then each move {seq, + the game's own} numbered from 1 in the order they
 * were made (whoever moves next posts the next number). Android-free.
 */
public final class CardDuels {
    public static final int DAYS = 7;

    private CardDuels() {}

    static byte[] pk(JSONObject friend) {
        byte[] pk = Hex.bytes(friend.optString("pk"));
        if (pk == null || pk.length != 32) {
            throw new IllegalStateException("nopair");
        }
        return pk;
    }

    static byte[] gameKey(Crypto c, byte[] pk, String kind, String g) {
        return c.derive(pk, kind + "game" + g);
    }

    static boolean post(Crypto c, String server, byte[] key, byte[] pk, JSONObject entry) throws IOException {
        byte[] sealed = c.seal(pk, 0, entry.toString().getBytes(StandardCharsets.UTF_8));
        try (Server s = new Server(server)) {
            return s.add(key, DAYS * 86400 + 3600, sealed);
        }
    }

    private static void merge(JSONObject into, JSONObject from) throws JSONException {
        if (from == null) {
            return;
        }
        for (java.util.Iterator<String> it = from.keys(); it.hasNext(); ) {
            String k = it.next();
            into.put(k, from.get(k));
        }
    }

    /**
     * Invites FRIEND to a game of KIND with the game's own PAYLOAD (a hand and rules, a deck...): the invite into their
     * inbox; the answer is this phone's record of the game {g, peer, pn, seat 0, seed, first, moves [], at, last, srv}
     */
    public static JSONObject invite(Crypto c, String server, JSONObject friendsSnap, JSONObject friend, String myNameHex,
                                    String kind, JSONObject payload) throws IOException {
        byte[] pk = pk(friend), key = Hex.bytes(friend.optString("key")), me = Hex.bytes(friendsSnap.optString("me"));
        if (key == null || key.length != 32 || me == null || me.length != 8) {
            throw new IllegalStateException("nopair");
        }
        String g = Hex.of(c.random(8));
        byte[] seedBytes = c.random(4);
        int seed = ((seedBytes[0] & 255) | (seedBytes[1] & 255) << 8 | (seedBytes[2] & 255) << 16 | (seedBytes[3] & 127) << 24);
        long now = System.currentTimeMillis() / 1000;
        try {
            JSONObject inv = new JSONObject().put("kind", kind).put("g", g).put("from", Hex.of(me)).put("n", myNameHex)
                    .put("seed", seed).put("first", seed >> 7 & 1).put("at", now);
            merge(inv, payload);
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
                    .put("seat", 0).put("seed", seed).put("first", seed >> 7 & 1).put("moves", new JSONArray())
                    .put("at", now).put("last", now).put("srv", server);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** Friends' invites of KIND in this save's inbox (each with "friend": its friends list entry), newest first */
    public static List<JSONObject> invites(Crypto c, String server, JSONObject friendsSnap, String kind) throws IOException {
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
                        if (kind.equals(o.optString("kind")) && from.equals(o.optString("from"))) {
                            out.add(0, o.put("friend", f));
                        }
                    } catch (JSONException ignored) {
                    }
                }
            }
        }
        return out;
    }

    /** Accepts INVITE with this side's ANSWER (its hand, its deck): the answer into the game's record (seq 0) */
    public static void accept(Crypto c, String server, JSONObject invite, String kind, JSONObject answer) throws IOException {
        JSONObject friend = invite.optJSONObject("friend");
        byte[] pk = pk(friend);
        try {
            JSONObject zero = new JSONObject().put("seq", 0);
            merge(zero, answer);
            if (!post(c, server, gameKey(c, pk, kind, invite.optString("g")), pk, zero)) {
                throw new IOException("the server didn't take the answer");
            }
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    public static boolean decline(Crypto c, String server, JSONObject invite, String kind) throws IOException {
        byte[] pk = pk(invite.optJSONObject("friend"));
        try {
            return post(c, server, gameKey(c, pk, kind, invite.optString("g")), pk,
                    new JSONObject().put("seq", 0).put("declined", 1));
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** The game's move number SEQ (1 the game's first), whoever made it */
    public static boolean move(Crypto c, String server, JSONObject friend, String kind, String g, int seq, JSONObject move)
            throws IOException {
        byte[] pk = pk(friend);
        try {
            JSONObject entry = new JSONObject().put("seq", seq);
            merge(entry, move);
            return post(c, server, gameKey(c, pk, kind, g), pk, entry);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /**
     * The game's record as the server has it: {"zero": the friend's answer (seq 0) or null, "declined": 0/1, "moves":
     * [entries in order] (the first of each number; a gap stops the list)}
     */
    public static JSONObject read(Crypto c, String server, JSONObject friend, String kind, String g) throws IOException {
        byte[] pk = pk(friend);
        TreeMap<Integer, JSONObject> bySeq = new TreeMap<>();
        try (Server s = new Server(server)) {
            for (byte[] e : s.get(gameKey(c, pk, kind, g))) {
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
            out.put("zero", zero == null || zero.optInt("declined") == 1 ? JSONObject.NULL : zero);
            out.put("declined", zero != null && zero.optInt("declined") == 1 ? 1 : 0);
            JSONArray moves = new JSONArray();
            for (int seq = 1; bySeq.containsKey(seq); seq++) {
                moves.put(bySeq.get(seq));
            }
            return out.put("moves", moves);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }
}
