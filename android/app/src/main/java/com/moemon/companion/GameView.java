package com.moemon.companion;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.widget.OverScroller;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;

import java.util.ArrayList;
import java.util.List;

/**
 * The app's one screen, drawn like the game's menus: a canvas in the game's pixels scaled by a whole number (at least
 * 240 pixels across), the link's state on top, the pages' tabs at the bottom, menus and messages in the game's
 * windows over them. Pages get taps, long presses, drags and swipes in the game's pixels.
 */
public final class GameView extends View {
    public static final int TOP = 20, TABS = 30;

    public final Companion app;
    public final GameArt art;
    private int scale = 4, w = 240, h = 400;
    private final List<Page> pages = new ArrayList<>();
    private int page;
    private Page over;                       // a page over the tabs (pairing, an entry, a box list)
    private Menu menu;                       // a menu or message on top of everything
    private final Paint fill = new Paint();
    private final OverScroller scroller;
    private final Runnable watcher = this::invalidate;

    // touch, in the game's pixels
    private float downX, downY, lastX, lastY;
    private long downAt;
    private boolean moved, longDone, dragging, scrolling;
    private final Runnable longPress = this::onLongPress;
    private android.view.VelocityTracker velocity;

    public GameView(Context c, Companion app) {
        super(c);
        this.app = app;
        this.art = app.art;
        scroller = new OverScroller(c);
        setFocusable(true);
        setContentDescription("Moemon Companion");
        pages.add(new BoxesPage(this));
        pages.add(new PartyPage(this));
        pages.add(new BagPage(this));
        pages.add(new DexPage(this));
        pages.add(new SpawnsPage(this));
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        app.watch(watcher);
    }

    @Override
    protected void onDetachedFromWindow() {
        app.unwatch(watcher);
        super.onDetachedFromWindow();
    }

    /** OPTIONS' text size: 0 = fit (the largest whole scale that keeps 240 pixels across), else that scale */
    private int textSize() {
        return app.context.getSharedPreferences("moemon", 0).getInt("scale", 0);
    }

    @Override
    protected void onSizeChanged(int pw, int ph, int ow, int oh) {
        int want = textSize();
        scale = Math.max(1, pw / 240);
        if (want > 0) {
            scale = Math.max(1, Math.min(scale, want));
        }
        w = pw / scale;
        h = ph / scale;
    }

    public void rescale() {
        onSizeChanged(getWidth(), getHeight(), 0, 0);
        invalidate();
    }

    public int width() {
        return w;
    }

    public int height() {
        return h;
    }

    public int contentHeight() {
        return h - TOP - TABS;
    }

    public void showOver(Page p) {
        over = p;
        if (p != null) {
            p.shown();
        }
        invalidate();
    }

    public Page overPage() {
        return over;
    }

    public void showMenu(Menu m) {
        menu = m;
        invalidate();
    }

    public void message(String text) {
        showMenu(Menu.message(this, text));
    }

    public void select(int i) {
        page = i;
        pages.get(i).shown();
        invalidate();
    }

    public void buzz(boolean strong) {
        performHapticFeedback(strong ? HapticFeedbackConstants.LONG_PRESS : HapticFeedbackConstants.VIRTUAL_KEY);
    }

    private Page active() {
        return over != null ? over : pages.get(page);
    }

    // ---------------------------------------------------------------------------------------------- drawing

    @Override
    protected void onDraw(Canvas c) {
        if (scroller.computeScrollOffset()) {
            active().scrollTo(scroller.getCurrY());
            postInvalidateOnAnimation();
        }
        c.save();
        c.scale(scale, scale);
        fill.setColor(0xFF70B8C8);
        c.drawRect(0, 0, w, h, fill);
        Page p = active();
        c.save();
        c.translate(0, TOP);
        c.clipRect(0, 0, w, h - TOP - (over != null && over.fullScreen() ? 0 : TABS));
        p.draw(c, w, h - TOP - (over != null && over.fullScreen() ? 0 : TABS));
        c.restore();
        drawTop(c);
        if (over == null || !over.fullScreen()) {
            drawTabs(c);
        }
        if (menu != null) {
            menu.draw(c, w, h);
        }
        // the last answer the game couldn't carry out, for 4 seconds
        if (!app.toast.isEmpty() && System.currentTimeMillis() - app.toastAt < 4000) {
            int tw = Math.min(w - 24, art.width(app.toast) + 8);
            art.window(c, 0, (w - tw) / 2, h - TABS - 34, tw, 16);
            art.text(c, app.toast, (w - tw) / 2 + 4, h - TABS - 34, GameArt.RED_TEXT, GameArt.SHADOW);
            postInvalidateDelayed(500);
        }
        c.restore();
    }

