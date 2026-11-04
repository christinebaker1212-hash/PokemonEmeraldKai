package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.Random;

/**
 * The MOEGATCHA on the phone ({"t":"gatcha"} and the "pull" order: mp_phone_kai.c, mp_gatcha.c MP_Gatcha_PhonePull):
 * the game rolls the prizes exactly as its own reel does (pity, the 10-pull's R or better, random Moemon and ball
 * pieces) and gives them; this page plays a reel of its own to show them: cells race past, slow and stop on the
 * pull's best prize (SUPER RARE and up flash), then every prize of the pull in a list.
 * Banners: < > (STANDARD, PICKUP, TRAINING, POKé BALLS); the free pulls (the daily one on each banner, the weekly 10 on
 * STANDARD), a TICKET, one pull or ten.
 */
public class GatchaPage extends Page {
    static final String[] RARITY = {"N", "U", "R", "SR", "SSR", "UR", "EX"};
    static final int[] RARITY_COLOR = {0xFF909890, 0xFF48A848, 0xFF4878E0, 0xFFA048D8, 0xFFE8A818, 0xFFE83848,
            0xFF202020};
    private static final int[] BANNER_COLOR = {0xFFD89818, 0xFFE05890, 0xFF38A858, 0xFF3870D0};
    private static final int[] FILLER = {4, 3, 2, 13, 14, 19, 22, 26, 68, 45, 46, 47, 48, 49, 50, 63, 64, 65, 1, 21};
    private static final int CELL = 32, SPIN_MS = 2600;
    private static final int HOW_FREE10 = 0, HOW_FREE1 = 1, HOW_TICKET = 2, HOW_ONE = 3, HOW_TEN = 4;

    private final Page parent;
    private final Paint fill = new Paint();
    private final Random rnd = new Random();
    private int banner;
    private long asked, sentAt, stopAt;
    private boolean waiting;          // a pull asked for, its prizes not here yet
    private JSONObject shown;         // the pull being shown (its prizes)
    private long seenPullAt;
    private final int[] reel = new int[64];
    private int scrollList;

    public GatchaPage(GameView v, Page parent) {
        super(v);
        this.parent = parent;
        for (int i = 0; i < reel.length; i++) {
            reel[i] = FILLER[rnd.nextInt(FILLER.length)];
        }
        seenPullAt = v.app.lastPullAt;
    }

    @Override
    public void shown() {
        ask();
    }

    private void ask() {
        if (v.app.client != null && System.currentTimeMillis() - asked > 3000) {
            asked = System.currentTimeMillis();
            try {
                v.app.client.send(new JSONObject().put("t", "gatcha"));
            } catch (org.json.JSONException ignored) {
            }
        }
    }

    private JSONArray banners() {
        JSONObject g = v.app.gatcha;
        return g == null ? null : g.optJSONArray("banners");
    }

    private JSONObject kai() {
        return v.app.model() == null ? null : v.app.model().snap.optJSONObject("kai");
    }

    // ----------------------------------------------------------------------------------------------- drawing

