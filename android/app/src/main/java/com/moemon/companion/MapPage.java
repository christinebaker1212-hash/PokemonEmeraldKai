package com.moemon.companion;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The live map (docs/COMPANION_APP.md "Live map"): the game's own town map pages with you on them, friends who share
 * their place, and what you've found at a place you tap: trainers beaten, items picked up, BERRY trees, wild POKéMON
 * seen. The regions you've been to; the one you're in opens first.
 */
public final class MapPage extends Page {
    // the town map pages (region_map.c QZMAP_*) and their regions
    private static final int[] PAGE_REGION = {0, 1, 2, 2, 2, 2, 3, 3};
    private static final String[] REGIONS = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    private static final int[] FRIEND = {0xFFF84040, 0xFF4080F8, 0xFF30C030, 0xFFF8C020};
    private static final int CHIPS = 22, GRID_X = 8, GRID_Y = 16, COLS = 28, ROWS = 15;
    private final String title;
    private int page = -1, selX = -1, selY = -1;
    private long asked;
    private final Paint fill = new Paint();

    public MapPage(GameView v, String title) {
        super(v);
        this.title = title;
    }

    private Model model() {
        return v.app.model();
    }

    private JSONObject place() {
        Model m = model();
        return m == null ? null : m.snap.optJSONObject("place");
    }

    /** Where the player is: [page, x, y], or null */
    private JSONArray here() {
        JSONObject p = place();
        return p == null ? null : p.optJSONArray("tm");
    }

    private boolean visited(int region) {
        Model m = model();
        if (m == null) {
            return region == 0;
        }
        JSONArray b = m.save().optJSONArray("rbadges");
        return region == m.save().optInt("region") || (b != null && b.optInt(region) > 0);
    }

    @Override
    public void shown() {
        JSONArray h = here();
        if (page < 0) {
            page = h != null ? h.optInt(0) : regionFirstPage(model() == null ? 0 : model().save().optInt("region"));
        }
    }

    private static int regionFirstPage(int region) {
        for (int p = 0; p < PAGE_REGION.length; p++) {
            if (PAGE_REGION[p] == region) {
                return p;
            }
        }
        return 0;
    }

    private int scale(int w) {
        return Math.max(1, (w - 8) / 240);
    }

    private int mapX(int w) {
        return (w - 240 * scale(w)) / 2;
    }

    private int mapY() {
        return 20 + CHIPS + 4;
    }

    private int cell(int x, int y) {
        JSONArray maps = art.data.optJSONArray("townmaps");
        JSONObject p = maps == null ? null : maps.optJSONObject(page);
        JSONArray cells = p == null ? null : p.optJSONArray("cells");
        return cells == null || x < 0 || y < 0 || x >= COLS || y >= ROWS ? 0xD5 : cells.optInt(y * COLS + x, 0xD5);
    }

    private static boolean nowhere(int sec) {
        return sec == 0xD5;   // MAPSEC_NONE
    }

    @Override
    public int scrollMax() {
        return Math.max(0, panelBottom - v.contentHeight());
    }

    private int panelBottom;

