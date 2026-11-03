package com.moemon.companion;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.LruCache;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.InputStream;

/**
 * The POKéMON cards as the game prints them: the scans in the game's own pack (graphics/tcg/printed.bin, in the app as
 * assets game/tcg_cards.bin with WebP pictures: "TCGP", each card's picture and where its illustration sits, then the
 * back), decoded at about the size they're shown and kept a while; the catalog from the game's engine (Tcg.catalog);
 * the types' colors and Energy symbols as the game's duel table draws them.
 */
public final class TcgArt {
    public static final float W = 360, H = 495;   // (a card's proportions)
    private static final int[] TYPE = {0xFFE85830, 0xFF50A838, 0xFFF0C020, 0xFF3880E0, 0xFFB86830, 0xFF9050C8, 0xFFD8D8D0};
    private static final int[] TYPE_DARK = {0xFF902010, 0xFF206818, 0xFF987000, 0xFF184890, 0xFF683810, 0xFF502078, 0xFF888880};

    private final AssetManager assets;
    private int[] table;   // per card (0 the back): offset, size, art x, y, w, h
    private int count;
    private JSONObject[] catalog;
    private int[][] packCards;
    private final Paint smooth = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.ANTI_ALIAS_FLAG);
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Rect src = new Rect();
    private final RectF dst = new RectF();
    private final LruCache<Integer, Bitmap> cache = new LruCache<Integer, Bitmap>(28 << 20) {
        @Override
        protected int sizeOf(Integer key, Bitmap b) {
            return b.getByteCount();
        }
    };

    public TcgArt(AssetManager assets) {
        this.assets = assets;
    }

    // ------------------------------------------------------------------------------------------------- data

    /** The card's catalog entry (Tcg.catalog), null for none */
    public synchronized JSONObject card(int id) {
        if (catalog == null) {
            catalog = new JSONObject[443];
            try {
                JSONArray a = new JSONArray(Tcg.catalog());
                catalog = new JSONObject[a.length() + 1];
                for (int i = 0; i < a.length(); i++) {
                    catalog[a.getJSONObject(i).getInt("id")] = a.getJSONObject(i);
                }
            } catch (Exception ignored) {
            }
        }
        return id > 0 && id < catalog.length ? catalog[id] : null;
    }

    public int maxId() {
        card(0);
        return catalog.length - 1;
    }

    public String name(int id) {
        JSONObject c = card(id);
        return c == null ? "" : c.optString("n");
    }

    /** The cards a booster (0-10) can hold */
    public synchronized int[] packCards(int pack) {
        if (packCards == null) {
            packCards = new int[Tcg.PACKS][];
            try {
                JSONArray a = new JSONArray(Tcg.packs());
                for (int p = 0; p < Tcg.PACKS; p++) {
                    JSONArray k = a.getJSONArray(p);
                    packCards[p] = new int[k.length()];
                    for (int i = 0; i < k.length(); i++) {
                        packCards[p][i] = k.getInt(i);
                    }
                }
            } catch (Exception e) {
                for (int p = 0; p < Tcg.PACKS; p++) {
                    packCards[p] = new int[0];
                }
            }
        }
        return pack >= 0 && pack < Tcg.PACKS ? packCards[pack] : new int[0];
    }

    /** A pack's face: its first holo POKéMON (tcg_ui.c PackMascot's choice for the first game's four) */
    public int packMascot(int pack) {
        if (pack < 4) {
            // (Charizard, Venusaur, Mewtwo and Zapdos: tcg_ui.c PackMascot)
            return firstNamed(new String[]{"Charizard", "Venusaur", "Mewtwo", "Zapdos"}[pack], 0);
        }
        int best = 0;
        for (int id : packCards(pack)) {
            JSONObject c = card(id);
            if (c == null || c.optInt("k") != Tcg.POKEMON) {
                continue;
            }
            if (c.optInt("h") == 1) {
                return id;
            }
            JSONObject b = card(best);
            if (b == null || c.optInt("ra") > b.optInt("ra") || (c.optInt("ra") == b.optInt("ra") && c.optInt("hp") > b.optInt("hp"))) {
                best = id;
            }
        }
        return best;
    }

    private int firstNamed(String name, int otherwise) {
        for (int id = 1; id <= maxId(); id++) {
            JSONObject c = card(id);
            if (c != null && c.optInt("h") == 1 && c.optString("n").equals(name)) {
                return id;
            }
        }
        return otherwise;
    }

    // -------------------------------------------------------------------------------------------- the pack

    private synchronized boolean table() {
        if (table != null) {
            return count > 0;
        }
        table = new int[0];
        try (InputStream in = assets.open("game/tcg_cards.bin")) {
            byte[] head = new byte[8];
            if (in.read(head) != 8 || head[0] != 'T' || head[1] != 'C' || head[2] != 'G' || head[3] != 'P') {
                return false;
            }
            int n = u32(head, 4);
            byte[] t = new byte[16 * (n + 1)];
            int got = 0;
            while (got < t.length) {
                int r = in.read(t, got, t.length - got);
                if (r <= 0) {
                    return false;
                }
                got += r;
            }
            table = new int[6 * (n + 1)];
            for (int i = 0; i <= n; i++) {
                table[i * 6] = u32(t, i * 16);
                table[i * 6 + 1] = u32(t, i * 16 + 4);
                for (int k = 0; k < 4; k++) {
                    table[i * 6 + 2 + k] = (t[i * 16 + 8 + k * 2] & 255) | (t[i * 16 + 9 + k * 2] & 255) << 8;
                }
            }
            count = n;
        } catch (Exception e) {
            count = 0;
        }
        return count > 0;
    }

    private static int u32(byte[] b, int at) {
        return (b[at] & 255) | (b[at + 1] & 255) << 8 | (b[at + 2] & 255) << 16 | (b[at + 3] & 255) << 24;
    }

    // entry i: card id - 1, the back last
    private int entry(int id) {
        return id <= 0 ? count : id - 1;
    }

    /** The card's picture decoded at 1/sample of its size (1, 2, 4 or 8), null when the pack hasn't got it */
    public Bitmap picture(int id, int sample) {
        if (!table() || (id > count)) {
            return null;
        }
        int key = (id <= 0 ? 0 : id) * 16 + sample;
        Bitmap b = cache.get(key);
        if (b != null) {
            return b;
        }
        int e = entry(id), off = table[e * 6], size = table[e * 6 + 1];
        if (size <= 0) {
            return null;
        }
        try (InputStream in = assets.open("game/tcg_cards.bin")) {
            long skip = off;
            while (skip > 0) {
                long k = in.skip(skip);
                if (k <= 0) {
                    return null;
                }
                skip -= k;
            }
            byte[] data = new byte[size];
            int got = 0;
            while (got < size) {
                int r = in.read(data, got, size - got);
                if (r <= 0) {
                    return null;
                }
                got += r;
            }
            BitmapFactory.Options o = new BitmapFactory.Options();
            o.inSampleSize = sample;
            b = BitmapFactory.decodeByteArray(data, 0, size, o);
        } catch (Exception ex) {
            return null;
        }
        if (b != null) {
            cache.put(key, b);
        }
        return b;
    }

    /** Where the card's illustration sits in its picture (at full size): x, y, w, h */
    public int[] artRect(int id) {
        if (!table() || id <= 0 || id > count) {
            return new int[]{0, 0, (int) W, (int) H};
        }
        int e = entry(id);
        int w = table[e * 6 + 4], h = table[e * 6 + 5];
        return w == 0 || h == 0 ? new int[]{0, 0, (int) W, (int) H}
                : new int[]{table[e * 6 + 2], table[e * 6 + 3], w, h};
    }

    private int sampleFor(float screenW) {
        int s = 1;
        while (s < 8 && W / (s * 2) >= screenW * 0.85f) {
            s *= 2;
        }
        return s;
    }

    /** The card (0 its back) at x, y in w x h of the page's pixels, `scale` screen pixels to one of those */
    public void draw(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        Bitmap b = picture(id, sampleFor(w * scale));
        dst.set(x, y, x + w, y + h);
        if (b == null) {
            fill.setColor(id == 0 ? 0xFF2050A0 : 0xFFE8E0C8);
            c.drawRoundRect(dst, w * 0.05f, w * 0.05f, fill);
            return;
        }
        smooth.setAlpha(alpha);
        src.set(0, 0, b.getWidth(), b.getHeight());
        c.drawBitmap(b, src, dst, smooth);
        smooth.setAlpha(255);
    }

    /** Only the card's illustration, filling x, y, w, h (cut to keep its shape) */
    public void drawArt(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        int[] r = artRect(id);
        float k = Math.max(w / r[2], h / r[3]);
        Bitmap b = picture(id, sampleFor(W * k * scale));
        if (b == null) {
            draw(c, id, x, y, w, h, scale, alpha);
            return;
        }
        float f = b.getWidth() / W, cw = w / k, ch = h / k;
        float cx = r[0] + (r[2] - cw) / 2, cy = r[1] + (r[3] - ch) / 2;
        src.set(Math.round(cx * f), Math.round(cy * f), Math.round((cx + cw) * f), Math.round((cy + ch) * f));
        dst.set(x, y, x + w, y + h);
        smooth.setAlpha(alpha);
        c.drawBitmap(b, src, dst, smooth);
        smooth.setAlpha(255);
    }

    // ------------------------------------------------------------------------------------------- the symbols

    public static int typeColor(int type) {
        return type >= 0 && type < 7 ? TYPE[type] : 0xFF888888;
    }

    public static int typeDark(int type) {
        return type >= 0 && type < 7 ? TYPE_DARK[type] : 0xFF404040;
    }

    /** An Energy symbol: a round orb in the type's color (Double Colorless: two) */
    public void orb(Canvas c, int type, float cx, float cy, float r) {
        fill.setColor(typeDark(type));
        c.drawCircle(cx, cy, r, fill);
        fill.setColor(typeColor(type));
        c.drawCircle(cx, cy, r * 0.78f, fill);
        fill.setColor(0x90FFFFFF);
        c.drawCircle(cx - r * 0.25f, cy - r * 0.25f, r * 0.3f, fill);
    }

    /** The Energy a card attached gives: its type, and how much (Double Colorless 2) */
    public int energyType(int id) {
        JSONObject c = card(id);
        return c == null ? Tcg.COLORLESS : c.optInt("t");
    }
}