    @Override
    public void draw(Canvas c, int w, int h) {
        // a pull's prizes arrived: the reel slows down onto the best
        if (waiting && v.app.lastPullAt != seenPullAt && v.app.lastPull != null) {
            seenPullAt = v.app.lastPullAt;
            shown = v.app.lastPull;
            waiting = false;
            // (from where the reel is now, slowing over SPIN_MS onto the stop cell, a lap or so on)
            long now = System.currentTimeMillis();
            stopAt = now + SPIN_MS;
            startPos = (now - sentAt) / 45f;
            int L = reel.length;
            targetPos = L / 2f + L * (float) Math.ceil((startPos + 20 - L / 2f) / L);
            reel[L / 2] = -1;   // (the stop cell: the best prize)
            ask();
        } else if (waiting && System.currentTimeMillis() - sentAt > 15000) {
            waiting = false;   // (refused, or the game didn't answer: the toast says why)
        }
        int color = BANNER_COLOR[Math.max(0, Math.min(3, banner))];
        // stripes in the banner's color
        fill.setColor(0xFF182030);
        c.drawRect(0, 0, w, h, fill);
        fill.setColor((color & 0x00FFFFFF) | 0x30000000);
        long t = System.currentTimeMillis();
        int off = (int) ((t / 40) % 24);
        for (int x = -h; x < w + 24; x += 24) {
            for (int k = 0; k < 8; k++) {
                c.drawLine(x + k + off, 0, x + k + off + h, h, fill);
            }
        }
        banner(c, w, "MOEGATCHA", "");
        JSONArray bs = banners();
        JSONObject kai = kai();
        String name = bs == null ? "..." : bs.optJSONArray(banner).optString(0).replace("POKE", "POKé");
        // the banner's name on a plate, its tagline under
        fill.setColor(color);
        c.drawRect(8, 24, w - 8, 44, fill);
        fill.setColor(0x40FFFFFF);
        c.drawRect(8, 24, w - 8, 28, fill);
        art.text(c, "<", 14, 26, GameArt.WHITE_TEXT, 0xFF202020);
        art.text(c, ">", w - 22, 26, GameArt.WHITE_TEXT, 0xFF202020);
        art.text(c, name, (w - art.width(name)) / 2, 26, GameArt.WHITE_TEXT, 0xFF202020);
        String tag = bs == null ? (v.app.client == null ? "Link with your game to pull." : "Asking your game...")
                : bs.optJSONArray(banner).optString(1);
        art.text(c, tag, (w - art.width(tag)) / 2, 46, 0xFFE0E8F0, 0xFF101820);
        drawReel(c, w, 66);
        int y = 66 + CELL + 30;
        if (shown != null && !spinning()) {
            drawPrizes(c, w, y, h);
        } else {
            drawStock(c, w, y, kai, bs);
        }
        if (spinning() || waiting) {
            v.postInvalidateOnAnimation();
        } else {
            v.postInvalidateDelayed(80);   // (the stripes drift)
        }
    }

    private boolean spinning() {
        return waiting || System.currentTimeMillis() < stopAt;
    }

    /** The reel: cells racing right to left past the selector, slowing onto the stop cell */
    private void drawReel(Canvas c, int w, int y) {
        long now = System.currentTimeMillis();
        float pos;   // the reel's position in cells (the selector at the middle)
        if (waiting) {
            pos = (now - sentAt) / 45f;
        } else if (now < stopAt) {
            // eases out: from the racing speed down to the stop cell
            float k = Math.max(0f, 1f - (stopAt - now) / (float) SPIN_MS);
            float e = 1f - (1f - k) * (1f - k) * (1f - k);
            pos = startPos + (targetPos - startPos) * e;
            int cell = (int) pos;
            if (cell != lastTick) {
                lastTick = cell;
                v.sound.play("se_rg_ball_click");
            }
            if (now + 40 >= stopAt && !revealed) {
                revealed = true;
                reveal();
            }
        } else {
            pos = reel.length / 2f;
        }
        fill.setColor(0xFF0C1018);
        c.drawRect(0, y - 4, w, y + CELL + 4, fill);
        int mid = w / 2;
        int first = (int) Math.floor(pos) - (mid / CELL + 2);
        for (int k = first; k < first + w / CELL + 5; k++) {
            int idx = ((k % reel.length) + reel.length) % reel.length;
            float cx = mid + (k - pos) * CELL - CELL / 2f;
            int item = reel[idx];
            int rarity = item == -1 ? best() : (idx * 7) % 3;
            fill.setColor(RARITY_COLOR[rarity]);
            c.drawRect(cx + 2, y, cx + CELL - 2, y + CELL, fill);
            fill.setColor(0xFFF8F8F8);
            c.drawRect(cx + 4, y + 2, cx + CELL - 4, y + CELL - 2, fill);
            if (item == -1) {
                drawPrizeIcon(c, bestPrize(), (int) cx, y);
            } else {
                art.itemIcon(c, item, (int) cx + 4, y + 4);
            }
        }
        // the selector
        fill.setColor(0xFFF8D040);
        c.drawRect(mid - CELL / 2 - 2, y - 6, mid + CELL / 2 + 2, y - 3, fill);
        c.drawRect(mid - CELL / 2 - 2, y + CELL + 3, mid + CELL / 2 + 2, y + CELL + 6, fill);
        // a SUPER RARE or better: the stop flashes
        if (!spinning() && shown != null && best() >= 3 && now - stopAt < 900) {
            int a = (int) (200 * (1 - (now - stopAt) / 900f));
            fill.setColor((a << 24) | (RARITY_COLOR[best()] & 0xFFFFFF));
            c.drawRect(0, y - 6, w, y + CELL + 6, fill);
        }
    }

