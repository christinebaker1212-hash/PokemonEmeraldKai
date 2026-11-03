package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * One set of the POKéMON BINDER: its cards in the game's order, the ones you have bright with their count, the
 * ones still to find dimmed so you can see what to look for. Tap a card to see it up close.
 */
public final class TcgBinderPage extends Page {
    private final int set;
    private final String title;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final List<int[]> hits = new ArrayList<>();
    private int bottom;

    public TcgBinderPage(GameView v, int set, String title) {
        super(v);
        this.set = set;
        this.title = title;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.height() + 10);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        background(c, w, h, scroll);
        TcgArt ta = v.app.tcgArt;
        int[] owned = CardsPage.ownedArray(CardsPage.tcg(v));
        int cols = 4, n = 0, got = 0;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.375f;
        float y0 = 24 - scroll;
        for (int id = 1; id <= ta.maxId(); id++) {
            JSONObject k = ta.card(id);
            if (k == null || k.optInt("set") != set) {
                continue;
            }
            float x = 8 + (n % cols) * (cw + 4), y = y0 + (n / cols) * (ch + 4);
            n++;
            got += owned[id] > 0 ? 1 : 0;
            hits.add(new int[]{(int) x, (int) y, (int) cw, (int) ch, id});
            if (y + ch < 0 || y > h) {
                continue;
            }
            ta.draw(c, id, x, y, cw, ch, v.pixelScale(), owned[id] > 0 ? 255 : 70);
            if (owned[id] > 1) {
                String t = "x" + owned[id];
                int tw = art.width(t) + 4;
                fill.setColor(0xE0102030);
                c.drawRoundRect(new RectF(x + cw - 2 - tw, y + ch - 13, x + cw - 2, y + ch - 2), 3, 3, fill);
                art.text(c, t, (int) (x + cw - tw), (int) (y + ch - 16), GameArt.WHITE_TEXT, 0);
            }
        }
        bottom = (int) (y0 + scroll + ((n + cols - 1) / cols) * (ch + 4)) + 10;
        banner(c, w, title, got + "/" + n);
    }

    @Override
    public void tap(int x, int y) {
        int[] owned = CardsPage.ownedArray(CardsPage.tcg(v));
        for (int[] r : hits) {
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                v.showOver(new TcgCardPage(v, r[4], owned[r[4]], this));
                return;
            }
        }
    }
}
