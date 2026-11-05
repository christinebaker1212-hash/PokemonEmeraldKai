package com.moemon.companion;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * The POKéDEX, made for a phone in the game's own colors (pokedex.c's green screen, yellow panels, its title and
 * badges): a search bar (type a name or a number), filters, and a grid of cards, each with its picture, number and
 * name, its types' colors along its foot, a POKé BALL when owned. Unseen ones are a shadow and "?????", as in the
 * game. Tap a card for the entry.
 */
public final class DexPage extends Page {
    static final int GREEN = 0xFF31D54A, GREEN_DARK = 0xFF188320, NAVY = 0xFF000029, YELLOW = 0xFFEEF639,
            BORDER = 0xFF293941, BORDER_LIGHT = 0xFF626273, GOLD = 0xFFBD9C5A, MAGENTA = 0xFFFF00BD;
    private static final String[] FILTERS = {"ALL", "SEEN", "OWNED", "MISSING"};
    private static final int HEAD = 96, CARD_H = 98, GAP = 6;
    private static final String[] SORTS = {"No.", "ABC", "TOTAL"};
    private int filter, sort, typeFilter = -1;   // (remembered: the app's settings)
    private boolean picking;                      // (the type picker open)
    private String search = "";
    private boolean typing;
    private final Paint fill = new Paint(), pix = new Paint();
    private final Rect src = new Rect(), dst = new Rect();
    private Bitmap screen;
    private List<Integer> shown = new ArrayList<>();

    public DexPage(GameView v) {
        super(v);
        pix.setFilterBitmap(false);
        android.content.SharedPreferences p = prefs();
        filter = Math.max(0, Math.min(FILTERS.length - 1, p.getInt("dex_filter", 0)));
        sort = Math.max(0, Math.min(SORTS.length - 1, p.getInt("dex_sort", 0)));
        typeFilter = p.getInt("dex_type", -1);
    }

    private android.content.SharedPreferences prefs() {
        return v.getContext().getSharedPreferences("moemon", 0);
    }

    private void remember() {
        prefs().edit().putInt("dex_filter", filter).putInt("dex_sort", sort).putInt("dex_type", typeFilter).apply();
    }

    private int total(int dex) {
        JSONArray bs = art.species(art.speciesOfDex(dex)).optJSONArray("bs");
        int t = 0;
        for (int i = 0; bs != null && i < bs.length(); i++) {
            t += bs.optInt(i);
        }
        return t;
    }

    /** How many bits a POKéDEX hex string has set (seen / caught) */
    static int count(String hex) {
        int n = 0;
        for (int i = 0; hex != null && i < hex.length(); i++) {
            n += Integer.bitCount(Character.digit(hex.charAt(i), 16) & 15);
        }
        return n;
    }

    static boolean bit(String hex, int dex) {
        int i = (dex - 1) / 4;
        if (hex == null || i >= hex.length()) {
            return false;
        }
        int nib = Character.digit(hex.charAt(i), 16);
        return (nib & (8 >> ((dex - 1) % 4))) != 0;
    }

    private JSONObject dex() {
        Model m = v.app.model();
        JSONObject d = m == null ? null : m.snap.optJSONObject("dex");
        return d != null ? d : new JSONObject();
    }

