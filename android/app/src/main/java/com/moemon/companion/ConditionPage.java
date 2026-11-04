package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The POKéNAV's CONDITION: a party POKéMON's five contest conditions on the POKéNAV's pentagon (COOL at the top, then
 * BEAUTY, CUTE, SMART and TOUGH going round), its sheen as sparkles, its nature. LEFT / RIGHT or a swipe goes through
 * the party; the boxes' POKéMON are under BOXES as in the POKéNAV's search.
 */
public final class ConditionPage extends Page {
    private static final String[] NAMES = {"COOL", "BEAUTY", "CUTE", "SMART", "TOUGH"};
    private static final int[] COLORS = {0xFFE85050, 0xFF5080F0, 0xFFF080C0, 0xFF50C050, 0xFFE8C030};
    private final Paint fill = new Paint(), line = new Paint();
    private final Path path = new Path();
    private int slot;

    public ConditionPage(GameView v) {
        super(v);
        line.setStyle(Paint.Style.STROKE);
        line.setStrokeWidth(1);
    }

    private JSONArray party() {
        Model m = v.app.model();
        JSONArray p = m == null ? null : m.party();
        return p != null ? p : new JSONArray();
    }

    private JSONObject mon() {
        JSONArray p = party();
        for (int k = 0; k < p.length(); k++) {
            JSONObject o = p.optJSONObject((slot + k) % p.length());
            if (o != null && o.optInt("e") != 1) {
                slot = (slot + k) % p.length();
                return o;
            }
        }
        return null;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        // the POKéNAV's CONDITION backdrop: deep blue with soft rings
        fill.setColor(0xFF203070);
        c.drawRect(0, 20, w, h, fill);
        JSONObject m = mon();
        // the party along the top: icons, the chosen one raised
        JSONArray p = party();
        int iw = Math.min(40, (w - 16) / Math.max(1, p.length()));
        for (int i = 0; i < p.length(); i++) {
            JSONObject o = p.optJSONObject(i);
            if (o == null) {
                continue;
            }
            int x = 8 + i * iw, y = 24 + (i == slot ? 0 : 4);
            if (i == slot) {
                fill.setColor(0xFF4868B8);
                c.drawRect(x, 24, x + iw - 2, 58, fill);
            }
            art.icon(c, o.optInt("e") == 1 ? art.data.optInt("egg") : o.optInt("s"), x + (iw - 34) / 2, y,
                    o.optInt("e") == 1 ? GameArt.ICON_GRAY : GameArt.ICON_PLAIN);
        }
        if (m == null) {
            art.text(c, v.app.model() == null ? "Nothing from the game yet." : "No POKéMON to show.", 16, 70,
                    GameArt.WHITE_TEXT, 0xFF101840);
            banner(c, w, "CONDITION", "BACK");
            return;
        }
        // name, level, nature
        int y = 64;
        art.textHex(c, m.optString("n"), 12, y, GameArt.WHITE_TEXT, 0xFF101840);
        String lv = "Lv" + m.optInt("l");
        art.text(c, lv, w - 12 - art.width(lv), y, GameArt.WHITE_TEXT, 0xFF101840);
        JSONArray natures = art.data.optJSONArray("natures");
        if (natures != null) {
            art.textHex(c, natures.optString(m.optInt("nat")), 12, y + 14, GOLD, 0xFF101840);
        }
        // the picture and the pentagon side by side (stacked on a narrow screen)
        JSONArray cond = m.optJSONArray("cond");
        int r = Math.min(56, (w - 24) / 4), cx = w / 2 + (w >= 300 ? 40 : 0), cy = y + 46 + r;
        if (w >= 300) {
            art.front(c, m.optInt("s"), art.monPal(m), 8, cy - 40, 80, false);
        }
        // the rings
        line.setColor(0xFF5868B8);
        for (int ring = 1; ring <= 4; ring++) {
            polygon(c, cx, cy, r * ring / 4, null, line);
        }
        // the POKéMON's shape
        float[] vals = new float[5];
        for (int i = 0; i < 5; i++) {
            vals[i] = cond == null ? 0 : Math.min(255, cond.optInt(i)) / 255f;
        }
        fill.setColor(0xC0F8B8E8);
        polygon(c, cx, cy, r, vals, fill);
        line.setColor(0xFFF8F8F8);
        polygon(c, cx, cy, r, vals, line);
        // the corners' names and numbers
        for (int i = 0; i < 5; i++) {
            double a = -Math.PI / 2 + i * 2 * Math.PI / 5;
            int lx = cx + (int) Math.round(Math.cos(a) * (r + 12)), ly = cy + (int) Math.round(Math.sin(a) * (r + 12));
            String n = NAMES[i];
            art.text(c, n, lx - art.width(n) / 2, ly - 12, COLORS[i], 0xFF101840);
            String num = "" + (cond == null ? 0 : cond.optInt(i));
            art.text(c, num, lx - art.width(num) / 2, ly, GameArt.WHITE_TEXT, 0xFF101840);
        }
        // the sheen: a sparkle for each 10% of 255, as the POKéNAV's
        int sheen = cond == null ? 0 : cond.optInt(5), sparks = (sheen * 10 + 254) / 255;
        int sy = cy + r + 30;
        art.window(c, 0, 12, sy, w - 24, 16);
        art.text(c, "SHEEN", 16, sy);
        for (int i = 0; i < 10; i++) {
            int sx = 70 + i * 14;
            fill.setColor(i < sparks ? 0xFFF8D040 : 0xFFD0D0D0);
            c.drawRect(sx + 3, sy + 2, sx + 6, sy + 13, fill);
            c.drawRect(sx, sy + 6, sx + 9, sy + 9, fill);
        }
        String sh = sheen + "/255";
        art.text(c, sh, w - 18 - art.width(sh), sy, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (cond == null) {
            int ty = sy + 26;
            for (String l : wrap("POKéBLOCKS raise these. They show in CONTESTS.", w - 24)) {
                art.text(c, l, 12, ty, GameArt.WHITE_TEXT, 0xFF101840);
                ty += 14;
            }
        }
        banner(c, w, "CONDITION", "BACK");
    }

    private void polygon(Canvas c, int cx, int cy, int r, float[] vals, Paint p) {
        path.reset();
        for (int i = 0; i < 5; i++) {
            double a = -Math.PI / 2 + i * 2 * Math.PI / 5;
            float k = vals == null ? 1 : Math.max(0.04f, vals[i]);
            float x = cx + (float) (Math.cos(a) * r * k), y = cy + (float) (Math.sin(a) * r * k);
            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        path.close();
        c.drawPath(path, p);
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
        JSONArray p = party();
        int iw = Math.min(40, (w - 16) / Math.max(1, p.length()));
        if (y < 60 && x >= 8) {
            int i = (x - 8) / iw;
            if (i < p.length() && p.optJSONObject(i) != null && p.optJSONObject(i).optInt("e") != 1) {
                slot = i;
                v.buzz(false);
            }
        }
    }

    @Override
    public void swipe(int dir) {
        JSONArray p = party();
        if (p.length() == 0) {
            return;
        }
        for (int k = 1; k <= p.length(); k++) {
            int i = (slot + dir * k + p.length() * 6) % p.length();
            JSONObject o = p.optJSONObject(i);
            if (o != null && o.optInt("e") != 1) {
                slot = i;
                return;
            }
        }
    }
}
