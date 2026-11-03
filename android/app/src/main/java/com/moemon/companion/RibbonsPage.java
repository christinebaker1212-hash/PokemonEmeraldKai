package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * The POKéNAV's RIBBONS: every POKéMON with ribbons (the party, then the boxes, as the POKéNAV lists them), and a
 * POKéMON's ribbons in the POKéNAV's order with the game's own words for the one tapped.
 */
public final class RibbonsPage extends Page {
    private static final int ROW = 36, TOP = 24, ICON = 32;
    private final Paint fill = new Paint();
    private JSONObject mon;     // the POKéMON whose ribbons are open
    private int chosen = -1;

    public RibbonsPage(GameView v) {
        super(v);
    }

    private List<JSONObject> withRibbons() {
        List<JSONObject> out = new ArrayList<>();
        Model m = v.app.model();
        if (m == null) {
            return out;
        }
        JSONArray p = m.party();
        for (int i = 0; p != null && i < p.length(); i++) {
            JSONObject o = p.optJSONObject(i);
            if (o != null && o.optJSONArray("rib") != null) {
                out.add(o);
            }
        }
        JSONArray boxes = m.boxes();
        for (int b = 0; boxes != null && b < boxes.length(); b++) {
            JSONArray s = boxes.optJSONObject(b).optJSONArray("m");
            for (int k = 0; s != null && k < s.length(); k++) {
                JSONObject o = s.optJSONObject(k);
                if (o != null && o.optJSONArray("rib") != null) {
                    out.add(o);
                }
            }
        }
        return out;
    }

    private int perRow() {
        return Math.max(1, (v.width() - 24) / (ICON + 4));
    }

    @Override
    public int scrollMax() {
        if (mon != null) {
            return 0;
        }
        return Math.max(0, withRibbons().size() * ROW + TOP + 8 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        if (mon != null) {
            drawMon(c, w, h);
            banner(c, w, "RIBBONS", "BACK");
            return;
        }
        List<JSONObject> l = withRibbons();
        for (int i = 0; i < l.size(); i++) {
            JSONObject o = l.get(i);
            int y = TOP + i * ROW - scroll;
            if (y < -ROW || y > h) {
                continue;
            }
            art.window(c, 0, 12, y + 4, w - 24, ROW - 10);
            art.icon(c, o.optInt("s"), 12, y + 1, GameArt.ICON_PLAIN);
            art.textHex(c, o.optString("n"), 48, y + 6, GameArt.TEXT, GameArt.SHADOW);
            String n = o.optJSONArray("rib").length() + " RIBBONS";
            art.text(c, n, w - 16 - art.width(n), y + 6, GameArt.BLUE_TEXT, GameArt.SHADOW);
        }
        if (l.isEmpty()) {
            art.window(c, 0, 12, TOP + 8, w - 24, 30);
            int y = TOP + 8;
            for (String line : wrap(v.app.model() == null ? "Nothing from the game yet."
                    : "No POKéMON has a RIBBON yet. CONTESTS and the BATTLE TOWER give them.", w - 32)) {
                art.text(c, line, 16, y);
                y += 14;
            }
        }
        banner(c, w, "RIBBONS", "BACK");
    }

    private void drawMon(Canvas c, int w, int h) {
        fill.setColor(0xFF305098);
        c.drawRect(0, 20, w, h, fill);
        art.icon(c, mon.optInt("s"), 8, 22, GameArt.ICON_PLAIN);
        art.textHex(c, mon.optString("n"), 44, 28, GameArt.WHITE_TEXT, 0xFF101840);
        JSONArray rib = mon.optJSONArray("rib");
        int per = perRow(), left = (w - per * (ICON + 4)) / 2;
        for (int i = 0; rib != null && i < rib.length(); i++) {
            int x = left + (i % per) * (ICON + 4), y = 60 + (i / per) * (ICON + 4);
            if (i == chosen) {
                fill.setColor(0xFFF8E070);
                c.drawRect(x - 2, y - 2, x + ICON + 2, y + ICON + 2, fill);
            }
            art.ribbon(c, rib.optInt(i), x, y, ICON);
        }
        int rows = rib == null ? 0 : (rib.length() + per - 1) / per;
        int wy = Math.max(60 + rows * (ICON + 4) + 12, h - 52);
        art.window(c, 0, 16, wy, w - 32, 32);
        if (rib == null || chosen < 0 || chosen >= rib.length()) {
            art.text(c, "Tap a RIBBON to read about it.", 20, wy + 8, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return;
        }
        String[] words = words(rib.optInt(chosen));
        art.paragraph(c, words[0], 20, wy + 1, w - 40, GameArt.RED_TEXT, GameArt.SHADOW);
        art.paragraph(c, words[1], 20, wy + 16, w - 40, GameArt.TEXT, GameArt.SHADOW);
    }

    /** A ribbon's two lines: a gift ribbon's are the save's own (the game's giftRibbons) */
    private String[] words(int id) {
        int firstGift = 25;   // MARINE_RIBBON (FIRST_GIFT_RIBBON)
        if (id < firstGift) {
            JSONArray r = art.data.optJSONArray("ribbons");
            JSONArray l = r == null ? null : r.optJSONArray(id);
            return new String[]{l == null ? "" : l.optString(0), l == null ? "" : l.optString(1)};
        }
        Model m = v.app.model();
        JSONArray gifts = m == null ? null : m.save().optJSONArray("gifts");
        int kind = gifts == null ? 0 : gifts.optInt(id - firstGift);
        JSONArray g = art.data.optJSONArray("giftRibbons");
        JSONArray l = kind > 0 && g != null ? g.optJSONArray(kind - 1) : null;
        return new String[]{l == null ? "" : l.optString(0), l == null ? "" : l.optString(1)};
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width();
        if (y < 20) {
            if (x > w - 60) {
                back();
            }
            return;
        }
        if (mon != null) {
            JSONArray rib = mon.optJSONArray("rib");
            int per = perRow(), left = (w - per * (ICON + 4)) / 2;
            int col = (x - left) / (ICON + 4), row = (y - 60) / (ICON + 4), i = row * per + col;
            if (x >= left && y >= 60 && col < per && rib != null && i < rib.length()) {
                chosen = i;
                v.buzz(false);
            }
            return;
        }
        List<JSONObject> l = withRibbons();
        int i = (y + scroll - TOP) / ROW;
        if (y + scroll >= TOP && i < l.size()) {
            mon = l.get(i);
            chosen = 0;
        }
    }

    @Override
    public void back() {
        if (mon != null) {
            mon = null;
            chosen = -1;
            v.invalidate();
            return;
        }
        super.back();
    }
}