    private List<Integer> list() {
        JSONObject d = dex();
        String seen = d.optString("seen"), caught = d.optString("caught");
        List<Integer> out = new ArrayList<>();
        String q = search.trim().toUpperCase(java.util.Locale.US);
        int last = 0;
        for (int n = 1; n <= d.optInt("count"); n++) {
            if (bit(seen, n)) {
                last = n;
            }
        }
        // (as the game's list: up to the last one seen, unless searching)
        int end = q.isEmpty() ? Math.max(last, 1) : d.optInt("count");
        for (int n = 1; n <= end; n++) {
            boolean s = bit(seen, n), c = bit(caught, n);
            if (filter == 1 && !s || filter == 2 && !c || filter == 3 && c) {
                continue;
            }
            if (typeFilter >= 0) {   // (a type: only the ones seen, as their types are known then)
                JSONArray t = s ? art.species(art.speciesOfDex(n)).optJSONArray("t") : null;
                if (t == null || t.optInt(0) != typeFilter && t.optInt(1) != typeFilter) {
                    continue;
                }
            }
            if (!q.isEmpty()) {
                boolean num = q.matches("[0-9]+") && String.valueOf(n).startsWith(q.replaceFirst("^0+(?=.)", ""));
                boolean name = s && art.plain(art.speciesName(art.speciesOfDex(n))).toUpperCase(java.util.Locale.US).contains(q);
                if (!num && !name) {
                    continue;
                }
            }
            out.add(n);
        }
        if (sort > 0) {
            Collections.sort(out, (a, b) -> {
                boolean sa = bit(seen, a), sb = bit(seen, b);
                if (sa != sb) {
                    return sa ? -1 : 1;
                }
                if (!sa) {
                    return a - b;
                }
                if (sort == 2) {   // (TOTAL: the strongest first)
                    int diff = total(b) - total(a);
                    return diff != 0 ? diff : a - b;
                }
                return art.plain(art.speciesName(art.speciesOfDex(a))).compareTo(art.plain(art.speciesName(art.speciesOfDex(b))));
            });
        }
        return out;
    }

    /** The green screen's stripes (the game's: two rows light, two dark) */
    static void stripes(Canvas c, Paint fill, int w, int h, int phase) {
        fill.setColor(GREEN);
        c.drawRect(0, 0, w, h, fill);
        fill.setColor(GREEN_DARK);
        for (int y = 1 - (Math.floorMod(phase, 4)); y < h; y += 4) {
            c.drawRect(0, y, w, y + 2, fill);
        }
    }

    /** A magenta badge like the game's START / SELECT ones, its word in white beside it */
    static void badge(Canvas c, GameArt art, Paint fill, String inBadge, String beside, int x, int y) {
        int bw = art.width(inBadge) + 10;
        fill.setColor(0xFF000000);
        c.drawRect(x + 2, y, x + bw - 2, y + 13, fill);
        c.drawRect(x, y + 2, x + bw, y + 11, fill);
        fill.setColor(MAGENTA);
        c.drawRect(x + 2, y + 1, x + bw - 2, y + 12, fill);
        c.drawRect(x + 1, y + 2, x + bw - 1, y + 11, fill);
        art.text(c, inBadge, x + 5, y - 2, 0xFFFFFFFF, 0xFF900060);
        if (beside != null) {
            art.text(c, beside, x + bw + 3, y - 2, 0xFFFFFFFF, 0xFF000000);
        }
    }

    /** A chip that's on (magenta, white letters, as the game's badges) or off (white, ink letters); returns its width */
    static int chip(Canvas c, GameArt art, Paint fill, String word, int x, int y, boolean on) {
        return chip(c, art, fill, word, x, y, on, art.width(word) + 12);
    }

    /** A chip BW wide, its word in the middle */
    static int chip(Canvas c, GameArt art, Paint fill, String word, int x, int y, boolean on, int bw) {
        fill.setColor(0xFF000000);
        c.drawRect(x + 2, y, x + bw - 2, y + 15, fill);
        c.drawRect(x, y + 2, x + bw, y + 13, fill);
        fill.setColor(on ? MAGENTA : 0xFFFFFFFF);
        c.drawRect(x + 2, y + 1, x + bw - 2, y + 14, fill);
        c.drawRect(x + 1, y + 2, x + bw - 1, y + 13, fill);
        art.text(c, word, x + (bw - art.width(word)) / 2, y - 1, on ? 0xFFFFFFFF : GameArt.TEXT, on ? 0xFF900060 : GameArt.SHADOW);
        return bw;
    }

    private boolean screenPokemon;

