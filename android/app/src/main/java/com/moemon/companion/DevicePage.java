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
 * The DEVICE page (docs/COMPANION_APP.md "Pillar 1: the second screen"): the phone takes on the device of the region
 * you're in, and you can switch to any other you own.
 * - POKéNAV (HOENN): its main menu's apps: the live map, CONDITION, MATCH CALL, RIBBONS
 * - POKéTCH (SINNOH): the game's own, live: its LCD comes from the game as it draws it (sinnoh/poketch.c), the finger
 *   is its stylus and the side button changes the app, so every app works here exactly as in the game
 * - POKéGEAR (JOHTO): CLOCK, MAP, PHONE (the game's own calls) and RADIO (the game's own shows)
 * In a battle it switches to battle mode: the MOVE TESTER (your moves against the foes on the field, by the game's
 * type chart) and your party's HP, live; only what the player can see.
 */
public final class DevicePage extends Page {
    public static final int POKETCH = 0, POKENAV = 1, POKEGEAR = 2;
    private static final String[] NAMES = {"POKéTCH", "POKéNAV", "POKéGEAR"};
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
    private static final int PW = 192, PH = 160, SWITCH = 20;
    private final Paint fill = new Paint(), pix = new Paint();
    private final Rect src = new Rect(0, 0, PW, PH), dst = new Rect();
    private Bitmap lcd;
    private JSONObject shown;          // the last frame drawn into lcd
    private boolean watching, touching;
    private long lastMove;
    private int device = -1;           // the device shown (-1: the region's)

    public DevicePage(GameView v) {
        super(v);
        pix.setFilterBitmap(false);
    }

    private boolean has(int d) {
        Model m = v.app.model();
        JSONArray a = m == null ? null : m.save().optJSONArray("devices");
        return a != null && a.optInt(d) == 1;
    }

    /** The device shown: the one picked, else the region's own (HOENN's POKéNAV, SINNOH's POKéTCH, JOHTO's POKéGEAR) */
    public int device() {
        if (device >= 0 && has(device)) {
            return device;
        }
        Model m = v.app.model();
        int region = m == null ? 0 : m.save().optInt("region");
        int[] order = region == 3 ? new int[]{POKETCH, POKENAV, POKEGEAR} : region == 1 ? new int[]{POKEGEAR, POKENAV, POKETCH}
                : region == 2 ? new int[]{POKEGEAR, POKENAV, POKETCH} : new int[]{POKENAV, POKETCH, POKEGEAR};
        for (int d : order) {
            if (has(d)) {
                return d;
            }
        }
        return -1;
    }

    /** Show device D (one the save has) */
    public void show(int d) {
        device = d;
    }

    private boolean linked() {
        return v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
    }

    private void watch(boolean on) {
        if (v.app.client == null) {
            return;
        }
        try {
            v.app.client.send(on ? v.app.watchOn() : new JSONObject().put("t", "watch").put("on", 0));
        } catch (org.json.JSONException ignored) {
        }
        watching = on;
        v.app.watching = on;
    }

    @Override
    public void shown() {
        if (device() == POKETCH && v.app.battle == null) {
            watch(true);
        }
    }

    @Override
    public void hidden() {
        if (watching) {
            watch(false);
        }
        radioOn = false;
    }

    private static final int TOP = 20 + SWITCH;

    @Override
    public void draw(Canvas c, int w, int h) {
        int d = device();
        JSONObject battle = v.app.battle;
        if (battle != null) {
            if (watching) {
                watch(false);
            }
            drawBattle(c, w, h, battle);
            banner(c, w, "BATTLE", "LIVE");
            return;
        }
        if (d == POKETCH && !watching && linked()) {
            watch(true);
        } else if (d != POKETCH && watching) {
            watch(false);
        }
        if (d < 0) {
            banner(c, w, "DEVICE", "");
            art.window(c, 0, 12, 32, w - 24, 72);
            int y = 32;
            for (String l : wrap(v.app.model() == null ? "Nothing from the game yet."
                    : "Your devices show here once you have them: the POKéNAV from DEVON CORP. in RUSTBORO CITY, "
                    + "the POKéTCH from the POKéTCH Co. in JUBILIFE CITY, and the POKéGEAR from MOM in NEW BARK TOWN.",
                    w - 32)) {
                art.text(c, l, 16, y);
                y += 14;
            }
            return;
        }
        if (d == POKETCH) {
            drawPoketch(c, w, h);
        } else if (d == POKENAV) {
            drawPokenav(c, w, h);
        } else {
            drawPokegear(c, w, h);
        }
        drawSwitcher(c, w, d);
        banner(c, w, NAMES[d], linked() ? "LIVE" : "");
    }

