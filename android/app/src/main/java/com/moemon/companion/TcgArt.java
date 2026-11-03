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
    private JSONObject[] catalog;
    private int[][] packCards;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);

    public TcgArt(AssetManager assets) {
        this.assets = assets;
        this.pack = new CardPack(assets, "game/tcg_cards.bin", "TCGP", W, H, 28 << 20);
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

    private final CardPack pack;

    public Bitmap picture(int id, int sample) {
        return pack.picture(id, sample);
    }

    public int[] artRect(int id) {
        return pack.artRect(id);
    }

    public void draw(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.draw(c, id, x, y, w, h, scale, alpha);
    }

    public void drawTurned(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.drawTurned(c, id, x, y, w, h, scale, alpha);
    }

    public void drawArt(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.drawArt(c, id, x, y, w, h, scale, alpha);
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
