package com.moemon.companion;

import android.graphics.Canvas;

import com.moemon.companion.core.LinkClient;

import org.json.JSONArray;
import org.json.JSONObject;

/** A POKéMON's Garden JOURNAL (the summary's JOURNAL page in the game: its memories and what you know of it) */
public final class JournalPage extends Page {
    private final JSONObject place, mon;
    private boolean asked;

    public JournalPage(GameView v, JSONObject place, JSONObject mon) {
        super(v);
        this.place = place;
        this.mon = mon;
    }

    @Override
    public void shown() {
        if (v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED && !asked) {
            asked = true;
            try {
                v.app.client.send(new JSONObject().put("t", "journal").put("at", place).put("p", mon.optLong("p"))
                        .put("o", mon.optLong("o")));
            } catch (org.json.JSONException ignored) {
            }
        }
    }

    private int bottom;

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONObject j = v.app.journals.get(mon.optLong("p"));
        int y = 26 - scroll;
        art.icon(c, mon.optInt("s"), 8, y - 4, GameArt.ICON_PLAIN);
        art.textHex(c, mon.optString("n"), 44, y, GameArt.WHITE_TEXT, 0xFF305060);
        y += 26;
        JSONArray lines = j == null ? null : j.optJSONArray("lines");
        if (lines == null) {
            art.window(c, 0, 12, y + 4, w - 24, 16);
            art.text(c, v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED ? "..."
                    : "Link with the game to read its JOURNAL.", 16, y + 4);
            y += 24;
        } else {
            for (int i = 0; i < lines.length(); i++) {
                int rows = art.wrap(GameArt.hexText(lines.optString(i)), w - 40).size();
                art.window(c, 0, 12, y + 4, w - 24, rows * 14 + 2);
                art.paragraph(c, lines.optString(i), 16, y + 4, w - 40, GameArt.TEXT, GameArt.SHADOW);
                y += rows * 14 + 14;
            }
        }
        bottom = y + scroll + 8;
        banner(c, w, "JOURNAL", "BACK");
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20 && x > v.width() - 60) {
            back();
        }
    }
}
