package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Walk;

import org.json.JSONObject;

import java.time.LocalDate;
import java.util.Random;

/**
 * MEET: a short catch game with a wild POKéMON of the place the walk began (docs/COMPANION_APP.md "Encounter"). A ring
 * closes in on it and opens again; THROW when it's small. Three POKé BALLS; how well it was timed and the species'
 * own catch rate (the game's) decide. Caught, it goes to the PC on the way home (the game checks it came from that
 * place's wild table, at its levels, 3 a day).
 */
public final class CatchPage extends Page {
    private final Walk walk;
    private final int species, level;
    private final Paint fill = new Paint(), ring = new Paint();
    private final Random random = new Random();
    private int balls = 3, phase;           // phase: 0 aiming, 1 the ball in the air, 2 shaking, 3 caught, 4 ran away
    private long phaseAt;
    private float quality;
    private boolean ok;

    public CatchPage(GameView v, Walk walk, int species, int level) {
        super(v);
        this.walk = walk;
        this.species = species;
        this.level = level;
        ring.setStyle(Paint.Style.STROKE);
        ring.setStrokeWidth(3);
        phaseAt = System.currentTimeMillis();
    }

    @Override
    public boolean fullScreen() {
        return false;
    }

    private float ringSize(long t) {
        double k = (t % 1400) / 1400.0;
        return (float) (0.2 + 0.8 * Math.abs(Math.cos(k * Math.PI)));
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        long t = System.currentTimeMillis();
        // the meadow
        fill.setColor(0xFF98D8F8);
        c.drawRect(0, 20, w, h, fill);
        fill.setColor(0xFF78C058);
        c.drawRect(0, 150, w, h, fill);
        fill.setColor(0xFF58A040);
        c.drawOval(w / 2f - 70, 130, w / 2f + 70, 170, fill);
        int cx = w / 2, cy = 110;
        if (phase <= 1 || phase == 4) {
            if (phase != 4 || (t - phaseAt) < 600) {
                art.front(c, species, null, cx - 48, cy - 70, 96, false);
            }
        }
        if (phase == 0) {
            float r = ringSize(t);
            ring.setColor(r < 0.35f ? 0xFF30C030 : r < 0.6f ? 0xFFF8C020 : 0xFFF84030);
            c.drawCircle(cx, cy - 22, 10 + 40 * r, ring);
        }
        // the ball: thrown, then shaking, then the verdict
        if (phase == 1) {
            float k = Math.min(1f, (t - phaseAt) / 500f);
            ball(c, (int) (cx), (int) (h - 60 - (h - 60 - (cy - 22)) * k - Math.sin(k * Math.PI) * 60));
            if (k >= 1f) {
                phase = 2;
                phaseAt = t;
            }
        } else if (phase == 2) {
            int shake = (int) ((t - phaseAt) / 450);
            int dx = (int) (Math.sin((t - phaseAt) / 70.0) * 3 * ((t - phaseAt) % 450 < 250 ? 1 : 0));
            ball(c, cx + dx, cy - 6);
            int needed = ok ? 3 : 1 + random.nextInt(2);
            if (shake >= needed) {
                phase = ok ? 3 : (balls > 0 ? 0 : 4);
                phaseAt = t;
                if (phase == 3 || phase == 4) {
                    walk.met(species, level, phase == 3, 4, LocalDate.now());
                    v.app.saveWalk();
                    v.buzz(true);
                }
            }
        } else if (phase == 3) {
            ball(c, cx, cy - 6);
        }
        // the words
        int wy = h - 64;
        art.window(c, 0, 12, wy, w - 24, 44);
        String name = art.plain(art.name("species", species));
        String line = phase == 3 ? "Gotcha! " + name + " will be in the PC when your walker comes home."
                : phase == 4 ? "Oh no! " + name + " ran away!"
                : phase == 0 && balls == 3 ? "A wild " + name + " (Lv" + level + ") appeared! Tap when the ring is small."
                : phase == 0 ? "Aww! It broke free! Tap to throw again." : "...";
        int ly = wy;
        for (String l : wrap(line, w - 32)) {
            art.text(c, l, 16, ly);
            ly += 14;
        }
        for (int i = 0; i < 3; i++) {
            fill.setColor(i < balls ? 0xFFF83838 : 0xFF888888);
            c.drawCircle(w - 30 - i * 14, wy - 12, 5, fill);
        }
        banner(c, w, "MEET", phase >= 3 ? "BACK" : "");
        v.postInvalidateOnAnimation();
    }

    private void ball(Canvas c, int x, int y) {
        fill.setColor(0xFF202020);
        c.drawCircle(x, y, 9, fill);
        fill.setColor(0xFFF83838);
        c.drawRect(x - 8, y - 8, x + 8, y, fill);
        fill.setColor(0xFFF8F8F8);
        c.drawRect(x - 8, y + 1, x + 8, y + 8, fill);
        fill.setColor(0xFF202020);
        c.drawRect(x - 9, y - 1, x + 9, y + 1, fill);
        fill.setColor(0xFFF8F8F8);
        c.drawCircle(x, y, 3, fill);
    }

    @Override
    public void tap(int x, int y) {
        long t = System.currentTimeMillis();
        if (phase >= 3) {
            back();
            return;
        }
        if (phase != 0 || balls <= 0) {
            return;
        }
        // how well it was timed (the ring's size), and the species' catch rate (the game's)
        float r = ringSize(t);
        quality = 1f - Math.min(1f, (r - 0.2f) / 0.8f);
        JSONObject sp = art.species(species);
        int rate = sp == null ? 45 : Math.max(3, sp.optInt("cr", 45));
        double p = Math.min(0.95, Math.max(0.05, rate / 255.0 * (0.35 + 0.65 * quality) + 0.15 * quality));
        ok = random.nextDouble() < p;
        balls--;
        phase = 1;
        phaseAt = t;
        v.buzz(false);
    }

    @Override
    public void back() {
        if (phase == 1 || phase == 2) {
            return;   // (the ball is out)
        }
        if (phase == 0 && balls < 3) {
            walk.met(species, level, false, 4, LocalDate.now());   // (left mid-meeting: it ran away)
            v.app.saveWalk();
        }
        v.showOver(null);
    }
}
