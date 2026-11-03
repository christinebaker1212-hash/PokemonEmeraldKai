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
