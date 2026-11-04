package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;

/** One set of the YU-GI-OH! BINDER: its cards in the set's order, the ones you have bright with their count. */
public final class YgoBinderPage extends Page {
    private final int set;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final List<int[]> hits = new ArrayList<>();
    private int bottom;

    public YgoBinderPage(GameView v, int set) {
        super(v);
        this.set = set;
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
        YgoArt ya = v.app.ygoArt;
        int[] owned = CardsPage.ygoOwned(CardsPage.ygo(v));
        JSONObject sj = ya.sets().optJSONObject(set);
        JSONArray cards = sj == null ? new JSONArray() : sj.optJSONArray("cards");
        LinkedHashSet<Integer> ids = new LinkedHashSet<>();
        for (int i = 0; i < cards.length(); i++) {
            ids.add(cards.optJSONArray(i).optInt(0));
        }
        int cols = 4, n = 0, got = 0;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.46f, y0 = 24 - scroll;
        for (int id : ids) {
            float x = 8 + (n % cols) * (cw + 4), y = y0 + (n / cols) * (ch + 4);
            n++;
            int have = id < owned.length ? owned[id] : 0;
            got += have > 0 ? 1 : 0;
            hits.add(new int[]{(int) x, (int) y, (int) cw, (int) ch, id, have});
            if (y + ch < 0 || y > h) {
                continue;
            }
            ya.draw(c, id, x, y, cw, ch, v.pixelScale(), have > 0 ? 255 : 70);
            if (have > 1) {
                String t = "x" + have;
                int tw = art.width(t) + 4;
                fill.setColor(0xE0102030);
                c.drawRoundRect(new RectF(x + cw - 2 - tw, y + ch - 13, x + cw - 2, y + ch - 2), 3, 3, fill);
                art.text(c, t, (int) (x + cw - tw), (int) (y + ch - 16), GameArt.WHITE_TEXT, 0);
            }
        }
        bottom = (int) (y0 + scroll + ((n + cols - 1) / cols) * (ch + 4)) + 10;
        banner(c, w, sj == null ? "" : sj.optString("short"), got + "/" + n);
    }

    @Override
    public void tap(int x, int y) {
        for (int[] r : hits) {
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                v.showOver(new YgoCardPage(v, r[4], r[5], this));
                return;
            }
        }
    }

    @Override
    public boolean swapsWords() {
        return false;   // (the card games' own words: the POKéMON card game is the POKéMON card game)
    }
}
