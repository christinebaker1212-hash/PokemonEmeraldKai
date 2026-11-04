package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Shader;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * A TRIPLE TRIAD pack opened (the game dealt the cards: ttopen): three cards face down, each turning over in turn
 * (tap to hurry), then their names and stars. A card new to the collection says NEW!
 */
public final class TtPackPage extends Page {
    private final int type;
    private final int[] cards;
    private final long start = System.currentTimeMillis();
    private final Paint fill = new Paint();

    public TtPackPage(GameView v, int type, int[] cards) {
        super(v);
        this.type = type;
        this.cards = cards;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    private int heard = -2;   // (the last card heard turning; -2: the pack not opened yet)
    private boolean fanfare;

    @Override
    public void draw(Canvas c, int w, int h) {
        if (heard == -2) {
            heard = -1;
            v.sound.play("se_rg_card_open");
        }
        if (!fanfare && System.currentTimeMillis() - start > 500 + cards.length * 700L + 400) {
            fanfare = true;
            v.sound.fanfare("mus_obtain_item");
        }
        TtArt ta = v.app.ttArt;
        long t = System.currentTimeMillis() - start;
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF203868, 0xFF0A1428, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        JSONArray names = ta.words == null ? null : ta.words.optJSONArray("packs");
        String title = (names == null ? "" : names.optString(type)) + " PACK";
        art.text(c, title, (w - art.width(title)) / 2, 12, 0xFFF8E070, 0xFF000000);
        float cw = Math.min((w - 32) / 3f, 84), ch = cw * TtArt.H / TtArt.W, y = h / 2f - ch / 2 - 20;
        for (int i = 0; i < cards.length; i++) {
            float x = (w - (cw * 3 + 16)) / 2f + i * (cw + 8);
            long at = 500 + i * 700L;
            float flip = 1;
            boolean front = t > at + 175;
            if (front && i > heard) {
                heard = i;   // (each card as it turns)
                v.sound.play("se_rg_card_flip");
            }
            if (t > at && t < at + 350) {
                flip = Math.max(0.04f, (float) Math.abs(Math.cos((t - at) / 350.0 * Math.PI)));
            }
            ta.draw(c, front ? cards[i] : 0, TtArt.SEAT_NONE, null, x, y, cw, ch, flip, 255);
            if (front && t > at + 350) {
                JSONObject k = ta.card(cards[i]);
                String n = k == null ? "?" : k.optString("n");
                int ty = (int) (y + ch + 6);
                for (String l : wrap(n, (int) cw + 6)) {
                    art.text(c, l, (int) (x + cw / 2 - art.width(l) / 2f), ty, GameArt.WHITE_TEXT, 0xFF000000);
                    ty += 13;
                }
                if (CardsPage.owned(tt(), cards[i]) <= 1) {
                    art.text(c, "NEW!", (int) (x + cw / 2 - art.width("NEW!") / 2f), (int) y - 14, 0xFFF8E070, 0xFF805010);
                }
            }
        }
        if (t > 500 + cards.length * 700L + 400) {
            button(c, "OK", w / 2 - 40, (int) (y + ch + 52), 80, true);
        } else {
            v.postInvalidateOnAnimation();
        }
    }

    private JSONObject tt() {
        com.moemon.companion.core.Model m = v.app.model();
        return m == null ? new JSONObject() : m.snap.optJSONObject("tt");
    }

    @Override
    public void tap(int x, int y) {
        if (System.currentTimeMillis() - start > 500 + cards.length * 700L + 400) {
            v.showOver(null);
        }
    }

    @Override
    public boolean swapsWords() {
        return false;   // (the card games' own words: the POKéMON card game is the POKéMON card game)
    }
}
