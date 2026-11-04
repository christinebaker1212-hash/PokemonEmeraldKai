package com.moemon.companion;

import android.graphics.Canvas;

/** One of the app's pages, in the game's pixels (0, 0 is the content area's top left) */
public abstract class Page {
    protected final GameView v;
    protected final GameArt art;
    protected int scroll;

    protected Page(GameView v) {
        this.v = v;
        this.art = v.art;
    }

    public abstract void draw(Canvas c, int w, int h);

    public void shown() {}
    /** Typed letters (the phone's keyboard, GameView.keyboard), a backspace, the keyboard's done */
    public void typed(String text) {}
    public void backspace() {}
    public void typingDone() {}
    public void hidden() {}
    public void press(int x, int y) {}
    public void tap(int x, int y) {}
    public boolean longPress(int x, int y) { return false; }
    public boolean dragStart(int x, int y) { return false; }
    public void dragMove(int x, int y) {}
    public void dragEnd(int x, int y) {}
    public void swipe(int dir) {}
    public void back() { v.showOver(null); }
    public boolean fullScreen() { return false; }

    // vertical scrolling: pages with lists say how far they go
    public int scrollMax() { return 0; }
    public int scrollPos() { return scroll; }
    public void scrollTo(int y) { scroll = Math.max(0, Math.min(scrollMax(), y)); }
    public void scrollBy(int dy) { scrollTo(scroll + dy); }

    // ------------------------------------------------------------------------------------- the app's look
    public static final int INK = 0xFF283848, GOLD = 0xFFF8E070;
    /** The theme's colors (MENU > LOOK: Look.apply sets them) */
    public static int SKY = 0xFF70B8C8, SKY2 = 0xFF68B0C0, BANNER = 0xFF3868A8, BANNER_DARK = 0xFF284878;
    private static final android.graphics.Paint sFill = new android.graphics.Paint();

    /** The pages' background: the PC's sky blue in soft diagonal stripes */
    protected static void background(Canvas c, int w, int h, int scroll) {
        sFill.setColor(SKY);
        c.drawRect(0, 0, w, h, sFill);
        sFill.setColor(SKY2);
        int off = (scroll / 2) % 16;
        for (int x = -h - 16; x < w + 16; x += 16) {
            for (int k = 0; k < 6; k++) {
                c.drawLine(x + k, -off, x + k + h + 16, h + 16 - off, sFill);
            }
        }
    }

    /** A page's banner: its name on the left, a word on the right (a count, a place) */
    protected void banner(Canvas c, int w, String title, String right) {
        sFill.setColor(BANNER_DARK);
        c.drawRect(0, 0, w, 20, sFill);
        sFill.setColor(BANNER);
        c.drawRect(0, 0, w, 18, sFill);
        sFill.setColor(0xFF5888C8);
        c.drawRect(0, 0, w, 2, sFill);
        art.text(c, title, 6, 1, GameArt.WHITE_TEXT, 0xFF202850);
        if (right != null && !right.isEmpty()) {
            art.text(c, right, w - 6 - art.width(right), 1, GOLD, 0xFF202850);
        }
    }

    /** A small colored label (a status, a count): a rounded box with white letters */
    protected void chip(Canvas c, String text, int x, int y, int color) {
        int tw = art.width(text) + 6;
        sFill.setColor(color);
        c.drawRect(x + 1, y, x + tw - 1, y + 11, sFill);
        c.drawRect(x, y + 1, x + tw, y + 10, sFill);
        art.text(c, text, x + 3, y - 3, GameArt.WHITE_TEXT, 0);
    }

    /** A button in the game's look: a small window with a word in it */
    protected void button(Canvas c, String label, int x, int y, int w, boolean enabled) {
        art.window(c, 0, x + 4, y + 4, w - 8, 16);
        int tw = art.width(label);
        art.text(c, label, x + (w - tw) / 2, y + 4, enabled ? GameArt.TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
    }

    /** Words in lines no wider than max */
    protected java.util.List<String> wrap(String text, int max) {
        java.util.List<String> out = new java.util.ArrayList<>();
        StringBuilder line = new StringBuilder();
        for (String word : text.split(" ")) {
            String next = line.length() == 0 ? word : line + " " + word;
            if (art.width(next) > max && line.length() > 0) {
                out.add(line.toString());
                line = new StringBuilder(word);
            } else {
                line = new StringBuilder(next);
            }
        }
        out.add(line.toString());
        return out;
    }

    protected static boolean in(int x, int y, int rx, int ry, int rw, int rh) {
        return x >= rx && y >= ry && x < rx + rw && y < ry + rh;
    }
}