    @Override
    public void draw(Canvas c, int w, int h) {
        if (page < 0) {
            shown();
        }
        int s = scale(w), mx = mapX(w), my = mapY() - scroll;
        // the map: the device's sea behind KANTO's narrower pages
        fill.setColor(0xFF68A0D8);
        c.drawRect(mx, my, mx + 240 * s, my + 160 * s, fill);
        Bitmap b = art.townMap(page);
        art.sharp(c, b, mx, my, 240 * s, 160 * s);
        long now = System.currentTimeMillis();
        // friends: a dot each in their player's color
        JSONObject pl = place();
        JSONArray fr = pl == null ? null : pl.optJSONArray("friends");
        for (int i = 0; fr != null && i < fr.length(); i++) {
            JSONArray f = fr.optJSONArray(i);
            if (f.optInt(1) != page) {
                continue;
            }
            int fx = mx + (GRID_X + f.optInt(2) * 8) * s, fy = my + (GRID_Y + f.optInt(3) * 8) * s;
            fill.setColor(0xFF202020);
            c.drawRect(fx + s, fy + s, fx + 7 * s, fy + 7 * s, fill);
            fill.setColor(FRIEND[(i + 1) % FRIEND.length]);
            c.drawRect(fx + 2 * s, fy + 2 * s, fx + 6 * s, fy + 6 * s, fill);
        }
        // your markers (press and hold a place): the MARKING MAP's six shapes
        JSONObject marks = marks();
        for (java.util.Iterator<String> it = marks.keys(); it.hasNext(); ) {
            String k = it.next();
            String[] f = k.split(",");
            if (f.length != 3 || Integer.parseInt(f[0]) != page) {
                continue;
            }
            int kx = mx + (GRID_X + Integer.parseInt(f[1]) * 8) * s, ky = my + (GRID_Y + Integer.parseInt(f[2]) * 8) * s;
            mark(c, marks.optInt(k), kx, ky, s);
        }
        // you: the town map's blinking cursor colors
        JSONArray here = here();
        if (here != null && here.optInt(0) == page) {
            int px = mx + (GRID_X + here.optInt(1) * 8) * s, py = my + (GRID_Y + here.optInt(2) * 8) * s;
            boolean on = (now / 400) % 2 == 0;
            fill.setColor(0xFFF8F8F8);
            c.drawRect(px, py, px + 8 * s, py + 8 * s, fill);
            fill.setColor(on ? 0xFFE83020 : 0xFF2848D8);
            c.drawRect(px + s, py + s, px + 7 * s, py + 7 * s, fill);
            v.postInvalidateDelayed(400);
        }
        // the place tapped: a frame around its cell
        if (selX >= 0) {
            int sx = mx + (GRID_X + selX * 8) * s, sy = my + (GRID_Y + selY * 8) * s;
            fill.setColor(0xFFF8E070);
            c.drawRect(sx - s, sy - s, sx + 9 * s, sy, fill);
            c.drawRect(sx - s, sy + 8 * s, sx + 9 * s, sy + 9 * s, fill);
            c.drawRect(sx - s, sy, sx, sy + 8 * s, fill);
            c.drawRect(sx + 8 * s, sy, sx + 9 * s, sy + 8 * s, fill);
        }
        drawPanel(c, w, my + 160 * s + 12);
        panelBottom += scroll;
        // the region chips (over the map when it's scrolled)
        fill.setColor(BANNER_DARK);
        c.drawRect(0, 20, w, 20 + CHIPS, fill);
        int x = 6;
        for (int r = 0; r < REGIONS.length; r++) {
            if (!visited(r)) {
                continue;
            }
            boolean on = PAGE_REGION[page] == r;
            int cw = art.width(REGIONS[r]) + 12;
            if (on) {
                art.window(c, 0, x + 2, 24, cw - 4, 12);
            }
            art.text(c, REGIONS[r], x + 6, 22, on ? GameArt.TEXT : GameArt.WHITE_TEXT, on ? GameArt.SHADOW : 0xFF202850);
            x += cw + 4;
        }
        // a region of several pages: arrows between them
        if (pages(PAGE_REGION[page]) > 1) {
            String n = (page - regionFirstPage(PAGE_REGION[page]) + 1) + "/" + pages(PAGE_REGION[page]);
            art.text(c, "<", w - 52, 22, GameArt.WHITE_TEXT, 0);
            art.text(c, n, w - 40, 22, GOLD, 0);
            art.text(c, ">", w - 14, 22, GameArt.WHITE_TEXT, 0);
        }
        banner(c, w, title, linkWord());
    }

    private String linkWord() {
        LinkClient cl = v.app.client;
        return cl != null && cl.status() == LinkClient.Status.LINKED ? "LIVE" : model() == null ? "" : "AS OF " + GameView.ago(model().asOf);
    }

    private static int pages(int region) {
        int n = 0;
        for (int r : PAGE_REGION) {
            n += r == region ? 1 : 0;
        }
        return n;
    }

    // ------------------------------------------------------------------------------------- the place's panel
    private static final String[] STAGES = {"", "PLANTED", "SPROUTED", "TALLER", "FLOWERING", "BERRIES!", "", ""};

