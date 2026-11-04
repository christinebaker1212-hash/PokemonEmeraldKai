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
    /** Whether the MOéMON / POKéMON words are swapped on this page (the card games keep their own words) */
    public boolean swapsWords() { return true; }

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

    /** Whether the finger is (or just was) on this rectangle of the page */
    protected static boolean pressed(int x, int y, int w, int h) {
        return System.currentTimeMillis() < GameView.pressUntil && GameView.pressX >= x && GameView.pressX < x + w
                && GameView.pressY >= y && GameView.pressY < y + h;
    }

    /**
     * A button (the touch screens' raised key of the DS era): a rounded white key with a dark outline and a lip
     * under it; pressed, it sinks onto the lip and darkens; disabled, it's flat and grey. (x, y, w) as before: the key
     * sits in the 24-pixel-tall slot at (x, y)
     */
    protected void button(Canvas c, String label, int x, int y, int w, boolean enabled) {
        int bx = x + 3, by = y + 3, bw = w - 6, bh = 18;
        boolean down = enabled && pressed(x, y, w, 24);
        int sink = down ? 2 : 0;
        // the lip (its shadow on the page)
        if (!down) {
            sFill.setColor(enabled ? 0xFF4A5868 : 0x60404850);
            c.drawRect(bx + 1, by + 2, bx + bw - 1, by + bh + 2, sFill);
        }
        // the outline, corners cut
        sFill.setColor(enabled ? 0xFF283848 : 0xFF8890A0);
        c.drawRect(bx + 1, by + sink, bx + bw - 1, by + bh + sink, sFill);
        c.drawRect(bx, by + 1 + sink, bx + bw, by + bh - 1 + sink, sFill);
        // the face: white with a light top, grey-blue when pressed, flat grey when it can't be used
        int face = !enabled ? 0xFFD8DCE0 : down ? 0xFFC8D8F0 : 0xFFF8F8F8;
        sFill.setColor(face);
        c.drawRect(bx + 1, by + 1 + sink, bx + bw - 1, by + bh - 1 + sink, sFill);
        if (enabled && !down) {
            sFill.setColor(0xFFE0E6EE);   // (the face's lower half a shade darker: a soft bevel)
            c.drawRect(bx + 1, by + bh / 2 + 1, bx + bw - 1, by + bh - 1, sFill);
            sFill.setColor(0xFFFFFFFF);
            c.drawRect(bx + 2, by + 1, bx + bw - 2, by + 2, sFill);
        }
        int tw = art.width(label);
        art.text(c, label, x + (w - tw) / 2, by + 1 + sink, enabled ? GameArt.TEXT : 0xFF9098A0,
                enabled ? GameArt.SHADOW : 0);
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
