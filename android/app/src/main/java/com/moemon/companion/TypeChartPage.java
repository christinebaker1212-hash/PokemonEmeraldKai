package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import java.util.ArrayList;
import java.util.List;

/**
 * The type chart, from the game's own (data.json's typeChart, as GameArt.effectiveness reads it), on the POKéDEX's
 * green: CHART is the whole table (attacking types down the side, defending ones along the top: green ×2, red ×1/2,
 * black ×0; tap a square to read it, a type's icon to look at it alone); DEFENSE takes one or two types (a POKéMON's)
 * and sorts every attacking type by what it does to them; ATTACK takes one type and sorts what it hits.
 */
public final class TypeChartPage extends Page {
    /** The types in the order the games list them (NORMAL, FIRE, WATER...); the numbers are Kai's (18: FAIRY) */
    static final int[] ORDER = {0, 10, 11, 13, 12, 15, 1, 3, 4, 2, 14, 6, 5, 7, 16, 17, 8, 18};
    private static final String[] MODES = {"CHART", "DEFENSE", "ATTACK"};
    static final int SUPER = 0xFF48B848, HALF = 0xFFE05848, NONE = 0xFF383838, EVEN = 0xFFF8F8F0;

    private int mode;
    private int atk = -1, def = -1;           // CHART's chosen square (types, by number)
    private int def1 = -1, def2 = -1;         // DEFENSE's types
    private int attack = 10;                  // ATTACK's type
    private long pickedAt;                    // (a choice's flash)
    private final Paint fill = new Paint();
    private int height;

    public TypeChartPage(GameView v) {
        super(v);
    }

    /** DEFENSE for a POKéMON's types (the POKéDEX's TYPE MATCHUPS) */
    public TypeChartPage defending(int t0, int t1) {
        mode = 1;
        def1 = t0;
        def2 = t1 != t0 ? t1 : -1;
        return this;
    }

    public TypeChartPage attacking(int t) {
        mode = 2;
        attack = t;
        return this;
    }

    static String mult(int tenths) {
        switch (tenths) {
            case 0: return "×0";
            case 2: case 3: return "×1/4";
            case 5: return "×1/2";
            case 10: return "×1";
            case 20: return "×2";
            case 40: return "×4";
            default: return "×" + tenths / 10.0;
        }
    }

    static int multColor(int tenths) {
        return tenths == 0 ? NONE : tenths < 10 ? HALF : tenths > 10 ? SUPER : EVEN;
    }

    private String typeName(int t) {
        return art.plain(art.name("types", t));
    }

    private int cell(int w) {
        return Math.max(9, Math.min(14, (w - 46) / ORDER.length));
    }

    @Override
    public int scrollMax() {
        return Math.max(0, height - v.contentHeight());
    }

    private float flash() {
        long t = System.currentTimeMillis() - pickedAt;
        if (t < 240) {
            v.postInvalidateDelayed(16);
            return 1f - t / 240f;
        }
        return 0f;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        DexPage.stripes(c, fill, w, h + 8, scroll);
        c.save();
        c.translate(0, -scroll);
        DexPage.chip(c, art, fill, "BACK", 6, 4, false);
        int mx = w - 6;
        for (int i = MODES.length - 1; i >= 0; i--) {
            mx -= art.width(MODES[i]) + 12;
            DexPage.chip(c, art, fill, MODES[i], mx, 4, i == mode);
            mx -= 4;
        }
        int y = 26;
        if (mode == 0) {
            y = chart(c, w, y);
        } else if (mode == 1) {
            y = defense(c, w, y);
        } else {
            y = attackPage(c, w, y);
        }
        height = y + 8;
        c.restore();
    }

    // ------------------------------------------------------------------------------------------------- CHART
    private int gridX(int w) {
        int cs = cell(w);
        return (w - 36 - cs * ORDER.length) / 2 + 36;
    }

