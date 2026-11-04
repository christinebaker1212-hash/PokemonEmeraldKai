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
    // the finger's press, in page coordinates: buttons under it draw pressed (Page.pressed)
    static int pressX = -1000, pressY = -1000;
    static long pressUntil;
    private static final long TOAST_MS = 3600;
    private int toastTop = -1;   // (the toast's top while it shows: a tap there closes it)

    public final Companion app;
    public final GameArt art;
    private int scale = 4, w = 240, h = 400;
    private final List<Page> pages = new ArrayList<>();
    private int page;
    // the animations (MENU > LOOK > ANIMATIONS): a page sliding in, the tabs' plate sliding to its tab
    private long transAt, tabAt;
    private int transDir;
    private float tabFrom = -1;
    static final int SLIDE_MS = 220, TAB_MS = 160;
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

    /** The game's own sounds (Sound.java) */
    public final Sound sound;
    private long toastHeard;

    public GameView(Context c, Companion app) {
        super(c);
        this.app = app;
        this.art = app.art;
        this.sound = Sound.get(c);
        scroller = new OverScroller(c);
        setFocusable(true);
        setContentDescription("Moemon Companion");
        pages.add(new BoxesPage(this));
        pages.add(new PartyPage(this));
        pages.add(new BagPage(this));
        pages.add(new DexPage(this));
        pages.add(new SpawnsPage(this));
        pages.add(new DevicePage(this));
        pages.add(new WalkPage(this));
        pages.add(new FriendsPage(this));
        pages.add(new CardsPage(this));
        pages.add(new MePage(this));
        Look.ensure(c);
        int start = Look.startPage(c);
        page = start >= 0 && start < pages.size() ? start : 0;
    }

    /** 0 to 1 over ms since t, eased out (1 at once without animations) */
    static float eased(long t, int ms) {
        if (!Look.animations) {
            return 1f;
        }
        float k = Math.min(1f, (System.currentTimeMillis() - t) / (float) ms);
        float u = 1f - k;
        return 1f - u * u * u;
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

    /** Screen pixels per page pixel (a page drawing at a factor of k keeps whole pixels when k x this is whole) */
    public int pixelScale() {
        return scale;
    }

    public int height() {
        return h;
    }

    public int contentHeight() {
        return h - TOP - TABS;
    }

    public void showOver(Page p) {
        if (p != over) {
            sound.play("se_select");   // (a page opened or closed: the game's A and B)
            transAt = System.currentTimeMillis();
            transDir = p != null ? 1 : -1;
        }
        if (over == null && p != null) {
            pages.get(page).hidden();
        } else if (over != null && over != p) {
            over.hidden();
        }
        if (p == null && over != null) {
            over = null;
            pages.get(page).shown();
            invalidate();
            return;
        }
        over = p;
        if (p != null) {
            p.shown();
        }
        invalidate();
    }

    public Page page(int i) {
        return pages.get(i);
    }

    public Page overPage() {
        return over;
    }

    public void showMenu(Menu m) {
        if (m != null && menu != null) {
            m.still = true;
        }
        menu = m;
        invalidate();
    }

    public void message(String text) {
        showMenu(Menu.message(this, text));
    }

    public void select(int i) {
        if (over != null) {
            over.hidden();
            over = null;
        }
        if (i != page) {
            pages.get(page).hidden();
            tabFrom = plateX(tabOf(page));
            tabAt = transAt = System.currentTimeMillis();
            transDir = tabOf(i) >= tabOf(page) ? 1 : -1;
            Look.rememberPage(getContext(), i);
        }
        page = i;
        pages.get(i).shown();
        invalidate();
    }

    // ----------------------------------------------------------------------------------- the phone's keyboard
    /** Show (or hide) the phone's keyboard; what's typed goes to the page in front (Page.typed) */
    public void keyboard(boolean show) {
        android.view.inputmethod.InputMethodManager im =
                (android.view.inputmethod.InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        if (show) {
            setFocusableInTouchMode(true);
            requestFocus();
            im.showSoftInput(this, 0);
        } else {
            im.hideSoftInputFromWindow(getWindowToken(), 0);
        }
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public android.view.inputmethod.InputConnection onCreateInputConnection(android.view.inputmethod.EditorInfo info) {
        info.inputType = android.text.InputType.TYPE_CLASS_TEXT | android.text.InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        info.imeOptions = android.view.inputmethod.EditorInfo.IME_ACTION_SEARCH;
        return new android.view.inputmethod.BaseInputConnection(this, false) {
            @Override
            public boolean commitText(CharSequence text, int newCursorPosition) {
                active().typed(text.toString());
                invalidate();
                return true;
            }

            @Override
            public boolean deleteSurroundingText(int before, int after) {
                for (int i = 0; i < Math.max(1, before); i++) {
                    active().backspace();
                }
                invalidate();
                return true;
            }

            @Override
            public boolean performEditorAction(int action) {
                active().typingDone();
                keyboard(false);
                invalidate();
                return true;
            }

            @Override
            public boolean sendKeyEvent(android.view.KeyEvent e) {
                if (e.getAction() == android.view.KeyEvent.ACTION_DOWN) {
                    if (e.getKeyCode() == android.view.KeyEvent.KEYCODE_DEL) {
                        active().backspace();
                    } else if (e.getKeyCode() == android.view.KeyEvent.KEYCODE_ENTER) {
                        active().typingDone();
                        keyboard(false);
                    } else if (e.getUnicodeChar() != 0) {
                        active().typed(String.valueOf((char) e.getUnicodeChar()));
                    }
                    invalidate();
                }
                return true;
            }
        };
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
        GameArt.pokemonArt = Look.pokemonArt(app);   // (MENU > LOOK > CREATURES, or the game's own)
        if (scroller.computeScrollOffset()) {
            active().scrollTo(scroller.getCurrY());
            postInvalidateOnAnimation();
        }
        c.save();
        c.scale(scale, scale);
        Page p = active();
        Page.background(c, w, h, p.scrollPos());
        c.save();
        c.translate(0, TOP);
        c.clipRect(0, 0, w, h - TOP - (over != null && over.fullScreen() ? 0 : TABS));
        float e = eased(transAt, SLIDE_MS);
        if (e < 1f) {   // (the page coming in: a short slide and a fade)
            c.translate(transDir * (1f - e) * w * 0.3f, 0);
            c.saveLayerAlpha(0, 0, w, h, (int) (90 + 165 * e));
            postInvalidateOnAnimation();
        }
        GameArt.swapWords = p.swapsWords();
        p.draw(c, w, h - TOP - (over != null && over.fullScreen() ? 0 : TABS));
        GameArt.swapWords = true;
        c.restore();
        drawTop(c);
        if (over == null || !over.fullScreen()) {
            drawTabs(c);
        }
        if (menu != null) {
            menu.draw(c, w, h);
        }
        // the last answer the game couldn't carry out, for 4 seconds
        if (!app.toast.isEmpty() && app.toastAt != toastHeard && System.currentTimeMillis() - app.toastAt < 1000) {
            toastHeard = app.toastAt;
            sound.play(app.toastGood ? "se_success" : "se_failure");
        }
        // the toast: a dark card that rises above the tabs, stays a moment and sinks away (a tap on it closes it)
        long age = System.currentTimeMillis() - app.toastAt;
        if (!app.toast.isEmpty() && age < TOAST_MS) {
            java.util.List<String> lines = new java.util.ArrayList<>();
            StringBuilder line = new StringBuilder();
            for (String word : app.toast.split(" ")) {
                String next = line.length() == 0 ? word : line + " " + word;
                if (art.width(next) > w - 52 && line.length() > 0) {
                    lines.add(line.toString());
                    line = new StringBuilder(word);
                } else {
                    line = new StringBuilder(next);
                }
            }
            lines.add(line.toString());
            float in = Look.animations ? Math.min(1f, age / 180f) : 1f;
            float out = Look.animations ? Math.min(1f, (TOAST_MS - age) / 220f) : 1f;
            float k = Math.min(in, out);
            k = 1f - (1f - k) * (1f - k);
            int th = lines.size() * 14 + 8;
            float ty = h - TABS - 6 - th + (1f - k) * (th + 12);
            int alpha = (int) (255 * Math.min(1f, k * 1.4f));
            c.save();
            c.saveLayerAlpha(0, 0, w, h, alpha);
            pill.set(10, ty, w - 10, ty + th);
            fill.setColor(0x60000000);
            c.drawRoundRect(pill.left + 1, pill.top + 2, pill.right + 1, pill.bottom + 2, 6, 6, fill);
            fill.setColor(0xF0202C3C);
            c.drawRoundRect(pill, 6, 6, fill);
            fill.setColor(app.toastGood ? 0xFF58E070 : 0xFFF86060);
            c.drawRoundRect(14, ty + 4, 17, ty + th - 4, 2, 2, fill);
            for (int i = 0; i < lines.size(); i++) {
                art.text(c, lines.get(i), 24, (int) ty + 1 + i * 14, 0xFFF0F4F8, 0xFF0C1420);
            }
            c.restore();
            c.restore();
            if (Look.animations) {
                postInvalidateOnAnimation();
            } else {
                postInvalidateDelayed(500);
            }
            toastTop = (int) ty;
        } else {
            toastTop = -1;
        }
        c.restore();
    }

    private final android.graphics.RectF pill = new android.graphics.RectF();

    /**
     * The top bar: whose game it is, the link as a pill (a colored light and a word: LINKED, IN BATTLE, AWAY and how
     * long ago the game was last heard from, NOT LINKED), the orders still to reach the game as a badge, and the
     * menu's key
     */
    private void drawTop(Canvas c) {
        fill.setColor(0xFF1C2838);
        c.drawRect(0, 0, w, TOP, fill);
        fill.setColor(0xFF2E4058);
        c.drawRect(0, TOP - 1, w, TOP, fill);
        Model m = app.model();
        LinkClient cl = app.client;
        if (m != null) {
            art.textHex(c, m.playerName(), 6, 2, GameArt.WHITE_TEXT, 0xFF0C1420);
        } else {
            art.text(c, "MOEMON COMPANION", 6, 2, GameArt.WHITE_TEXT, 0xFF0C1420);
        }
        // the menu's key: three bars, lit while pressed
        boolean menuDown = System.currentTimeMillis() < pressUntil && pressY < 0 && pressX >= w - 26;
        if (menuDown) {
            fill.setColor(0xFF3A5070);
            c.drawRect(w - 24, 1, w - 1, TOP - 2, fill);
        }
        fill.setColor(0xFFF0F4F8);
        for (int i = 0; i < 3; i++) {
            c.drawRect(w - 18, 5 + i * 4, w - 7, 7 + i * 4, fill);
        }
        if (cl == null) {
            return;
        }
        String s;
        int light;
        switch (cl.status()) {
            case LINKED:
                s = app.locked() ? "IN BATTLE" : cl.viaServer() ? "LINKED FROM AWAY" : "LINKED";
                light = app.locked() ? 0xFFF8A040 : 0xFF58E070;
                break;
            case LINKING:
                s = m == null ? "LINKING" : "AWAY " + ago(m.asOf);
                light = 0xFFF8D040;
                break;
            case REFUSED:
                s = "NOT LINKED";
                light = 0xFFF85858;
                break;
            default:
                s = m == null ? "AWAY" : "AWAY " + ago(m.asOf);
                light = 0xFFF8D040;
        }
        int x1 = w - 30;
        // the orders on their way: a round badge left of the pill
        int waiting = cl.orders.pending().size();
        int sw = art.smallWidth(s), pw = sw + 16;
        int px = x1 - pw;
        pill.set(px, 3, x1, TOP - 4);
        fill.setColor(0xFF2A3A50);
        c.drawRoundRect(pill, 6, 6, fill);
        fill.setColor(light);
        c.drawCircle(px + 6.5f, TOP / 2f - 0.5f, 2.5f, fill);
        art.small(c, s, px + 11, 1, 0xFFE8F0F8, 0);
        if (waiting > 0) {
            String n = waiting > 99 ? "99" : String.valueOf(waiting);
            int bw = Math.max(13, art.smallWidth(n) + 7);
            pill.set(px - bw - 3, 3, px - 3, TOP - 4);
            fill.setColor(0xFFF8A040);
            c.drawRoundRect(pill, 6, 6, fill);
            art.small(c, n, px - 3 - bw + (bw - art.smallWidth(n)) / 2, 1, 0xFF202020, 0);
        }
        if (cl.status() != LinkClient.Status.LINKED) {
            postInvalidateDelayed(30000);   // (the "how long ago" moves on)
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

    // the tabs and the pages they open (SPAWNS, page 4, opens from the POKéDEX: its tab is DEX's)
    private static final String[] TAB_NAMES = {"BOXES", "PARTY", "BAG", "DEX", "DEVICE", "WALK", "CARDS", "FRIENDS", "ME"};
    private static final String[] TAB_SHORT = {"BOX", "TEAM", "BAG", "DEX", "DEV", "WALK", "CARD", "PALS", "ME"};
    private static final int[] TAB_PAGES = {0, 1, 2, 3, 5, 6, 8, 7, 9};

    private int tabOf(int p) {
        for (int i = 0; i < TAB_PAGES.length; i++) {
            if (TAB_PAGES[i] == p) {
                return i;
            }
        }
        return 3;   // (SPAWNS: under DEX)
    }

    private float plateX(int tab) {
        return tab * (w / (float) TAB_NAMES.length);
    }

    private final android.graphics.RectF plate = new android.graphics.RectF();

    private void drawTabs(Canvas c) {
        int y = h - TABS, tw = w / TAB_NAMES.length;
        fill.setColor(0xFF283848);
        c.drawRect(0, y, w, h, fill);
        fill.setColor(0xFF3A5068);
        c.drawRect(0, y, w, y + 1, fill);
        if (over == null) {
            // the chosen one: a raised white plate, as the DS's touch menus, sliding over from the last tab
            float to = tabOf(page) * tw, e = tabFrom < 0 ? 1f : eased(tabAt, TAB_MS);
            float px = tabFrom < 0 ? to : tabFrom + (to - tabFrom) * e;
            if (e < 1f) {
                postInvalidateOnAnimation();
            }
            fill.setColor(0xFF98A8B8);
            plate.set(px + 1, y + 2, px + tw - 1, h - 1);
            c.drawRoundRect(plate, 3, 3, fill);
            fill.setColor(0xFFF8F8F8);
            plate.set(px + 1, y + 2, px + tw - 1, h - 3);
            c.drawRoundRect(plate, 3, 3, fill);
        }
        for (int i = 0; i < TAB_NAMES.length; i++) {
            int x = i * tw;
            boolean on = i == tabOf(page) && over == null;
            drawTabIcon(c, i, x + (tw - 12) / 2, y + 3, !on);
            // (the whole name where it fits under the icon, else its short one)
            String name = art.smallWidth(TAB_NAMES[i]) <= tw - 3 ? TAB_NAMES[i] : TAB_SHORT[i];
            int nw = art.width(name);
            nw = art.smallWidth(name);
            art.small(c, name, x + (tw - nw) / 2, y + 15, on ? GameArt.TEXT : 0xFFC8D8E8,
                    on ? GameArt.SHADOW : 0xFF101820);
        }
    }

    // The tabs' icons: 12x12 pixel art in the game's way (. clear, k outline, w white, r red, y yellow, o orange,
    // n brown, b blue, l light blue, g green, s grey, p pink, d dark red)
    private static final String[][] TAB_ICONS = {
        {"............", ".kkkkkkkkkk.", ".knnnnnnnnk.", ".kkkkkkkkkk.", ".knnnnnnnnk.", ".knnkkkknnk.",
         ".knnkookknk.", ".knnkkkknnk.", ".knnnnnnnnk.", ".knnnnnnnnk.", ".kkkkkkkkkk.", "............"},
        {"....kkkk....", "..kkrrrrkk..", ".krrrwrrrrk.", ".krrwrrrrrk.", "krrrrkkrrrrk", "kkkkkwwkkkkk",
         "kwwwkwwkwwwk", ".kwwwkkwwwk.", ".kwwwwwwwwk.", "..kkwwwwkk..", "....kkkk....", "............"},
        {"....kkkk....", "...kyyyyk...", "..kkkkkkkk..", ".kooooooook.", ".koyyyyyyok.", ".kooookoook.",
         ".koooykyook.", ".kooookoook.", ".kooooooook.", ".koooooooook", ".kkkkkkkkkk.", "............"},
        {".kkkkkkkkk..", ".krrrrrrrrk.", ".krwwwwwwrk.", ".krwlllwwrk.", ".krwlllwwrk.", ".krwwwwwwrk.",
         ".krrrrrrrrk.", ".krrdrrrrrk.", ".krrrrrkrrk.", ".krrrrkkkrk.", ".kkkkkkkkkk.", "............"},
        {"............", "..kkkkkkkk..", ".kyyyyyyyyk.", ".kykkkkkkyk.", ".kyklllllkyk", ".kyklllllkyk",
         ".kykkkkkkyk.", ".kyyyyyyyyk.", ".kyykyykyyk.", ".kyyyyyyyyk.", "..kkkkkkkk..", "............"},
        {"............", "............", "..kkkk......", ".kbbbbk.....", ".kbwbbk.....", ".kbbwbbkk...",
         ".kbbbbbbbkk.", ".kbbbbbbbbbk", ".kwwwwwwwwwk", "..kkkkkkkkk.", "............", "............"},
        {"..kkkkkkk...", "..kbbbbbk...", "..kblllbkkk.", "..kblwlbkrk.", "..kblllbkrk.", "..kbbbbbkrk.",
         "..kbbbbbkrk.", "..kkkkkkkrk.", "....krrrrrk.", "....krrrrrk.", "....kkkkkkk.", "............"},
        {"..kkk.......", ".kpppk..kkk.", ".kpppk.kbbbk", ".kpppk.kbbbk", "..kkk...kkk.", ".kkkkk.kkkkk",
         "kpppppkbbbbb", "kpppppkbbbbb", "kpppppkbbbbb", "kkkkkkkkkkkk", "............", "............"},
        {"............", "....kkkk....", "..kkrrrrkk..", ".krrrwwrrrk.", ".krrwwwwrrk.", "krrrrwwrrrrk",
         "kkkkkkkkkkkk", "kwwwwwwwwwwk", "kkkkkkkkwwwk", "........kkk.", "............", "............"},
    };
    private static final int[] ICON_COLORS = new int[128];
    static {
        ICON_COLORS['k'] = 0xFF202830; ICON_COLORS['w'] = 0xFFF8F8F8; ICON_COLORS['r'] = 0xFFE83838;
        ICON_COLORS['y'] = 0xFFF8D038; ICON_COLORS['o'] = 0xFFF09030; ICON_COLORS['n'] = 0xFFB07038;
        ICON_COLORS['b'] = 0xFF3868D8; ICON_COLORS['l'] = 0xFF98D0F8; ICON_COLORS['g'] = 0xFF48B048;
        ICON_COLORS['s'] = 0xFFA0A8B0; ICON_COLORS['p'] = 0xFFF8A0A8; ICON_COLORS['d'] = 0xFF901818;
    }

    private void drawTabIcon(Canvas c, int i, int x, int y, boolean dim) {
        String[] rows = TAB_ICONS[i];
        for (int r = 0; r < rows.length; r++) {
            for (int k = 0; k < rows[r].length(); k++) {
                char ch = rows[r].charAt(k);
                if (ch == '.') {
                    continue;
                }
                int col = ICON_COLORS[ch];
                fill.setColor(dim ? (col & 0x00FFFFFF) | 0xB0000000 : col);
                c.drawRect(x + k, y + r, x + k + 1, y + r + 1, fill);
            }
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
                pressX = (int) x;   // (a finger on a button: Page.button draws it pressed)
                pressY = (int) y - TOP;
                pressUntil = Long.MAX_VALUE;
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
                    pressUntil = 0;
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
                if (pressUntil != 0) {
                    pressUntil = System.currentTimeMillis() + 90;   // (the press shows a moment after the finger lifts)
                    postInvalidateDelayed(100);
                }
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
                pressUntil = 0;
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
        if (toastTop >= 0 && y >= toastTop && y < h - TABS && menu == null) {
            app.toastAt = 0;   // (a tap on the toast closes it)
            invalidate();
            return;
        }
        if (y < TOP) {
            if (x >= w - 60) {
                sound.play("se_win_open");   // (the game's START menu)
                showMenu(Menu.main(this));
            } else if (over != null) {
                over.back();
            }
            return;
        }
        if (y >= h - TABS && (over == null || !over.fullScreen())) {
            int i = Math.min(TAB_NAMES.length - 1, x / (w / TAB_NAMES.length));
            if (TAB_PAGES[i] != page || over != null) {
                sound.play("se_rg_bag_pocket");   // (as the BAG's pockets)
            }
            select(TAB_PAGES[i]);
            return;
        }
        active().tap(x, y - TOP);
    }

    /** The back button: a menu, a page over the tabs, then the app */
    public boolean back() {
        if (menu != null) {
            menu = menu.parent;
            sound.play("se_select");
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