    private void drawTop(Canvas c) {
        fill.setColor(0xFF283848);
        c.drawRect(0, 0, w, TOP, fill);
        Model m = app.model();
        LinkClient cl = app.client;
        int x = 4;
        if (m != null) {
            x += art.textHex(c, m.playerName(), x, 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW) + 6;
        } else if (cl == null) {
            art.text(c, "MOEMON COMPANION", x, 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        }
        String s;
        int color;
        if (cl == null) {
            s = "";
            color = GameArt.WHITE_TEXT;
        } else {
            switch (cl.status()) {
                case LINKED:
                    s = "LINKED";
                    color = 0xFF88F888;
                    break;
                case LINKING:
                    s = m == null ? "LINKING..." : "AWAY " + ago(m.asOf);
                    color = 0xFFF8E070;
                    break;
                case REFUSED:
                    s = "NOT LINKED";
                    color = 0xFFF87070;
                    break;
                default:
                    s = m == null ? "AWAY" : "AWAY " + ago(m.asOf);
                    color = 0xFFF8E070;
            }
            int waiting = cl.orders.pending().size();
            if (waiting > 0) {
                s += "  " + waiting + " WAITING";
            }
        }
        int sw = art.width(s);
        art.text(c, s, w - sw - 16, 2, color, GameArt.WHITE_SHADOW);
        // the menu's dots
        fill.setColor(0xFFF8F8F8);
        for (int i = 0; i < 3; i++) {
            c.drawRect(w - 8, 5 + i * 4, w - 6, 7 + i * 4, fill);
        }
    }

    /** "as of" in the game's words: how long ago the snapshot came */
    public static String ago(long t) {
        long s = Math.max(0, (System.currentTimeMillis() - t) / 1000);
        if (s < 60) {
            return "NOW";
        }
        if (s < 3600) {
            return (s / 60) + " MIN AGO";
        }
        if (s < 86400) {
            return (s / 3600) + " HR AGO";
        }
        return (s / 86400) + " DAYS AGO";
    }

    private static final String[] TAB_NAMES = {"BOXES", "PARTY", "BAG", "DEX", "SPAWNS"};

    private void drawTabs(Canvas c) {
        int y = h - TABS, tw = w / TAB_NAMES.length;
        fill.setColor(0xFF283848);
        c.drawRect(0, y, w, h, fill);
        for (int i = 0; i < TAB_NAMES.length; i++) {
            int x = i * tw;
            if (i == page && over == null) {
                fill.setColor(0xFFF8F8F8);
                c.drawRect(x + 2, y + 3, x + tw - 2, h - 3, fill);
            }
            int nw = art.width(TAB_NAMES[i]);
            boolean on = i == page && over == null;
            art.text(c, TAB_NAMES[i], x + (tw - nw) / 2, y + 7, on ? GameArt.TEXT : GameArt.WHITE_TEXT,
                    on ? GameArt.SHADOW : GameArt.WHITE_SHADOW);
        }
    }

    // ------------------------------------------------------------------------------------------------ touch

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        float x = e.getX() / scale, y = e.getY() / scale;
        if (velocity == null) {
            velocity = android.view.VelocityTracker.obtain();
        }
        velocity.addMovement(e);
        switch (e.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                scroller.forceFinished(true);
                downX = lastX = x;
                downY = lastY = y;
                downAt = System.currentTimeMillis();
                moved = longDone = dragging = scrolling = false;
                postDelayed(longPress, 450);
                if (menu == null && y >= TOP && y < h - TABS) {
                    active().press((int) x, (int) y - TOP);
                }
                return true;
            case MotionEvent.ACTION_MOVE:
                if (!moved && Math.hypot(x - downX, y - downY) > 4) {
                    moved = true;
                    removeCallbacks(longPress);
                    if (menu == null && downY >= TOP && downY < h - TABS && !longDone) {
                        dragging = active().dragStart((int) downX, (int) downY - TOP);
                        scrolling = !dragging && Math.abs(y - downY) > Math.abs(x - downX);
                    }
                }
                if (dragging) {
                    active().dragMove((int) x, (int) y - TOP);
                } else if (longDone && menu == null) {
                    active().dragMove((int) x, (int) y - TOP);
                } else if (scrolling && menu == null) {
                    active().scrollBy((int) (lastY - y));
                }
                lastX = x;
                lastY = y;
                invalidate();
                return true;
            case MotionEvent.ACTION_UP:
                removeCallbacks(longPress);
                velocity.computeCurrentVelocity(1000);
                float vx = velocity.getXVelocity() / scale, vy = velocity.getYVelocity() / scale;
                velocity.recycle();
                velocity = null;
                if (menu != null) {
                    if (!moved) {
                        menu.tap((int) x, (int) y);
                    }
                } else if (dragging || longDone) {
                    active().dragEnd((int) x, (int) y - TOP);
                } else if (scrolling) {
                    int pos = active().scrollPos();
                    scroller.fling(0, pos, 0, (int) -vy, 0, 0, 0, Math.max(0, active().scrollMax()));
                } else if (moved && Math.abs(x - downX) > 40 && Math.abs(x - downX) > 2 * Math.abs(y - downY)) {
                    if (downY >= TOP && downY < h - TABS) {
                        active().swipe(x < downX ? 1 : -1);
                    }
                } else if (!moved) {
                    tap((int) x, (int) y);
                }
                invalidate();
                return true;
            case MotionEvent.ACTION_CANCEL:
                removeCallbacks(longPress);
                if (dragging || longDone) {
                    active().dragEnd(-1000, -1000);
                }
                return true;
        }
        return true;
    }

    private void onLongPress() {
        if (!moved && menu == null && downY >= TOP && downY < h - TABS) {
            longDone = active().longPress((int) downX, (int) downY - TOP);
            invalidate();
        }
    }

    private void tap(int x, int y) {
        if (y < TOP) {
            if (x >= w - 60) {
                showMenu(Menu.main(this));
            } else if (over != null) {
                over.back();
            }
            return;
        }
        if (y >= h - TABS && (over == null || !over.fullScreen())) {
            int i = Math.min(TAB_NAMES.length - 1, x / (w / TAB_NAMES.length));
            over = null;
            select(i);
            return;
        }
        active().tap(x, y - TOP);
    }

    /** The back button: a menu, a page over the tabs, then the app */
    public boolean back() {
        if (menu != null) {
            menu = menu.parent;
            invalidate();
            return true;
        }
        if (over != null) {
            over.back();
            return true;
        }
        return false;
    }
}
