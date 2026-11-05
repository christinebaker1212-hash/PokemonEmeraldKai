package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/** All 40 boxes on their wallpapers, with their names and how full they are; tap one to open it */
public final class BoxListPage extends Page {
    private final BoxesPage boxes;
    private static final int CW = 86, CH = 84, TOP = 24;
    private final Paint shade = new Paint();

    public BoxListPage(GameView v, BoxesPage boxes) {
        super(v);
        this.boxes = boxes;
        shade.setColor(0xA0283848);
    }

    private JSONArray list() {
        Model m = v.app.model();
        return m == null ? new JSONArray() : m.boxes();
    }

    private int cols() {
        return Math.max(1, (v.width() - 8) / CW);
    }

    @Override
    public int scrollMax() {
        return Math.max(0, (list().length() + cols() - 1) / cols() * CH + TOP + 8 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = list();
        int cols = cols(), left = (w - cols * CW) / 2;
        for (int i = 0; i < l.length(); i++) {
            JSONObject b = l.optJSONObject(i);
            int x = left + (i % cols) * CW + 3, y = TOP + (i / cols) * CH - scroll;
            if (y < TOP - CH || y > h) {
                continue;
            }
            art.wallpaperHalf(c, b.optInt("w"), x, y);
            int n = 0;
            JSONArray s = b.optJSONArray("m");
            for (int k = 0; s != null && k < s.length(); k++) {
                n += s.optJSONObject(k) != null ? 1 : 0;
            }
            byte[] name = GameArt.hexText(b.optString("n"));
            int nw = art.width(name);
            art.text(c, name, x + (80 - nw) / 2, y - 2, GameArt.WHITE_TEXT, 0xFF383838);
            c.drawRect(x + 22, y + 56, x + 58, y + 69, shade);
            String count = n + "/30";
            art.text(c, count, x + (80 - art.width(count)) / 2, y + 54, n == 30 ? GOLD : GameArt.WHITE_TEXT, 0);
        }
        banner(c, w, "BOXES", "BACK");
    }

    @Override
    public void back() {
        v.showOver(boxes.isPicker() ? boxes : null);
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20) {
            if (x > v.width() - 60) {
                if (boxes.isPicker()) {
                    v.showOver(boxes);
                } else {
                    back();
                }
            }
            return;
        }
        int cols = cols(), left = (v.width() - cols * CW) / 2;
        int col = (x - left) / CW, row = (y + scroll - TOP) / CH;
        int i = row * cols + col;
        if (col >= 0 && col < cols && y + scroll >= TOP && i < list().length()) {
            boxes.showBox(i);
            v.showOver(boxes.isPicker() ? boxes : null);   // (a picker: back to it)
        }
    }
}
