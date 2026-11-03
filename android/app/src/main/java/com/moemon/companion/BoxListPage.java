package com.moemon.companion;

import android.graphics.Canvas;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/** All 40 boxes: name and how full, tap one to open it */
public final class BoxListPage extends Page {
    private final BoxesPage boxes;
    private static final int ROW = 22;

    public BoxListPage(GameView v, BoxesPage boxes) {
        super(v);
        this.boxes = boxes;
    }

    private JSONArray list() {
        Model m = v.app.model();
        return m == null ? new JSONArray() : m.boxes();
    }

    @Override
    public int scrollMax() {
        return Math.max(0, (list().length() + 1) / 2 * ROW + 30 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = list();
        art.text(c, "BOXES", 8, 4 - scroll, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        int cw = (w - 12) / 2;
        for (int i = 0; i < l.length(); i++) {
            JSONObject b = l.optJSONObject(i);
            int x = 6 + (i % 2) * cw, y = 24 + (i / 2) * ROW - scroll;
            if (y < -ROW || y > h) {
                continue;
            }
            art.window(c, 0, x + 6, y + 4, cw - 12, 14);
            art.textHex(c, b.optString("n"), x + 8, y + 3, GameArt.TEXT, GameArt.SHADOW);
            int n = 0;
            JSONArray s = b.optJSONArray("m");
            for (int k = 0; s != null && k < s.length(); k++) {
                n += s.optJSONObject(k) != null ? 1 : 0;
            }
            String count = n + "/30";
            art.text(c, count, x + cw - 8 - art.width(count), y + 3, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
    }

    @Override
    public void tap(int x, int y) {
        int cw = (v.width() - 12) / 2;
        int i = (y + scroll - 24) / ROW * 2 + Math.min(1, Math.max(0, (x - 6) / cw));
        if (y + scroll >= 24 && i >= 0 && i < list().length()) {
            boxes.showBox(i);
            v.showOver(null);
        }
    }
}
