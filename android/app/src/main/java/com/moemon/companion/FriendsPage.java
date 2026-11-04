package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;
import com.moemon.companion.core.Server;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * FRIENDS (docs/COMPANION_APP.md "Pillar 4: friends"): the people this save has played with, who's playing now, where
 * and doing what, as far as each shares; JOIN sends your game to their room. Your own sharing: EVERYTHING (where you
 * are, your room, your party), ONLINE ONLY, or HIDDEN. TRADES by mail: friends' offers (ACCEPT with one of yours, or
 * DECLINE), your own offers (CANCEL while nobody has answered), and how each trade is getting on; TRADE on a friend
 * offers one of yours (docs/PHONE_LINK.md "Trades by mail").
 */
public final class FriendsPage extends Page {
    private static final String[] SHARE = {"EVERYTHING", "ONLINE ONLY", "HIDDEN"};
    private final Paint fill = new Paint();
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    private int bottom;

    public FriendsPage(GameView v) {
        super(v);
    }

    @Override
    public void shown() {
        v.app.refreshFriends();
    }

    private JSONObject friendsSnap() {
        Model m = v.app.model();
        return m == null ? null : m.snap.optJSONObject("friends");
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.contentHeight());
    }

    private static String doing(String d) {
        switch (d) {
            case "battle": return "In a battle";
            case "coop": return "In a co-op battle";
            case "cards": return "In a card duel";
            case "field": return "Exploring";
            default: return "Busy";
        }
    }

    private String nameOf(String hex) {
        return art.plain(GameArt.hexText(hex));
    }

    private String monName(JSONObject mon) {
        return mon == null ? "?" : mon.optInt("e") == 1 ? "EGG" : nameOf(mon.optString("n"));
    }

    private String wants(int species) {
        return species == 0 ? "anything" : art.plain(art.speciesName(species));
    }

    private void icon(Canvas c, JSONObject mon, int x, int y) {
        if (mon != null) {
            art.icon(c, mon.optInt("e") == 1 ? art.data.optInt("egg") : mon.optInt("s"), x, y, GameArt.ICON_PLAIN);
        }
    }

    private void tapArea(int x, int y, int w, int h, Runnable r) {
        hits.add(new int[]{x, y, w, h});
        actions.add(r);
    }

    private int heading(Canvas c, int w, int y, String title, String right) {
        sectionBar(c, 8, w - 8, y + 2, 16, title, right);
        return y + 22;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        JSONObject fs = friendsSnap();
        int y = 24 - scroll;
        // your sharing
        int share = fs == null ? 0 : fs.optInt("share");
        art.text(c, "YOU SHARE:", 12, y + 2, GameArt.WHITE_TEXT, 0xFF305060);
        int x = 12 + art.width("YOU SHARE:") + 6;
        for (int i = 0; i < SHARE.length; i++) {
            int cw = art.width(SHARE[i]) + 10;
            boolean on = i == share;
            if (on) {
                art.window(c, 0, x + 2, y + 4, cw - 4, 12);
            }
            art.text(c, SHARE[i], x + 5, y + 2, on ? GameArt.TEXT : GameArt.WHITE_TEXT, on ? GameArt.SHADOW : 0xFF305060);
            final int level = i;
            tapArea(x, y, cw, 18, () -> setShare(level));
            x += cw + 2;
        }
        y += 24;
        y = drawTrades(c, w, y);
        JSONArray list = fs == null ? null : fs.optJSONArray("list");
        long now = System.currentTimeMillis() / 1000;
        int online = 0;
        if (list != null && list.length() > 0) {
            y = heading(c, w, y, "FRIENDS", null);
        }
        for (int i = 0; list != null && i < list.length(); i++) {
            JSONObject f = list.optJSONObject(i);
            JSONObject p = v.app.friends.get(f.optString("id"));
            boolean on = com.moemon.companion.core.Friends.online(p, now);
            online += on ? 1 : 0;
            int rh = p != null && p.has("party") ? 78 : 50;
            art.window(c, 0, 12, y + 4, w - 24, rh - 8);
            fill.setColor(on ? 0xFF40D040 : 0xFFA0A0A0);
            c.drawCircle(20, y + 12, 4, fill);
            art.textHex(c, f.optString("n"), 28, y + 4, GameArt.TEXT, GameArt.SHADOW);
            String state;
            if (p == null) {
                state = "Not seen online lately";
            } else if (!on) {
                state = "Last online " + GameView.ago(p.optLong("t") * 1000);
            } else if (p.has("do")) {
                JSONArray secs = art.data.optJSONArray("mapsecs");
                String where = secs == null ? "" : art.plain(GameArt.hexText(secs.optString(p.optInt("sec"))));
                state = doing(p.optString("do")) + (where.isEmpty() ? "" : " in " + where);
            } else {
                state = "Playing now";
            }
            art.text(c, state, 28, y + 28, GameArt.GRAY_TEXT, GameArt.SHADOW);
            if (p != null && p.has("party")) {
                JSONArray party = p.optJSONArray("party");
                for (int k = 0; k < party.length(); k++) {
                    JSONArray m = party.optJSONArray(k);
                    art.icon(c, m.optInt(2) == 1 ? art.data.optInt("egg") : m.optInt(0), 24 + k * 30, y + 40,
                            GameArt.ICON_PLAIN);
                }
            }
            final JSONObject friend = f, presence = p;
            final String name = nameOf(f.optString("n"));
            int bx = w - 70;
            if (on && p != null && (p.has("room") || p.has("code"))) {
                button(c, "JOIN", bx, y + 2, 56, true);
                tapArea(bx, y + 2, 56, 24, () -> join(name, presence));
                bx -= 60;
            }
            button(c, "TRADE", bx, y + 2, 56, f.has("pk"));
            tapArea(bx, y + 2, 56, 24, () -> startTrade(friend, presence));
            y += rh;
        }
        if (list == null || list.length() == 0) {
            art.window(c, 0, 12, y + 4, w - 24, 44);
            int ty = y + 4;
            for (String l : wrap(v.app.model() == null ? "Nothing from the game yet."
                    : "Friends are the players you've been in a co-op room with. Play together once and they show here.",
                    w - 32)) {
                art.text(c, l, 16, ty);
                ty += 14;
            }
            y = ty + 12;
        } else if (!v.app.friendsError.isEmpty()) {
            art.text(c, v.app.friendsError, 12, y + 4, GameArt.WHITE_TEXT, 0xFF305060);
            y += 20;
        }
        bottom = y + scroll + 8;
        banner(c, w, "FRIENDS", online + " ONLINE");
        // (fresh every 30 seconds while shown)
        if (System.currentTimeMillis() - v.app.friendsAt > 30000) {
            v.app.friendsAt = System.currentTimeMillis();
            v.app.refreshFriends();
        }
    }

    // ------------------------------------------------------------------------------------------------ trades

    private JSONObject gameTrade(String t) {
        Model m = v.app.model();
        JSONArray l = m == null ? null : m.snap.optJSONArray("trades");
        for (int i = 0; l != null && i < l.length(); i++) {
            if (t.equals(l.optJSONObject(i).optString("t"))) {
                return l.optJSONObject(i);
            }
        }
        return null;
    }

    private static String tradeState(JSONObject t, String peer) {
        switch (t.optString("st")) {
            case "offered": return "On offer to " + peer + ". Waiting for an answer.";
            case "accepting": return "You said yes. Your game sends it when it's next online.";
            case "depositing":
            case "deposited": return "On its way. Waiting for " + peer + "'s game.";
            case "receiving": return "Arriving in your game now.";
            case "done": return t.optInt("shown") == 1 ? "Traded!" : "Traded! The trade plays in your game next time you're free.";
            case "off":
                switch (t.optInt("why")) {
                    case 1: return peer + " turned it down.";
                    case 2: return "No answer in 7 days: it ran out.";
                    case 3: return "Called off: that POKéMON was gone.";
                    default: return "Called off.";
                }
            default: return "";
        }
    }

    private int drawTrades(Canvas c, int w, int y) {
        Model m = v.app.model();
        long now = System.currentTimeMillis() / 1000;
        List<JSONObject> offers = new ArrayList<>();
        for (JSONObject o : v.app.offers) {
            // a friend's offer this game doesn't know of yet (once answered, the game's own trade shows)
            if ((o.optInt("esc") & (Server.CLOSED | Server.HAVE1)) == 0 && o.optLong("exp") > now && gameTrade(o.optString("t")) == null) {
                offers.add(o);
            }
        }
        List<JSONObject> mine = new ArrayList<>();
        JSONArray l = m == null ? null : m.snap.optJSONArray("trades");
        for (int i = 0; l != null && i < l.length(); i++) {
            JSONObject t = l.optJSONObject(i);
            boolean finished = "done".equals(t.optString("st")) || "off".equals(t.optString("st"));
            if (!finished || now - t.optLong("changed") < 3 * 86400) {
                mine.add(t);
            }
        }
        if (offers.isEmpty() && mine.isEmpty()) {
            return y;
        }
        y = heading(c, w, y, "TRADES BY MAIL", offers.isEmpty() ? null : offers.size() + " NEW");
        for (JSONObject o : offers) {
            JSONObject mon = o.optJSONObject("mon");
            String peer = nameOf(o.optString("n"));
            List<String> lines = new ArrayList<>(wrap(peer + " offers " + monName(mon) + " Lv" + (mon == null ? 0 : mon.optInt("l")),
                    w - 72));
            List<String> sub = wrap("For " + wants(o.optInt("want")) + ". " + Math.max(1, (o.optLong("exp") - now) / 86400)
                    + " days left.", w - 72);
            int ch = 8 + 14 * (lines.size() + sub.size()) + 30;
            art.window(c, 0, 12, y + 4, w - 24, ch);
            icon(c, mon, 16, y + 8);
            int ty = y + 6;
            for (String line : lines) {
                art.text(c, line, 54, ty);
                ty += 14;
            }
            for (String line : sub) {
                art.text(c, line, 54, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
                ty += 14;
            }
            final JSONObject offer = o;
            button(c, "ACCEPT", w - 148, ty + 2, 64, true);
            tapArea(w - 148, ty + 2, 64, 24, () -> accept(offer));
            button(c, "DECLINE", w - 80, ty + 2, 64, true);
            tapArea(w - 80, ty + 2, 64, 24, () -> decline(offer));
            y += ch + 6;
        }
        for (JSONObject t : mine) {
            String peer = nameOf(t.optString("pn"));
            JSONObject a = t.optJSONObject("mine"), b = t.optJSONObject("theirs");
            String head = monName(a) + (b != null ? " for " + monName(b) : t.optInt("want") != 0 ? " for " + wants(t.optInt("want")) : "")
                    + (t.optInt("role") == 0 ? " to " : " from ") + peer;
            List<String> lines = wrap(head, w - 112), sub = wrap(tradeState(t, peer), w - 112);
            boolean canCancel = "offered".equals(t.optString("st"));
            int ch = Math.max(40, 8 + 14 * (lines.size() + sub.size()) + (canCancel ? 30 : 0));
            art.window(c, 0, 12, y + 4, w - 24, ch);
            icon(c, a, 16, y + 8);
            art.text(c, ">", 50, y + 16);
            if (b != null) {
                icon(c, b, 58, y + 8);
            } else {
                art.text(c, "?", 70, y + 16, GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
            int ty = y + 6;
            for (String line : lines) {
                art.text(c, line, 96, ty);
                ty += 14;
            }
            for (String line : sub) {
                art.text(c, line, 96, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
                ty += 14;
            }
            if (canCancel) {
                final String id = t.optString("t");
                button(c, "CALL OFF", w - 92, ty + 2, 76, true);
                tapArea(w - 92, ty + 2, 76, 24, () -> cancel(id, peer));
            }
            y += ch + 6;
        }
        return y + 4;
    }

    // the save's POKéMON a trade may take (party and the box on screen; never one ON OFFER, walking or training)
    private void pickMine(String title, int want, java.util.function.BiConsumer<JSONObject, JSONObject> picked) {
        Model m = v.app.model();
        if (m == null) {
            return;
        }
        Menu menu = Menu.make(v, title);
        JSONArray party = m.party();
        int count = 0;
        for (int k = 0; party != null && k < party.length(); k++) {
            JSONObject mon = party.optJSONObject(k);
            if (mon != null && (want == 0 || mon.optInt("s") == want)) {
                final JSONObject place = Model.partyPlace(k);
                menu.add(monName(mon) + " Lv" + mon.optInt("l"), () -> picked.accept(mon, place));
                count++;
            }
        }
        int b = m.snap.optInt("box");
        JSONArray box = m.box(b).optJSONArray("m");
        for (int k = 0; box != null && k < box.length(); k++) {
            JSONObject mon = box.optJSONObject(k);
            if (mon != null && (want == 0 || mon.optInt("s") == want)) {
                final JSONObject place = Model.boxPlace(b, k);
                menu.add(monName(mon) + " Lv" + mon.optInt("l") + " (BOX)", () -> picked.accept(mon, place));
                count++;
            }
        }
        if (count == 0) {
            v.message(want == 0 ? "No POKéMON to trade here." : "No " + wants(want) + " in your party or this box. Open the box it's in on BOXES, then come back.");
            return;
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void startTrade(JSONObject friend, JSONObject presence) {
        String name = nameOf(friend.optString("n"));
        if (!friend.has("pk")) {
            v.message("Play together with " + name + " in a room once more: then you can trade by mail.");
            return;
        }
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        pickMine("OFFER " + name + "...", 0, (mon, place) -> {
            Menu w = Menu.make(v, "FOR WHAT?");
            w.add("ANYTHING (" + name + " CHOOSES)", () -> sendOffer(friend, mon, place, 0));
            JSONArray party = presence == null ? null : presence.optJSONArray("party");
            java.util.Set<Integer> seen = new java.util.HashSet<>();
            for (int k = 0; party != null && k < party.length(); k++) {
                JSONArray p = party.optJSONArray(k);
                final int s = p.optInt(0);
                if (p.optInt(2) != 1 && seen.add(s)) {
                    w.add(art.plain(art.speciesName(s)), () -> sendOffer(friend, mon, place, s));
                }
            }
            w.add("CANCEL", null);
            v.showMenu(w);
        });
    }

    private void sendOffer(JSONObject friend, JSONObject mon, JSONObject place, int want) {
        String name = nameOf(friend.optString("n"));
        v.buzz(true);
        v.app.offerTrade(friend, mon, place, want, why -> v.message(why != null ? why
                : monName(mon) + " is ON OFFER to " + name + ". It stays in your game until " + name + " answers."));
    }

    private void accept(JSONObject offer) {
        String peer = nameOf(offer.optString("n"));
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        if (v.app.client == null) {
            v.message("Link with your game first.");
            return;
        }
        int want = offer.optInt("want");
        pickMine("GIVE " + peer + "...", want, (mon, place) -> {
            try {
                v.app.client.order(com.moemon.companion.core.Trades.accept(offer, mon, place));
            } catch (JSONException ignored) {
            }
            v.buzz(true);
            v.message("Deal! " + monName(mon) + " goes to " + peer + " the next time your game is online. "
                    + monName(offer.optJSONObject("mon")) + " comes back the same way.");
        });
    }

    private void decline(JSONObject offer) {
        String peer = nameOf(offer.optString("n"));
        Menu m = Menu.make(v, "TURN DOWN " + peer + "'S OFFER?");
        m.add("TURN IT DOWN", () -> v.app.callOffTrade(offer.optString("t"), 1, why -> v.message(why != null ? why : "Turned down.")));
        m.add("CANCEL", null);
        v.showMenu(m);
    }

    private void cancel(String t, String peer) {
        Menu m = Menu.make(v, "CALL OFF THE TRADE?");
        m.add("CALL IT OFF", () -> v.app.callOffTrade(t, 0, why -> v.message(why != null ? why
                : "Called off. It's yours again, and " + peer + " can't take it.")));
        m.add("KEEP IT ON OFFER", null);
        v.showMenu(m);
    }

    // --------------------------------------------------------------------------------------------- sharing, JOIN

    private void setShare(int level) {
        if (v.app.client == null) {
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "share").put("level", level));
        } catch (JSONException ignored) {
        }
        JSONObject fs = friendsSnap();
        if (fs != null) {
            try {
                fs.put("share", level);   // (the game confirms it with its next snapshot)
            } catch (JSONException ignored) {
            }
        }
        v.buzz(false);
    }

    private void join(String name, JSONObject p) {
        boolean linked = v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
        if (!linked || !p.has("room")) {
            String code = p.optString("code");
            if (code.isEmpty()) {
                v.message("Link with your game first: it joins " + name + "'s room for you.");
                return;
            }
            android.content.ClipboardManager cm = (android.content.ClipboardManager) v.getContext()
                    .getSystemService(android.content.Context.CLIPBOARD_SERVICE);
            if (cm != null) {
                cm.setPrimaryClip(android.content.ClipData.newPlainText("invite code", code));
            }
            v.message(name + "'s invite code " + code + " is copied. Type it in CO-OP > JOIN A GAME > ENTER CODE OR IP.");
            return;
        }
        Menu m = Menu.make(v, "JOIN " + name + "?");
        m.add("JOIN", () -> {
            try {
                v.app.client.order(new JSONObject().put("op", "join").put("srv", p.optString("srv"))
                        .put("room", p.optString("room")).put("rkey", p.optString("rkey")));
                v.message("Your game is joining " + name + "'s room.");
            } catch (JSONException ignored) {
            }
        });
        m.add("CANCEL", null);
        v.showMenu(m);
    }

    @Override
    public void tap(int x, int y) {
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                actions.get(i).run();
                return;
            }
        }
    }
}
