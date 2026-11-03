package com.moemon.companion;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The DEVICE page (docs/COMPANION_APP.md "Pillar 1: the second screen"): the devices the save has. The POKéTCH is the
 * game's own, live: its LCD comes from the game as it draws it (sinnoh/poketch.c), the finger is its stylus and the
 * side button changes the app, so every app works here exactly as in the game.
 */
public final class DevicePage extends Page {
    // the LCD's four shades, lightest first, per COLOR CHANGER color (poketch.c sColors)
    private static final int[][][] COLORS = {
            {{115, 180, 115}, {82, 131, 82}, {57, 82, 49}, {16, 41, 24}},
            {{214, 198, 115}, {164, 148, 74}, {115, 98, 41}, {49, 41, 16}},
            {{238, 164, 98}, {197, 115, 57}, {139, 74, 33}, {57, 24, 8}},
            {{238, 139, 139}, {197, 90, 98}, {139, 49, 57}, {57, 16, 24}},
            {{205, 148, 222}, {156, 98, 180}, {106, 57, 131}, {41, 16, 57}},
            {{139, 172, 238}, {90, 123, 197}, {57, 82, 148}, {16, 24, 65}},
            {{123, 205, 205}, {74, 156, 164}, {41, 106, 115}, {8, 41, 49}},
            {{197, 197, 197}, {139, 139, 139}, {90, 90, 90}, {24, 24, 24}},
    };
    private static final int PW = 192, PH = 160;
    private final Paint fill = new Paint(), pix = new Paint();
    private final Rect src = new Rect(0, 0, PW, PH), dst = new Rect();
    private Bitmap lcd;
    private JSONObject shown;          // the last frame drawn into lcd
    private boolean watching, touching;
    private long lastMove;

    public DevicePage(GameView v) {
        super(v);
        pix.setFilterBitmap(false);
    }

    private boolean has(int device) {
        Model m = v.app.model();
        JSONArray d = m == null ? null : m.save().optJSONArray("devices");
        return d != null && d.optInt(device) == 1;
    }

    private boolean linked() {
        return v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
    }

    private void watch(boolean on) {
        if (v.app.client == null) {
            return;
        }
        try {
            v.app.client.send(new JSONObject().put("t", "watch").put("on", on ? 1 : 0));
        } catch (org.json.JSONException ignored) {
        }
        watching = on;
        v.app.watching = on;
    }

    @Override
    public void shown() {
        if (has(0)) {
            watch(true);
        }
    }

    @Override
    public void hidden() {
        if (watching) {
            watch(false);
        }
    }

    // the LCD's place on the page: as wide as fits, a whole multiple of its pixels
    private int lcdScale(int w) {
        return Math.max(1, (w - 40) / PW);
    }

    private int lcdX(int w) {
        return (w - PW * lcdScale(w) - 16) / 2;
    }

    private static final int LCD_Y = 40;

    @Override
    public void draw(Canvas c, int w, int h) {
        banner(c, w, "POKéTCH", linked() ? "LIVE" : "");
        if (!has(0)) {
            art.window(c, 0, 12, LCD_Y, w - 24, 64);
            int y = LCD_Y;
            for (String l : wrap(v.app.model() == null ? "Nothing from the game yet."
                    : "The POKéTCH comes from the POKéTCH Co. in JUBILIFE CITY, SINNOH. Its apps show here once you have it.",
                    w - 32)) {
                art.text(c, l, 16, y);
                y += 16;
            }
            return;
        }
        if (!watching && linked()) {
            watch(true);
        }
        int s = lcdScale(w), lx = lcdX(w), lw = PW * s, lh = PH * s;
        // the case: Platinum's red, its side button on the right
        fill.setColor(0xFF701820);
        c.drawRect(lx - 10, LCD_Y - 10, lx + lw + 34, LCD_Y + lh + 10, fill);
        fill.setColor(0xFFE85058);
        c.drawRect(lx - 8, LCD_Y - 8, lx + lw + 32, LCD_Y + lh + 8, fill);
        fill.setColor(0xFFF88890);
        c.drawRect(lx - 8, LCD_Y - 8, lx + lw + 32, LCD_Y - 6, fill);
        int bx = lx + lw + 8;
        fill.setColor(0xFF802028);
        c.drawRect(bx, LCD_Y + lh / 5, bx + 18, LCD_Y + lh * 4 / 5, fill);
        fill.setColor(0xFFF8A0A8);
        c.drawRect(bx + 2, LCD_Y + lh / 5 + 2, bx + 16, LCD_Y + lh * 4 / 5 - 2, fill);
        // the LCD
        JSONObject f = v.app.watchFrame;
        if (f != null && f != shown) {
            decode(f);
            shown = f;
        }
        if (lcd != null && linked()) {
            dst.set(lx, LCD_Y, lx + lw, LCD_Y + lh);
            c.drawBitmap(lcd, src, dst, pix);
        } else {
            fill.setColor(0xFF395231);
            c.drawRect(lx, LCD_Y, lx + lw, LCD_Y + lh, fill);
            art.text(c, linked() ? "..." : "Link with the game", lx + 8, LCD_Y + 8, 0xFF73B473, 0);
        }
        int ty = LCD_Y + lh + 18;
        for (String l : wrap("Touch the screen to use the app. The button on the side changes the app.", w - 24)) {
            art.text(c, l, 12, ty, GameArt.WHITE_TEXT, 0xFF305060);
            ty += 16;
        }
    }

