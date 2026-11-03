package com.moemon.companion.core;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * Trades by mail, the phone's side (docs/PHONE_LINK.md "Trades by mail"; the game's mp_phone_trade.c). An offer goes
 * straight into the friend's inbox on the co-op server (sealed with the key only the two saves share: the snapshot's
 * friend "pk"), and the same offer goes to this phone's game as an order, which keeps the POKéMON ON OFFER. The friend's
 * phone reads its inbox, and ACCEPT is an order to its own game, which deposits its POKéMON in the server's escrow; the
 * games do the swap. DECLINE and CANCEL go straight to the escrow (the side's secret: from the snapshot's "tk").
 * Android-free (the desktop test trades with it).
 */
public final class Trades {
    public static final int DAYS = 7;
    // what an offer shows of the POKéMON (the snapshot's own keys)
    private static final String[] MON_KEYS = {"s", "e", "p", "o", "l", "i", "g", "sh", "nat", "ab", "ball", "f", "n", "pal",
            "m", "st", "ot", "otn", "met"};

    private Trades() {}

    /** A side's secret for trade T: the game works out the same (mp_phone_trade.c SideSecret) */
    public static byte[] sideSecret(Crypto c, JSONObject friendsSnap, byte[] t) {
        byte[] tk = Hex.bytes(friendsSnap.optString("tk"));
        return tk == null || tk.length != 32 ? null : c.derive(tk, Hex.of(t));
    }

    static JSONObject trimmed(JSONObject mon) throws JSONException {
        JSONObject o = new JSONObject();
        for (String k : MON_KEYS) {
            if (mon.has(k)) {
                o.put(k, mon.get(k));
            }
        }
        return o;
    }

    /**
     * Offers MON (at PLACE in this save) to FRIEND (an entry of the snapshot's friends list) for a POKéMON of theirs
     * (WANT: a species, 0 for anything): into their inbox now; the answer is the order for this phone's game.
     */
    public static JSONObject offer(Crypto c, String server, JSONObject friendsSnap, JSONObject friend, String myNameHex,
                                   JSONObject mon, JSONObject place, int want) throws IOException {
        byte[] pk = Hex.bytes(friend.optString("pk")), key = Hex.bytes(friend.optString("key"));
        byte[] me = Hex.bytes(friendsSnap.optString("me"));
        if (pk == null || pk.length != 32 || key == null || key.length != 32 || me == null || me.length != 8) {
            throw new IllegalStateException("nopair");
        }
        byte[] t = c.random(16);
        long now = System.currentTimeMillis() / 1000, exp = now + DAYS * 86400L;
        try {
            JSONObject o = new JSONObject().put("t", Hex.of(t)).put("from", Hex.of(me)).put("n", myNameHex)
                    .put("mon", trimmed(mon)).put("want", want).put("at", now).put("exp", exp).put("srv", server);
            byte[] sealed = c.seal(pk, 0, o.toString().getBytes(StandardCharsets.UTF_8));
            byte[] entry = new byte[8 + sealed.length];
            System.arraycopy(me, 0, entry, 0, 8);
            System.arraycopy(sealed, 0, entry, 8, sealed.length);
            try (Server s = new Server(server)) {
                if (!s.add(c.derive(key, "inbox"), DAYS * 86400 + 3600, entry)) {
                    throw new IOException("the server didn't take the offer");
                }
            }
            return new JSONObject().put("op", "offer").put("trade", Hex.of(t)).put("to", friend.optString("id"))
                    .put("at", place).put("p", mon.optLong("p")).put("o", mon.optLong("o")).put("want", want)
                    .put("exp", exp).put("srv", server);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /**
     * Offers friends have made to this save, newest first, each with "esc" (the escrow's state: Server.HAVE1 = this
     * save accepted, CLOSED = turned down or called off) and "friend" (the friends list entry it's from)
     */
    public static List<JSONObject> incoming(Crypto c, String server, JSONObject friendsSnap) throws IOException {
        List<JSONObject> out = new ArrayList<>();
        byte[] inbox = Hex.bytes(friendsSnap.optString("inbox"));
        org.json.JSONArray list = friendsSnap.optJSONArray("list");
        if (inbox == null || inbox.length != 16 || list == null) {
            return out;
        }
        try (Server s = new Server(server)) {
            for (byte[] e : s.get(inbox)) {
                if (e.length < 9) {
                    continue;
                }
                String from = Hex.of(java.util.Arrays.copyOf(e, 8));
                JSONObject friend = null;
                for (int i = 0; i < list.length() && friend == null; i++) {
                    if (from.equals(list.optJSONObject(i).optString("id"))) {
                        friend = list.optJSONObject(i);
                    }
                }
                byte[] pk = friend == null ? null : Hex.bytes(friend.optString("pk"));
                if (pk == null || pk.length != 32) {
                    continue;   // (not a friend this save can trade with: never shown)
                }
                byte[] opened = c.open(pk, java.util.Arrays.copyOfRange(e, 8, e.length));
                if (opened == null || opened.length < 5) {
                    continue;
                }
                try {
                    JSONObject o = new JSONObject(new String(opened, 4, opened.length - 4, StandardCharsets.UTF_8));
                    if (!from.equals(o.optString("from")) || o.has("kind")) {
                        continue;   // (sealed by that friend, from that friend; a card duel's invite isn't a trade)
                    }
                    byte[] t = Hex.bytes(o.optString("t"));
                    if (t == null || t.length != 16) {
                        continue;
                    }
                    o.put("esc", s.look(t)).put("friend", friend);
                    out.add(0, o);
                } catch (JSONException ignored) {
                }
            }
        }
        return out;
    }

    /** ACCEPT: the order for this phone's game, giving MON (at PLACE) for the offer */
    public static JSONObject accept(JSONObject offer, JSONObject mon, JSONObject place) throws JSONException {
        return new JSONObject().put("op", "accept").put("trade", offer.optString("t")).put("from", offer.optString("from"))
                .put("at", place).put("p", mon.optLong("p")).put("o", mon.optLong("o")).put("want", offer.optInt("want"))
                .put("exp", offer.optLong("exp")).put("srv", offer.optString("srv"));
    }

    /** Calls trade T off from SIDE (0: our offer, CANCEL; 1: theirs, DECLINE): TRUE when it's off (both deposits not in) */
    public static boolean callOff(Crypto c, String server, JSONObject friendsSnap, String tHex, int side) throws IOException {
        byte[] t = Hex.bytes(tHex), secret = t == null ? null : sideSecret(c, friendsSnap, t);
        if (secret == null) {
            return false;
        }
        try (Server s = new Server(server)) {
            int st = s.cancel(t, side, secret);
            return st >= 0 && (st & Server.CLOSED) != 0;
        }
    }
}
