package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.List;

/**
 * One POKéMON card up close: the printed card as large as the screen allows, then its words from the game's catalog
 * (its attacks and what they do, Weakness, Resistance, Retreat Cost), the boosters it comes in, and how many you have.
 */
public final class TcgCardPage extends Page {
    private static final String[] RARITY = {"COMMON", "UNCOMMON", "RARE", "PROMO"};
    private final int id, count;
    private final Page back;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private int bottom;

    /** COUNT: copies owned (-1: don't say); BACK: the page to go back to (null: the tabs) */
    public TcgCardPage(GameView v, int id, int count, Page back) {
        super(v);
        this.id = id;
        this.count = count;
        this.back = back;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.height() + 10);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        TcgArt ta = v.app.tcgArt;
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF203868, 0xFF0A1428, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        float cw = Math.min(w - 40, (h - 60) / 1.375f), ch = cw * 1.375f;
        int y = 8 - scroll;
        ta.draw(c, id, (w - cw) / 2, y, cw, ch, v.pixelScale(), 255);
        y += (int) ch + 8;
        JSONObject k = ta.card(id);
        if (k != null) {
            String head = k.optString("n") + (k.optInt("k") == Tcg.POKEMON ? "  HP " + k.optInt("hp") : "");
            art.text(c, head, (w - art.width(head)) / 2, y, 0xFFF8E070, 0xFF000000);
            y += 16;
            String kind = k.optInt("k") == Tcg.POKEMON ? (k.optInt("st") == 0 ? "BASIC" : "STAGE " + k.optInt("st"))
                    + " " + Tcg.TYPE_NAMES[Math.min(6, k.optInt("t"))] + " POKéMON"
                    + (k.optString("ef").isEmpty() ? "" : " (from " + k.optString("ef") + ")")
                    : k.optInt("k") == Tcg.ENERGY ? "ENERGY" : "TRAINER";
            y = para(c, w, y, kind, 0xFFC8D0E0);
            JSONArray atk = k.optJSONArray("a");
            for (int i = 0; atk != null && i < atk.length(); i++) {
                JSONObject a = atk.optJSONObject(i);
                StringBuilder cost = new StringBuilder();
                JSONArray cc = a.optJSONArray("c");
                // the cost as orbs, then the name and damage
                int ox = 14;
                for (int t = 0; cc != null && t < 7; t++) {
                    for (int n = 0; n < cc.optInt(t); n++) {
                        ta.orb(c, t, ox, y + 7, 4.5f);
                        ox += 10;
                    }
                }
                String line = (a.optInt("cat") == 4 ? "POKéMON POWER: " : "") + a.optString("n")
                        + (a.optInt("dmg") > 0 ? "  " + a.optInt("dmg") + new String[]{"", "+", "-", "x", "", ""}[Math.min(5, a.optInt("cat"))] : "");
                art.text(c, line, ox + 4, y, GameArt.WHITE_TEXT, 0xFF000000);
                y += 14;
                if (!a.optString("txt").isEmpty()) {
                    y = para(c, w, y, a.optString("txt"), 0xFFC8D0E0);
                }
                cost.setLength(0);
            }
            if (k.optInt("k") == Tcg.POKEMON) {
                String wk = "WEAKNESS " + (k.optInt("w") < 7 ? Tcg.TYPE_NAMES[k.optInt("w")] : "NONE") + "   RESISTANCE "
                        + (k.optInt("r") < 7 ? Tcg.TYPE_NAMES[k.optInt("r")] : "NONE") + "   RETREAT " + k.optInt("rc");
                y = para(c, w, y + 2, wk, 0xFFB8C8E8);
            } else if (!k.optString("txt").isEmpty()) {
                y = para(c, w, y, k.optString("txt"), 0xFFC8D0E0);
            }
            StringBuilder packs = new StringBuilder();
            for (int p = 0; p < Tcg.PACKS; p++) {
                for (int x : ta.packCards(p)) {
                    if (x == id) {
                        packs.append(packs.length() == 0 ? "" : ", ").append(Tcg.PACK_NAMES[p]);
                        break;
                    }
                }
            }
            String rar = RARITY[Math.min(3, k.optInt("ra"))] + (k.optInt("h") == 1 ? " (HOLO)" : "");
            y = para(c, w, y + 4, rar + (packs.length() > 0 ? ". In " + packs + " boosters." : "."), 0xFFF8E070);
            if (count >= 0) {
                y = para(c, w, y, count == 0 ? "You don't have this card yet." : "You have " + count + ".", GameArt.WHITE_TEXT);
            }
        }
        bottom = y + scroll + 34;
        button(c, "BACK", w / 2 - 40, Math.min(h - 28, y + 6), 80, true);
    }

    private int para(Canvas c, int w, int y, String text, int color) {
        List<String> lines = wrap(text, w - 28);
        for (String l : lines) {
            art.text(c, l, 14, y, color, 0xFF000000);
            y += 13;
        }
        return y + 2;
    }

    @Override
    public void tap(int x, int y) {
        back();
    }

    @Override
    public void back() {
        v.showOver(back);
    }

    @Override
    public boolean swapsWords() {
        return false;   // (the card games' own words: the POKéMON card game is the POKéMON card game)
    }
}
