package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;

import org.json.JSONObject;

/**
 * A POKéMON booster opened (the game dealt it: tcgopen): ten cards face down in two rows, turning over one after
 * another with the rare last (a holo shines), then NEW! on the first copy of a card. Tap a card to see it up close.
 */
public final class TcgPackPage extends Page {
    private final int type, fresh;
    private final int[] cards;
    private final long start = System.currentTimeMillis();
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final float[] cx = new float[10], cy = new float[10];
    private float cw, ch;
    private static final int STEP = 260, FIRST = 600;

    public TcgPackPage(GameView v, int type, int[] cards, int fresh) {
        super(v);
        this.type = type;
        this.cards = cards;
        this.fresh = fresh;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    private boolean done() {
        return System.currentTimeMillis() - start > FIRST + cards.length * (long) STEP + 600;
    }

    private int heard = -2;   // (the last card heard turning; -2: the pack not opened yet)
    private boolean fanfare;

    @Override
    public void draw(Canvas c, int w, int h) {
        if (heard == -2) {
            heard = -1;
            v.sound.play("se_rg_card_open");
        }
        if (!fanfare && done()) {
            fanfare = true;
            v.sound.fanfare("mus_obtain_item");
        }
        TcgArt ta = v.app.tcgArt;
        long t = System.currentTimeMillis() - start;
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF203868, 0xFF0A1428, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        String title = (type >= 0 && type < Tcg.PACKS ? Tcg.PACK_NAMES[type] : "") + " BOOSTER";
        art.text(c, title, (w - art.width(title)) / 2, 12, 0xFFF8E070, 0xFF000000);
        cw = Math.min((w - 12 - 4 * 4) / 5f, 70);
        ch = cw * 1.375f;
        float top = Math.max(34, h / 2f - ch - 30);
        for (int i = 0; i < cards.length; i++) {
            float x = (w - (cw * 5 + 16)) / 2f + (i % 5) * (cw + 4), y = top + (i / 5) * (ch + 28);
            cx[i] = x;
            cy[i] = y;
            long at = FIRST + i * (long) STEP;
            float flip = 1;
            boolean front = t > at + 120;
            if (front && i > heard) {
                heard = i;   // (as the game's pack screen: each card as it turns, a rare one shining)
                v.sound.play(ta.card(cards[i]) != null && ta.card(cards[i]).optInt("ra") >= 2 ? "se_shiny" : "se_rg_card_flip");
            }
            if (t > at && t < at + 240) {
                flip = Math.max(0.04f, (float) Math.abs(Math.cos((t - at) / 240.0 * Math.PI)));
            }
            float fw = cw * flip;
            ta.draw(c, front ? cards[i] : 0, x + (cw - fw) / 2, y, fw, ch, v.pixelScale(), 255);
            JSONObject k = ta.card(cards[i]);
            if (front && t > at + 240 && k != null && k.optInt("h") == 1) {
                // a holo: a band of light passes over it
                float p = ((t - at - 240) % 1400) / 1400f;
                fill.setShader(new LinearGradient(x + cw * (p * 2 - 1), y, x + cw * (p * 2 - 0.4f), y + ch,
                        new int[]{0x00FFFFFF, 0x70FFFFFF, 0x00FFFFFF}, null, Shader.TileMode.CLAMP));
                c.drawRect(x, y, x + cw, y + ch, fill);
                fill.setShader(null);
            }
            if (front && t > at + 240 && (fresh >> i & 1) != 0) {
                fill.setColor(0xFFE83838);
                c.drawRoundRect(new RectF(x + cw / 2 - 14, y - 7, x + cw / 2 + 14, y + 5), 3, 3, fill);
                art.text(c, "NEW!", (int) (x + cw / 2 - art.width("NEW!") / 2f), (int) y - 9, GameArt.WHITE_TEXT, 0xFF801010);
            }
            if (front && t > at + 240 && k != null) {
                String n = k.optString("n");
                while (art.width(n) > cw + 2 && n.length() > 3) {
                    n = n.substring(0, n.length() - 1);
                }
                art.text(c, n, (int) (x + cw / 2 - art.width(n) / 2f), (int) (y + ch + 1), GameArt.WHITE_TEXT, 0xFF000000);
            }
        }
        if (done()) {
            button(c, "OK", w / 2 - 40, (int) (top + 2 * (ch + 28) + 8), 80, true);
        }
        v.postInvalidateOnAnimation();
    }

    @Override
    public void tap(int x, int y) {
        if (!done()) {
            return;
        }
        for (int i = 0; i < cards.length; i++) {
            if (in(x, y, (int) cx[i], (int) cy[i], (int) cw, (int) ch)) {
                v.showOver(new TcgCardPage(v, cards[i], -1, this));
                return;
            }
        }
        v.showOver(null);
    }
}
