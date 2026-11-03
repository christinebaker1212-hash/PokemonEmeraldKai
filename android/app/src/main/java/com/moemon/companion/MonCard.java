package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONArray;
import org.json.JSONObject;

/** A POKeMON's details in a window, the way the PC's left panel and the summary show them */
public final class MonCard {
    private static final Paint sLine = new Paint();

    private MonCard() {}

    public static final int HEIGHT = 136;

    public static void draw(Canvas c, GameArt art, JSONObject mon, int x, int y, int w) {
        art.window(c, 0, x, y, w, HEIGHT);
        if (mon == null) {
            art.text(c, "Nothing here.", x + 6, y + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return;
        }
        int species = mon.optInt("s");
        boolean egg = mon.optInt("e") == 1;
        if (egg) {
            art.icon(c, eggSpecies(art), x + 18, y + 18, GameArt.ICON_PLAIN);   // (the EGG has no front picture)
        } else {
            art.front(c, species, mon.optString("pal", null), x + 2, y + 2, false);
        }
        int tx = x + 70;
        art.textHex(c, mon.optString("n"), tx, y, GameArt.TEXT, GameArt.SHADOW);
        if (egg) {
            art.text(c, "An EGG. It will hatch", tx, y + 18);
            art.text(c, "after some walking.", tx, y + 34);
            return;
        }
        int lw = art.text(c, "Lv" + mon.optInt("l"), tx, y + 16);
        int g = mon.optInt("g");
        if (g == 0) {
            art.text(c, "♂", tx + lw + 4, y + 16, GameArt.BLUE_TEXT, GameArt.SHADOW);
        } else if (g == 254) {
            art.text(c, "♀", tx + lw + 4, y + 16, GameArt.RED_TEXT, GameArt.SHADOW);
        }
        if (mon.optInt("sh") == 1) {
            art.text(c, "SHINY", tx + lw + 18, y + 16, 0xFFD89010, GameArt.SHADOW);
        }
        if (mon.optInt("off") == 1) {
            String o = "ON OFFER";
            art.text(c, o, tx + lw + 18 + (mon.optInt("sh") == 1 ? art.width("SHINY") + 6 : 0), y + 16, 0xFFE07820, GameArt.SHADOW);
        }
        art.text(c, art.speciesName(species), tx, y + 32, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (mon.has("hp")) {
            hp(c, art, tx, y + 50, mon.optInt("hp"), mon.optInt("mhp"));
        }
        // nature, ability, item, then the moves under the picture
        int ly = y + 66;
        art.text(c, art.name("natures", mon.optInt("nat")), x + 4, ly, GameArt.TEXT, GameArt.SHADOW);
        art.text(c, art.name("abilities", mon.optInt("ab")), x + 4, ly + 16, GameArt.TEXT, GameArt.SHADOW);
        int item = mon.optInt("i");
        if (item != 0) {
            art.itemIcon(c, item, x + 2, ly + 32);
            art.text(c, art.name("items", item), x + 28, ly + 36, GameArt.TEXT, GameArt.SHADOW);
        } else {
            art.text(c, "No item", x + 4, ly + 36, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        JSONArray moves = mon.optJSONArray("m");
        int mx = x + w / 2 + 6;
        for (int i = 0; moves != null && i < 4; i++) {
            int mv = moves.optInt(i);
            if (mv != 0 && w >= 200) {
                art.text(c, art.name("moves", mv), mx, ly + i * 16, GameArt.TEXT, GameArt.SHADOW);
            }
        }
    }

    public static int eggSpecies(GameArt art) {
        return art.data.optInt("egg");   // (SPECIES_EGG: its picture is the EGG's)
    }

    /** The game's HP bar: green, yellow under half, red under a fifth */
    public static void hp(Canvas c, GameArt art, int x, int y, int hp, int max) {
        int bw = 48;
        sLine.setColor(0xFF404040);
        c.drawRect(x, y, x + bw + 2, y + 5, sLine);
        sLine.setColor(0xFF506858);
        c.drawRect(x + 1, y + 1, x + bw + 1, y + 4, sLine);
        int fill = max > 0 ? Math.max(hp > 0 ? 1 : 0, hp * bw / max) : 0;
        sLine.setColor(hp * 5 <= max ? 0xFFF85838 : hp * 2 <= max ? 0xFFF8E038 : 0xFF70F8A8);
        c.drawRect(x + 1, y + 1, x + 1 + fill, y + 4, sLine);
        art.text(c, hp + "/" + max, x + bw + 6, y - 6, GameArt.TEXT, GameArt.SHADOW);
    }
}