    private int lastTick = -1;
    private float startPos, targetPos;
    private boolean revealed;

    private void reveal() {
        int b = best();
        if (b >= 5) {
            v.sound.fanfare("mus_slots_win");
            v.buzz(true);
        } else if (b >= 3) {
            v.sound.play("se_shiny");
            v.buzz(true);
        } else {
            v.sound.fanfare("mus_obtain_item");
        }
    }

    private JSONArray prizes() {
        return shown == null ? null : shown.optJSONArray("prizes");
    }

    private int best() {
        JSONArray p = prizes();
        int b = 0;
        for (int i = 0; p != null && i < p.length(); i++) {
            b = Math.max(b, p.optJSONObject(i).optInt("r"));
        }
        return b;
    }

    private JSONObject bestPrize() {
        JSONArray p = prizes();
        JSONObject out = null;
        for (int i = 0; p != null && i < p.length(); i++) {
            if (out == null || p.optJSONObject(i).optInt("r") > out.optInt("r")) {
                out = p.optJSONObject(i);
            }
        }
        return out;
    }

    /** A prize's picture in a 32x32 cell: a Moemon's icon, else its item's */
    private void drawPrizeIcon(Canvas c, JSONObject p, int x, int y) {
        if (p == null) {
            return;
        }
        if (p.optInt("k") == 2) {
            art.icon(c, p.optInt("id"), x, y, GameArt.ICON_PLAIN);
        } else {
            art.itemIcon(c, p.optInt("id"), x + 4, y + 4);
        }
    }

    private void drawPrizes(Canvas c, int w, int y, int h) {
        JSONArray p = prizes();
        art.text(c, p.length() == 1 ? "YOUR PRIZE" : "YOUR " + p.length() + " PRIZES", 12, y - 2, GOLD, 0xFF101820);
        y += 16;
        c.save();
        c.clipRect(0, y, w, h - 30);
        int ry = y - scrollList;
        for (int i = 0; i < p.length(); i++) {
            JSONObject o = p.optJSONObject(i);
            int r = o.optInt("r");
            art.window(c, 0, 12, ry + 4, w - 24, 30);
            fill.setColor(RARITY_COLOR[r]);
            c.drawRect(12, ry + 4, 16, ry + 34, fill);
            drawPrizeIcon(c, o, 18, ry + 3);
            chip(c, RARITY[r], 54, ry + 6, RARITY_COLOR[r]);
            art.textHex(c, o.optString("n"), 54 + art.width(RARITY[r]) + 12, ry + 2, GameArt.TEXT, GameArt.SHADOW);
            String note = o.optString("note");
            if (!note.isEmpty()) {
                c.save();
                c.scale(0.75f, 0.75f, 54, ry + 22);
                art.text(c, note, 54, ry + 16, GameArt.GRAY_TEXT, GameArt.SHADOW);
                c.restore();
            }
            ry += 36;
        }
        c.restore();
        button(c, "AGAIN", 8, h - 28, (w - 16) / 2, true);
        button(c, "BACK", 8 + (w - 16) / 2, h - 28, (w - 16) / 2, true);
    }

    private int[] hows(JSONObject kai, JSONArray bs) {
        // the pulls this banner can take now, in the game's menu order
        JSONArray b = bs == null ? null : bs.optJSONArray(banner);
        int pulls = kai == null ? 0 : kai.optInt("pulls"), tickets = kai == null ? 0 : kai.optInt("tickets");
        int free1 = b == null ? 0 : b.optInt(2), free10 = b == null ? 0 : b.optInt(3);
        return new int[]{free10 > 0 ? 1 : 0, free1 > 0 ? 1 : 0, tickets > 0 ? 1 : 0, pulls >= 1 ? 1 : 0, pulls >= 10 ? 1 : 0};
    }

