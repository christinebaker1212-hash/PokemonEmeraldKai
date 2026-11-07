package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * ME: the player's own page (Kai's things, docs/PHONE_LINK.md "The ME page"): the TRAINER CARD (name, ID, money, play
 * time, POKéDEX, each region's BADGES), ACHIEVEMENTS (done of all; the list on AchievementsPage), the MOEGATCHA (pulls,
 * tickets and free ones; pulling on GatchaPage), the CANDY JAR (each type's candies; ten of a kind make a RARE CANDY)
 * and the GARDEN's BERRY farm (each open plot as it grows).
 */
public class MePage extends Page {
    private static final String[] REGIONS = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    // the jar's kinds in the type chart's order (the game's type numbers; no ??? or FAIRY: those are NORMAL candies)
    private static final int[] CANDY_TYPES = {0, 10, 11, 13, 12, 15, 1, 3, 4, 2, 14, 6, 5, 7, 16, 17, 8};
    private static final String[] STAGES = {"", "PLANTED", "SPROUTED", "TALLER", "FLOWERING", "BERRIES!", "BERRIES!"};
    private final Paint fill = new Paint();
    private int achY = -1, gatchaY = -1, giftY = -1, jarY = -1, jarRows, farmY = -1, contentH;

    public MePage(GameView v) {
        super(v);
    }

    private JSONObject kai() {
        Model m = v.app.model();
        return m == null ? null : m.snap.optJSONObject("kai");
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        background(c, w, h, scroll);
        Model m = v.app.model();
        banner(c, w, "TRAINER", m == null ? "" : "");
        if (m == null) {
            art.window(c, 0, 12, 30 - scroll, w - 24, 30);
            art.text(c, "Link with your game to see your card.", 18, 36 - scroll);
            // (MYSTERY GIFT scans without a link: kept on the phone for the game)
            contentH = drawGift(c, w, 70 - scroll, null) + 8 + scroll;
            return;
        }
        JSONObject kai = kai();
        int y = 24 - scroll;
        y = drawCard(c, w, y, m);
        if (kai == null) {
            art.window(c, 0, 12, y + 6, w - 24, 30);
            art.text(c, "Update your game to see the rest here.", 18, y + 10);
            contentH = y + 50 + scroll;
            return;
        }
        y = drawAch(c, w, y + 4, kai);
        y = drawGatcha(c, w, y + 4, kai);
        y = drawGift(c, w, y + 4, kai);
        y = drawJar(c, w, y + 4, kai);
        y = drawFarm(c, w, y + 4, kai);
        contentH = y + 8 + scroll;
    }

    private int heading(Canvas c, int w, int y, String text, String right) {
        sectionBar(c, 8, w - 8, y + 1, 15, text, right);
        return y + 18;
    }

    // ------------------------------------------------------------------------------------------- the card

    private int drawCard(Canvas c, int w, int y, Model m) {
        JSONObject save = m.save();
        JSONObject kai = kai();
        int x0 = 12, cw = w - 24, ch = 112;
        // the card itself: a pale panel with a colored top band, like the game's card
        fill.setColor(0xFF203050);
        c.drawRect(x0 - 2, y + 2, x0 + cw + 2, y + ch + 6, fill);
        fill.setColor(0xFFF0F4F8);
        c.drawRect(x0, y + 4, x0 + cw, y + ch + 4, fill);
        fill.setColor(BANNER);
        c.drawRect(x0, y + 4, x0 + cw, y + 22, fill);
        art.text(c, "TRAINER CARD", x0 + 6, y + 4, GameArt.WHITE_TEXT, 0xFF202850);
        String id = String.format(java.util.Locale.US, "IDNo%05d", save.optInt("tid"));
        art.text(c, id, x0 + cw - 6 - art.width(id), y + 4, GOLD, 0xFF202850);
        // the trainer's picture
        art.trainerPic(c, save.optInt("gender") == 1 ? 72 : 71, x0 + cw - 70, y + 26, 64);
        int ty = y + 26;
        int nx = art.text(c, "NAME", x0 + 6, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.textHex(c, save.optString("name"), x0 + 52, ty, GameArt.TEXT, GameArt.SHADOW);
        ty += 15;
        art.text(c, "MONEY", x0 + 6, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, "$" + save.optLong("money"), x0 + 52, ty);
        ty += 15;
        JSONObject dex = m.snap.optJSONObject("dex");
        art.text(c, "POKéDEX", x0 + 6, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, String.valueOf(dex == null ? 0 : DexPage.count(dex.optString("caught"))), x0 + 70, ty);
        ty += 15;
        JSONArray t = kai == null ? null : kai.optJSONArray("time");
        art.text(c, "TIME", x0 + 6, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, t == null ? "-" : t.optInt(0) + ":" + String.format(java.util.Locale.US, "%02d", t.optInt(1)), x0 + 52, ty);
        ty += 18;
        // BADGES: a row of eight per region the player has been to, the earned ones gold
        JSONArray rb = save.optJSONArray("rbadges");
        int bx = x0 + 6;
        for (int r = 0; r < 4; r++) {
            int n = rb == null ? (r == 0 ? save.optInt("badges") : 0) : rb.optInt(r);
            if (r > 0 && n == 0) {
                continue;
            }
            int rx = bx + (r % 2) * (cw / 2);
            int ry = ty + (r / 2) * 14;
            art.text(c, REGIONS[r].substring(0, 1), rx, ry - 3, GameArt.GRAY_TEXT, GameArt.SHADOW);
            for (int k = 0; k < 8; k++) {
                int cx = rx + 10 + k * 9;
                fill.setColor(0xFF404850);
                c.drawCircle(cx + 3, ry + 5, 4, fill);
                fill.setColor(k < n ? 0xFFF8C838 : 0xFFC8D0D8);
                c.drawCircle(cx + 3, ry + 5, 3, fill);
            }
        }
        return y + ch + 8;
    }

    // ------------------------------------------------------------------------------------ ACHIEVEMENTS

    private int drawAch(Canvas c, int w, int y, JSONObject kai) {
        JSONArray a = kai.optJSONArray("ach");
        int done = a == null ? 0 : a.optInt(0), all = a == null ? 0 : a.optInt(1);
        y = heading(c, w, y, "ACHIEVEMENTS", done + " / " + all);
        achY = y + scroll;
        art.window(c, 0, 12, y + 4, w - 24, 22);
        int bw = w - 24 - 72;
        fill.setColor(0xFF404850);
        c.drawRect(16, y + 10, 16 + bw + 2, y + 18, fill);
        fill.setColor(0xFFD0D8E0);
        c.drawRect(17, y + 11, 17 + bw, y + 17, fill);
        fill.setColor(0xFFF8C838);
        c.drawRect(17, y + 11, 17 + (all > 0 ? bw * done / all : 0), y + 17, fill);
        art.text(c, "SEE ALL >", w - 16 - art.width("SEE ALL >"), y + 4, GameArt.BLUE_TEXT, GameArt.SHADOW);
        return y + 30;
    }

    // -------------------------------------------------------------------------------------- MOEGATCHA

    private int drawGatcha(Canvas c, int w, int y, JSONObject kai) {
        y = heading(c, w, y, "MOEGATCHA", null);
        gatchaY = y + scroll;
        art.window(c, 0, 12, y + 4, w - 24, 38);
        String line = kai.optInt("pulls") + " PULLS   " + kai.optInt("tickets") + " TICKETS   " + kai.optInt("free") + " FREE";
        art.text(c, line, 18, y + 4);
        art.text(c, "More from daily goals.", 18, y + 20, GameArt.GRAY_TEXT, GameArt.SHADOW);
        button(c, "PULL", w - 76, y + 8, 60, kai.optInt("pulls") + kai.optInt("tickets") + kai.optInt("free") > 0);
        return y + 46;
    }

    // ----------------------------------------------------------------------------------- MYSTERY GIFT

    private int drawGift(Canvas c, int w, int y, JSONObject kai) {
        if (kai != null && !kai.has("gift")) {
            return y;   // (an older game)
        }
        // linked: the game's count for today; without a link: scans wait on the phone until it links
        int waiting = v.app.giftsWaiting(), left = kai == null ? 1 : kai.optInt("gift");
        y = heading(c, w, y, "MYSTERY GIFT", waiting > 0 ? waiting + " WAITING" : kai == null ? "" : left + " LEFT TODAY");
        giftY = y + scroll;
        art.window(c, 0, 12, y + 4, w - 24, 38);
        if (kai == null) {
            art.text(c, "Scan now: the gifts open when", 18, y + 4);
            art.text(c, "your game is linked again.", 18, y + 20, GameArt.GRAY_TEXT, GameArt.SHADOW);
        } else {
            art.text(c, "Scan any QR code, or the", 18, y + 4);
            art.text(c, "bar code on something from a store.", 18, y + 20, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        button(c, "SCAN", w - 76, y + 8, 60, left > 0);
        return y + 46;
    }

    // ------------------------------------------------------------------------------------- the CANDY JAR

    private int drawJar(Canvas c, int w, int y, JSONObject kai) {
        JSONArray candy = kai.optJSONArray("candy");
        int total = 0, rare = 0;
        for (int t = 0; candy != null && t < candy.length(); t++) {
            total += candy.optInt(t);
            rare += candy.optInt(t) / 10;
        }
        y = heading(c, w, y, "CANDY JAR", kai.optInt("jar") == 1 ? total + " CANDIES" : null);
        jarY = y + scroll;
        if (kai.optInt("jar") != 1) {
            art.window(c, 0, 12, y + 4, w - 24, 22);
            art.text(c, "Release a POKéMON in the PC: PROF. BIRCH", 18, y + 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.text(c, "brings a jar for its CANDY.", 18, y + 14, GameArt.GRAY_TEXT, GameArt.SHADOW);
            jarRows = 0;
            return y + 30;
        }
        int cols = Math.max(3, (w - 24) / 52), cw = (w - 24) / cols;
        jarRows = (CANDY_TYPES.length + cols - 1) / cols;
        int bh = jarRows * 20 + 4;
        art.window(c, 0, 12, y + 4, w - 24, bh + 24);
        for (int i = 0; i < CANDY_TYPES.length; i++) {
            int t = CANDY_TYPES[i], n = candy == null ? 0 : candy.optInt(t);
            int x = 12 + (i % cols) * cw, cy = y + 6 + (i / cols) * 20;
            art.typeIcon(c, t, x + 2, cy + 1);
            String s = String.valueOf(n);
            art.text(c, s, x + cw - 4 - art.width(s), cy - 1, n >= 10 ? GameArt.TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
            if (n >= 10) {
                fill.setColor(0xFFF8C838);
                c.drawRect(x + 34, cy + 13, x + cw - 4, cy + 14, fill);
            }
        }
        int by = y + 6 + jarRows * 20;
        String conv = rare > 0 ? "CONVERT: " + rare + " RARE CAND" + (rare == 1 ? "Y" : "IES") : "10 OF A KIND MAKE A RARE CANDY";
        button(c, conv, 12, by - 2, w - 24, rare > 0);
        return y + bh + 32;
    }

    // ---------------------------------------------------------------------------------- the BERRY farm

    private int drawFarm(Canvas c, int w, int y, JSONObject kai) {
        JSONArray farm = kai.optJSONArray("farm");
        int ready = 0;
        for (int i = 0; farm != null && i < farm.length(); i++) {
            ready += farm.optJSONArray(i).optInt(1) >= 5 ? 1 : 0;
        }
        y = heading(c, w, y, "GARDEN BERRY FARM", farm == null || farm.length() == 0 ? null
                : ready > 0 ? ready + " READY" : farm.length() + " PLOTS");
        farmY = y + scroll;
        if (farm == null || farm.length() == 0) {
            art.window(c, 0, 12, y + 4, w - 24, 22);
            art.text(c, "The GARDEN upstairs in a POKéMON CENTER.", 18, y + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 30;
        }
        int cols = Math.max(4, (w - 24) / 46), cw = (w - 24) / cols, rows = (farm.length() + cols - 1) / cols;
        art.window(c, 0, 12, y + 4, w - 24, rows * 46);
        for (int i = 0; i < farm.length(); i++) {
            JSONArray p = farm.optJSONArray(i);
            int x = 12 + (i % cols) * cw, py = y + 6 + (i / cols) * 46;
            // the soil
            fill.setColor(0xFF8C5A30);
            c.drawRect(x + 4, py + 2, x + cw - 4, py + 30, fill);
            fill.setColor(0xFFA87040);
            c.drawRect(x + 5, py + 3, x + cw - 5, py + 29, fill);
            int item = p.optInt(0), stage = p.optInt(1);
            if (item > 0) {
                art.itemIcon(c, item, x + (cw - 24) / 2, py + 4);
                if (stage < 5) {
                    fill.setColor(0x90A87040);   // (still growing: its berry faint)
                    c.drawRect(x + 5, py + 3, x + cw - 5, py + 29, fill);
                }
            }
            String label = item == 0 ? "EMPTY" : stage >= 5 ? "x" + p.optInt(3) : minutes(p.optInt(2));
            art.small(c, label, x + (cw - art.smallWidth(label)) / 2, py + 30, stage >= 5 ? 0xFF208830 : GameArt.GRAY_TEXT,
                    GameArt.SHADOW);
        }
        return y + rows * 46 + 8;
    }

    private static String minutes(int m) {
        return m >= 60 ? (m / 60) + "h" + (m % 60 > 0 ? " " + m % 60 + "m" : "") : m + "m";
    }

    // ------------------------------------------------------------------------------------------- input

    @Override
    public int scrollMax() {
        return Math.max(0, contentH - v.contentHeight());
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width();
        int ay = y + scroll;
        JSONObject kai = kai();
        if (giftY >= 0 && ay >= giftY && ay < giftY + 44) {
            if (kai != null && kai.optInt("gift") <= 0 && kai.has("gift")) {
                v.message(Companion.why("giftdone"));
            } else {
                MysteryGiftPage.openScan(v);
            }
            return;
        }
        if (kai == null) {
            return;
        }
        if (achY >= 0 && ay >= achY && ay < achY + 28) {
            v.showOver(new AchievementsPage(v, this));
            return;
        }
        if (gatchaY >= 0 && ay >= gatchaY && ay < gatchaY + 44) {
            v.showOver(new GatchaPage(v, this));
            return;
        }
        if (jarY >= 0 && jarRows > 0 && ay >= jarY + 4 + jarRows * 20 && ay < jarY + 4 + jarRows * 20 + 26) {
            JSONArray candy = kai.optJSONArray("candy");
            int rare = 0;
            for (int t = 0; candy != null && t < candy.length(); t++) {
                rare += candy.optInt(t) / 10;
            }
            if (rare == 0) {
                v.message("Ten CANDIES of a kind make a RARE CANDY. Releasing a POKéMON in the PC gives one of its type.");
            } else if (v.app.client == null || v.app.locked()) {
                v.message(v.app.client == null ? "Link with your game first." : Companion.LOCKED);
            } else {
                final int made = rare;
                Menu m = Menu.message(v, "Turn every ten CANDIES of a kind into a RARE CANDY? That makes " + made + ".").setRows();
                m.add("YES", () -> {
                    try {
                        v.app.client.order(new JSONObject().put("op", "candy").put("type", -1));
                    } catch (org.json.JSONException ignored) {
                    }
                });
                m.add("NO", null);
                v.showMenu(m);
            }
        }
    }
}
