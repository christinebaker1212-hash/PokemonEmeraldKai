package com.moemon.companion;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.InputStream;
import java.util.List;

/**
 * A POKéDEX entry on the game's own entry screen (pokedex.c's info page, exported by the game: dex_info.png), each
 * line where the game prints it: the picture on its stage, the number and name, the category, HT and WT, the
 * footprint, the entry text in the box below. Under the screen, on the POKéDEX's green: the four games' entries
 * (RUBY, SAPPHIRE, EMERALD, FIRERED: the TEXT tab's), WHERE TO FIND (the game's wild tables, by the spawn viewer's
 * rules) and the base stats, in the list's yellow panels. Swipe or the arrows for the next one; unseen ones are a
 * shadow and "?????", seen ones have no entry text until caught, as in the game.
 */
public final class DexEntryPage extends Page {
    private static final String[] GAMES = {"RUBY", "SAPPHIRE", "EMERALD", "FIRERED"};
    private static final String[] STATS = {"HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF", "SPEED"};
    private final List<Integer> nums;
    private int index, height, game = -1;
    private final Paint fill = new Paint(), pix = new Paint();
    private final Rect src = new Rect(), dst = new Rect();
    private static Bitmap sScreen, sFootprints;

    public DexEntryPage(GameView v, List<Integer> nums, int index) {
        super(v);
        this.nums = nums;
        this.index = index;
        pix.setFilterBitmap(false);
    }

    private Bitmap asset(String name) {
        try (InputStream in = v.getContext().getAssets().open("game/" + name)) {
            BitmapFactory.Options o = new BitmapFactory.Options();
            o.inScaled = false;
            return BitmapFactory.decodeStream(in, null, o);
        } catch (Exception e) {
            return null;
        }
    }

