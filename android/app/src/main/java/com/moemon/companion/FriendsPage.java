package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * FRIENDS (docs/COMPANION_APP.md "Pillar 4: friends"): the people this save has played with, who's playing now, where
 * and doing what, as far as each shares; JOIN sends your game to their room. Your own sharing: EVERYTHING (where you
 * are, your room, your party), ONLINE ONLY, or HIDDEN.
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

    private static final String doing(String d) {
        switch (d) {
            case "battle": return "In a battle";
            case "coop": return "In a co-op battle";
            case "cards": return "In a card duel";
            case "field": return "Exploring";
            default: return "Busy";
        }
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
            hits.add(new int[]{x, y, cw, 18});
            actions.add(() -> setShare(level));
            x += cw + 2;
        }
        y += 24;
        JSONArray list = fs == null ? null : fs.optJSONArray("list");
        long now = System.currentTimeMillis() / 1000;
        int online = 0;
        for (int i = 0; list != null && i < list.length(); i++) {
            JSONObject f = list.optJSONObject(i);
            JSONObject p = v.app.friends.get(f.optString("id"));
            boolean on = com.moemon.companion.core.Friends.online(p, now);
            online += on ? 1 : 0;
            int rh = p != null && p.has("party") ? 64 : 40;
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
            art.text(c, state, 28, y + 18, GameArt.GRAY_TEXT, GameArt.SHADOW);
            if (p != null && p.has("party")) {
                JSONArray party = p.optJSONArray("party");
                for (int k = 0; k < party.length(); k++) {
                    JSONArray m = party.optJSONArray(k);
                    art.icon(c, m.optInt(2) == 1 ? art.data.optInt("egg") : m.optInt(0), 24 + k * 30, y + 28,
                            GameArt.ICON_PLAIN);
                }
            }
            if (on && p != null && (p.has("room") || p.has("code"))) {
                final JSONObject presence = p;
                final String name = art.plain(GameArt.hexText(f.optString("n")));
                button(c, "JOIN", w - 70, y + 2, 56, true);
                hits.add(new int[]{w - 70, y + 2, 56, 24});
                actions.add(() -> join(name, presence));
            }
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
