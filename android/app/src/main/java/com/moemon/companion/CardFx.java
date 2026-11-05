package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import java.util.ArrayList;
import java.util.List;

/**
 * The card tables' motion (the user, 2026-11-05: animate the app the way the game's card tables move): cards flying
 * from a deck to a hand or onto the field, an attacker lunging, a hit shaking with a red flash, a Knock Out or a
 * destroyed card fading and sinking, glows and sparkles, condition rings, a flash over the whole table and the turn's
 * banner sliding across. A page asks where its places are (Geom), draws each place with slotDx / slotDy / slotAlpha /
 * slotTint applied, and calls drawOver on top. A card that flew in stays where it landed until the page's shown
 * state catches up (settle()).
 */
final class CardFx {
    interface Geom {
        /** Where a place is on the screen now (null: not drawn) */
        RectF slot(int who, int slot);
    }

    interface Painter {
        void card(Canvas c, int card, boolean faceDown, boolean turned, float x, float y, float w, float h, int alpha);
    }

    static final int FLY = 1, LUNGE = 2, SHAKE = 3, FADE = 4, GLOW = 5, BANNER = 6, FLASH = 7, SPARKLE = 8, RING = 9,
            POP = 10, SPIN = 11;

    static final class Anim {
        int kind, who, slot, card, color, dur;
        long start;
        boolean faceDown, turned, park;
        float x0, y0, w0, h0, x1, y1, w1, h1, dir;
        String text = "";
    }

