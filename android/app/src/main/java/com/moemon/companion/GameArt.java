package com.moemon.companion;

import com.moemon.companion.core.Hex;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.Rect;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

/**
 * The game's own art and words (assets/game, from the game's export: android/build_assets.py): the font with its
 * widths, the POKeMON's icons and pictures, the BAG's icons, the PC wallpapers, the window frames. Everything is drawn
 * in the game's pixels (a canvas scaled by a whole number, no smoothing).
 */
public final class GameArt {
    public static final int TEXT = 0xFF404040, SHADOW = 0xFFD8D8D8, WHITE_TEXT = 0xFFF8F8F8, WHITE_SHADOW = 0xFF606060;
    public static final int RED_TEXT = 0xFFE01818, BLUE_TEXT = 0xFF2848D8, GRAY_TEXT = 0xFF909090;

    private final AssetManager assets;
    public final JSONObject data;
    private final int[] widths = new int[256];
    private final Map<Character, Integer> charmap = new HashMap<>();
    private final char[] letters = new char[256];
    private final Bitmap fontMain, fontShadow, smallMain, smallShadow;
    private final int[] smallWidths = new int[256];
    private Bitmap icons, items, wallpapers, frames, types;
    private final Bitmap[] fronts = new Bitmap[8], frontsP = new Bitmap[8];
    private final Map<Integer, Integer> dexToSpecies = new HashMap<>();
    private final Paint pix = new Paint(), tint = new Paint(), silhouette = new Paint(), gray = new Paint();
    private final Rect src = new Rect(), dst = new Rect();
    private final Paint windowFill = new Paint();

    public GameArt(AssetManager assets) throws Exception {
        this.assets = assets;
        data = new JSONObject(readText("game/data.json"));
        JSONArray w = data.getJSONArray("fontWidths");
        for (int i = 0; i < 256; i++) {
            widths[i] = w.optInt(i, 6);
        }
        JSONObject cm = data.getJSONObject("charmap");
        for (java.util.Iterator<String> it = cm.keys(); it.hasNext(); ) {
            String k = it.next();
            if (k.length() == 1 && k.charAt(0) < 0x3000) {
                charmap.put(k.charAt(0), cm.getInt(k));
                int g = cm.getInt(k) & 255;
                if (letters[g] == 0 || (k.charAt(0) < 128 && letters[g] >= 128)) {
                    letters[g] = k.charAt(0);
                }
            }
        }
        // the font's two colors (text, shadow) as white masks, tinted when drawn
        Bitmap[] big = masks("game/font.png");
        fontMain = big[0];
        fontShadow = big[1];
        // the game's small font: the second size of words (crisp, never a scaled-down big one)
        Bitmap[] small = masks("game/font_small.png");
        smallMain = small != null ? small[0] : fontMain;
        smallShadow = small != null ? small[1] : fontShadow;
        JSONArray sw = data.optJSONArray("fontSmallWidths");
        for (int i = 0; i < 256; i++) {
            smallWidths[i] = sw != null ? sw.optInt(i, 5) : widths[i];
        }
        JSONArray sp = data.getJSONArray("species");
        for (int s = sp.length() - 1; s > 0; s--) {
            int dex = sp.getJSONObject(s).optInt("dex");
            if (dex > 0 && s < 1100) {
                dexToSpecies.put(dex, s);
            }
        }
        windowFill.setColor(0xFFF8F8F8);
        pix.setFilterBitmap(false);
        pix.setAntiAlias(false);
        tint.setFilterBitmap(false);
        silhouette.setFilterBitmap(false);
        silhouette.setColorFilter(new PorterDuffColorFilter(0xFF303030, PorterDuff.Mode.SRC_IN));
        ColorMatrix cm2 = new ColorMatrix();
        cm2.setSaturation(0);
        gray.setFilterBitmap(false);
        gray.setColorFilter(new ColorMatrixColorFilter(cm2));
        gray.setAlpha(140);
    }