    private static final String[] HOW_NAMES = {"FREE x10", "FREE x1", "TICKET x1", "PULL x1", "PULL x10"};

    private void drawStock(Canvas c, int w, int y, JSONObject kai, JSONArray bs) {
        art.window(c, 0, 12, y + 4, w - 24, 22);
        String s = (kai == null ? 0 : kai.optInt("pulls")) + " PULLS    " + (kai == null ? 0 : kai.optInt("tickets"))
                + " TICKETS";
        art.text(c, s, (w - art.width(s)) / 2, y + 4);
        y += 34;
        int[] can = hows(kai, bs);
        for (int i = 0; i < 5; i++) {
            if (i == HOW_FREE10 && banner != 0) {
                continue;
            }
            button(c, HOW_NAMES[i], 12, y, w - 24, can[i] == 1 && !waiting);
            y += 26;
        }
        art.text(c, "SR or better at least every 40 pulls; a 10-pull", 14, y + 2, 0xFFC8D0D8, 0xFF101820);
        art.text(c, "always has a RARE.", 14, y + 14, 0xFFC8D0D8, 0xFF101820);
    }

    // ----------------------------------------------------------------------------------------------- input

    @Override
    public void tap(int x, int y) {
        int w = v.width(), h = v.contentHeight();
        if (spinning()) {
            return;
        }
        if (y >= 24 && y < 44) {
            JSONArray bs = banners();
            int n = bs == null ? 4 : bs.length();
            banner = (banner + (x < w / 2 ? n - 1 : 1)) % n;
            shown = null;
            v.sound.play("se_select");
            return;
        }
        if (shown != null) {
            if (y >= h - 28) {
                if (x < w / 2) {
                    shown = null;   // AGAIN: back to the buttons
                } else {
                    back();
                }
                v.sound.play("se_select");
            }
            return;
        }
        int by = 66 + CELL + 30 + 34;
        int[] can = hows(kai(), banners());
        for (int i = 0; i < 5; i++) {
            if (i == HOW_FREE10 && banner != 0) {
                continue;
            }
            if (y >= by && y < by + 24) {
                if (can[i] == 0) {
                    v.sound.play("se_failure");
                    v.message(i == HOW_TEN ? "Not enough pulls for x10. Daily goals and achievements give more!"
                            : i == HOW_TICKET ? "No TICKETS: a ball piece you already have from a pull becomes one."
                            : i <= HOW_FREE1 ? "That free pull isn't here yet: one a day on each banner, ten a week on STANDARD."
                            : "No pulls left. Daily goals and achievements give more!");
                } else {
                    pull(i);
                }
                return;
            }
            by += 26;
        }
    }

    private void pull(int how) {
        if (v.app.client == null || v.app.locked()) {
            v.message(v.app.client == null ? "Link with your game to pull." : Companion.LOCKED);
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "pull").put("banner", banner).put("how", how));
        } catch (org.json.JSONException ignored) {
        }
        v.sound.play("se_shop");
        waiting = true;
        revealed = false;
        sentAt = System.currentTimeMillis();
        stopAt = 0;
        shown = null;
        scrollList = 0;
        for (int i = 0; i < reel.length; i++) {
            reel[i] = FILLER[rnd.nextInt(FILLER.length)];
        }
    }

    @Override
    public void scrollBy(int dy) {
        JSONArray p = prizes();
        int max = p == null ? 0 : Math.max(0, p.length() * 36 - (v.contentHeight() - (66 + CELL + 46) - 34));
        scrollList = Math.max(0, Math.min(max, scrollList + dy));
    }

    @Override
    public void back() {
        if (spinning()) {
            return;
        }
        v.showOver(parent == null || parent instanceof MePage ? null : parent);
    }

    // (the screens test: a pull's answer as if the game had sent it)
    void showForTest(JSONObject done) {
        shown = done;
        waiting = false;
        stopAt = 0;
        revealed = true;
        reel[reel.length / 2] = -1;
    }
}
