package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONObject;

/** One TRIPLE TRIAD card up close: the face large, its name, stars and type, how many you have, and its story */
public final class TtCardPage extends Page {
    private final int id, count;
    private final Paint fill = new Paint();

    public TtCardPage(GameView v, int id, int count) {
        super(v);
        this.id = id;
        this.count = count;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        TtArt ta = v.app.ttArt;
        JSONObject k = ta.card(id);
        fill.setColor(0xFF14284A);
        c.drawRect(0, 0, w, h, fill);
        float cw = Math.min(w - 80, 156), ch = cw * TtArt.H / TtArt.W;
        ta.draw(c, count > 0 ? id : id, TtArt.SEAT_NONE, null, (w - cw) / 2, 10, cw, ch, 1, count > 0 ? 255 : 110);
        int y = (int) (ch + 18);
        String name = k == null ? "?" : k.optString("n");
        art.text(c, name, (w - art.width(name)) / 2, y, 0xFFF8E070, 0xFF000000);
        y += 16;
        int stars = k == null ? 0 : k.optInt("st");
        String info = stars + (stars == 1 ? " STAR  " : " STARS  ")
                + (ta.words == null || k == null || k.optInt("ty") == 0 ? "" : ta.words.optJSONArray("types").optString(k.optInt("ty")) + "  ")
                + (count > 0 ? "YOU HAVE " + count : "NOT YOURS YET");
        art.text(c, info, (w - art.width(info)) / 2, y, 0xFFB8C8E8, 0xFF000000);
        y += 20;
        java.util.List<String> lines = wrap(k == null ? "" : k.optString("t"), w - 32);
        art.window(c, 0, 10, y, w - 20, lines.size() * 14 + 4);
        int ty = y + 2;
        for (String l : lines) {
            art.text(c, l, 16, ty);
            ty += 14;
        }
    }

    @Override
    public void tap(int x, int y) {
        v.showOver(null);
    }
}