    private int chart(Canvas c, int w, int y) {
        int cs = cell(w), x0 = gridX(w), rows = ORDER.length, rh = 16;
        art.text(c, "DEFENDING", x0, y - 2, 0xFFFFFFFF, 0xFF000000);
        y += 14;
        int top = y + cs + 2;
        // the defending types along the top: their colors
        for (int j = 0; j < rows; j++) {
            int t = ORDER[j];
            fill.setColor(0xFF000000);
            c.drawRect(x0 + j * cs, y, x0 + (j + 1) * cs, y + cs, fill);
            fill.setColor(art.typeColor(t));
            c.drawRect(x0 + j * cs + 1, y + 1, x0 + (j + 1) * cs - 1, y + cs - 1, fill);
            if (t == def) {
                fill.setColor(0xFFFFFFFF);
                c.drawRect(x0 + j * cs + 1, y + cs - 3, x0 + (j + 1) * cs - 1, y + cs - 1, fill);
            }
        }
        float f = flash();
        for (int i = 0; i < rows; i++) {
            int a = ORDER[i], ry = top + i * rh;
            art.typeIcon(c, a, x0 - 34, ry);
            for (int j = 0; j < rows; j++) {
                int d = ORDER[j], m = art.effectiveness(a, d, d);
                fill.setColor(0xFF203020);
                c.drawRect(x0 + j * cs, ry, x0 + (j + 1) * cs, ry + rh, fill);
                fill.setColor(multColor(m));
                c.drawRect(x0 + j * cs + 1, ry + 1, x0 + (j + 1) * cs - 1, ry + rh - 1, fill);
                if (m != 10) {
                    String s = m == 0 ? "0" : m < 10 ? "-" : "2";
                    art.text(c, s, x0 + j * cs + (cs - art.width(s)) / 2, ry - 1, 0xFFFFFFFF, 0xFF000000);
                }
            }
            if (a == atk) {   // (the chosen row and column: framed)
                fill.setColor(0xFFFFFFFF);
                c.drawRect(x0 - 35, ry, x0 + rows * cs, ry + 1, fill);
                c.drawRect(x0 - 35, ry + rh - 1, x0 + rows * cs, ry + rh, fill);
            }
        }
        if (def >= 0) {
            for (int j = 0; j < rows; j++) {
                if (ORDER[j] == def) {
                    fill.setColor(0xFFFFFFFF);
                    c.drawRect(x0 + j * cs, top, x0 + j * cs + 1, top + rows * rh, fill);
                    c.drawRect(x0 + (j + 1) * cs - 1, top, x0 + (j + 1) * cs, top + rows * rh, fill);
                }
            }
        }
        if (f > 0 && atk >= 0 && def >= 0) {
            fill.setColor(((int) (f * 110) << 24) | 0xFFFFFF);
            for (int i = 0; i < rows; i++) {
                if (ORDER[i] == atk) {
                    c.drawRect(x0, top + i * rh, x0 + rows * cs, top + (i + 1) * rh, fill);
                }
            }
        }
        y = top + rows * rh + 6;
        // the key, then what the chosen square says
        y = key(c, w, y);
        int bh = 44;
        art.window(c, 0, 6, y, w - 12, bh - 8);
        if (atk >= 0 && def >= 0) {
            int m = art.effectiveness(atk, def, def);
            art.typeIcon(c, atk, 12, y + 4);
            art.text(c, ">", 48, y + 3, GameArt.TEXT, GameArt.SHADOW);
            art.typeIcon(c, def, 58, y + 4);
            String word = m == 0 ? "No effect" : m < 10 ? "Not very effective" : m > 10 ? "Super effective" : "Normal damage";
            art.text(c, mult(m) + "  " + word, 96, y + 3, m == 10 ? GameArt.TEXT : m > 10 ? 0xFF208020 : GameArt.RED_TEXT,
                    GameArt.SHADOW);
        } else {
            art.text(c, "Tap a square to read it; a type for its own page.", 12, y + 3, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        return y + bh;
    }

    private int key(Canvas c, int w, int y) {
        int x = 8;
        int[][] keys = {{20, 0}, {5, 0}, {0, 0}};
        String[] words = {"×2", "×1/2", "×0"};
        for (int k = 0; k < keys.length; k++) {
            fill.setColor(0xFF000000);
            c.drawRect(x, y + 3, x + 12, y + 15, fill);
            fill.setColor(multColor(keys[k][0]));
            c.drawRect(x + 1, y + 4, x + 11, y + 14, fill);
            x += 16;
            x += art.text(c, words[k], x, y, 0xFFFFFFFF, 0xFF000000) + 10;
        }
        return y + 22;
    }

    // ------------------------------------------------------------------------------------------------- DEFENSE / ATTACK
    /** The types to pick, six to a row; returns the bottom */
    private int picker(Canvas c, int w, int y, int a, int b) {
        int per = Math.max(4, (w - 12) / 38), gx = (w - per * 38) / 2 + 3;
        for (int i = 0; i < ORDER.length; i++) {
            int t = ORDER[i], x = gx + (i % per) * 38, ty = y + (i / per) * 20;
            boolean on = t == a || t == b;
            if (on) {
                fill.setColor(0xFFFFFFFF);
                c.drawRect(x - 2, ty - 2, x + 34, ty + 18, fill);
            }
            art.typeIcon(c, t, x, ty);
        }
        return y + (ORDER.length + per - 1) / per * 20 + 4;
    }

    private int pickerHit(int w, int y0, int x, int y) {
        int per = Math.max(4, (w - 12) / 38), gx = (w - per * 38) / 2 + 3;
        for (int i = 0; i < ORDER.length; i++) {
            int tx = gx + (i % per) * 38, ty = y0 + (i / per) * 20;
            if (in(x, y, tx - 2, ty - 2, 36, 20)) {
                return ORDER[i];
            }
        }
        return -1;
    }

    private int pickerTop;

    /** A group: its title and multiplier, then its types' icons, wrapping; returns the bottom */
    private int group(Canvas c, int w, int y, String title, int color, List<Integer> types) {
        if (types.isEmpty()) {
            return y;
        }
        int per = Math.max(4, (w - 24) / 36), rows = (types.size() + per - 1) / per;
        int bh = 22 + rows * 18 + 4;
        fill.setColor(DexPage.BORDER);
        c.drawRect(6, y, w - 6, y + bh, fill);
        fill.setColor(0xFFF8F8F0);
        c.drawRect(7, y + 1, w - 7, y + bh - 1, fill);
        fill.setColor(color);
        c.drawRect(7, y + 1, w - 7, y + 17, fill);
        art.text(c, title, 12, y - 1, color == EVEN ? GameArt.TEXT : 0xFFFFFFFF, color == EVEN ? GameArt.SHADOW : 0xFF000000);
        for (int i = 0; i < types.size(); i++) {
            art.typeIcon(c, types.get(i), 12 + (i % per) * 36, y + 21 + (i / per) * 18);
        }
        return y + bh + 6;
    }

    private int defense(Canvas c, int w, int y) {
        art.text(c, "A POKéMON's types (one or two):", 8, y - 2, 0xFFFFFFFF, 0xFF000000);
        pickerTop = y + 16;
        y = picker(c, w, pickerTop, def1, def2);
        if (def1 < 0) {
            art.window(c, 0, 6, y, w - 12, 20);
            art.text(c, "Pick a type to see what hits it hard.", 12, y + 3, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 30;
        }
        int d2 = def2 >= 0 ? def2 : def1;
        List<Integer> x4 = new ArrayList<>(), x2 = new ArrayList<>(), half = new ArrayList<>(), quarter = new ArrayList<>(),
                none = new ArrayList<>(), even = new ArrayList<>();
        for (int a : ORDER) {
            int m = art.effectiveness(a, def1, d2);
            (m == 0 ? none : m >= 40 ? x4 : m >= 20 ? x2 : m <= 3 ? quarter : m < 10 ? half : even).add(a);
        }
        y = group(c, w, y, "WEAK ×4", 0xFF208820, x4);
        y = group(c, w, y, "WEAK ×2", SUPER, x2);
        y = group(c, w, y, "RESISTS ×1/2", HALF, half);
        y = group(c, w, y, "RESISTS ×1/4", 0xFFB03828, quarter);
        y = group(c, w, y, "IMMUNE ×0", NONE, none);
        y = group(c, w, y, "NORMAL ×1", EVEN, even);
        return y;
    }

    private int attackPage(Canvas c, int w, int y) {
        art.text(c, "A move's type:", 8, y - 2, 0xFFFFFFFF, 0xFF000000);
        pickerTop = y + 16;
        y = picker(c, w, pickerTop, attack, -1);
        List<Integer> x2 = new ArrayList<>(), half = new ArrayList<>(), none = new ArrayList<>(), even = new ArrayList<>();
        for (int d : ORDER) {
            int m = art.effectiveness(attack, d, d);
            (m == 0 ? none : m > 10 ? x2 : m < 10 ? half : even).add(d);
        }
        y = group(c, w, y, "SUPER EFFECTIVE ×2", SUPER, x2);
        y = group(c, w, y, "NOT VERY EFFECTIVE ×1/2", HALF, half);
        y = group(c, w, y, "NO EFFECT ×0", NONE, none);
        y = group(c, w, y, "NORMAL ×1", EVEN, even);
        return y;
    }

    // ------------------------------------------------------------------------------------------------- touch
    @Override
    public void tap(int x, int y) {
        int w = v.width(), cy = y + scroll;
        if (cy < 22) {
            if (x < 60) {
                back();
                return;
            }
            int mx = w - 6;
            for (int i = MODES.length - 1; i >= 0; i--) {
                int bw = art.width(MODES[i]) + 12;
                mx -= bw;
                if (x >= mx && x < mx + bw + 4) {
                    if (mode != i) {
                        mode = i;
                        scroll = 0;
                        v.sound.play("se_select");
                    }
                    return;
                }
                mx -= 4;
            }
            return;
        }
        if (mode == 0) {
            int cs = cell(w), x0 = gridX(w), top = 26 + 14 + cs + 2, rh = 16;
            int i = (cy - top) / rh, j = (x - x0) / cs;
            if (cy >= top && i < ORDER.length) {
                if (x < x0 && x >= x0 - 34) {   // (a type's icon: ATTACK for it)
                    attacking(ORDER[i]);
                    scroll = 0;
                    v.sound.play("se_select");
                    return;
                }
                if (x >= x0 && j < ORDER.length) {
                    atk = ORDER[i];
                    def = ORDER[j];
                    pickedAt = System.currentTimeMillis();
                    v.buzz(false);
                }
            } else if (cy >= top - cs - 2 && cy < top && x >= x0 && j < ORDER.length) {
                defending(ORDER[j], -1);   // (a defending color along the top: DEFENSE for it)
                scroll = 0;
                v.sound.play("se_select");
            }
            return;
        }
        int t = pickerHit(w, pickerTop, x, cy);
        if (t < 0) {
            return;
        }
        v.buzz(false);
        if (mode == 2) {
            attack = t;
        } else if (t == def1) {
            def1 = def2;
            def2 = -1;
        } else if (t == def2) {
            def2 = -1;
        } else if (def1 < 0) {
            def1 = t;
        } else if (def2 < 0) {
            def2 = t;
        } else {
            def1 = def2;
            def2 = t;
        }
        pickedAt = System.currentTimeMillis();
    }

    @Override
    public void swipe(int dir) {
        mode = (mode + dir + MODES.length) % MODES.length;
        scroll = 0;
    }
}
