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

    /** Its cry, once seen (as the game's POKéDEX plays it on its page) */
    private void cry() {
        int n = nums.get(index);
        if (DexPage.bit(dex().optString("seen"), n)) {
            v.sound.cry(art.speciesOfDex(n));
        }
    }

    private int criedFor = -1;

    @Override
    public void shown() {
        if (criedFor != index) {
            criedFor = index;   // (back from a page over it: no cry again)
            cry();
        }
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
        int y = hy + hh + 6;
        // a chip for each part below, to jump to it
        jumpsY = y;
        if (seen) {
            int jx = 6;
            int[] jw = jumpWidths(w);
            for (int k = 0; k < JUMPS.length; k++) {
                jx += DexPage.chip(c, art, fill, JUMPS[k], jx, y, false, jw[k]) + 3;
            }
            y += 20;
        }
        y += 2;
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
            sectionY[0] = y;
            y += bh + 8;
        } else {
            int top = panel(c, 6, y, w - 12, 40, "ENTRY");
            art.text(c, seen ? "Catch it to read its entry." : "Not seen yet.", 14, top, GameArt.GRAY_TEXT, GameArt.SHADOW);
            gamesY = -100;
            sectionY[0] = y;
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
        sectionY[1] = y;
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
        if (seen) {
            JSONObject info = art.species(sp);
            y = drawAbilities(c, w, y, info);
            sectionY[2] = y;
            y = drawMatchups(c, w, y, info);
            sectionY[3] = y;
            y = drawEvolution(c, w, y, sp);
            sectionY[4] = y;
            y = drawMoves(c, w, y, info);
            sectionY[5] = y;
            y = drawBreeding(c, w, y, info);
            y = drawTraining(c, w, y, info);
        }
        height = y + 8;
        c.restore();
    }

    private int gamesY = -100;

    // ------------------------------------------------------------------------------------------------- the details
    private static final String[] JUMPS = {"ENTRY", "STATS", "TYPES", "EVOLVES", "MOVES", "MORE"};
    private final int[] sectionY = new int[JUMPS.length];

    /** The jump chips' widths: the row fills the page's width and never runs off it */
    private int[] jumpWidths(int w) {
        int n = JUMPS.length, text = 0;
        int[] tw = new int[n], out = new int[n];
        for (int k = 0; k < n; k++) {
            tw[k] = art.width(JUMPS[k]);
            text += tw[k];
        }
        int room = w - 12 - 3 * (n - 1), pad = Math.max(4, (room - text) / n), used = 0;
        for (int k = 0; k < n; k++) {
            out[k] = tw[k] + pad;
            used += out[k];
        }
        if (used < room)   // (the pixels left over go to the last one, so the row ends at the margin)
            out[n - 1] += Math.min(room - used, pad);
        return out;
    }
    private int jumpsY = -100, matchupY = -100, movesTabY = -100;
    private boolean tmTab;
    private final java.util.List<int[]> evoRows = new java.util.ArrayList<>();    // {top, bottom, species}
    private final java.util.List<int[]> moveRows = new java.util.ArrayList<>();   // {top, bottom, move}
    static final String[] EGG_GROUPS = {"-", "MONSTER", "WATER 1", "BUG", "FLYING", "FIELD", "FAIRY", "GRASS",
            "HUMAN-LIKE", "WATER 3", "MINERAL", "AMORPHOUS", "WATER 2", "DITTO", "DRAGON", "UNDISCOVERED"};
    static final String[] GROWTH = {"MEDIUM FAST", "ERRATIC", "FLUCTUATING", "MEDIUM SLOW", "FAST", "SLOW"};
    private static final String[] EV_NAMES = {"HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF", "SPEED"};

    private int drawAbilities(Canvas c, int w, int y, JSONObject info) {
        JSONArray ab = info.optJSONArray("ab");
        if (ab == null) {
            return y;
        }
        JSONArray desc = art.data.optJSONArray("abilityDesc");
        java.util.List<Object[]> rows = new java.util.ArrayList<>();
        for (int i = 0; i < 2; i++) {
            int a = ab.optInt(i);
            if (a == 0 || (i == 1 && a == ab.optInt(0))) {
                continue;
            }
            java.util.List<byte[]> lines = desc == null ? new java.util.ArrayList<>()
                    : art.wrap(GameArt.clean(GameArt.hexText(desc.optString(a))), w - 40);
            rows.add(new Object[]{a, lines});
        }
        if (rows.isEmpty()) {
            return y;
        }
        int bh = 22;
        for (Object[] r : rows) {
            bh += 16 + ((java.util.List<?>) r[1]).size() * 14 + 4;
        }
        int top = panel(c, 6, y, w - 12, bh, rows.size() > 1 ? "ABILITIES" : "ABILITY");
        for (Object[] r : rows) {
            art.text(c, art.name("abilities", (Integer) r[0]), 14, top, GameArt.BLUE_TEXT, GameArt.SHADOW);
            top += 16;
            for (Object l : (java.util.List<?>) r[1]) {
                art.text(c, (byte[]) l, 22, top, GameArt.TEXT, GameArt.SHADOW);
                top += 14;
            }
            top += 4;
        }
        return y + bh + 8;
    }

    /** Its weaknesses and resistances (the game's type chart), a row for each multiplier; tap for the TYPE CHART */
    private int drawMatchups(Canvas c, int w, int y, JSONObject info) {
        JSONArray t = info.optJSONArray("t");
        if (t == null) {
            return y;
        }
        int t0 = t.optInt(0), t1 = t.optInt(1);
        String[] titles = {"×4", "×2", "×1/2", "×1/4", "×0"};
        int[] colors = {0xFF208820, TypeChartPage.SUPER, TypeChartPage.HALF, 0xFFB03828, TypeChartPage.NONE};
        java.util.List<java.util.List<Integer>> groups = new java.util.ArrayList<>();
        for (int k = 0; k < titles.length; k++) {
            groups.add(new java.util.ArrayList<>());
        }
        for (int a : TypeChartPage.ORDER) {
            int m = art.effectiveness(a, t0, t1);
            int k = m >= 40 ? 0 : m > 10 ? 1 : m == 0 ? 4 : m <= 3 ? 3 : m < 10 ? 2 : -1;
            if (k >= 0) {
                groups.get(k).add(a);
            }
        }
        int per = Math.max(3, (w - 74) / 36);
        int bh = 22;
        for (java.util.List<Integer> g : groups) {
            if (!g.isEmpty()) {
                bh += ((g.size() + per - 1) / per) * 19 + 3;
            }
        }
        matchupY = y;
        int top = panel(c, 6, y, w - 12, bh, "TYPE MATCHUPS");
        art.text(c, "TYPE CHART >", w - 12 - art.width("TYPE CHART >"), y - 1, GameArt.BLUE_TEXT, GameArt.SHADOW);
        for (int k = 0; k < titles.length; k++) {
            java.util.List<Integer> g = groups.get(k);
            if (g.isEmpty()) {
                continue;
            }
            // the multiplier in its color, then the types
            fill.setColor(colors[k]);
            c.drawRect(12, top + 2, 50, top + 16, fill);
            art.text(c, titles[k], 31 - art.width(titles[k]) / 2, top - 1, 0xFFFFFFFF, 0xFF000000);
            for (int i = 0; i < g.size(); i++) {
                art.typeIcon(c, g.get(i), 56 + (i % per) * 36, top + 1 + (i / per) * 19);
            }
            top += ((g.size() + per - 1) / per) * 19 + 3;
        }
        return y + bh + 8;
    }

    /** What it evolves from and into, all the way: each stage's icon, name and how; tap one for its entry */
    private int drawEvolution(Canvas c, int w, int y, int sp) {
        evoRows.clear();
        JSONArray all = art.data.optJSONArray("species");
        if (all == null) {
            return y;
        }
        int root = sp;
        for (int guard = 0; guard < 8; guard++) {
            int pre = preEvolution(all, root);
            if (pre <= 0) {
                break;
            }
            root = pre;
        }
        java.util.List<int[]> chain = new java.util.ArrayList<>();   // {species, depth, method index into evo list, from}
        walk(all, root, 0, -1, -1, chain, 0);
        if (chain.size() <= 1) {
            int top = panel(c, 6, y, w - 12, 40, "EVOLUTION");
            art.text(c, "It doesn't evolve.", 14, top, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 48;
        }
        JSONObject dexBits = dex();
        String seenBits = dexBits.optString("seen");
        int bh = 22 + chain.size() * 34;
        int top = panel(c, 6, y, w - 12, bh, "EVOLUTION");
        for (int[] st : chain) {
            int s = st[0], depth = st[1], ry = top + chain.indexOf(st) * 34 - 2;
            int x = 12 + depth * 18;
            boolean known = DexPage.bit(seenBits, art.species(s).optInt("dex"));
            if (s == sp) {
                fill.setColor(0xFFF8F0B0);
                c.drawRect(7, ry, w - 7, ry + 33, fill);
            }
            if (depth > 0) {
                art.text(c, "▶", x - 12, ry + 8, GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
            art.icon(c, s, x, ry, known ? GameArt.ICON_PLAIN : GameArt.ICON_SILHOUETTE);
            art.text(c, known ? art.speciesName(s) : art.encode("?????"), x + 34, ry + 1, GameArt.TEXT, GameArt.SHADOW);
            if (depth > 0) {
                JSONArray evo = art.species(st[3]).optJSONArray("evo");
                JSONArray how = evo == null ? null : evo.optJSONArray(st[2]);
                if (how != null) {
                    art.text(c, method(how.optInt(0), how.optInt(1)), x + 34, ry + 15, GameArt.GRAY_TEXT, GameArt.SHADOW);
                }
            }
            if (s != sp && known) {
                art.text(c, ">", w - 18, ry + 8, GameArt.BLUE_TEXT, GameArt.SHADOW);
            }
            evoRows.add(new int[]{ry, ry + 34, s});
        }
        return y + bh + 8;
    }

    private static int preEvolution(JSONArray all, int sp) {
        for (int i = 1; i < all.length(); i++) {
            JSONArray evo = all.optJSONObject(i) == null ? null : all.optJSONObject(i).optJSONArray("evo");
            for (int k = 0; evo != null && k < evo.length(); k++) {
                if (evo.optJSONArray(k).optInt(2) == sp && i != sp) {
                    return i;
                }
            }
        }
        return 0;
    }

    private void walk(JSONArray all, int s, int depth, int how, int from, java.util.List<int[]> out, int guard) {
        if (guard > 6 || out.size() > 24) {
            return;
        }
        for (int[] seen : out) {
            if (seen[0] == s) {
                return;   // (a loop: drawn once)
            }
        }
        out.add(new int[]{s, depth, how, from});
        JSONArray evo = art.species(s).optJSONArray("evo");
        for (int k = 0; evo != null && k < evo.length(); k++) {
            walk(all, evo.optJSONArray(k).optInt(2), depth + 1, k, s, out, guard + 1);
        }
    }

    /** How an evolution happens, in a few words (the game's evolution methods) */
    String method(int m, int p) {
        String item = art.plain(art.name("items", p)), move = art.plain(art.name("moves", p));
        switch (m) {
            case 1: return "High friendship";
            case 2: return "High friendship, day";
            case 3: return "High friendship, night";
            case 4: case 13: return "Lv. " + p;
            case 5: return "Trade";
            case 6: return "Trade holding " + item;
            case 7: return "Use " + item;
            case 8: return "Lv. " + p + ", ATK > DEF";
            case 9: return "Lv. " + p + ", ATK = DEF";
            case 10: return "Lv. " + p + ", ATK < DEF";
            case 11: case 12: return "Lv. " + p + ", by personality";
            case 14: return "Lv. " + p + ", a free slot";
            case 15: return "High beauty";
            case 16: return "Level up holding " + item + ", day";
            case 17: return "Level up holding " + item + ", night";
            case 18: return "Level up holding " + item;
            case 19: return "Level up knowing " + move;
            case 20: return "Use " + item + " (male)";
            case 21: return "Use " + item + " (female)";
            case 22: return "Lv. " + p + " (male)";
            case 23: return "Lv. " + p + " (female)";
            case 24: return "Level up with " + art.plain(art.speciesName(p)) + " in the party";
            default: return "?";
        }
    }

    private static String tmName(int i) {
        return i < 50 ? String.format(java.util.Locale.US, "TM%02d", i + 1)
                : i < 58 ? String.format(java.util.Locale.US, "HM%02d", i - 49)
                : String.format(java.util.Locale.US, "TM%02d", i - 58 + 51);
    }

    /** The moves it learns: by level, or by TM / HM (tabs); tap one to read about it */
    private int drawMoves(Canvas c, int w, int y, JSONObject info) {
        moveRows.clear();
        java.util.List<int[]> list = new java.util.ArrayList<>();   // {label number (-1: none), move}
        java.util.List<String> labels = new java.util.ArrayList<>();
        if (!tmTab) {
            JSONArray lv = info.optJSONArray("lv");
            for (int i = 0; lv != null && i < lv.length(); i++) {
                JSONArray r = lv.optJSONArray(i);
                list.add(new int[]{r.optInt(0), r.optInt(1)});
                labels.add(r.optInt(0) <= 1 ? "-" : String.valueOf(r.optInt(0)));
            }
        } else {
            String tm = info.optString("tm");
            JSONArray tms = art.data.optJSONArray("tms");
            for (int i = 0; tms != null && i < tms.length(); i++) {
                int nib = i / 4 < tm.length() ? Character.digit(tm.charAt(i / 4), 16) : 0;
                if ((nib & (8 >> (i % 4))) != 0) {
                    list.add(new int[]{i, tms.optInt(i)});
                    labels.add(tmName(i));
                }
            }
        }
        int rows = Math.max(1, list.size()), bh = 42 + rows * 18;
        int top = panel(c, 6, y, w - 12, bh, "MOVES");
        movesTabY = top;
        int tx = 12;
        tx += DexPage.chip(c, art, fill, "BY LEVEL", tx, top, !tmTab) + 4;
        DexPage.chip(c, art, fill, "TM & HM", tx, top, tmTab);
        String count = list.size() + (list.size() == 1 ? " MOVE" : " MOVES");
        art.text(c, count, w - 14 - art.width(count), top - 1, GameArt.GRAY_TEXT, GameArt.SHADOW);
        int ry = top + 20;
        if (list.isEmpty()) {
            art.text(c, tmTab ? "It can't learn any TM or HM." : "None.", 14, ry, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        for (int i = 0; i < list.size(); i++) {
            int mv = list.get(i)[1];
            JSONObject mo = art.move(mv);
            if (i % 2 == 1) {
                fill.setColor(0xFFF0F0E0);
                c.drawRect(7, ry, w - 7, ry + 18, fill);
            }
            art.text(c, labels.get(i), 14, ry, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.typeIcon(c, mo.optInt("t"), 46, ry + 1);
            art.text(c, art.name("moves", mv), 82, ry, GameArt.TEXT, GameArt.SHADOW);
            int pw = mo.optInt("p");
            String pwr = pw > 1 ? String.valueOf(pw) : "-";
            art.text(c, pwr, w - 14 - art.width(pwr), ry, GameArt.TEXT, GameArt.SHADOW);
            moveRows.add(new int[]{ry, ry + 18, mv});
            ry += 18;
        }
        return y + bh + 8;
    }

    private int drawBreeding(Canvas c, int w, int y, JSONObject info) {
        JSONArray eg = info.optJSONArray("eg");
        int g = info.optInt("g", 127);
        int top = panel(c, 6, y, w - 12, g == 255 ? 76 : 92, "BREEDING");
        String groups = "-";
        if (eg != null) {
            int a = eg.optInt(0), b = eg.optInt(1);
            groups = name(EGG_GROUPS, a) + (b != a ? ", " + name(EGG_GROUPS, b) : "");
        }
        art.text(c, "EGG GROUPS", 14, top);
        art.text(c, groups, 100, top, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, "HATCHES", 14, top + 16);
        art.text(c, "about " + (info.optInt("ec") * 256) + " steps", 100, top + 16);
        art.text(c, "GENDER", 14, top + 32);
        if (g == 255) {
            art.text(c, "Unknown", 100, top + 32);
        } else {
            int female = g == 254 ? 100 : g == 0 ? 0 : Math.round(g * 100f / 256);
            int bx = 100, bw = w - 12 - 100 - 8;
            fill.setColor(0xFF5878E0);
            c.drawRect(bx, top + 36, bx + bw * (100 - female) / 100, top + 44, fill);
            fill.setColor(0xFFF07898);
            c.drawRect(bx + bw * (100 - female) / 100, top + 36, bx + bw, top + 44, fill);
            String words = (100 - female) + "% ♂  " + female + "% ♀";
            art.text(c, words, bx, top + 46, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 92 + 8;
        }
        return y + 76 + 8;
    }

    private static String name(String[] names, int i) {
        return i >= 0 && i < names.length ? names[i] : "?";
    }

    private int drawTraining(Canvas c, int w, int y, JSONObject info) {
        StringBuilder evs = new StringBuilder();
        JSONArray ev = info.optJSONArray("evy");
        for (int i = 0; ev != null && i < 6; i++) {
            if (ev.optInt(i) > 0) {
                evs.append(evs.length() > 0 ? ", " : "").append(ev.optInt(i)).append(" ").append(EV_NAMES[i]);
            }
        }
        JSONArray it = info.optJSONArray("it");
        int common = it == null ? 0 : it.optInt(0), rare = it == null ? 0 : it.optInt(1);
        java.util.List<String> held = new java.util.ArrayList<>();
        if (common != 0 && common == rare) {
            held.add(art.plain(art.name("items", common)) + " (always)");
        } else {
            if (common != 0) {
                held.add(art.plain(art.name("items", common)) + " (50%)");
            }
            if (rare != 0) {
                held.add(art.plain(art.name("items", rare)) + " (5%)");
            }
        }
        int rows = 5 + Math.max(1, held.size());
        int bh = 22 + rows * 16;
        int top = panel(c, 6, y, w - 12, bh, "TRAINING");
        String[][] lines = {
                {"CATCH RATE", info.optInt("cr") + " / 255"},
                {"BASE EXP.", String.valueOf(info.optInt("ey"))},
                {"EV YIELD", evs.length() > 0 ? evs.toString() : "-"},
                {"GROWTH", name(GROWTH, info.optInt("gr"))},
                {"FRIENDSHIP", String.valueOf(info.optInt("fr"))},
        };
        for (String[] l : lines) {
            art.text(c, l[0], 14, top);
            art.text(c, l[1], 100, top, GameArt.BLUE_TEXT, GameArt.SHADOW);
            top += 16;
        }
        art.text(c, "WILD ITEMS", 14, top);
        if (held.isEmpty()) {
            art.text(c, "NONE", 100, top, GameArt.BLUE_TEXT, GameArt.SHADOW);
        }
        for (String h : held) {
            art.text(c, h, 100, top, GameArt.BLUE_TEXT, GameArt.SHADOW);
            top += 16;
        }
        return y + bh + 8;
    }

    /** Another entry: the one for that species, among this list if it's there (else on its own) */
    private void open(int species) {
        int dexNum = art.species(species).optInt("dex");
        int i = nums.indexOf(dexNum);
        if (i >= 0) {
            index = i;
        } else {
            nums.add(index + 1, dexNum);
            index++;
        }
        scroll = 0;
        game = -1;
        v.sound.play("se_dex_page");
        cry();
    }

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
        // the jump chips
        if (cy >= jumpsY && cy < jumpsY + 16) {
            int jx = 6;
            int[] jw = jumpWidths(v.width());
            for (int k = 0; k < JUMPS.length; k++) {
                int bw = jw[k];
                if (x >= jx && x < jx + bw + 3) {
                    v.buzz(false);
                    scrollTo(Math.max(0, sectionY[k] - 4));
                    return;
                }
                jx += bw + 3;
            }
            return;
        }
        if (cy >= matchupY && cy < matchupY + 18) {
            JSONArray tt = art.species(art.speciesOfDex(nums.get(index))).optJSONArray("t");
            if (tt != null) {
                v.showOver(new TypeChartPage(v).defending(tt.optInt(0), tt.optInt(1)));
            }
            return;
        }
        if (cy >= movesTabY && cy < movesTabY + 16) {
            boolean tm = x >= 12 + art.width("BY LEVEL") + 16;
            if (tm != tmTab && x < 12 + art.width("BY LEVEL") + art.width("TM & HM") + 32) {
                tmTab = tm;
                v.buzz(false);
            }
            return;
        }
        for (int[] r : moveRows) {
            if (cy >= r[0] && cy < r[1]) {
                moveInfo(r[2]);
                return;
            }
        }
        for (int[] r : evoRows) {
            if (cy >= r[0] && cy < r[1] && r[2] != art.speciesOfDex(nums.get(index))
                    && DexPage.bit(dex().optString("seen"), art.species(r[2]).optInt("dex"))) {
                open(r[2]);
                return;
            }
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

    /** A move's card: its type, category, power, accuracy, PP and what it does */
    private void moveInfo(int mv) {
        JSONObject mo = art.move(mv);
        String[] cats = {"PHYSICAL", "SPECIAL", "STATUS"};
        int cat = mo.optInt("c");
        String head = art.plain(art.name("moves", mv)) + "\n" + art.plain(art.name("types", mo.optInt("t"))) + " / "
                + (cat >= 0 && cat < 3 ? cats[cat] : "?") + "\nPOWER " + (mo.optInt("p") > 1 ? mo.optInt("p") : "-")
                + "   ACC. " + (mo.optInt("a") > 0 ? mo.optInt("a") : "-") + "   PP " + mo.optInt("pp") + "\n"
                + art.plain(GameArt.clean(GameArt.hexText(mo.optString("d"))));
        v.showMenu(Menu.message(v, head));
    }

    @Override
    public void swipe(int dir) {
        int i = index + dir;
        if (i >= 0 && i < nums.size()) {
            index = i;
            scroll = 0;
            game = -1;
            tmTab = false;
            v.buzz(false);
            v.sound.play("se_dex_page");
            cry();
        }
    }
}