    /** A font sheet's text and shadow pixels as two white masks (null: no such sheet) */
    private Bitmap[] masks(String path) {
        Bitmap f = bitmap(path);
        if (f == null) {
            return null;
        }
        int n = f.getWidth() * f.getHeight();
        int[] px = new int[n], main = new int[n], shadow = new int[n];
        f.getPixels(px, 0, f.getWidth(), 0, 0, f.getWidth(), f.getHeight());
        for (int i = 0; i < n; i++) {
            if (Color.alpha(px[i]) != 0) {
                if (Color.red(px[i]) < 128) {
                    main[i] = 0xFFFFFFFF;
                } else {
                    shadow[i] = 0xFFFFFFFF;
                }
            }
        }
        return new Bitmap[]{Bitmap.createBitmap(main, f.getWidth(), f.getHeight(), Bitmap.Config.ARGB_8888),
                Bitmap.createBitmap(shadow, f.getWidth(), f.getHeight(), Bitmap.Config.ARGB_8888)};
    }

    private String readText(String path) throws Exception {
        try (InputStream in = assets.open(path)) {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] buf = new byte[65536];
            int n;
            while ((n = in.read(buf)) > 0) {
                out.write(buf, 0, n);
            }
            return out.toString("UTF-8");
        }
    }

    private Bitmap bitmap(String path) {
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inScaled = false;
        o.inPreferredConfig = Bitmap.Config.ARGB_8888;
        try (InputStream in = assets.open(path)) {
            return BitmapFactory.decodeStream(in, null, o);
        } catch (Exception e) {
            return null;
        }
    }

    // ------------------------------------------------------------------------------------------------- words

    /** The game's text bytes (hex, as the link sends names) */
    public static byte[] hexText(String hex) {
        if (hex == null) {
            return new byte[0];
        }
        int n = hex.length() / 2;
        byte[] b = new byte[n];
        for (int i = 0; i < n; i++) {
            b[i] = (byte) Integer.parseInt(hex.substring(2 * i, 2 * i + 2), 16);
        }
        return b;
    }

    /** The app's own words in the game's letters (charmap.txt); letters the font lacks become spaces */
    /** Whether the game's font has this letter (names typed on the phone) */
    public boolean hasLetter(char ch) {
        return ch != '$' && charmap.get(ch) != null;
    }

    public byte[] encode(String s) {
        byte[] b = new byte[s.length()];
        for (int i = 0; i < s.length(); i++) {
            // ('$' is the end of a string in the game's text; its money sign is the letter '¥' stands for)
            Integer c = charmap.get(s.charAt(i) == '$' ? '\u00a5' : s.charAt(i));
            b[i] = (byte) (c != null ? c : 0);
        }
        return b;
    }

    /** The app's own words as the game's text in hex (where a page draws game text by its hex) */
    public String toHex(String s) {
        return Hex.of(encode(s));
    }

    /** A game letter as plain text (for sorting and menu rows) */
    public char letter(int g) {
        return letters[g & 255] != 0 ? letters[g & 255] : ' ';
    }

    public String plain(byte[] t) {
        StringBuilder s = new StringBuilder();
        for (byte b : t) {
            if ((b & 255) == 0xFF) {
                break;
            }
            s.append(letter(b));
        }
        return s.toString();
    }

    // ---------------------------------------------------------------------------- MOéMON or POKéMON (CREATURES)

    /** The pictures and words shown: the original POKéMON's (true) or Kai's MOéMON (Look.creatures, as the game) */
    public static boolean pokemonArt;
    /** Whether the words are swapped at all (off on the card games' pages: Page.swapsWords) */
    public static boolean swapWords = true;
    private byte[][][][] swaps;   // [mode][pair][from, to]

    /**
     * The words swapped as the game's OPTIONS > CREATURES swaps them (mp_textopts.c): with POKéMON, Kai's MOé becomes
     * POKé (MOéMON, MOéDEX...) and MOEMON POKéMON; with MOéMON, the app's own POKéMON, POKéDEX and POKéNAV become Kai's
     * (POKé BALL stays: Kai kept it)
     */
    private byte[] words(byte[] t) {
        if (!swapWords) {
            return t;
        }
        if (swaps == null) {
            String[][][] w = {
                    {{"POK\u00e9MON", "MO\u00e9MON"}, {"POK\u00e9DEX", "MO\u00e9DEX"}, {"POK\u00e9NAV", "MO\u00e9NAV"},
                     {"Pok\u00e9mon", "Mo\u00e9mon"}},
                    {{"MO\u00e9", "POK\u00e9"}, {"Mo\u00e9", "Pok\u00e9"}, {"MOEMON", "POK\u00e9MON"}, {"Moemon", "Pok\u00e9mon"}}};
            swaps = new byte[2][][][];
            for (int m = 0; m < 2; m++) {
                swaps[m] = new byte[w[m].length][][];
                for (int i = 0; i < w[m].length; i++) {
                    swaps[m][i] = new byte[][]{encode(w[m][i][0]), encode(w[m][i][1])};
                }
            }
        }
        byte[][][] pairs = swaps[pokemonArt ? 1 : 0];
        java.io.ByteArrayOutputStream out = null;
        for (int i = 0; i < t.length; ) {
            byte[][] hit = null;
            for (byte[][] pair : pairs) {
                if (startsAt(t, i, pair[0])) {
                    hit = pair;
                    break;
                }
            }
            if (hit != null) {
                if (out == null) {
                    out = new java.io.ByteArrayOutputStream(t.length + 4);
                    out.write(t, 0, i);
                }
                out.write(hit[1], 0, hit[1].length);
                i += hit[0].length;
            } else {
                if ((t[i] & 255) == 0xFF) {
                    if (out != null) {
                        out.write(t, i, t.length - i);
                    }
                    break;
                }
                if (out != null) {
                    out.write(t[i]);
                }
                i++;
            }
        }
        return out == null ? t : out.toByteArray();
    }

    private static boolean startsAt(byte[] t, int at, byte[] p) {
        if (at + p.length > t.length) {
            return false;
        }
        for (int k = 0; k < p.length; k++) {
            if (t[at + k] != p[k]) {
                return false;
            }
        }
        return true;
    }

    public int width(byte[] t) {
        t = words(t);
        int w = 0;
        for (byte c : t) {
            if ((c & 255) == 0xFF) {
                break;
            }
            w += widths[c & 255];
        }
        return w;
    }

    public int width(String s) {
        return width(encode(s));
    }

    /** Text at (x, y) (its 16-pixel cell's top), in a color and its shadow; returns the width */
    public int text(Canvas c, byte[] t, int x, int y, int color, int shadow) {
        t = words(t);
        int x0 = x;
        for (byte ch : t) {
            int g = ch & 255;
            if (g == 0xFF) {
                break;
            }
            // (only the glyph's own width: the rest of its 16x16 cell isn't part of it)
            int gw = Math.max(1, Math.min(16, widths[g]));
            src.set((g % 16) * 16, (g / 16) * 16, (g % 16) * 16 + gw, (g / 16) * 16 + 16);
            if (shadow != 0) {
                tint.setColorFilter(filter(shadow));
                dst.set(x, y, x + gw, y + 16);
                c.drawBitmap(fontShadow, src, dst, tint);
            }
            tint.setColorFilter(filter(color));
            dst.set(x, y, x + gw, y + 16);
            c.drawBitmap(fontMain, src, dst, tint);
            x += widths[g];
        }
        return x - x0;
    }

    private final Map<Integer, PorterDuffColorFilter> filters = new HashMap<>();

    private PorterDuffColorFilter filter(int color) {
        PorterDuffColorFilter f = filters.get(color);
        if (f == null) {
            f = new PorterDuffColorFilter(color, PorterDuff.Mode.SRC_IN);
            filters.put(color, f);
        }
        return f;
    }

    public int text(Canvas c, byte[] t, int x, int y) {
        return text(c, t, x, y, TEXT, SHADOW);
    }

    public int text(Canvas c, String s, int x, int y) {
        return text(c, encode(s), x, y, TEXT, SHADOW);
    }

    public int text(Canvas c, String s, int x, int y, int color, int shadow) {
        return text(c, encode(s), x, y, color, shadow);
    }

    /** The small font (the game's FONT_SMALL): secondary words, notes, labels; y is its cell's top as with text() */
    public int small(Canvas c, String s, int x, int y, int color, int shadow) {
        return small(c, encode(s), x, y, color, shadow);
    }

    public int small(Canvas c, byte[] t, int x, int y, int color, int shadow) {
        t = words(t);
        int x0 = x;
        for (byte ch : t) {
            int g = ch & 255;
            if (g == 0xFF) {
                break;
            }
            int gw = Math.max(1, Math.min(16, smallWidths[g]));
            src.set((g % 16) * 16, (g / 16) * 16, (g % 16) * 16 + gw, (g / 16) * 16 + 16);
            dst.set(x, y, x + gw, y + 16);
            if (shadow != 0) {
                tint.setColorFilter(filter(shadow));
                c.drawBitmap(smallShadow, src, dst, tint);
            }
            tint.setColorFilter(filter(color));
            c.drawBitmap(smallMain, src, dst, tint);
            x += smallWidths[g];
        }
        return x - x0;
    }

    public int smallWidth(String s) {
        byte[] t = words(encode(s));
        int w = 0;
        for (byte b : t) {
            if ((b & 255) == 0xFF) {
                break;
            }
            w += smallWidths[b & 255];
        }
        return w;
    }

    /** Words wrapped to max pixels in the small font */
    public java.util.List<String> wrapSmall(String text, int max) {
        java.util.List<String> out = new java.util.ArrayList<>();
        StringBuilder line = new StringBuilder();
        for (String word : text.split(" ")) {
            String next = line.length() == 0 ? word : line + " " + word;
            if (smallWidth(next) > max && line.length() > 0) {
                out.add(line.toString());
                line = new StringBuilder(word);
            } else {
                line = new StringBuilder(next);
            }
        }
        out.add(line.toString());
        return out;
    }

    public int textHex(Canvas c, String hex, int x, int y, int color, int shadow) {
        return text(c, hexText(hex), x, y, color, shadow);
    }

    // ------------------------------------------------------------------------------------------------ tables

    public JSONObject species(int s) {
        JSONArray a = data.optJSONArray("species");
        JSONObject o = a == null ? null : a.optJSONObject(s);
        return o != null ? o : new JSONObject();
    }

    public byte[] speciesName(int s) {
        return hexText(species(s).optString("n"));
    }

    public int speciesOfDex(int dex) {
        Integer s = dexToSpecies.get(dex);
        return s != null ? s : 0;
    }

    public byte[] name(String table, int i) {
        JSONArray a = data.optJSONArray(table);
        if (a == null) {
            return new byte[0];
        }
        Object o = a.opt(i);
        if (o instanceof JSONObject) {
            return hexText(((JSONObject) o).optString("n"));
        }
        return hexText(o == null ? "" : o.toString());
    }

    public JSONObject item(int i) {
        JSONArray a = data.optJSONArray("items");
        JSONObject o = a == null ? null : a.optJSONObject(i);
        return o != null ? o : new JSONObject();
    }

    public JSONObject move(int i) {
        JSONArray a = data.optJSONArray("moves");
        JSONObject o = a == null ? null : a.optJSONObject(i);
        return o != null ? o : new JSONObject();
    }

    // ------------------------------------------------------------------------------------------------- art

    private void cell(Canvas c, Bitmap b, int index, int cw, int ch, int cols, int x, int y, Paint p) {
        if (b == null) {
            return;
        }
        int sx = (index % cols) * cw, sy = (index / cols) * ch;
        if (sy + ch > b.getHeight()) {
            return;
        }
        src.set(sx, sy, sx + cw, sy + ch);
        dst.set(x, y, x + cw, y + ch);
        c.drawBitmap(b, src, dst, p);
    }

    public static final int ICON_PLAIN = 0, ICON_SILHOUETTE = 1, ICON_GRAY = 2;

    /** A POKeMON's party icon (32x32), its top left at (x, y) */
    public void icon(Canvas c, int species, int x, int y, int how) {
        if (icons == null) {
            icons = bitmap("game/icons.png");
        }
        cell(c, icons, species, 32, 32, 32, x, y, how == ICON_SILHOUETTE ? silhouette : how == ICON_GRAY ? gray : pix);
    }

    /** A POKeMON's front picture (64x64) in its species' colors */
    public void front(Canvas c, int species, int x, int y, boolean silhouetted) {
        front(c, species, null, x, y, silhouetted);
    }

    // the pictures are palette indexes (gray = index x 17): colored here by the species' palette or a POKeMON's own
    private final java.util.LinkedHashMap<String, Bitmap> colored = new java.util.LinkedHashMap<String, Bitmap>(64, 0.75f, true) {
        @Override
        protected boolean removeEldestEntry(Map.Entry<String, Bitmap> e) {
            return size() > 64;
        }
    };

    /** The 16 colors of a palette written as the game's BGR555 in hex (4 digits each), as ARGB */
    static int[] palette(String hex) {
        int[] p = new int[16];
        for (int i = 0; i < 16 && hex != null && hex.length() >= (i + 1) * 4; i++) {
            int v = Integer.parseInt(hex.substring(i * 4, i * 4 + 4), 16);
            p[i] = 0xFF000000 | (((v & 31) * 255 / 31) << 16) | ((((v >> 5) & 31) * 255 / 31) << 8) | (((v >> 10) & 31) * 255 / 31);
        }
        return p;
    }

    /** A POKéMON's own colors in the set shown (the snapshot's "pal" / "ppal": shiny, a Stadium hue), or null */
    public String monPal(org.json.JSONObject mon) {
        return mon == null ? null : mon.optString(pokemonArt ? "ppal" : "pal", null);
    }

    /** A front picture colored by pal (a POKeMON's own palette, the snapshot's "pal"), or its species' when null */
    public void front(Canvas c, int species, String pal, int x, int y, boolean silhouetted) {
        front(c, species, pal, x, y, 64, silhouetted);
    }

    /** The same at size x size (a whole multiple of 64 keeps the pixels sharp) */
    public void front(Canvas c, int species, String pal, int x, int y, int size, boolean silhouetted) {
        int k = species / 256;
        if (k >= fronts.length) {
            return;
        }
        boolean po = pokemonArt;
        String palKey = po ? "ppal" : "pal";
        if (pal == null || pal.isEmpty()) {
            pal = species(species).optString(palKey, null);
            if (pal == null) {
                // (UNOWN's letters past the EGG: UNOWN's colors)
                pal = species(201).optString(palKey, "");
            }
        }
        String key = (po ? "p" : "") + species + ":" + pal;
        Bitmap b = colored.get(key);
        if (b == null) {
            Bitmap[] set = po ? frontsP : fronts;
            if (set[k] == null) {
                set[k] = bitmap(po ? "game/front_p_" + k + ".png" : "game/front_" + k + ".png");
            }
            Bitmap sheet = set[k];
            int i = species % 256, sx = (i % 16) * 64, sy = (i / 16) * 64;
            if (sheet == null || sy + 64 > sheet.getHeight()) {
                return;
            }
            int[] px = new int[64 * 64], colors = palette(pal);
            sheet.getPixels(px, 0, 64, sx, sy, 64, 64);
            for (int p = 0; p < px.length; p++) {
                int idx = Color.alpha(px[p]) == 0 ? 0 : Math.round(Color.red(px[p]) / 17f) & 15;
                px[p] = idx == 0 ? 0 : colors[idx];
            }
            b = Bitmap.createBitmap(px, 64, 64, Bitmap.Config.ARGB_8888);
            colored.put(key, b);
        }
        src.set(0, 0, 64, 64);
        dst.set(x, y, x + size, y + size);
        c.drawBitmap(b, src, dst, silhouetted ? silhouette : pix);
    }

    /** The game's type icon (32x16) for a type (18 = FAIRY's place in Kai: by number), as the summary shows it */
    public void typeIcon(Canvas c, int type, int x, int y) {
        if (types == null) {
            types = bitmap("game/types.png");
        }
        cell(c, types, type, 32, 16, 1, x, y, pix);
    }

    /** A type's own color (from its icon's fill), for the POKéDEX's cards */
    public int typeColor(int type) {
        if (types == null) {
            types = bitmap("game/types.png");
        }
        if (types == null || type * 16 + 8 >= types.getHeight()) {
            return 0xFFA8A8A8;
        }
        return types.getPixel(30, type * 16 + 8) | 0xFF000000;
    }

    /** A POKé BALL's icon (MON_DATA_POKEBALL), the ball's BAG icon */
    public void ballIcon(Canvas c, int ball, int x, int y) {
        // (the snapshot's "ball" is the ball's item, as MON_DATA_POKEBALL keeps it, or its special ball's)
        itemIcon(c, ball > 0 ? ball : 4, x, y);
    }

    /**
     * Game text made plain for wrapping: its line and page breaks (the GBA's windows' own) become spaces, control codes
     * go (descriptions are written for a 240-pixel screen; here they're wrapped to the window)
     */
    public static byte[] clean(byte[] t) {
        java.io.ByteArrayOutputStream o = new java.io.ByteArrayOutputStream();
        for (int i = 0; i < t.length; i++) {
            int g = t[i] & 255;
            if (g == 0xFF) {
                break;
            } else if (g == 0xFE || g == 0xFA || g == 0xFB) {
                o.write(0);
            } else if (g == 0xFC || g == 0xFD) {
                i++;   // (a control code and its argument)
            } else {
                o.write(g);
            }
        }
        return o.toByteArray();
    }

    /** Game text in lines no wider than max pixels */
    public java.util.List<byte[]> wrap(byte[] text, int max) {
        byte[] t = clean(text);
        java.util.List<byte[]> lines = new java.util.ArrayList<>();
        int start = 0;
        while (start < t.length) {
            while (start < t.length && t[start] == 0) {
                start++;
            }
            int end = start, fit = -1, w = 0;
            while (end < t.length) {
                w += widths[t[end] & 255];
                if (w > max) {
                    break;
                }
                if (end + 1 == t.length || t[end + 1] == 0) {
                    fit = end + 1;
                }
                end++;
            }
            if (end == t.length) {
                fit = end;
            }
            if (fit <= start) {
                fit = Math.max(start + 1, end);   // (a word wider than the line: cut)
            }
            lines.add(java.util.Arrays.copyOfRange(t, start, fit));
            start = fit;
        }
        return lines;
    }

    /** Game text by its hex (the tables' and the snapshot's), wrapped and drawn; returns the height used */
    public int paragraph(Canvas c, String hex, int x, int y, int max, int color, int shadow) {
        int yy = y;
        for (byte[] line : wrap(hexText(hex), max)) {
            text(c, line, x, yy, color, shadow);
            yy += 14;
        }
        return yy - y;
    }

    // ------------------------------------------------------------------------------------------- the device's art
    private final Map<Integer, Bitmap> townMaps = new HashMap<>();
    private Bitmap ribbons, trainers;

    /** A town map page as its full view shows it (240x160; SINNOH's at twice that); null when it isn't there */
    public Bitmap townMap(int page) {
        if (!townMaps.containsKey(page)) {
            townMaps.put(page, bitmap("game/townmap_" + page + ".png"));
        }
        return townMaps.get(page);
    }

    /** A ribbon's big icon (32x32), drawn at a whole SIZE multiple of 32 */
    public void ribbon(Canvas c, int id, int x, int y, int size) {
        if (ribbons == null) {
            ribbons = bitmap("game/ribbons.png");
        }
        if (ribbons == null || id < 0 || (id + 1) * 32 > ribbons.getHeight()) {
            return;
        }
        src.set(0, id * 32, 32, id * 32 + 32);
        dst.set(x, y, x + size, y + size);
        c.drawBitmap(ribbons, src, dst, pix);
    }

    /** A trainer's picture (64x64), as MATCH CALL shows them */
    public void trainerPic(Canvas c, int pic, int x, int y, int size) {
        if (trainers == null) {
            trainers = bitmap("game/trainers.png");
        }
        if (trainers == null || pic < 0 || (pic + 1) * 64 > trainers.getHeight()) {
            return;
        }
        src.set(0, pic * 64, 64, pic * 64 + 64);
        dst.set(x, y, x + size, y + size);
        c.drawBitmap(trainers, src, dst, pix);
    }

    /** Any bitmap at a size, its pixels kept sharp */
    public void sharp(Canvas c, Bitmap b, int x, int y, int w, int h) {
        if (b == null) {
            return;
        }
        src.set(0, 0, b.getWidth(), b.getHeight());
        dst.set(x, y, x + w, y + h);
        c.drawBitmap(b, src, dst, pix);
    }

    /** How ATK hits a POKéMON of types T0 / T1, in tenths (the game's type chart: 0, 5, 10, 20, 40...) */
    public int effectiveness(int atk, int t0, int t1) {
        JSONArray chart = data.optJSONArray("typeChart");
        int m = 10;
        for (int i = 0; chart != null && i < chart.length(); i++) {
            JSONArray e = chart.optJSONArray(i);
            if (e.optInt(0) != atk) {
                continue;
            }
            int def = e.optInt(1), mul = e.optInt(2);
            if (def == t0) {
                m = m * mul / 10;
            }
            if (def == t1 && t1 != t0) {
                m = m * mul / 10;
            }
        }
        return m;
    }

    /** A BAG icon (24x24) */
    public void itemIcon(Canvas c, int item, int x, int y) {
        if (items == null) {
            items = bitmap("game/items.png");
        }
        cell(c, items, item, 24, 24, 32, x, y, pix);
    }

    /** A BAG icon's 24x24 pixels (ARGB), for drawing it changed (the POKé BALL page's colors and decorations) */
    public int[] itemPixels(int item) {
        if (items == null) {
            items = bitmap("game/items.png");
        }
        int[] px = new int[24 * 24];
        int sx = (item % 32) * 24, sy = (item / 32) * 24;
        if (items == null || sy + 24 > items.getHeight()) {
            return px;
        }
        items.getPixels(px, 0, 24, sx, sy, 24, 24);
        return px;
    }

    /** A PC box's wallpaper (160x144); Walda's (16) shows as PLAIN */
    public void wallpaper(Canvas c, int id, int x, int y) {
        if (wallpapers == null) {
            wallpapers = bitmap("game/wallpapers.png");
        }
        cell(c, wallpapers, id >= 16 ? 15 : id, 160, 144, 4, x, y, pix);
    }

    /** A wallpaper's framed inside (below its name banner: 160x122) at any size (the multi-box view's boxes) */
    public void wallpaperAt(Canvas c, int id, float x, float y, float w, float h) {
        if (wallpapers == null) {
            wallpapers = bitmap("game/wallpapers.png");
        }
        int i = id >= 16 ? 15 : id;
        src.set((i % 4) * 160, (i / 4) * 144 + 22, (i % 4) * 160 + 160, (i / 4) * 144 + 144);
        c.drawBitmap(wallpapers, src, new android.graphics.RectF(x, y, x + w, y + h), pix);
    }

    /** A POKéMON's icon at SIZE (16: half size, still whole pixels) */
    public void iconAt(Canvas c, int species, float x, float y, float size, int how) {
        c.save();
        c.translate(x, y);
        c.scale(size / 32f, size / 32f);
        icon(c, species, 0, 0, how);
        c.restore();
    }

    /** A wallpaper at half size (80x72), for the box list */
    public void wallpaperHalf(Canvas c, int id, int x, int y) {
        if (wallpapers == null) {
            wallpapers = bitmap("game/wallpapers.png");
        }
        int i = id >= 16 ? 15 : id;
        src.set((i % 4) * 160, (i / 4) * 144, (i % 4) * 160 + 160, (i / 4) * 144 + 144);
        dst.set(x, y, x + 80, y + 72);
        c.drawBitmap(wallpapers, src, dst, pix);
    }

    /** A window: OPTIONS > FRAME's frame (its 8-pixel tiles) around x, y, w, h, filled with the window's white */
    /** The player's window frame (MENU > LOOK), for the windows drawn with frame 0 */
    public static int userFrame;

    public void window(Canvas c, int frame, int x, int y, int w, int h) {
        if (frame == 0) {
            frame = userFrame;
        }
        if (frames == null) {
            frames = bitmap("game/frames.png");
        }
        c.drawRect(x, y, x + w, y + h, windowFill);
        if (frames == null) {
            return;
        }
        int fx = (frame % 32) * 24;
        // corners, then the sides stretched by repeating their tile
        tile(c, fx, 0, x - 8, y - 8);
        tile(c, fx + 16, 0, x + w, y - 8);
        tile(c, fx, 16, x - 8, y + h);
        tile(c, fx + 16, 16, x + w, y + h);
        for (int i = 0; i < w; i += 8) {
            int tw = Math.min(8, w - i);
            part(c, fx + 8, 0, tw, 8, x + i, y - 8);
            part(c, fx + 8, 16, tw, 8, x + i, y + h);
        }
        for (int j = 0; j < h; j += 8) {
            int th = Math.min(8, h - j);
            part(c, fx, 8, 8, th, x - 8, y + j);
            part(c, fx + 16, 8, 8, th, x + w, y + j);
        }
    }

    private void tile(Canvas c, int sx, int sy, int x, int y) {
        part(c, sx, sy, 8, 8, x, y);
    }

    private void part(Canvas c, int sx, int sy, int w, int h, int x, int y) {
        src.set(sx, sy, sx + w, sy + h);
        dst.set(x, y, x + w, y + h);
        c.drawBitmap(frames, src, dst, pix);
    }
}
