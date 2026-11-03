package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;

import org.json.JSONObject;

/**
 * A YU-GI-OH! booster opened (the game dealt it: ygoopen): nine cards face down, turning over one after another with
 * the rare last (a foil shines), NEW! on the first copy of a card. Tap a card to see it up close.
 */
public final class YgoPackPage extends Page {
    private final int set, fresh;
    private final int[] cards;
    private final long start = System.currentTimeMillis();
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final float[] cx = new float[12], cy = new float[12];
    private float cw, ch;
    private static final int STEP = 280, FIRST = 600;

    public YgoPackPage(GameView v, int set, int[] cards, int fresh) {
        super(v);
        this.set = set;
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

    @Override
    public void draw(Canvas c, int w, int h) {
        YgoArt ya = v.app.ygoArt;
        long t = System.currentTimeMillis() - start;
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF302868, 0xFF0C0A20, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        JSONObject sj = ya.sets().optJSONObject(set);
        String title = sj == null ? "BOOSTER" : sj.optString("short");
        for (String l : wrap(title, w - 16)) {
            art.text(c, l, (w - art.width(l)) / 2, 10, 0xFFF8E070, 0xFF000000);
        }
        cw = Math.min((w - 12 - 2 * 6) / 3f, 76);
        ch = cw * 1.46f;
        float top = Math.max(30, h / 2f - ch * 1.5f - 30);
        for (int i = 0; i < cards.length; i++) {
            float x = (w - (cw * 3 + 12)) / 2f + (i % 3) * (cw + 6), y = top + (i / 3) * (ch + 18);
            cx[i] = x;
            cy[i] = y;
            long at = FIRST + i * (long) STEP;
            float flip = 1;
            boolean front = t > at + 130;
            if (t > at && t < at + 260) {
                flip = Math.max(0.04f, (float) Math.abs(Math.cos((t - at) / 260.0 * Math.PI)));
            }
            float fw = cw * flip;
            ya.draw(c, front ? cards[i] : 0, x + (cw - fw) / 2, y, fw, ch, v.pixelScale(), 255);
            JSONObject k = ya.card(cards[i]);
            if (front && t > at + 260 && k != null && k.optInt("ra") >= 3 && i == cards.length - 1) {
                float p = ((t - at - 260) % 1400) / 1400f;
                fill.setShader(new LinearGradient(x + cw * (p * 2 - 1), y, x + cw * (p * 2 - 0.4f), y + ch,
                        new int[]{0x00FFFFFF, 0x70FFFFFF, 0x00FFFFFF}, null, Shader.TileMode.CLAMP));
                c.drawRect(x, y, x + cw, y + ch, fill);
                fill.setShader(null);
            }
            if (front && t > at + 260 && (fresh >> i & 1) != 0) {
                fill.setColor(0xFFE83838);
                c.drawRoundRect(new RectF(x + cw / 2 - 14, y - 7, x + cw / 2 + 14, y + 5), 3, 3, fill);
                art.text(c, "NEW!", (int) (x + cw / 2 - art.width("NEW!") / 2f), (int) y - 9, GameArt.WHITE_TEXT, 0xFF801010);
            }
            if (front && t > at + 260 && k != null) {
                String n = k.optString("n");
                while (art.width(n) > cw + 4 && n.length() > 3) {
                    n = n.substring(0, n.length() - 1);
                }
                art.text(c, n, (int) (x + cw / 2 - art.width(n) / 2f), (int) (y + ch + 1), GameArt.WHITE_TEXT, 0xFF000000);
            }
        }
        if (done()) {
            button(c, "OK", w / 2 - 40, (int) (top + 3 * (ch + 18) + 4), 80, true);
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
                v.showOver(new YgoCardPage(v, cards[i], -1, this));
                return;
            }
        }
        v.showOver(null);
    }
}