    private JSONObject dex() {
        JSONObject d = v.app.model() == null ? null : v.app.model().snap.optJSONObject("dex");
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

    private int left() {
        return (v.width() - 240) / 2;
    }

    /** A card: the POKéDEX's yellow strip with its title in blue, a white body; returns the body's top */
    private int panel(Canvas c, int x, int y, int w, int h, String title) {
        fill.setColor(DexPage.BORDER);
        c.drawRect(x + 1, y, x + w - 1, y + h, fill);
        c.drawRect(x, y + 1, x + w, y + h - 1, fill);
        fill.setColor(0xFFF8F8F0);
        c.drawRect(x + 1, y + 1, x + w - 1, y + h - 1, fill);
        fill.setColor(DexPage.YELLOW);
        c.drawRect(x + 1, y + 1, x + w - 1, y + 16, fill);
        fill.setColor(DexPage.BORDER_LIGHT);
        c.drawRect(x + 1, y + 16, x + w - 1, y + 17, fill);
        art.text(c, title, x + 6, y - 1, GameArt.BLUE_TEXT, GameArt.SHADOW);
        return y + 20;
    }

    private static int lighter(int c, int k) {   // (k of 4 toward white)
        int r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
        return 0xFF000000 | ((r + (255 - r) * k / 4) << 16) | ((g + (255 - g) * k / 4) << 8) | (b + (255 - b) * k / 4);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        int n = nums.get(index);
        JSONObject d = dex();
        boolean seen = DexPage.bit(d.optString("seen"), n), caught = DexPage.bit(d.optString("caught"), n);
        int sp = art.speciesOfDex(n);
        JSONArray entries = art.data.optJSONArray("dex");
        JSONObject e = entries == null ? null : entries.optJSONObject(n);
        if (e == null) {
            e = new JSONObject();
        }
        JSONArray tx = e.optJSONArray("tx");
        if (game < 0 || tx == null || tx.isNull(game)) {
            game = 2;
            for (int g : new int[]{2, 0, 1, 3}) {
                if (tx != null && !tx.isNull(g)) {
                    game = g;
                    break;
                }
            }
        }
        DexPage.stripes(c, fill, w, h + 8, scroll);
        c.save();
        c.translate(0, -scroll);
        // the top row: BACK, where it is, the arrows
        DexPage.chip(c, art, fill, "BACK", 6, 4, false);
        String where = (index + 1) + " / " + nums.size();
        art.text(c, where, (w - art.width(where)) / 2, 2, 0xFFFFFFFF, 0xFF000000);
        DexPage.chip(c, art, fill, ">", w - 26, 4, true);
        DexPage.chip(c, art, fill, "<", w - 50, 4, true);
        // the hero: its picture, twice the size, on its type's color
        JSONArray t = seen ? art.species(sp).optJSONArray("t") : null;
        int tc = t != null ? art.typeColor(t.optInt(0)) : 0xFF909090;
        int hx = 6, hy = 24, hw = w - 12, hh = 196;
        fill.setColor(DexPage.BORDER);
        c.drawRect(hx + 1, hy, hx + hw - 1, hy + hh, fill);
        c.drawRect(hx, hy + 1, hx + hw, hy + hh - 1, fill);
        fill.setColor(0xFFF8F8F0);
        c.drawRect(hx + 1, hy + 1, hx + hw - 1, hy + hh - 1, fill);
        fill.setColor(lighter(tc, 2));
        c.drawRect(hx + 1, hy + 1, hx + hw - 1, hy + 132, fill);
        fill.setColor(lighter(tc, 1));
        c.drawRect(hx + 1, hy + 118, hx + hw - 1, hy + 132, fill);
        art.front(c, sp, null, (w - 128) / 2, hy + 4, 128, !seen);
        art.text(c, String.format(java.util.Locale.US, "No%03d", n), hx + 6, hy + 1, 0xFFFFFFFF, 0xFF303030);
        if (caught) {
            footprint(c, sp, hx + hw - 22, hy + 4);
        }
        // the name, large, then its category and types
        byte[] name = seen ? art.speciesName(sp) : art.encode("?????");
        c.save();
        c.scale(2, 2);
        art.text(c, name, (w - 2 * art.width(name)) / 4, (hy + 132) / 2, GameArt.TEXT, GameArt.SHADOW);
        c.restore();
        String cat = seen ? art.plain(GameArt.hexText(e.optString("cat"))) + " POKéMON" : "?????";
        art.text(c, cat, (w - art.width(cat)) / 2, hy + 160, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (t != null) {
            boolean two = t.optInt(1) != t.optInt(0);
            int tx0 = (w - (two ? 68 : 32)) / 2;
            art.typeIcon(c, t.optInt(0), tx0, hy + 177);
            if (two) {
                art.typeIcon(c, t.optInt(1), tx0 + 36, hy + 177);
            }
        }
        DexPage.chip(c, art, fill, caught ? "OWNED" : seen ? "SEEN" : "NOT SEEN", hx + 6, hy + 175, caught);
        int y = hy + hh + 8;
        // its entry, from each game that has one
        if (caught && tx != null && !tx.isNull(game)) {
            java.util.List<byte[]> lines = art.wrap(GameArt.hexText(tx.optString(game)), w - 32);
            int bh = 20 + 22 + lines.size() * 15 + 6;
            int top = panel(c, 6, y, w - 12, bh, "ENTRY");
            int bx = 12;
            for (int g = 0; g < 4; g++) {
                if (tx.isNull(g)) {
                    continue;
                }
                bx += DexPage.chip(c, art, fill, GAMES[g], bx, top, g == game) + 4;
            }
            int ly = top + 20;
            for (byte[] l : lines) {
                art.text(c, l, 14, ly);
                ly += 15;
            }
            gamesY = top;
            y += bh + 8;
        } else {
            int top = panel(c, 6, y, w - 12, 40, "ENTRY");
            art.text(c, seen ? "Catch it to read its entry." : "Not seen yet.", 14, top, GameArt.GRAY_TEXT, GameArt.SHADOW);
            gamesY = -100;
            y += 48;
        }
        // its size
        if (seen) {
            int top = panel(c, 6, y, w - 12, 66, "SIZE");
            art.text(c, "HEIGHT", 14, top);
            art.text(c, caught ? feet(e.optInt("h")) : "?'??\"", 90, top);
            art.text(c, "WEIGHT", 14, top + 16);
            art.text(c, caught ? pounds(e.optInt("w")) : "????.? lbs.", 90, top + 16);
            if (caught) {
                art.text(c, "FOOTPRINT", w - 92, top - 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
                footprint(c, sp, w - 62, top + 12, 32);
            }
            y += 74;
        }
        // where it lives
        if (seen) {
            y = drawWhere(c, 6, y, sp, w - 12);
        }
        // base stats
        JSONArray bs = art.species(sp).optJSONArray("bs");
        if (seen && bs != null) {
            int top = panel(c, 6, y, w - 12, 6 * 15 + 26, "BASE STATS");
            int total = 0, bw = w - 12 - 120;
            for (int i = 0; i < 6; i++) {
                int val = bs.optInt(i);
                total += val;
                int ry = top + i * 15;
                art.text(c, STATS[i], 14, ry);
                String sv = String.valueOf(val);
                art.text(c, sv, 106 - art.width(sv), ry);
                fill.setColor(0xFFD8D8D0);
                c.drawRect(112, ry + 5, 112 + bw, ry + 11, fill);
                fill.setColor(val >= 120 ? 0xFF30C860 : val >= 90 ? 0xFF88D040 : val >= 60 ? 0xFFF8C838 : 0xFFF07838);
                c.drawRect(112, ry + 5, 112 + Math.min(bw, val * bw / 200), ry + 11, fill);
            }
            String tot = "TOTAL " + total;
            art.text(c, tot, w - 12 - art.width(tot), y - 1, GameArt.GRAY_TEXT, GameArt.SHADOW);
            y += 6 * 15 + 34;
        }
        height = y + 8;
        c.restore();
    }

    private int gamesY = -100;

    private void footprint(Canvas c, int species, int x, int y) {
        footprint(c, species, x, y, 16);
    }

    private void footprint(Canvas c, int species, int x, int y, int size) {
        if (sFootprints == null) {
            sFootprints = asset("footprints.png");
        }
        if (sFootprints == null || species >= 32 * (sFootprints.getHeight() / 16)) {
            return;
        }
        src.set((species % 32) * 16, (species / 32) * 16, (species % 32) * 16 + 16, (species / 32) * 16 + 16);
        dst.set(x, y, x + size, y + size);
        c.drawBitmap(sFootprints, src, dst, pix);
    }

    private static final class Place {
        int group, num, index, rate, min = 999, max;
        String method;
        boolean rare = true, met;
    }

    private final java.util.List<int[]> placeRows = new java.util.ArrayList<>();   // {top, bottom, map index}
    private int asked = -1;

    private int drawWhere(Canvas c, int x0, int y, int sp, int pw) {
        placeRows.clear();
        JSONObject where = v.app.where.get(sp);
        boolean linked = v.app.client != null
                && v.app.client.status() == com.moemon.companion.core.LinkClient.Status.LINKED;
        if (where == null && linked && asked != sp) {
            asked = sp;
            try {
                v.app.client.send(new JSONObject().put("t", "where").put("s", sp));
            } catch (org.json.JSONException ignored) {
            }
        }
        SpawnsPage spawns = (SpawnsPage) v.page(4);
        java.util.List<Place> places = new java.util.ArrayList<>();
        int hiddenRare = 0;
        JSONArray slots = where == null ? null : where.optJSONArray("slots");
        java.util.Map<String, Place> by = new java.util.LinkedHashMap<>();
        for (int i = 0; slots != null && i < slots.length(); i++) {
            JSONArray q = slots.optJSONArray(i);
            String method = SpawnsPage.method(q.optInt(2), q.optInt(3));
            String key = q.optInt(0) + "," + q.optInt(1) + "," + method;
            Place p = by.get(key);
            if (p == null) {
                p = new Place();
                p.group = q.optInt(0);
                p.num = q.optInt(1);
                p.method = method;
                p.index = spawns.indexOf(p.group, p.num);
                by.put(key, p);
            }
            int r = SpawnsPage.rate(q.optInt(2), q.optInt(3));
            p.rate += r;
            p.rare &= r <= 1;
            p.met |= q.optInt(6) == 1;
            p.min = Math.min(p.min, q.optInt(4));
            p.max = Math.max(p.max, q.optInt(5));
        }
        for (Place p : by.values()) {
            if (p.rare && !p.met) {
                hiddenRare++;
            } else {
                places.add(p);
            }
        }
        int rows = Math.max(1, places.size()) + (hiddenRare > 0 ? 1 : 0);
        int bh = 24 + rows * 16;
        int ry = panel(c, x0, y, pw, bh, "WHERE TO FIND") - 1;
        if (where == null) {
            art.text(c, linked ? "Asking the game..." : "Link with the game to search.", x0 + 8, ry, GameArt.GRAY_TEXT,
                    GameArt.SHADOW);
        } else if (places.isEmpty() && hiddenRare == 0) {
            art.text(c, "It isn't found in the wild.", x0 + 8, ry, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        for (Place p : places) {
            String name = p.index >= 0 ? spawns.label(p.index) : "?";
            art.text(c, name, x0 + 8, ry, GameArt.TEXT, GameArt.SHADOW);
            String right = p.method + " " + p.rate + "%";
            art.text(c, right, x0 + pw - 8 - art.width(right), ry, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.text(c, ">", x0 + pw - 8 - art.width(right) - 10, ry, GameArt.BLUE_TEXT, GameArt.SHADOW);
            placeRows.add(new int[]{ry + 2, ry + 16, p.index});
            ry += 16;
        }
        if (hiddenRare > 0) {
            art.text(c, "Rare in " + hiddenRare + (hiddenRare == 1 ? " more place." : " more places."), x0 + 8, ry,
                    GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        return y + bh + 8;
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width(), cy = y + scroll;
        if (cy < 22) {
            if (x < 60) {
                back();
            } else if (x >= w - 26) {
                swipe(1);
            } else if (x >= w - 50) {
                swipe(-1);
            }
            return;
        }
        // the games' chips on the ENTRY card
        int n = nums.get(index);
        JSONArray entries = art.data.optJSONArray("dex");
        JSONObject e = entries == null ? null : entries.optJSONObject(n);
        JSONArray tx = e == null ? null : e.optJSONArray("tx");
        if (cy >= gamesY && cy < gamesY + 16 && tx != null) {
            int bx = 12;
            for (int g = 0; g < 4; g++) {
                if (tx.isNull(g)) {
                    continue;
                }
                int bw = art.width(GAMES[g]) + 12;
                if (x >= bx && x < bx + bw + 4) {
                    game = g;
                    v.buzz(false);
                    return;
                }
                bx += bw + 4;
            }
        }
        for (int[] r : placeRows) {
            if (cy >= r[0] && cy < r[1] && r[2] >= 0) {
                SpawnsPage spawns = (SpawnsPage) v.page(4);
                v.select(4);
                spawns.openMap(r[2]);
                return;
            }
        }
    }

    @Override
    public void swipe(int dir) {
        int i = index + dir;
        if (i >= 0 && i < nums.size()) {
            index = i;
            scroll = 0;
            game = -1;
            v.buzz(false);
        }
    }
}