    // the devices owned, as chips under the banner; the one shown lit
    private void drawSwitcher(Canvas c, int w, int d) {
        fill.setColor(BANNER_DARK);
        c.drawRect(0, 20, w, TOP, fill);
        int x = 6;
        for (int k : new int[]{POKENAV, POKETCH, POKEGEAR}) {
            if (!has(k)) {
                continue;
            }
            int cw = art.width(NAMES[k]) + 12;
            if (k == d) {
                art.window(c, 0, x + 2, 24, cw - 4, 12);
            }
            art.text(c, NAMES[k], x + 6, 22, k == d ? GameArt.TEXT : GameArt.WHITE_TEXT, k == d ? GameArt.SHADOW : 0xFF202850);
            x += cw + 4;
        }
    }

    private boolean tapSwitcher(int x) {
        int cx = 6;
        for (int k : new int[]{POKENAV, POKETCH, POKEGEAR}) {
            if (!has(k)) {
                continue;
            }
            int cw = art.width(NAMES[k]) + 12;
            if (x >= cx && x < cx + cw + 4) {
                if (k != device()) {
                    device = k;
                    radioOn = false;
                    v.buzz(false);
                }
                return true;
            }
            cx += cw + 4;
        }
        return false;
    }

    // ================================================================================================ POKéTCH

    // the LCD's place on the page: as wide as fits, a whole multiple of its pixels
    private int lcdScale(int w) {
        return Math.max(1, (w - 40) / PW);
    }

    private int lcdX(int w) {
        return (w - PW * lcdScale(w) - 16) / 2;
    }

    private static final int LCD_Y = TOP + 16;

    private void drawPoketch(Canvas c, int w, int h) {
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
        // HIDE ON PC: the game's own POKéTCH put away while the phone shows it
        hideY = ty + 6;
        button(c, "HIDE ON PC: " + (v.app.hideOnPc() ? "ON" : "OFF"), 12, hideY, w - 24, true);
    }