    final List<Anim> list = new ArrayList<>();
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);

    static float clamp(float t) {
        return t < 0 ? 0 : t > 1 ? 1 : t;
    }

    static float easeOut(float t) {
        t = clamp(t);
        return 1 - (1 - t) * (1 - t) * (1 - t);
    }

    static float easeInOut(float t) {
        t = clamp(t);
        return t < 0.5f ? 4 * t * t * t : 1 - (float) Math.pow(-2 * t + 2, 3) / 2;
    }

    private Anim add(int kind, long start, int dur) {
        Anim a = new Anim();
        a.kind = kind;
        a.start = start;
        a.dur = dur;
        list.add(a);
        return a;
    }

    /** A card flying from one rectangle to another along a little arc; PARK: it stays where it landed until settle() */
    Anim fly(long start, int dur, int card, boolean faceDown, boolean turned, RectF from, RectF to, boolean park) {
        Anim a = add(FLY, start, dur);
        a.card = card;
        a.faceDown = faceDown;
        a.turned = turned;
        a.park = park;
        a.x0 = from.left;
        a.y0 = from.top;
        a.w0 = from.width();
        a.h0 = from.height();
        a.x1 = to.left;
        a.y1 = to.top;
        a.w1 = to.width();
        a.h1 = to.height();
        return a;
    }

    /** The place's card darting DIR (-1 up, +1 down) toward its target and back */
    void lunge(long start, int dur, int who, int slot, float dir) {
        Anim a = add(LUNGE, start, dur);
        a.who = who;
        a.slot = slot;
        a.dir = dir;
    }

    void shake(long start, int dur, int who, int slot, int color) {
        Anim a = add(SHAKE, start, dur);
        a.who = who;
        a.slot = slot;
        a.color = color;
    }

    void fade(long start, int dur, int who, int slot) {
        Anim a = add(FADE, start, dur);
        a.who = who;
        a.slot = slot;
        a.park = true;   // (gone for good: until the shown state catches up)
    }

    void glow(long start, int dur, int who, int slot, int color) {
        Anim a = add(GLOW, start, dur);
        a.who = who;
        a.slot = slot;
        a.color = color;
    }

    void pop(long start, int dur, int who, int slot) {
        Anim a = add(POP, start, dur);
        a.who = who;
        a.slot = slot;
    }

    void sparkle(long start, int dur, int who, int slot, int color) {
        Anim a = add(SPARKLE, start, dur);
        a.who = who;
        a.slot = slot;
        a.color = color;
    }

    void ring(long start, int dur, int who, int slot, int color) {
        Anim a = add(RING, start, dur);
        a.who = who;
        a.slot = slot;
        a.color = color;
    }

    void banner(long start, int dur, String text, int color) {
        Anim a = add(BANNER, start, dur);
        a.text = text;
        a.color = color;
    }

    void flash(long start, int dur, int color) {
        Anim a = add(FLASH, start, dur);
        a.color = color;
    }

    /** The shown state caught up: what was parked is now drawn by the page itself */
    void settle() {
        for (int i = list.size() - 1; i >= 0; i--) {
            if (list.get(i).park) {
                list.remove(i);
            }
        }
    }

    void clear() {
        list.clear();
    }

    /** Anything still moving (a parked card isn't) */
    boolean moving(long now) {
        for (Anim a : list) {
            if (now < a.start + a.dur) {
                return true;
            }
        }
        return false;
    }

    private float t(Anim a, long now) {
        return clamp((now - a.start) / (float) Math.max(1, a.dur));
    }

    private boolean on(Anim a, long now) {
        return now >= a.start && (now < a.start + a.dur || a.park);
    }

    // ------------------------------------------------------------------------- what the page applies to its places

    float slotDx(int who, int slot, long now, float w) {
        float dx = 0;
        for (Anim a : list) {
            if (a.kind == SHAKE && a.who == who && a.slot == slot && now >= a.start && now < a.start + a.dur) {
                float t = t(a, now);
                dx += (float) Math.sin(t * 38) * Math.max(2, w * 0.07f) * (1 - t);
            }
        }
        return dx;
    }

    float slotDy(int who, int slot, long now, float h) {
        float dy = 0;
        for (Anim a : list) {
            if (a.who != who || a.slot != slot || now < a.start) {
                continue;
            }
            float t = t(a, now);
            if (a.kind == LUNGE && now < a.start + a.dur) {
                float p = t < 0.35f ? easeOut(t / 0.35f) : 1 - easeInOut((t - 0.35f) / 0.65f);
                dy += a.dir * h * 0.32f * p;
            } else if (a.kind == FADE && on(a, now)) {
                dy += h * 0.25f * easeInOut(t);
            }
        }
        return dy;
    }

    float slotScale(int who, int slot, long now) {
        float s = 1;
        for (Anim a : list) {
            if (a.kind == POP && a.who == who && a.slot == slot && now >= a.start && now < a.start + a.dur) {
                float t = t(a, now);
                s *= 1 + 0.18f * (float) Math.sin(Math.PI * t) * (1 - t * 0.3f);
            }
        }
        return s;
    }

    int slotAlpha(int who, int slot, long now) {
        float al = 1;
        for (Anim a : list) {
            if (a.kind == FADE && a.who == who && a.slot == slot && on(a, now)) {
                al *= 1 - easeInOut(t(a, now));
            }
        }
        return (int) (255 * clamp(al));
    }

    /** A color over the place's card (0: none) */
    int slotTint(int who, int slot, long now) {
        int out = 0;
        for (Anim a : list) {
            if (a.who != who || a.slot != slot || now < a.start || now >= a.start + a.dur) {
                continue;
            }
            float t = t(a, now);
            int alpha = 0;
            if (a.kind == SHAKE) {
                alpha = (int) (150 * (1 - t));
            } else if (a.kind == GLOW) {
                alpha = (int) (190 * Math.sin(Math.PI * t));
            }
            if (alpha > (out >>> 24)) {
                out = alpha << 24 | (a.color & 0xFFFFFF);
            }
        }
        return out;
    }

    // --------------------------------------------------------------------------------------------- over the table

    void drawOver(Canvas c, int w, int h, long now, Geom g, Painter p, GameArt art) {
        for (Anim a : list) {
            if (!on(a, now)) {
                continue;
            }
            float t = t(a, now);
            switch (a.kind) {
                case FLY: {
                    float e = easeInOut(t);
                    float x = a.x0 + (a.x1 - a.x0) * e, y = a.y0 + (a.y1 - a.y0) * e;
                    float cw = a.w0 + (a.w1 - a.w0) * e, ch = a.h0 + (a.h1 - a.h0) * e;
                    float arc = (float) Math.sin(Math.PI * e) * Math.min(40, Math.abs(a.y1 - a.y0) * 0.25f + 10);
                    float lift = t < 1 ? 1.06f : 1;
                    c.save();
                    c.translate(x + cw / 2, y + ch / 2 - arc);
                    c.rotate((float) Math.sin(Math.PI * e) * (a.x1 >= a.x0 ? 8 : -8));
                    c.scale(lift, lift);
                    if (t < 1) {
                        fill.setColor(0x50000000);   // (its shadow on the table)
                        c.drawRect(-cw / 2 + 3, -ch / 2 + 4 + arc * 0.5f, cw / 2 + 3, ch / 2 + 4 + arc * 0.5f, fill);
                    }
                    p.card(c, a.card, a.faceDown, a.turned, -cw / 2, -ch / 2, cw, ch, 255);
                    c.restore();
                    break;
                }
                case SPARKLE: {
                    RectF r = g.slot(a.who, a.slot);
                    if (r == null) {
                        break;
                    }
                    int n = 14;
                    for (int k = 0; k < n; k++) {
                        double ang = k * 2 * Math.PI / n + t * 2.2;
                        float rad = (0.35f + 0.5f * easeOut(t)) * Math.max(r.width(), r.height()) * 0.6f;
                        float sx = (float) (r.centerX() + Math.cos(ang) * rad), sy = (float) (r.centerY() + Math.sin(ang) * rad * 0.8f);
                        float sz = 2.5f * (1 - t) + 0.5f;
                        fill.setColor((int) (255 * (1 - t)) << 24 | (a.color & 0xFFFFFF));
                        c.drawRect(sx - sz, sy - sz, sx + sz, sy + sz, fill);
                        fill.setColor((int) (200 * (1 - t)) << 24 | 0xFFFFFF);
                        c.drawRect(sx - sz / 3, sy - sz / 3, sx + sz / 3, sy + sz / 3, fill);
                    }
                    break;
                }
                case RING: {
                    RectF r = g.slot(a.who, a.slot);
                    if (r == null) {
                        break;
                    }
                    float grow = easeOut(t);
                    fill.setStyle(Paint.Style.STROKE);
                    fill.setStrokeWidth(3 * (1 - t) + 1);
                    fill.setColor((int) (230 * (1 - t)) << 24 | (a.color & 0xFFFFFF));
                    float rr = Math.max(r.width(), r.height()) * (0.35f + 0.45f * grow);
                    c.drawOval(new RectF(r.centerX() - rr, r.centerY() - rr * 0.6f, r.centerX() + rr, r.centerY() + rr * 0.6f), fill);
                    fill.setStyle(Paint.Style.FILL);
                    break;
                }
                case FLASH:
                    fill.setColor((int) (90 * (1 - t) * (1 - t)) << 24 | (a.color & 0xFFFFFF));   // (a quick wash, not a whiteout)
                    c.drawRect(0, 0, w, h, fill);
                    break;
                case BANNER: {
                    // in from the left, a pause, out to the right
                    float in = easeOut(t / 0.18f), out = easeInOut((t - 0.75f) / 0.25f);
                    float off = t < 0.75f ? (1 - in) * -w : out * w;
                    float by = h * 0.5f - 18;
                    fill.setColor(0xD0000000 | (a.color & 0xFFFFFF));
                    c.drawRect(off, by, off + w, by + 36, fill);
                    fill.setColor(0x60FFFFFF);
                    c.drawRect(off, by, off + w, by + 2, fill);
                    c.drawRect(off, by + 34, off + w, by + 36, fill);
                    c.save();
                    float tw = art.width(a.text);
                    c.translate(off + w / 2f, by + 18);
                    c.scale(2, 2);
                    art.text(c, a.text, (int) (-tw / 2), -8, GameArt.WHITE_TEXT, 0xFF000000);
                    c.restore();
                    break;
                }
                default:
                    break;
            }
        }
        // (done ones go; parked ones wait for settle())
        for (int i = list.size() - 1; i >= 0; i--) {
            Anim a = list.get(i);
            if (!a.park && now >= a.start + a.dur) {
                list.remove(i);
            }
        }
    }

    /** A coin tossed: up and down with a spin, landing on HEADS (or not) */
    void drawCoin(Canvas c, float cx, float cy, long age, boolean heads, GameArt art) {
        float t = clamp(age / 900f);
        float up = (float) Math.sin(Math.PI * Math.min(1, t / 0.7f)) * 46;
        float spin = t < 0.7f ? (float) Math.cos(age / 45.0) : 1;
        boolean face = spin >= 0 ? heads : !heads;
        float r = 16, sx = Math.max(0.08f, Math.abs(spin));
        float y = cy - up;
        fill.setColor(0x40000000);
        float sh = 1 - up / 60f;
        c.drawOval(new RectF(cx - r * sh, cy + r - 3, cx + r * sh, cy + r + 3), fill);
        RectF o = new RectF(cx - r * sx, y - r, cx + r * sx, y + r);
        fill.setColor(0xFF705008);
        c.drawOval(o, fill);
        o.inset(2 * sx, 2);
        fill.setColor(face ? 0xFFF8D040 : 0xFFD0D4E0);
        c.drawOval(o, fill);
        if (sx > 0.5f) {
            fill.setColor(face ? 0xFFFFF0A0 : 0xFFF8F8FF);
            c.drawOval(new RectF(cx - r * sx * 0.6f, y - r * 0.75f, cx - r * sx * 0.1f, y - r * 0.3f), fill);
            String s = face ? "H" : "T";
            art.text(c, s, (int) (cx - art.width(s) / 2f), (int) y - 7, face ? 0xFF684800 : 0xFF484858, 0);
        }
    }
}
