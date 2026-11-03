package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.LinkClient;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The POKéNAV's MATCH CALL (docs/COMPANION_APP.md "MOENAV calls"): the list as the POKéNAV builds it, each person's
 * picture, where they are and the POKé BALL of a trainer ready for a rematch. CALL asks the game for the call (its
 * own words, made the way the POKéNAV makes them: a trainer can ask for a rematch on the call); CHECK reads their
 * check page.
 */
public final class CallsPage extends Page {
    private static final int ROW = 40, TOP = 24;
    private final Paint fill = new Paint();
    private JSONObject open;        // the person whose call or check page is shown
    private boolean checking;       // CHECK (else CALL)
    private long calledAt;

    public CallsPage(GameView v) {
        super(v);
    }

    private boolean linked() {
        return v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
    }

    @Override
    public void shown() {
        if (linked()) {
            v.app.client.ask("calls");
        }
    }

    private JSONArray list() {
        JSONObject c = v.app.calls;
        JSONArray l = c == null ? null : c.optJSONArray("list");
        return l != null ? l : new JSONArray();
    }

    @Override
    public int scrollMax() {
        return Math.max(0, list().length() * ROW + TOP + 8 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = list();
        JSONArray secs = art.data.optJSONArray("mapsecs");
        for (int i = 0; i < l.length(); i++) {
            JSONObject e = l.optJSONObject(i);
            int y = TOP + i * ROW - scroll;
            if (y < -ROW || y > h) {
                continue;
            }
            // a row as the POKéNAV's list draws it: a light blue strip with the picture's square
            fill.setColor(0xFF183870);
            c.drawRect(8, y + 2, w - 8, y + ROW - 1, fill);
            fill.setColor(open == e ? 0xFFF8E8B0 : 0xFFE0F0F8);
            c.drawRect(9, y + 3, w - 9, y + ROW - 2, fill);
            fill.setColor(0xFF88B8E0);
            c.drawRect(10, y + 4, 44, y + ROW - 3, fill);
            art.trainerPic(c, e.optInt("pic"), 11, y + 4, 32);
            int x = 50;
            x += art.textHex(c, e.optString("c"), x, y + 3, GameArt.TEXT, GameArt.SHADOW) + 4;
            art.textHex(c, e.optString("n"), x, y + 3, GameArt.BLUE_TEXT, GameArt.SHADOW);
            art.textHex(c, secs == null ? "" : secs.optString(e.optInt("sec")), 50, y + 19, GameArt.GRAY_TEXT, GameArt.SHADOW);
            if (e.optInt("r") == 1) {
                art.ballIcon(c, 4, w - 34, y + 8);
            }
        }
        if (l.length() == 0) {
            art.window(c, 0, 12, TOP + 8, w - 24, 30);
            int y = TOP + 8;
            String why = v.app.calls == null ? (linked() ? "..." : "Link with the game to see your MATCH CALL list.")
                    : v.app.calls.optInt("has") == 0 ? "The POKéNAV comes from DEVON CORP. in RUSTBORO CITY."
                    : "No one is registered yet.";
            for (String line : wrap(why, w - 32)) {
                art.text(c, line, 16, y);
                y += 14;
            }
        }
        if (open != null) {
            drawOpen(c, w, h);
        }
        banner(c, w, "MATCH CALL", "BACK");
    }

    // the call's words, or the check page's, in the game's message window at the bottom
    private void drawOpen(Canvas c, int w, int h) {
        String text;
        JSONObject call = v.app.call;
        if (checking) {
            StringBuilder s = new StringBuilder();
            JSONArray chk = open.optJSONArray("chk");
            for (int i = 0; chk != null && i < chk.length(); i++) {
                s.append(chk.optString(i)).append(i + 1 < chk.length() ? "00" : "");   // (a space between)
            }
            text = s.toString();
        } else if (call != null && call.optInt("k") == open.optInt("k") && call.optInt("i") == open.optInt("i")
                && call.optLong("at", 0) >= 0 && v.app.callAt >= calledAt) {
            text = call.optInt("busy") == 1 ? null : call.optString("text");
            if (text == null) {
                text = "";
            }
        } else {
            text = "";
        }
        int lines = Math.max(2, art.wrap(GameArt.hexText(text), w - 40).size());
        int wh = lines * 14 + 8, wy = h - wh - 12;
        art.window(c, 0, 16, wy, w - 32, wh);
        if (!checking && text.isEmpty()) {
            boolean busy = call != null && call.optInt("busy") == 1 && v.app.callAt >= calledAt;
            art.text(c, busy ? "The game is busy. Try again." : "CALLING...", 20, wy + 2);
            if (!busy) {
                v.postInvalidateDelayed(300);
            }
            return;
        }
        art.paragraph(c, text, 20, wy + 2, w - 40, GameArt.TEXT, GameArt.SHADOW);
        if (!checking && call != null && call.optInt("r") == 1) {
            art.ballIcon(c, 4, w - 44, wy - 22);
        }
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20) {
            if (x > v.width() - 60) {
                back();
            }
            return;
        }
        if (open != null) {
            open = null;
            return;
        }
        int i = (y + scroll - TOP) / ROW;
        JSONArray l = list();
        if (y + scroll < TOP || i < 0 || i >= l.length()) {
            return;
        }
        final JSONObject e = l.optJSONObject(i);
        Menu m = Menu.make(v, art.plain(GameArt.hexText(e.optString("n"))));
        m.add("CALL", () -> call(e));
        if (e.has("chk")) {
            m.add("CHECK", () -> {
                open = e;
                checking = true;
            });
        }
        m.add("CANCEL", null);
        v.showMenu(m);
    }

    private void call(JSONObject e) {
        if (!linked()) {
            v.message("Link with the game to make a call.");
            return;
        }
        open = e;
        checking = false;
        calledAt = System.currentTimeMillis();
        try {
            v.app.client.send(new JSONObject().put("t", "call").put("k", e.optInt("k")).put("i", e.optInt("i")));
        } catch (org.json.JSONException ignored) {
        }
        v.buzz(false);
    }

    @Override
    public void back() {
        if (open != null) {
            open = null;
            v.invalidate();
            return;
        }
        super.back();
    }
}
