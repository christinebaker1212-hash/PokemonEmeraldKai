package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Which box? All 40 boxes on their wallpapers with their free places; only a box with room for every one coming home
 * can be chosen (the EGG INCUBATOR's BRING HOME: the game checks it again).
 */
public final class BoxChoosePage extends Page {
    public interface Choose {
        void box(int box);
    }

    private static final int CW = 86, CH = 84, TOP = 24;
    private final Paint shade = new Paint();
    private final String title;
    private final int need;
    private final Choose choose;

    public BoxChoosePage(GameView v, String title, int need, Choose choose) {
        super(v);
        this.title = title;
        this.need = need;
        this.choose = choose;
        shade.setColor(0xA0283848);
    }

    private JSONArray list() {
        Model m = v.app.model();
        return m == null ? new JSONArray() : m.boxes();
    }

    private int cols() {
        return Math.max(1, (v.width() - 8) / CW);
    }

    /** Free places in box I */
    public static int free(JSONObject box) {
        int n = 0;
        JSONArray s = box == null ? null : box.optJSONArray("m");
        for (int k = 0; s != null && k < s.length(); k++) {
            n += s.optJSONObject(k) != null ? 1 : 0;
        }
        return 30 - n;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, (list().length() + cols() - 1) / cols() * CH + TOP + 24 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = list();
        int cols = cols(), left = (w - cols * CW) / 2;
        String hint = need == 1 ? "A box with a free place" : "A box with " + need + " free places";
        art.text(c, hint, (w - art.width(hint)) / 2, TOP - scroll, GameArt.WHITE_TEXT, 0xFF305060);
        for (int i = 0; i < l.length(); i++) {
            JSONObject b = l.optJSONObject(i);
            int x = left + (i % cols) * CW + 3, y = TOP + 18 + (i / cols) * CH - scroll;
            if (y < TOP - CH || y > h) {
                continue;
            }
            int free = free(b);
            art.wallpaperHalf(c, b.optInt("w"), x, y);
            byte[] name = GameArt.hexText(b.optString("n"));
            art.text(c, name, x + (80 - art.width(name)) / 2, y - 2, GameArt.WHITE_TEXT, 0xFF383838);
            c.drawRect(x + 14, y + 56, x + 66, y + 69, shade);
            String s = free + " FREE";
            art.text(c, s, x + (80 - art.width(s)) / 2, y + 54, free >= need ? GameArt.WHITE_TEXT : 0xFFF86858, 0);
            if (free < need) {
                c.drawRect(x, y + 8, x + 80, y + 52, shade);   // (no room: greyed)
            }
        }
        banner(c, w, title, "BACK");
    }

    @Override
    public void back() {
        v.showOver(null);
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20) {
            if (x > v.width() - 60) {
                back();
            }
            return;
        }
        int cols = cols(), left = (v.width() - cols * CW) / 2;
        int col = (x - left) / CW, row = (y + scroll - TOP - 18) / CH;
        int i = row * cols + col;
        JSONArray l = list();
        if (col < 0 || col >= cols || y + scroll < TOP + 18 || i >= l.length()) {
            return;
        }
        if (free(l.optJSONObject(i)) < need) {
            v.message(need == 1 ? "That box is full." : "That box hasn't room for all " + need + ".");
            return;
        }
        v.showOver(null);
        choose.box(i);
    }
}
