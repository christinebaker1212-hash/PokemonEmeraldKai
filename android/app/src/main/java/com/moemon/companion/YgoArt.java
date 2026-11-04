package com.moemon.companion;

import android.content.res.AssetManager;
import android.graphics.Canvas;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

/**
 * YU-GI-OH! for the app: the engine's pack (assets game/ygo_edo.pak, handed to the engine once), the catalog and the
 * sets from the game's own tables (Ygo.catalog / Ygo.sets), the printed cards (game/ygo_cards.bin through CardPack),
 * and the words for attributes, Types and frames as the game's screens use them.
 */
public final class YgoArt {
    public static final float W = 160, H = 233;   // (a card's proportions)
    public static final String[] ATTRIBUTES = {"", "LIGHT", "DARK", "WATER", "FIRE", "EARTH", "WIND", "DIVINE"};
    public static final String[] RACES = {"", "Aqua", "Beast", "Beast-Warrior", "Dinosaur", "Dragon", "Fairy", "Fiend", "Fish",
            "Insect", "Machine", "Plant", "Pyro", "Reptile", "Rock", "Sea Serpent", "Spellcaster", "Thunder", "Warrior",
            "Winged Beast", "Zombie"};
    public static final String[] PROPERTIES = {"Normal", "Continuous", "Equip", "Field", "Quick-Play", "Ritual", "Counter"};
    public static final String[] FRAMES = {"Normal Monster", "Effect Monster", "Fusion Monster", "Ritual Monster", "Spell Card",
            "Trap Card", "Token"};
    public static final String[] RARITIES = {"Common", "Short Print", "Super Short Print", "Rare", "Super Rare", "Ultra Rare",
            "Secret Rare"};
    private static final int[] FRAME_COLORS = {0xFFD8B860, 0xFFC86830, 0xFF9050B0, 0xFF5878C8, 0xFF209080, 0xFFB03878, 0xFF909090};

    private final AssetManager assets;
    private final CardPack pack;
    private volatile boolean engine;
    private JSONObject[] catalog;
    private JSONArray sets;

    public YgoArt(AssetManager assets) {
        this.assets = assets;
        this.pack = new CardPack(assets, "game/ygo_cards.bin", "YGOP", W, H, 20 << 20);
    }

    /** The engine has its pack (read once from the app's files: about 0.1 s) */
    public synchronized boolean engine() {
        if (!engine) {
            try (InputStream in = assets.open("game/ygo_edo.pak")) {
                ByteArrayOutputStream o = new ByteArrayOutputStream(8 << 20);
                byte[] buf = new byte[1 << 16];
                for (int n; (n = in.read(buf)) > 0; ) {
                    o.write(buf, 0, n);
                }
                engine = Ygo.setPak(o.toByteArray());
            } catch (Exception e) {
                engine = false;
            }
        }
        return engine;
    }

    /** The card (Ygo.catalog's entry; past the catalog: a card a duel brought in), null for none */
    public synchronized JSONObject card(int id) {
        if (catalog == null) {
            catalog = new JSONObject[1];
            if (engine()) {
                try {
                    JSONArray a = new JSONArray(Ygo.catalog());
                    catalog = new JSONObject[a.length() + 1];
                    for (int i = 0; i < a.length(); i++) {
                        catalog[a.getJSONObject(i).getInt("id")] = a.getJSONObject(i);
                    }
                } catch (Exception ignored) {
                }
            }
        }
        if (id > 0 && id < catalog.length) {
            return catalog[id];
        }
        if (id >= catalog.length && engine()) {
            try {
                String e = Ygo.extraCard(id);
                return e == null ? null : new JSONObject(e);
            } catch (Exception ignored) {
            }
        }
        return null;
    }

    public int maxId() {
        card(0);
        return catalog.length - 1;
    }

    public String name(int id) {
        JSONObject c = card(id);
        return c == null ? "" : c.optString("n");
    }

    public synchronized JSONArray sets() {
        if (sets == null) {
            try {
                sets = engine() ? new JSONArray(Ygo.sets()) : new JSONArray();
            } catch (Exception e) {
                sets = new JSONArray();
            }
        }
        return sets;
    }

    public String setName(int set) {
        JSONObject s = sets().optJSONObject(set);
        return s == null ? "" : s.optString("name");
    }

    public static boolean monster(JSONObject c) {
        return c != null && c.optInt("f") <= Ygo.FRAME_RITUAL;
    }

    public static boolean extraDeck(JSONObject c) {
        return c != null && c.optInt("f") == Ygo.FRAME_FUSION;
    }

    public static int frameColor(int frame) {
        return frame >= 0 && frame < FRAME_COLORS.length ? FRAME_COLORS[frame] : 0xFF808080;
    }

    /** The card's kind in words: "LIGHT Spellcaster / Effect, Level 7" or "Quick-Play Spell Card" */
    public static String kind(JSONObject c) {
        if (c == null) {
            return "";
        }
        int f = c.optInt("f");
        if (f == Ygo.FRAME_SPELL || f == Ygo.FRAME_TRAP) {
            int p = c.optInt("p");
            return (p > 0 && p < PROPERTIES.length ? PROPERTIES[p] + " " : "") + FRAMES[f];
        }
        String a = c.optInt("a") < ATTRIBUTES.length ? ATTRIBUTES[c.optInt("a")] : "";
        String r = c.optInt("r") < RACES.length ? RACES[c.optInt("r")] : "";
        return a + " " + r + " / " + (f < FRAMES.length ? FRAMES[f].replace(" Monster", "") : "") + ", Level " + c.optInt("lv");
    }

    public void draw(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.draw(c, id, x, y, w, h, scale, alpha);
    }

    /** The opponent's cards, turned to face them (as the game's table does) */
    public void drawTurned(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.drawTurned(c, id, x, y, w, h, scale, alpha);
    }

    public void drawArt(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        pack.drawArt(c, id, x, y, w, h, scale, alpha);
    }
}