    private int hideY = -100;

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
        if (device() == POKETCH && v.app.battle == null) {
            int[] p = onLcd(x, y);
            if (p != null && watching) {
                touching = true;
                touch(p, true);
            }
        } else if (device() == POKEGEAR && card == CARD_RADIO && stepAt(x, y) == 0 && inDial(x, y)) {
            tuning = true;
            tuneTo(x);
        }
    }

    @Override
    public boolean dragStart(int x, int y) {
        return touching || tuning;   // (a finger on the LCD drags the stylus, on the dial the needle)
    }

    @Override
    public void dragMove(int x, int y) {
        if (tuning) {
            tuneTo(x);
            return;
        }
        int[] p = onLcd(x, y);
        long now = System.currentTimeMillis();
        if (touching && p != null && now - lastMove > 30) {
            lastMove = now;
            touch(p, true);
        }
    }

    @Override
    public void dragEnd(int x, int y) {
        tuning = false;
        release(x, y);
    }

    private void release(int x, int y) {
        if (touching) {
            int[] p = onLcd(x, y);
            touch(p != null ? p : new int[]{0, 0}, false);
            touching = false;
        }
    }

    // ================================================================================================ POKéNAV

    private static final String[] NAV_APPS = {"MAP", "CONDITION", "MATCH CALL", "RIBBONS"};
    private static final int[] NAV_COLORS = {0xFF48A848, 0xFFD85898, 0xFF4878D8, 0xFFE8A030};

    private int navTop() {
        return TOP + 12;
    }

    private void drawPokenav(Canvas c, int w, int h) {
        // Emerald's POKéNAV: a silver-blue shell around the screen, the main menu's options as its pills
        int x0 = 10, y0 = navTop(), x1 = w - 10, y1 = Math.min(h - 10, y0 + 4 * 44 + 64);
        fill.setColor(0xFF384870);
        c.drawRect(x0, y0, x1, y1, fill);
        fill.setColor(0xFFA8B8D8);
        c.drawRect(x0 + 2, y0 + 2, x1 - 2, y1 - 2, fill);
        fill.setColor(0xFFD8E0F0);
        c.drawRect(x0 + 2, y0 + 2, x1 - 2, y0 + 6, fill);
        // the screen
        int sx0 = x0 + 10, sy0 = y0 + 12, sx1 = x1 - 10, sy1 = y1 - 12;
        fill.setColor(0xFF101828);
        c.drawRect(sx0 - 2, sy0 - 2, sx1 + 2, sy1 + 2, fill);
        fill.setColor(0xFF6890C8);
        c.drawRect(sx0, sy0, sx1, sy1, fill);
        fill.setColor(0xFF78A0D0);
        for (int y = sy0; y < sy1; y += 8) {
            c.drawRect(sx0, y, sx1, y + 4, fill);
        }
        // the clock line, as the POKéNAV's top bar
        JSONObject pl = v.app.model() == null ? null : v.app.model().snap.optJSONObject("place");
        JSONArray t = pl == null ? null : pl.optJSONArray("time");
        String clock = t == null ? "" : String.format(java.util.Locale.US, "%02d:%02d", t.optInt(1), t.optInt(2));
        art.text(c, "MAIN MENU", sx0 + 6, sy0 + 2, GameArt.WHITE_TEXT, 0xFF283868);
        art.text(c, clock, sx1 - 6 - art.width(clock), sy0 + 2, GameArt.WHITE_TEXT, 0xFF283868);
        for (int i = 0; i < NAV_APPS.length; i++) {
            int by = sy0 + 22 + i * 44, bx0 = sx0 + 10, bx1 = sx1 - 10;
            String label = i == 0 ? regionName() + " MAP" : NAV_APPS[i];
            // a pill: dark rim, the app's color, a shine
            fill.setColor(0xFF283868);
            c.drawRect(bx0 + 2, by, bx1 - 2, by + 36, fill);
            c.drawRect(bx0, by + 2, bx1, by + 34, fill);
            fill.setColor(0xFFF0F4F8);
            c.drawRect(bx0 + 2, by + 2, bx1 - 2, by + 34, fill);
            fill.setColor(NAV_COLORS[i]);
            c.drawRect(bx0 + 4, by + 4, bx0 + 34, by + 32, fill);
            fill.setColor(0x60FFFFFF);
            c.drawRect(bx0 + 4, by + 4, bx0 + 34, by + 12, fill);
            navIcon(c, i, bx0 + 10, by + 9);
            art.text(c, label, bx0 + 42, by + 10, GameArt.TEXT, GameArt.SHADOW);
            art.text(c, ">", bx1 - 14, by + 10, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
    }

    private void navIcon(Canvas c, int i, int x, int y) {
        fill.setColor(0xFFF8F8F8);
        switch (i) {
            case 0:   // a map's square with the you-are-here dot
                c.drawRect(x, y, x + 18, y + 18, fill);
                fill.setColor(0xFF48A848);
                c.drawRect(x + 2, y + 2, x + 16, y + 16, fill);
                fill.setColor(0xFFE83020);
                c.drawRect(x + 7, y + 7, x + 11, y + 11, fill);
                break;
            case 1:   // the pentagon's points
                c.drawRect(x + 7, y, x + 11, y + 4, fill);
                c.drawRect(x, y + 7, x + 4, y + 11, fill);
                c.drawRect(x + 14, y + 7, x + 18, y + 11, fill);
                c.drawRect(x + 3, y + 14, x + 7, y + 18, fill);
                c.drawRect(x + 11, y + 14, x + 15, y + 18, fill);
                break;
            case 2:   // a handset
                c.drawRect(x + 2, y + 2, x + 16, y + 6, fill);
                c.drawRect(x + 2, y + 6, x + 6, y + 16, fill);
                c.drawRect(x + 12, y + 6, x + 16, y + 10, fill);
                break;
            default:  // a ribbon
                c.drawRect(x + 5, y, x + 13, y + 8, fill);
                c.drawRect(x + 3, y + 8, x + 7, y + 18, fill);
                c.drawRect(x + 11, y + 8, x + 15, y + 18, fill);
        }
    }

    private String regionName() {
        Model m = v.app.model();
        int r = m == null ? 0 : m.save().optInt("region");
        return new String[]{"HOENN", "JOHTO", "KANTO", "SINNOH"}[Math.max(0, Math.min(3, r))];
    }

    private void tapPokenav(int x, int y) {
        int w = v.width(), sx0 = 20, sy0 = navTop() + 12;
        for (int i = 0; i < NAV_APPS.length; i++) {
            int by = sy0 + 22 + i * 44;
            if (y >= by && y < by + 36 && x >= sx0 + 10 && x < w - 30) {
                v.buzz(false);
                switch (i) {
                    case 0: v.showOver(new MapPage(v, regionName() + " MAP")); break;
                    case 1: v.showOver(new ConditionPage(v)); break;
                    case 2: v.showOver(new CallsPage(v)); break;
                    default: v.showOver(new RibbonsPage(v));
                }
                return;
            }
        }
    }

    // =============================================================================================== POKéGEAR

    private static final int CARD_CLOCK = 0, CARD_MAP = 1, CARD_PHONE = 2, CARD_RADIO = 3;
    private static final String[] CARDS = {"CLOCK", "MAP", "PHONE", "RADIO"};
    private static final String[] DAYS = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    private static final int LCD_BG = 0xFFA8D088, LCD_INK = 0xFF203818, LCD_MID = 0xFF78A060;
    private int card, freq = 9, callPage;
    private boolean tuning, radioOn, calling;
    private long radioAsked, calledAt;
    private final String[] radioLines = {"", ""};
    private String radioName = "";

    private int gearX0() {
        return 12;
    }

    private int gearY0() {
        return TOP + 10;
    }

    private void drawPokegear(Canvas c, int w, int h) {
        int x0 = gearX0(), y0 = gearY0(), x1 = w - 12, y1 = Math.min(h - 12, y0 + 300);
        // Crystal's orange body
        fill.setColor(0xFF803010);
        c.drawRect(x0, y0, x1, y1, fill);
        fill.setColor(0xFFF07828);
        c.drawRect(x0 + 2, y0 + 2, x1 - 2, y1 - 2, fill);
        fill.setColor(0xFFF8A868);
        c.drawRect(x0 + 2, y0 + 2, x1 - 2, y0 + 5, fill);
        // the LCD
        int lx0 = x0 + 10, ly0 = y0 + 10, lx1 = x1 - 10, ly1 = y1 - 46;
        fill.setColor(0xFF304020);
        c.drawRect(lx0 - 2, ly0 - 2, lx1 + 2, ly1 + 2, fill);
        fill.setColor(LCD_BG);
        c.drawRect(lx0, ly0, lx1, ly1, fill);
        // the status bar: the cards as icons (the open one inverted), the time
        JSONObject pl = v.app.model() == null ? null : v.app.model().snap.optJSONObject("place");
        JSONArray t = pl == null ? null : pl.optJSONArray("time");
        int cw = (lx1 - lx0) / CARDS.length;
        for (int i = 0; i < CARDS.length; i++) {
            int cx = lx0 + i * cw;
            if (i == card) {
                fill.setColor(LCD_INK);
                c.drawRect(cx + 1, ly0 + 1, cx + cw - 1, ly0 + 15, fill);
            }
            art.text(c, CARDS[i], cx + (cw - art.width(CARDS[i])) / 2, ly0, i == card ? LCD_BG : LCD_INK, 0);
        }
        fill.setColor(LCD_INK);
        c.drawRect(lx0, ly0 + 16, lx1, ly0 + 17, fill);
        int cy = ly0 + 22;
        switch (card) {
            case CARD_CLOCK: {
                String hm = t == null ? "--:--" : String.format(java.util.Locale.US, "%2d:%02d",
                        t.optInt(1) % 12 == 0 ? 12 : t.optInt(1) % 12, t.optInt(2));
                int dx = (lx0 + lx1) / 2 - hm.length() * 13;
                for (int i = 0; i < hm.length(); i++) {
                    segDigit(c, hm.charAt(i), dx + i * 26, cy + 14, 20, 40);
                }
                String ampm = t == null ? "" : t.optInt(1) < 12 ? "AM" : "PM";
                art.text(c, ampm, lx1 - 26, cy + 40, LCD_INK, 0);
                String day = t == null ? "" : DAYS[Math.max(0, Math.min(6, t.optInt(0)))];
                art.text(c, day, (lx0 + lx1 - art.width(day)) / 2, cy + 70, LCD_INK, 0);
                break;
            }
            case CARD_MAP:
                art.text(c, "Tap to see the", lx0 + 8, cy + 10, LCD_INK, 0);
                art.text(c, "TOWN MAP.", lx0 + 8, cy + 26, LCD_INK, 0);
                break;
            case CARD_PHONE:
                drawPhone(c, lx0, cy, lx1, ly1);
                break;
            default:
                drawRadio(c, lx0, cy, lx1, ly1);
        }
        // the soft keys under the LCD
        String left = card == CARD_PHONE && calling ? "BACK" : "", right = card == CARD_MAP ? "OPEN" : "";
        art.text(c, left, x0 + 14, y1 - 34, GameArt.WHITE_TEXT, 0xFF803010);
        art.text(c, right, x1 - 14 - art.width(right), y1 - 34, GameArt.WHITE_TEXT, 0xFF803010);
        fill.setColor(0xFFB05018);
        c.drawRect((x0 + x1) / 2 - 20, y1 - 32, (x0 + x1) / 2 + 20, y1 - 18, fill);
    }

    // a seven-segment digit as the POKéGEAR's clock draws them
    private void segDigit(Canvas c, char ch, int x, int y, int dw, int dh) {
        if (ch == ':') {
            fill.setColor(LCD_INK);
            c.drawRect(x + dw / 2 - 2, y + dh / 3 - 2, x + dw / 2 + 2, y + dh / 3 + 2, fill);
            c.drawRect(x + dw / 2 - 2, y + 2 * dh / 3 - 2, x + dw / 2 + 2, y + 2 * dh / 3 + 2, fill);
            return;
        }
        if (ch < '0' || ch > '9') {
            return;
        }
        int[] segs = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
        int s = segs[ch - '0'], k = 4, half = dh / 2;
        for (int i = 0; i < 7; i++) {
            fill.setColor((s & (1 << i)) != 0 ? LCD_INK : LCD_MID & 0x40FFFFFF);
            switch (i) {
                case 0: c.drawRect(x + k, y, x + dw - k, y + k, fill); break;
                case 1: c.drawRect(x + dw - k, y + k, x + dw, y + half, fill); break;
                case 2: c.drawRect(x + dw - k, y + half, x + dw, y + dh - k, fill); break;
                case 3: c.drawRect(x + k, y + dh - k, x + dw - k, y + dh, fill); break;
                case 4: c.drawRect(x, y + half, x + k, y + dh - k, fill); break;
                case 5: c.drawRect(x, y + k, x + k, y + half, fill); break;
                default: c.drawRect(x + k, y + half - k / 2, x + dw - k, y + half + k / 2, fill);
            }
        }
    }

    private void drawPhone(Canvas c, int lx0, int cy, int lx1, int ly1) {
        JSONObject g = v.app.gear;
        if (!calling) {
            String[] who = {"MOM", "PROF.ELM"};
            for (int i = 0; i < who.length; i++) {
                int y = cy + 4 + i * 20;
                art.text(c, who[i] + ":", lx0 + 8, y, LCD_INK, 0);
            }
            for (int i = 2; i < 5; i++) {
                art.text(c, "----------:", lx0 + 8, cy + 4 + i * 20, LCD_MID, 0);
            }
            art.text(c, "Whom do you want to call?", lx0 + 6, ly1 - 18, LCD_INK, 0);
            return;
        }
        boolean fresh = g != null && v.app.gearAt >= calledAt && (g.has("call") || g.has("answer"));
        if (!fresh) {
            art.text(c, "CALLING...", lx0 + 8, cy + 6, LCD_INK, 0);
            v.postInvalidateDelayed(300);
            return;
        }
        if (g.optInt("busy") == 1) {
            art.text(c, "The game is busy.", lx0 + 8, cy + 6, LCD_INK, 0);
            return;
        }
        JSONArray lines = g.optJSONArray("lines");
        int pages = lines == null ? 0 : (lines.length() + 1) / 2;
        callPage = Math.min(callPage, Math.max(0, pages - 1));
        for (int i = 0; i < 2 && lines != null; i++) {
            int k = callPage * 2 + i;
            if (k < lines.length()) {
                art.textHex(c, lines.optString(k), lx0 + 6, cy + 6 + i * 16, LCD_INK, 0);
            }
        }
        boolean last = callPage >= pages - 1;
        if (last && g.optInt("ask") == 1) {
            art.text(c, "YES", lx0 + 12, ly1 - 20, LCD_INK, 0);
            art.text(c, "NO", lx1 - 12 - art.width("NO"), ly1 - 20, LCD_INK, 0);
        } else {
            art.text(c, last ? "Tap to hang up." : "Tap for more.", lx0 + 6, ly1 - 18, LCD_MID, 0);
        }
    }

    private void drawRadio(Canvas c, int lx0, int cy, int lx1, int ly1) {
        // the black FM dial with its red needle, 0 to 20.5
        int dx0 = lx0 + 8, dx1 = lx1 - 8, dy = cy + 6;
        fill.setColor(0xFF181818);
        c.drawRect(dx0, dy, dx1, dy + 26, fill);
        for (int k = 0; k <= 20; k++) {
            int x = dx0 + 4 + (dx1 - dx0 - 8) * k / 20;
            fill.setColor(0xFFC0C0C0);
            c.drawRect(x, dy + (k % 5 == 0 ? 14 : 18), x + 1, dy + 24, fill);
            if (k % 5 == 0) {
                String n = "" + k;
                art.text(c, n, x - art.width(n) / 2, dy - 2, 0xFFE0E0E0, 0);
            }
        }
        int nx = dx0 + 4 + (dx1 - dx0 - 8) * freq / 40;
        fill.setColor(0xFFE82820);
        c.drawRect(nx - 1, dy + 2, nx + 2, dy + 26, fill);
        String f = (freq / 2) + "." + (freq % 2 == 0 ? "0" : "5");
        art.text(c, f, lx1 - 10 - art.width(f), dy + 30, LCD_INK, 0);
        // fine tuning half a step at a time (a finger on the dial is wider than its ticks)
        for (int k = 0; k < 2; k++) {
            int bx = stepX(lx0, k);
            fill.setColor(LCD_INK);
            c.drawRect(bx, dy + 30, bx + 18, dy + 45, fill);
            fill.setColor(stepFlash == k + 1 && System.currentTimeMillis() - stepTime < 150 ? LCD_MID : 0xFFC8E0A0);
            c.drawRect(bx + 1, dy + 31, bx + 17, dy + 44, fill);
            String a = k == 0 ? "<" : ">";
            art.text(c, a, bx + 9 - art.width(a) / 2, dy + 29, LCD_INK, 0);
        }
        // the station and its show, a line at a time as the game's radio prints them
        long now = System.currentTimeMillis();
        if (radioOn && linked() && now - radioAsked > 1500) {
            radioAsked = now;
            try {
                v.app.client.send(new JSONObject().put("t", "gear").put("radio", freq));
            } catch (org.json.JSONException ignored) {
            }
        }
        JSONObject g = v.app.gear;
        if (g != null && g.has("radio") && g.optInt("radio") == freq && g != lastRadio) {
            lastRadio = g;
            radioName = g.optString("name");
            String line = g.optString("line");
            if (!line.isEmpty() && !line.equals("FF")) {
                radioLines[0] = radioLines[1];
                radioLines[1] = line;
            }
        }
        if (!radioOn) {
            art.text(c, "Tap the dial to tune in.", lx0 + 6, dy + 50, LCD_INK, 0);
        } else {
            art.textHex(c, radioName.isEmpty() ? art.toHex("- - - - -") : radioName, lx0 + 6, dy + 46, LCD_INK, 0);
            art.textHex(c, radioLines[0], lx0 + 6, dy + 66, LCD_MID, 0);
            art.textHex(c, radioLines[1], lx0 + 6, dy + 82, LCD_INK, 0);
            v.postInvalidateDelayed(500);
        }
    }

    private JSONObject lastRadio;
    private int stepFlash;
    private long stepTime;

    private static int stepX(int lx0, int k) {
        return lx0 + 8 + k * 24;
    }

    /** the fine-tuning button under the dial at x, y: -1 down, 1 up, 0 none */
    private int stepAt(int x, int y) {
        int lx0 = gearX0() + 10, dy = gearY0() + 10 + 22 + 6;
        if (y < dy + 28 || y >= dy + 48) {
            return 0;
        }
        for (int k = 0; k < 2; k++) {
            int bx = stepX(lx0, k);
            if (x >= bx - 3 && x < bx + 21) {
                return k == 0 ? -1 : 1;
            }
        }
        return 0;
    }

    private void step(int d) {
        int f = Math.max(0, Math.min(41, freq + d));
        stepFlash = d < 0 ? 1 : 2;
        stepTime = System.currentTimeMillis();
        if (f != freq) {
            freq = f;
            radioLines[0] = radioLines[1] = "";
            radioName = "";
            radioAsked = 0;
            v.buzz(false);
        }
        radioOn = true;
        v.postInvalidateDelayed(160);
    }

    private boolean inDial(int x, int y) {
        int lx0 = gearX0() + 10, lx1 = v.width() - 22, dy = gearY0() + 10 + 22 + 6;
        return x >= lx0 && x < lx1 && y >= dy - 6 && y < dy + 34;
    }

    private void tuneTo(int x) {
        int dx0 = gearX0() + 18, dx1 = v.width() - 30;
        int f = Math.max(0, Math.min(41, Math.round((x - dx0 - 4) * 40f / Math.max(1, dx1 - dx0 - 8))));
        if (f != freq) {
            freq = f;
            radioLines[0] = radioLines[1] = "";
            radioName = "";
            radioAsked = 0;
            v.buzz(false);
        }
        radioOn = true;
    }

    private void tapPokegear(int x, int y) {
        int w = v.width(), lx0 = gearX0() + 10, lx1 = w - 22, ly0 = gearY0() + 10, cw = (lx1 - lx0) / CARDS.length;
        if (y >= ly0 && y < ly0 + 16) {
            int i = (x - lx0) / cw;
            if (i >= 0 && i < CARDS.length && i != card) {
                card = i;
                calling = false;
                radioOn = i == CARD_RADIO && radioOn;
                v.buzz(false);
            }
            return;
        }
        int cy = ly0 + 22;
        switch (card) {
            case CARD_MAP:
                v.showOver(new MapPage(v, "TOWN MAP"));
                break;
            case CARD_PHONE:
                if (!calling) {
                    int i = (y - cy - 4) / 20;
                    if (i == 0 || i == 1) {
                        gearSend("call", i);
                        calling = true;
                        callPage = 0;
                    }
                } else {
                    JSONObject g = v.app.gear;
                    JSONArray lines = g == null ? null : g.optJSONArray("lines");
                    int pages = lines == null ? 0 : (lines.length() + 1) / 2;
                    boolean last = callPage >= pages - 1;
                    if (g != null && last && g.optInt("ask") == 1) {
                        gearSend("answer", x < w / 2 ? 1 : 0);
                        callPage = 0;
                    } else if (!last) {
                        callPage++;
                    } else {
                        calling = false;
                    }
                }
                break;
            case CARD_RADIO:
                if (stepAt(x, y) != 0) {
                    step(stepAt(x, y));
                } else if (inDial(x, y)) {
                    tuneTo(x);
                } else {
                    radioOn = !radioOn;
                }
                break;
            default:
                break;
        }
    }

    private void gearSend(String what, int value) {
        if (!linked()) {
            v.message("Link with the game to use the POKéGEAR.");
            return;
        }
        calledAt = System.currentTimeMillis();
        try {
            v.app.client.send(new JSONObject().put("t", "gear").put(what, value));
        } catch (org.json.JSONException ignored) {
        }
        v.buzz(false);
    }

    // ============================================================================================= battle mode

    private void drawBattle(Canvas c, int w, int h, JSONObject b) {
        // the device's colors around the battle: the POKéNAV's blue, the POKéTCH's red, the POKéGEAR's orange
        int d = device(), body = d == POKETCH ? 0xFFB83840 : d == POKEGEAR ? 0xFFD06020 : 0xFF3858A0;
        fill.setColor(body);
        c.drawRect(0, 20, w, h, fill);
        JSONArray mons = b.optJSONArray("mons");
        java.util.List<JSONObject> foes = new java.util.ArrayList<>(), mine = new java.util.ArrayList<>();
        for (int i = 0; mons != null && i < mons.length(); i++) {
            JSONObject m = mons.optJSONObject(i);
            if (m.optInt("side") == 1) {
                foes.add(m);
            } else if (m.optInt("me") == 1) {
                mine.add(m);
            }
        }
        int y = 26;
        // the foes: name, level, HP as the bar shows it, their types now
        for (JSONObject f : foes) {
            art.window(c, 0, 12, y + 4, w - 24, 26);
            art.icon(c, f.optInt("s"), 10, y - 2, GameArt.ICON_PLAIN);
            art.text(c, art.name("species", f.optInt("s")), 46, y + 2, GameArt.TEXT, GameArt.SHADOW);
            String lv = "Lv" + f.optInt("l");
            art.text(c, lv, w - 18 - art.width(lv), y + 2, GameArt.TEXT, GameArt.SHADOW);
            JSONArray ty = f.optJSONArray("ty");
            if (ty != null) {
                art.typeIcon(c, ty.optInt(0), 46, y + 17);
                if (ty.optInt(1) != ty.optInt(0)) {
                    art.typeIcon(c, ty.optInt(1), 80, y + 17);
                }
            }
            hpBar(c, w - 92, y + 20, 72, f.optInt("pct"), 100);
            y += 36;
        }
        y += 4;
        // yours, and the MOVE TESTER: each move against each foe, by the game's type chart
        for (JSONObject m : mine) {
            JSONArray moves = m.optJSONArray("m"), pp = m.optJSONArray("pp"), ty = m.optJSONArray("ty");
            int rows = 0;
            for (int k = 0; moves != null && k < 4; k++) {
                rows += moves.optInt(k) != 0 ? 1 : 0;
            }
            int boxH = 26 + rows * 30;
            art.window(c, 0, 12, y + 4, w - 24, boxH);
            art.icon(c, m.optInt("s"), 10, y - 2, GameArt.ICON_PLAIN);
            art.text(c, art.name("species", m.optInt("s")), 46, y + 2, GameArt.BLUE_TEXT, GameArt.SHADOW);
            String hp = m.optInt("hp") + "/" + m.optInt("mhp");
            art.text(c, hp, w - 18 - art.width(hp), y + 2, GameArt.TEXT, GameArt.SHADOW);
            hpBar(c, w - 92, y + 18, 72, m.optInt("hp"), Math.max(1, m.optInt("mhp")));
            int my = y + 26;
            for (int k = 0; moves != null && k < 4; k++) {
                int mv = moves.optInt(k);
                if (mv == 0) {
                    continue;
                }
                JSONObject md = art.move(mv);
                int type = md.optInt("t"), power = md.optInt("p");
                art.typeIcon(c, type, 16, my + 4);
                art.text(c, art.name("moves", mv), 52, my, GameArt.TEXT, GameArt.SHADOW);
                String ppS = "PP " + (pp == null ? 0 : pp.optInt(k));
                art.text(c, ppS, w - 18 - art.width(ppS), my, GameArt.GRAY_TEXT, GameArt.SHADOW);
                // the verdict against each foe (a status move: none)
                int fx = 52;
                boolean stab = ty != null && (ty.optInt(0) == type || ty.optInt(1) == type);
                for (JSONObject f : foes) {
                    if (power == 0) {
                        break;
                    }
                    JSONArray ft = f.optJSONArray("ty");
                    int e = art.effectiveness(type, ft == null ? 0 : ft.optInt(0), ft == null ? 0 : ft.optInt(1));
                    String word = e == 0 ? "NO EFFECT" : e > 10 ? "x" + (e / 10) + " SUPER" : e < 10 ? "x0." + (e == 5 ? "5" : "25") : "x1";
                    int col = e == 0 ? GameArt.GRAY_TEXT : e > 10 ? 0xFF209020 : e < 10 ? GameArt.RED_TEXT : GameArt.TEXT;
                    fx += art.text(c, word, fx, my + 13, col, GameArt.SHADOW) + 8;
                }
                if (power > 0 && stab) {
                    art.text(c, "STAB", w - 18 - art.width("STAB"), my + 13, GameArt.BLUE_TEXT, GameArt.SHADOW);
                }
                my += 30;
            }
            y += boxH + 14;
        }
        // the party's HP
        JSONArray party = b.optJSONArray("party");
        if (party != null && party.length() > 0) {
            int per = Math.max(1, Math.min(6, (w - 24) / 76)), cw = (w - 24) / per;
            for (int i = 0; i < party.length(); i++) {
                JSONArray p = party.optJSONArray(i);
                int x = 12 + (i % per) * cw, py = y + (i / per) * 40;
                art.window(c, 0, x + 4, py + 6, cw - 8, 28);
                art.icon(c, p.optInt(4) == 1 ? art.data.optInt("egg") : p.optInt(0), x, py,
                        p.optInt(1) == 0 && p.optInt(4) != 1 ? GameArt.ICON_GRAY : GameArt.ICON_PLAIN);
                if (p.optInt(4) != 1) {
                    hpBar(c, x + 34, py + 22, cw - 42, p.optInt(1), Math.max(1, p.optInt(2)));
                }
            }
        }
    }

    // an HP bar as the game's: green, then yellow under half, red under a fifth
    private void hpBar(Canvas c, int x, int y, int w, int hp, int max) {
        fill.setColor(0xFF404040);
        c.drawRect(x, y, x + w, y + 5, fill);
        fill.setColor(0xFF586858);
        c.drawRect(x + 1, y + 1, x + w - 1, y + 4, fill);
        int f = (int) ((long) (w - 2) * Math.max(0, hp) / Math.max(1, max));
        fill.setColor(hp * 5 <= max ? 0xFFF85838 : hp * 2 <= max ? 0xFFF8D038 : 0xFF70F8A8);
        c.drawRect(x + 1, y + 1, x + 1 + f, y + 4, fill);
    }

    // ================================================================================================ touch

    @Override
    public void tap(int x, int y) {
        if (touching) {
            release(x, y);
            return;
        }
        if (v.app.battle != null) {
            return;
        }
        if (y >= 20 && y < TOP) {
            tapSwitcher(x);
            return;
        }
        int d = device();
        if (d == POKENAV) {
            tapPokenav(x, y);
        } else if (d == POKEGEAR) {
            tapPokegear(x, y);
        } else if (d == POKETCH) {
            if (y >= hideY && y < hideY + 24) {
                boolean on = !v.app.hideOnPc();
                v.app.context.getSharedPreferences("moemon", 0).edit().putBoolean("hideOnPc", on).apply();
                watch(true);
                v.buzz(false);
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
    }

    @Override
    public void swipe(int dir) {
        if (device() == POKETCH && v.app.client != null && watching) {
            try {
                v.app.client.send(new JSONObject().put("t", "watchapp").put("dir", dir));
            } catch (org.json.JSONException ignored) {
            }
        } else if (device() == POKEGEAR) {
            card = (card + dir + CARDS.length) % CARDS.length;
            calling = false;
        }
    }
}