    private void decode(JSONObject f) {
        String rle = f.optString("rle");
        int color = Math.max(0, Math.min(7, f.optInt("color"))), flags = f.optInt("flags");
        int[] shade = new int[4];
        for (int i = 0; i < 4; i++) {
            int r = COLORS[color][i][0], g = COLORS[color][i][1], b = COLORS[color][i][2];
            if ((flags & 1) != 0) {
                r += (255 - r) / 4;
                g += (255 - g) / 4;
                b += (255 - b) / 4;
            }
            if ((flags & 2) != 0) {
                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
            }
            shade[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
        int[] px = new int[PW * PH];
        int p = 0;
        for (int i = 0; i + 1 < rle.length() && p < px.length; i += 2) {
            int code = Integer.parseInt(rle.substring(i, i + 2), 16), run = (code >> 2) + 1, col = shade[code & 3];
            for (int k = 0; k < run && p < px.length; k++) {
                px[p++] = col;
            }
        }
        if (lcd == null) {
            lcd = Bitmap.createBitmap(PW, PH, Bitmap.Config.ARGB_8888);
        }
        lcd.setPixels(px, 0, PW, 0, 0, PW, PH);
    }

    /** A point on the page in the LCD's pixels, or null outside it */
    private int[] onLcd(int x, int y) {
        int w = v.width(), s = lcdScale(w), lx = lcdX(w);
        int px = (x - lx) / s, py = (y - LCD_Y) / s;
        return x >= lx && y >= LCD_Y && px < PW && py < PH ? new int[]{px, py} : null;
    }

    private void touch(int[] p, boolean down) {
        if (v.app.client == null || p == null) {
            return;
        }
        try {
            v.app.client.send(new JSONObject().put("t", "touch").put("x", p[0]).put("y", p[1]).put("d", down ? 1 : 0));
        } catch (org.json.JSONException ignored) {
        }
    }

    @Override
    public void press(int x, int y) {
        int[] p = onLcd(x, y);
        if (p != null && watching) {
            touching = true;
            touch(p, true);
        }
    }

    @Override
    public boolean dragStart(int x, int y) {
        return touching;   // (a finger on the LCD drags the stylus; elsewhere the page doesn't move)
    }

    @Override
    public void dragMove(int x, int y) {
        int[] p = onLcd(x, y);
        long now = System.currentTimeMillis();
        if (touching && p != null && now - lastMove > 30) {
            lastMove = now;
            touch(p, true);
        }
    }

    @Override
    public void dragEnd(int x, int y) {
        release(x, y);
    }

    private void release(int x, int y) {
        if (touching) {
            int[] p = onLcd(x, y);
            touch(p != null ? p : new int[]{0, 0}, false);
            touching = false;
        }
    }

    @Override
    public void tap(int x, int y) {
        if (touching) {
            release(x, y);
            return;
        }
        int w = v.width(), s = lcdScale(w), lx = lcdX(w), lh = PH * s, bx = lx + PW * s + 8;
        if (x >= bx - 4 && x < bx + 24 && y >= LCD_Y && y < LCD_Y + lh && v.app.client != null) {
            try {
                v.app.client.send(new JSONObject().put("t", "watchapp").put("dir", 1));
                v.buzz(false);
            } catch (org.json.JSONException ignored) {
            }
        }
    }

    @Override
    public void swipe(int dir) {
        if (v.app.client != null && watching) {
            try {
                v.app.client.send(new JSONObject().put("t", "watchapp").put("dir", dir));
            } catch (org.json.JSONException ignored) {
            }
        }
    }
}