    private Bitmap screen() {
        if (screen == null || screenPokemon != GameArt.pokemonArt) {
            screenPokemon = GameArt.pokemonArt;   // (CREATURES: its header says POKéDEX or MOéDEX)
            String name = screenPokemon ? "game/dex_list_p.png" : "game/dex_list.png";
            try (InputStream in = v.getContext().getAssets().open(name)) {
                BitmapFactory.Options o = new BitmapFactory.Options();
                o.inScaled = false;
                screen = BitmapFactory.decodeStream(in, null, o);
            } catch (Exception e) {
                return null;
            }
        }
        return screen;
    }

    private int cols(int w) {
        return Math.max(2, (w - GAP) / 86);
    }

    @Override
    public int scrollMax() {
        int cols = cols(v.width());
        return Math.max(0, HEAD + (shown.size() + cols - 1) / cols * (CARD_H + GAP) + 8 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONObject d = dex();
        String seen = d.optString("seen"), caught = d.optString("caught");
        shown = list();
        stripes(c, fill, w, h, scroll);
        // the cards
        int cols = cols(w), cw = (w - GAP * (cols + 1)) / cols;
        for (int i = 0; i < shown.size(); i++) {
            int x = GAP + (i % cols) * (cw + GAP), y = HEAD + (i / cols) * (CARD_H + GAP) - scroll;
            if (y + CARD_H < HEAD - 10 || y > h) {
                continue;
            }
            card(c, shown.get(i), x, y, cw, bit(seen, shown.get(i)), bit(caught, shown.get(i)));
        }
        if (shown.isEmpty()) {
            art.window(c, 0, 14, HEAD + 6, w - 28, 16);
            art.text(c, v.app.model() == null ? "Nothing from the game yet." : "No POKéMON match.", 16, HEAD + 6,
                    GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        // the header over them: the game's title, SEEN and OWN, the search, the filters
        stripes(c, fill, w, HEAD - 4, 0);
        fill.setColor(BORDER);
        c.drawRect(0, HEAD - 4, w, HEAD - 2, fill);
        Bitmap s = screen();
        if (s != null) {
            src.set(0, 4, 132, 18);   // (the logo's bar is rows 8-16 of the game's screen)
            dst.set(4, 2, 136, 16);
            c.drawBitmap(s, src, dst, pix);
        }
        int ns = 0, no = 0;
        for (int n = 1; n <= d.optInt("count"); n++) {
            ns += bit(seen, n) ? 1 : 0;
            no += bit(caught, n) ? 1 : 0;
        }
        String counts = "SEEN " + ns + "  OWN " + no;
        art.text(c, counts, w - 6 - art.width(counts), 0, 0xFFFFFFFF, 0xFF000000);
        // the search bar
        fill.setColor(BORDER);
        c.drawRect(6, 22, w - 70, 42, fill);
        fill.setColor(typing ? 0xFFFFFFFF : 0xFFF0F0E8);
        c.drawRect(7, 23, w - 71, 41, fill);
        // SPAWNS: where they live, map by map
        fill.setColor(BORDER);
        c.drawRect(w - 66, 22, w - 6, 42, fill);
        fill.setColor(0xFF58A848);
        c.drawRect(w - 65, 23, w - 7, 41, fill);
        art.text(c, "SPAWNS", w - 36 - art.width("SPAWNS") / 2, 24, GameArt.WHITE_TEXT, 0xFF285820);
        if (search.isEmpty() && !typing) {
            art.text(c, "Search: a name or a number", 12, 24, GameArt.GRAY_TEXT, GameArt.SHADOW);
        } else {
            int tw = art.text(c, search, 12, 24);
            if (typing && (System.currentTimeMillis() / 500) % 2 == 0) {
                fill.setColor(GameArt.TEXT);
                c.drawRect(13 + tw, 26, 14 + tw, 38, fill);
            }
            if (typing) {
                v.postInvalidateDelayed(500);
            }
        }
        if (!search.isEmpty()) {
            art.text(c, "x", w - 82, 24, GameArt.RED_TEXT, GameArt.SHADOW);
        }
        // the filters and the order
        int fx = 6;
        for (int i = 0; i < FILTERS.length; i++) {
            fx += chip(c, art, fill, FILTERS[i], fx, 50, i == filter) + 4;
        }
        String order = SORTS[sort];
        chip(c, art, fill, order, w - 6 - art.width(order) - 12, 50, true);
        String found = shown.size() + " POKéMON";
        art.text(c, found, 8, 72, 0xFFFFFFFF, 0xFF000000);
        // the TYPE CHART, and the type filter (its icon when one is chosen)
        int cx = w - 6 - art.width("CHART") - 12;
        chip(c, art, fill, "CHART", cx, 72, false);
        if (typeFilter >= 0) {
            fill.setColor(0xFFFFFFFF);
            c.drawRect(cx - 40, 71, cx - 4, 89, fill);
            art.typeIcon(c, typeFilter, cx - 38, 72);
        } else {
            chip(c, art, fill, "TYPE", cx - 4 - art.width("TYPE") - 12, 72, picking);
        }
        if (picking) {
            typePicker(c, w);
        }
    }

    /** The type filter's choices: ALL, then each type's icon, in a window under the header */
    private void typePicker(Canvas c, int w) {
        int per = Math.max(4, (w - 24) / 38), rows = (TypeChartPage.ORDER.length + per - 1) / per;
        int ph = 26 + rows * 20 + 6;
        art.window(c, 0, 8, HEAD, w - 16, ph - 8);
        chip(c, art, fill, "ALL TYPES", 14, HEAD + 2, typeFilter < 0);
        for (int i = 0; i < TypeChartPage.ORDER.length; i++) {
            int t = TypeChartPage.ORDER[i], x = 14 + (i % per) * 38, y = HEAD + 22 + (i / per) * 20;
            if (t == typeFilter) {
                fill.setColor(DexPage.MAGENTA);
                c.drawRect(x - 2, y - 2, x + 34, y + 18, fill);
            }
            art.typeIcon(c, t, x, y);
        }
    }

    private boolean pickerTap(int x, int y) {
        int w = v.width(), per = Math.max(4, (w - 24) / 38);
        if (y >= HEAD && y < HEAD + 18 && x < 14 + art.width("ALL TYPES") + 12) {
            typeFilter = -1;
            return true;
        }
        for (int i = 0; i < TypeChartPage.ORDER.length; i++) {
            int tx = 14 + (i % per) * 38, ty = HEAD + 22 + (i / per) * 20;
            if (in(x, y, tx - 2, ty - 2, 36, 20)) {
                typeFilter = TypeChartPage.ORDER[i];
                return true;
            }
        }
        return false;
    }

    /** One card: the picture on white, the number, the name, its types' colors along its foot */
    private void card(Canvas c, int n, int x, int y, int cw, boolean seen, boolean owned) {
        int sp = art.speciesOfDex(n);
        fill.setColor(BORDER);
        c.drawRect(x + 1, y, x + cw - 1, y + CARD_H, fill);
        c.drawRect(x, y + 1, x + cw, y + CARD_H - 1, fill);
        fill.setColor(seen ? 0xFFF8F8F0 : 0xFFD8D8C8);
        c.drawRect(x + 1, y + 1, x + cw - 1, y + CARD_H - 1, fill);
        // its types along the foot
        JSONArray t = seen ? art.species(sp).optJSONArray("t") : null;
        if (t != null) {
            fill.setColor(art.typeColor(t.optInt(0)));
            c.drawRect(x + 1, y + CARD_H - 22, x + cw - 1, y + CARD_H - 1, fill);
            if (t.optInt(1) != t.optInt(0)) {
                fill.setColor(art.typeColor(t.optInt(1)));
                c.drawRect(x + cw / 2, y + CARD_H - 22, x + cw - 1, y + CARD_H - 1, fill);
            }
        } else {
            fill.setColor(0xFFB0B0A0);
            c.drawRect(x + 1, y + CARD_H - 22, x + cw - 1, y + CARD_H - 1, fill);
        }
        art.front(c, sp, null, x + (cw - 64) / 2, y + 8, !seen);
        art.text(c, String.format(java.util.Locale.US, "No%03d", n), x + 4, y - 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (owned) {
            ball(c, x + cw - 11, y + 4);
        }
        byte[] name = seen ? art.speciesName(sp) : art.encode("?????");
        int nw = art.width(name);
        art.text(c, name, x + (cw - nw) / 2, y + CARD_H - 21, 0xFFFFFFFF, 0xFF303030);
    }

    /** The game's little POKé BALL beside an owned entry */
    private void ball(Canvas c, int x, int y) {
        fill.setColor(0xFF000000);
        c.drawRect(x + 1, y, x + 6, y + 7, fill);
        c.drawRect(x, y + 1, x + 7, y + 6, fill);
        fill.setColor(0xFFF83030);
        c.drawRect(x + 1, y + 1, x + 6, y + 3, fill);
        fill.setColor(0xFFFFFFFF);
        c.drawRect(x + 1, y + 4, x + 6, y + 6, fill);
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width();
        if (picking) {   // (the type picker: a choice, or a tap anywhere else closes it)
            picking = false;
            if (pickerTap(x, y)) {
                scroll = 0;
                remember();
                v.buzz(false);
            }
            return;
        }
        if (y < HEAD) {
            if (y >= 70 && y < 90) {
                int cx = w - 6 - art.width("CHART") - 12;
                if (x >= cx) {
                    v.showOver(new TypeChartPage(v));
                } else if (x >= cx - 4 - Math.max(40, art.width("TYPE") + 12)) {
                    if (typeFilter >= 0 && x >= cx - 40) {
                        typeFilter = -1;   // (the chosen type's icon: tap again for all of them)
                        scroll = 0;
                        remember();
                    } else {
                        picking = true;
                    }
                    v.buzz(false);
                }
                return;
            }
            if (y >= 20 && y < 44) {
                if (x > w - 68) {
                    typing = false;
                    v.keyboard(false);
                    v.select(4);   // (SPAWNS)
                    return;
                }
                if (!search.isEmpty() && x > w - 88) {
                    search = "";
                    scroll = 0;
                    return;
                }
                typing = true;
                v.keyboard(true);
                return;
            }
            if (y >= 48 && y < 68) {
                typing = false;
                v.keyboard(false);
                String order = SORTS[sort];
                if (x > w - 6 - art.width(order) - 12) {
                    sort = (sort + 1) % SORTS.length;
                    scroll = 0;
                    remember();
                    return;
                }
                int fx = 6;
                for (int i = 0; i < FILTERS.length; i++) {
                    int bw = art.width(FILTERS[i]) + 12;
                    if (x >= fx && x < fx + bw + 4) {
                        filter = i;
                        scroll = 0;
                        remember();
                        return;
                    }
                    fx += bw + 4;
                }
            }
            return;
        }
        if (typing) {
            typing = false;
            v.keyboard(false);
        }
        int cols = cols(w), cw = (w - GAP * (cols + 1)) / cols;
        int col = (x - GAP) / (cw + GAP), row = (y + scroll - HEAD) / (CARD_H + GAP);
        int i = row * cols + col;
        if (col >= 0 && col < cols && i >= 0 && i < shown.size()) {
            v.buzz(false);
            v.showOver(new DexEntryPage(v, new ArrayList<>(shown), i));
        }
    }

    @Override
    public void typed(String text) {
        String t = text.toUpperCase(java.util.Locale.US).replaceAll("[^A-Z0-9 .'-]", "");
        if (search.length() + t.length() <= 12) {
            search += t;
            scroll = 0;
        }
    }

    @Override
    public void backspace() {
        if (!search.isEmpty()) {
            search = search.substring(0, search.length() - 1);
        }
    }

    @Override
    public void typingDone() {
        typing = false;
    }

    @Override
    public void hidden() {
        if (typing) {
            typing = false;
            v.keyboard(false);
        }
    }

    @Override
    public void swipe(int dir) {
        filter = (filter + dir + FILTERS.length) % FILTERS.length;
        scroll = 0;
        remember();
    }
}
