package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * One YU-GI-OH! card up close: the printed card, then its words from the game's catalog (its kind, ATK / DEF, its
 * text, today's wording), its limit under your Forbidden & Limited list, the sets it comes in, and how many you have.
 */
public final class YgoCardPage extends Page {
    private static final String[] LIMITS = {"FORBIDDEN", "LIMITED (1 copy)", "SEMI-LIMITED (2 copies)", ""};
    private final int id, count;
    private final Page back;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private int bottom;

    public YgoCardPage(GameView v, int id, int count, Page back) {
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
        YgoArt ya = v.app.ygoArt;
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF282058, 0xFF0C0A20, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        float cw = Math.min(w - 60, (h * 0.55f) / 1.46f), ch = cw * 1.46f;
        int y = 8 - scroll;
        ya.draw(c, id, (w - cw) / 2, y, cw, ch, v.pixelScale(), 255);
        y += (int) ch + 8;
        JSONObject k = ya.card(id);
        if (k != null) {
            y = para(c, w, y, k.optString("n"), 0xFFF8E070);
            y = para(c, w, y, YgoArt.kind(k), 0xFFC8D0E8);
            if (YgoArt.monster(k)) {
                y = para(c, w, y, "ATK " + (k.optInt("atk") < 0 ? "?" : k.optInt("atk")) + "   DEF "
                        + (k.optInt("def") < 0 ? "?" : k.optInt("def")), GameArt.WHITE_TEXT);
            }
            for (String p : k.optString("t").split("\n")) {
                y = para(c, w, y, p, 0xFFE0E4F0);
            }
            int lim = k.optInt("lim", 3);
            if (lim < 3) {
                y = para(c, w, y + 2, LIMITS[lim], 0xFFF88878);
            }
            StringBuilder in = new StringBuilder();
            JSONArray sets = ya.sets();
            for (int s = 0; s < sets.length(); s++) {
                JSONArray cards = sets.optJSONObject(s).optJSONArray("cards");
                for (int i = 0; cards != null && i < cards.length(); i++) {
                    if (cards.optJSONArray(i).optInt(0) == id) {
                        in.append(in.length() == 0 ? "" : ", ").append(sets.optJSONObject(s).optString("short"));
                        break;
                    }
                }
            }
            String rar = YgoArt.RARITIES[Math.min(6, k.optInt("ra"))];
            y = para(c, w, y + 4, rar + (in.length() > 0 ? ". In " + in + "." : "."), 0xFFF8E070);
            if (count >= 0) {
                y = para(c, w, y, count == 0 ? "You don't have this card yet." : "You have " + count + ".", GameArt.WHITE_TEXT);
            }
        }
        bottom = y + scroll + 34;
        button(c, "BACK", w / 2 - 40, Math.min(h - 28, y + 6), 80, true);
    }

    private int para(Canvas c, int w, int y, String text, int color) {
        for (String l : wrap(text, w - 28)) {
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
}
