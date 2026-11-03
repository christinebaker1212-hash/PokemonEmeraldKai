package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.List;

/**
 * A POKéDEX entry as the game keeps it: the picture, its category, height and weight, types, and the entries of
 * RUBY, SAPPHIRE, EMERALD and FIRERED (the TEXT tab's), with its base stats. Unseen: a shadow and "?????"; seen
 * but not caught: no entry text yet, as in the game. Swipe for the next.
 */
public final class DexEntryPage extends Page {
    private static final String[] GAMES = {"RUBY", "SAPPHIRE", "EMERALD", "FIRERED"};
    private static final String[] STATS = {"HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF", "SPEED"};
    private final List<Integer> nums;
    private int index, height;
    private final Paint fill = new Paint();

    public DexEntryPage(GameView v, List<Integer> nums, int index) {
        super(v);
        this.nums = nums;
        this.index = index;
    }

    private JSONObject dex() {
        org.json.JSONObject d = v.app.model() == null ? null : v.app.model().snap.optJSONObject("dex");
        return d != null ? d : new JSONObject();
    }

    @Override
    public int scrollMax() {
        return Math.max(0, height - v.contentHeight());
    }

    static String feet(int dm) {
        int inches = (int) Math.round(dm * 3.937);
        return (inches / 12) + "'" + String.format(java.util.Locale.US, "%02d", inches % 12) + "\"";
    }

    static String pounds(int hg) {
        return String.format(java.util.Locale.US, "%.1f lbs.", hg * 0.22046);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        int n = nums.get(index);
        JSONObject d = dex();
        boolean seen = DexPage.bit(d.optString("seen"), n), caught = DexPage.bit(d.optString("caught"), n);
        int sp = art.speciesOfDex(n);
        JSONArray entries = art.data.optJSONArray("dex");
        JSONObject e = entries == null ? new JSONObject() : entries.optJSONObject(n);
        if (e == null) {
            e = new JSONObject();
        }
        c.save();
        c.translate(0, -scroll);
        int y = 24;
        // the picture, twice the size, on its stage
        art.window(c, 0, 12, y + 4, 132, 132);
        fill.setColor(0xFFE8F0F8);
        c.drawRect(12, y + 4, 144, y + 136, fill);
        art.front(c, sp, null, 14, y + 6, 128, !seen);
        // the facts
        int x = 154;
        art.window(c, 0, x, y + 4, w - x - 12, 132);
        String no = String.format(java.util.Locale.US, "No%03d", n);
        art.text(c, no, x + 2, y + 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (seen) {
            art.text(c, art.speciesName(sp), x + 2, y + 18, GameArt.TEXT, GameArt.SHADOW);
            byte[] cat = GameArt.hexText(e.optString("cat"));
            art.text(c, art.plain(cat) + " POKéMON", x + 2, y + 34, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.text(c, "HT  " + feet(e.optInt("h")), x + 2, y + 52);
            art.text(c, "WT  " + pounds(e.optInt("w")), x + 2, y + 68);
            JSONArray t = art.species(sp).optJSONArray("t");
            if (t != null) {
                art.typeIcon(c, t.optInt(0), x + 2, y + 90);
                if (t.optInt(1) != t.optInt(0)) {
                    art.typeIcon(c, t.optInt(1), x + 38, y + 90);
                }
            }
            chip(c, caught ? "OWNED" : "SEEN", x + 2, y + 116, caught ? 0xFFD84838 : 0xFF6888A8);
        } else {
            art.text(c, "?????", x + 2, y + 18);
            art.text(c, "HT  ?'??\"", x + 2, y + 52);
            art.text(c, "WT  ????.? lbs.", x + 2, y + 68);
        }
        y += 146;
        // the entries
        JSONArray tx = e.optJSONArray("tx");
        if (caught && tx != null) {
            for (int g = 0; g < 4; g++) {
                if (tx.isNull(g)) {
                    continue;
                }
                java.util.List<byte[]> lines = art.wrap(GameArt.hexText(tx.optString(g)), w - 40);
                int bh = 18 + lines.size() * 14;
                art.window(c, 0, 12, y + 4, w - 24, bh);
                art.text(c, GAMES[g], 16, y + 2, GameArt.BLUE_TEXT, GameArt.SHADOW);
                int ly = y + 18;
                for (byte[] l : lines) {
                    art.text(c, l, 16, ly);
                    ly += 14;
                }
                y += bh + 12;
            }
        } else {
            art.window(c, 0, 12, y + 4, w - 24, 16);
            art.text(c, seen ? "Catch it to read its entry." : "Not seen yet.", 16, y + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            y += 30;
        }
        // base stats
        JSONArray bs = art.species(sp).optJSONArray("bs");
        if (seen && bs != null) {
            art.window(c, 0, 12, y + 4, w - 24, 6 * 14 + 22);
            art.text(c, "BASE STATS", 16, y + 2, GameArt.BLUE_TEXT, GameArt.SHADOW);
            int total = 0;
            for (int i = 0; i < 6; i++) {
                int val = bs.optInt(i);
                total += val;
                int ry = y + 18 + i * 14;
                art.text(c, STATS[i], 16, ry);
                String sv = String.valueOf(val);
                art.text(c, sv, 106 - art.width(sv), ry);
                fill.setColor(val >= 120 ? 0xFF30C860 : val >= 90 ? 0xFF88D040 : val >= 60 ? 0xFFF8C838 : 0xFFF07838);
                c.drawRect(112, ry + 5, 112 + Math.min(w - 140, val * (w - 140) / 200), ry + 10, fill);
            }
            String tot = "TOTAL " + total;
            art.text(c, tot, w - 18 - art.width(tot), y + 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
            y += 6 * 14 + 34;
        }
        height = y + 8;
        c.restore();
        banner(c, w, "POKéDEX", (index + 1) + "/" + nums.size() + "  BACK");
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20 && x > v.width() - 60) {
            back();
        }
    }

    @Override
    public void swipe(int dir) {
        index = Math.max(0, Math.min(nums.size() - 1, index + dir));
        scroll = 0;
    }
}