    private void drawPanel(Canvas c, int w, int y) {
        int sec = selX >= 0 ? cell(selX, selY) : -1;
        JSONArray here = here();
        if (sec < 0 && here != null && here.optInt(0) == page) {
            sec = cell(here.optInt(1), here.optInt(2));
        }
        if (sec < 0 || nowhere(sec)) {
            art.window(c, 0, 12, y + 4, w - 24, 30);
            int yy = y + 4;
            for (String l : wrap("Tap a place on the map to see what you've found there. Press and hold one to mark it.", w - 32)) {
                art.text(c, l, 16, yy);
                yy += 14;
            }
            panelBottom = yy + 12;
            return;
        }
        JSONArray secs = art.data.optJSONArray("mapsecs");
        String name = secs == null ? "" : secs.optString(sec);
        JSONObject info = v.app.places.get(sec);
        boolean linked = v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
        if (linked && (info == null || System.currentTimeMillis() - asked > 3000) && v.app.client != null) {
            asked = System.currentTimeMillis();
            try {
                v.app.client.send(new JSONObject().put("t", "place").put("sec", sec));
            } catch (org.json.JSONException ignored) {
            }
        }
        int top = y;
        // the place's name in its own banner, as the town map's box shows it
        fill.setColor(0xFF3868A8);
        c.drawRect(12, y, w - 12, y + 18, fill);
        art.textHex(c, name, 18, y + 1, GameArt.WHITE_TEXT, 0xFF202850);
        y += 22;
        if (info == null) {
            art.text(c, linked ? "..." : "Link with the game to see what you've found here.", 16, y, GameArt.WHITE_TEXT,
                    0xFF305060);
            panelBottom = y + 30;
            return;
        }
        // trainers
        JSONArray tr = info.optJSONArray("trainers");
        if (tr != null && tr.length() > 0) {
            int beaten = 0;
            for (int i = 0; i < tr.length(); i++) {
                beaten += tr.optJSONArray(i).optInt(1);
            }
            y = heading(c, w, y, "TRAINERS", beaten + "/" + tr.length());
            for (int i = 0; i < tr.length(); i++) {
                JSONArray t = tr.optJSONArray(i);
                boolean b = t.optInt(1) == 1;
                int x = 20 + (i % 2) * ((w - 40) / 2), ly = y + (i / 2) * 14;
                int tx = x + art.textHex(c, t.optString(2), x, ly, b ? GameArt.TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW) + 3;
                art.textHex(c, t.optString(3), tx, ly, b ? GameArt.BLUE_TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
            y += ((tr.length() + 1) / 2) * 14 + 6;
            y = close(c, w, y);
        }
        // items: hidden ones aren't named until found
        JSONArray it = info.optJSONArray("items");
        if (it != null && it.length() > 0) {
            int got = 0;
            for (int i = 0; i < it.length(); i++) {
                got += it.optJSONArray(i).optInt(1);
            }
            y = heading(c, w, y, "ITEMS", got + "/" + it.length());
            int per = Math.max(1, (w - 40) / 26);
            for (int i = 0; i < it.length(); i++) {
                JSONArray e = it.optJSONArray(i);
                int x = 20 + (i % per) * 26, iy = y + (i / per) * 28;
                boolean found = e.optInt(1) == 1, hidden = e.optInt(2) == 1;
                if (found || !hidden) {
                    art.itemIcon(c, e.optInt(0), x, iy);
                    if (found) {
                        fill.setColor(0x90F8F8F8);
                        c.drawRect(x, iy, x + 24, iy + 24, fill);
                        art.text(c, "OK", x + 6, iy + 10, 0xFF209020, GameArt.SHADOW);
                    }
                } else {
                    art.text(c, "?", x + 9, iy + 5, GameArt.GRAY_TEXT, GameArt.SHADOW);
                }
            }
            y += ((it.length() + per - 1) / per) * 28 + 4;
            y = close(c, w, y);
        }
        // BERRY trees
        JSONArray be = info.optJSONArray("berries");
        if (be != null && be.length() > 0) {
            y = heading(c, w, y, "BERRY TREES", "" + be.length());
            for (int i = 0; i < be.length(); i++) {
                JSONArray t = be.optJSONArray(i);
                int x = 20 + (i % 2) * ((w - 40) / 2), ly = y + (i / 2) * 26;
                if (t.optInt(1) > 0 && t.optInt(2) > 0) {
                    art.itemIcon(c, t.optInt(1), x, ly);
                    art.text(c, STAGES[Math.min(7, t.optInt(2))], x + 26, ly + 5, t.optInt(2) >= 5 ? 0xFF209020 : GameArt.TEXT,
                            GameArt.SHADOW);
                } else {
                    art.text(c, "Soft soil", x + 2, ly + 5, GameArt.GRAY_TEXT, GameArt.SHADOW);
                }
            }
            y += ((be.length() + 1) / 2) * 26 + 4;
            y = close(c, w, y);
        }
        // wild POKéMON: the ones you've seen, the rest as shapes
        JSONArray wi = info.optJSONArray("wild");
        if (wi != null && wi.length() > 0) {
            int seen = 0, caught = 0;
            for (int i = 0; i < wi.length(); i++) {
                seen += wi.optJSONArray(i).optInt(1);
                caught += wi.optJSONArray(i).optInt(2);
            }
            y = heading(c, w, y, "WILD POKéMON", "SEEN " + seen + "  OWN " + caught);
            int per = Math.max(1, (w - 40) / 34);
            for (int i = 0; i < wi.length(); i++) {
                JSONArray e = wi.optJSONArray(i);
                int x = 18 + (i % per) * 34, iy = y + (i / per) * 32;
                art.icon(c, e.optInt(0), x, iy, e.optInt(1) == 1 ? GameArt.ICON_PLAIN : GameArt.ICON_SILHOUETTE);
                if (e.optInt(2) == 1) {
                    art.ballIcon(c, 4, x + 18, iy + 18);
                }
            }
            y += ((wi.length() + per - 1) / per) * 32 + 4;
            y = close(c, w, y);
        }
        if (y == top + 22) {
            art.text(c, "Nothing to find here.", 16, y, GameArt.WHITE_TEXT, 0xFF305060);
            y += 18;
        }
        panelBottom = y + 8;
    }

    private int heading(Canvas c, int w, int y, String what, String count) {
        art.window(c, 0, 12, y + 2, w - 24, 14);
        art.text(c, what, 16, y + 1, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, count, w - 16 - art.width(count), y + 1, GameArt.TEXT, GameArt.SHADOW);
        return y + 20;
    }

    private int close(Canvas c, int w, int y) {
        return y + 4;
    }

    // ------------------------------------------------------------------------------------------------ touch
    @Override
    public void tap(int x, int y) {
        int w = v.width();
        if (y < 20) {
            if (x > w - 60) {
                back();
            }
            return;
        }
        if (y < 20 + CHIPS) {
            int region = PAGE_REGION[page];
            if (pages(region) > 1 && x > w - 60) {
                int first = regionFirstPage(region), n = pages(region);
                int k = (page - first + (x > w - 30 ? 1 : n - 1)) % n;
                page = first + k;
                selX = selY = -1;
                v.buzz(false);
                return;
            }
            int cx = 6;
            for (int r = 0; r < REGIONS.length; r++) {
                if (!visited(r)) {
                    continue;
                }
                int cw = art.width(REGIONS[r]) + 12;
                if (x >= cx && x < cx + cw + 4 && PAGE_REGION[page] != r) {
                    JSONArray here = here();
                    page = here != null && PAGE_REGION[here.optInt(0)] == r ? here.optInt(0) : regionFirstPage(r);
                    selX = selY = -1;
                    scroll = 0;
                    v.buzz(false);
                    return;
                }
                cx += cw + 4;
            }
            return;
        }
        int s = scale(w), mx = mapX(w), my = mapY() - scroll;
        int gx = (x - mx) / s - GRID_X, gy = (y - my) / s - GRID_Y;
        if (gx >= 0 && gy >= 0 && gx / 8 < COLS && gy / 8 < ROWS && !nowhere(cell(gx / 8, gy / 8))) {
            selX = gx / 8;
            selY = gy / 8;
            asked = 0;
            v.buzz(false);
        }
    }

    // ------------------------------------------------------------------------------------------- your markers
    private static final int[] MARK_COLORS = {0, 0xFFE83030, 0xFF3070E8, 0xFF30B030, 0xFFF080B0, 0xFFF8C020, 0xFF9050D0};

    private String marksKey() {
        Model m = model();
        return "marks_" + (m == null ? "" : m.saveId());
    }

    private JSONObject marks() {
        try {
            return new JSONObject(v.app.context.getSharedPreferences("moemon", 0).getString(marksKey(), "{}"));
        } catch (org.json.JSONException e) {
            return new JSONObject();
        }
    }

    // a marker: 1 circle, 2 square, 3 triangle, 4 heart, 5 star, 6 diamond, in its color, on a dark rim
    private void mark(Canvas c, int shape, int x, int y, int s) {
        fill.setColor(0xFF181818);
        c.drawRect(x + s, y + s, x + 7 * s, y + 7 * s, fill);
        fill.setColor(MARK_COLORS[Math.max(1, Math.min(6, shape))]);
        int[][] rows = {
                {},
                {0x3C, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C},
                {0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E},
                {0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E},
                {0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18},
                {0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x66},
                {0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18},
        };
        int[] r = rows[Math.max(1, Math.min(6, shape))];
        for (int j = 0; j < r.length; j++) {
            for (int i = 0; i < 8; i++) {
                if ((r[j] & (0x80 >> i)) != 0) {
                    c.drawRect(x + i * s, y + (j + 1) * s, x + (i + 1) * s, y + (j + 2) * s, fill);
                }
            }
        }
    }

    @Override
    public boolean longPress(int x, int y) {
        int w = v.width(), s = scale(w), mx = mapX(w), my = mapY() - scroll;
        int gx = (x - mx) / s - GRID_X, gy = (y - my) / s - GRID_Y;
        if (y < 20 + CHIPS || gx < 0 || gy < 0 || gx / 8 >= COLS || gy / 8 >= ROWS) {
            return false;
        }
        JSONObject marks = marks();
        String k = page + "," + gx / 8 + "," + gy / 8;
        int next = (marks.optInt(k) + 1) % 7;
        try {
            if (next == 0) {
                marks.remove(k);
            } else {
                marks.put(k, next);
            }
        } catch (org.json.JSONException ignored) {
        }
        v.app.context.getSharedPreferences("moemon", 0).edit().putString(marksKey(), marks.toString()).apply();
        v.buzz(true);
        return true;
    }

    @Override
    public void swipe(int dir) {
        int region = PAGE_REGION[page], n = pages(region);
        if (n > 1) {
            int first = regionFirstPage(region);
            page = first + (page - first + dir + n) % n;
            selX = selY = -1;
        }
    }
}
