package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Planned;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.List;

/**
 * A POKéMON's summary, the game's three pages: INFO (its Trainer, item, nature and where it was met), SKILLS (stats
 * with its nature's marks, EXP, ability) and MOVES (type, PP, power, accuracy, what it does). Swipe or the arrows
 * for the next one in the same box or party; GIVE / TAKE / MOVE from here.
 */
public final class SummaryPage extends Page {
    private static final String[] TABS = {"INFO", "SKILLS", "MOVES", "BALL"};
    private static final String[] STAT_NAMES = {"HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF", "SPEED"};
    // natureStats order (gNatureStatTable): ATTACK, DEFENSE, SPEED, SP. ATK, SP. DEF -> the summary's rows 1-5
    private static final int[] NATURE_ROW = {1, 2, 5, 3, 4};
    private final List<JSONObject> places;
    private final BoxesPage boxes;   // for MOVE (null from the party)
    private int index, tab, move = -1;
    private String renaming;   // the new name being typed (null: not renaming)
    private final Paint fill = new Paint();

    public SummaryPage(GameView v, List<JSONObject> places, int index, BoxesPage boxes) {
        super(v);
        this.places = places;
        this.index = index;
        this.boxes = boxes;
    }

    /** Its cry, as the game's summary plays it when a POKéMON comes up (not an EGG's) */
    private void cry() {
        JSONObject mon = mon();
        if (mon != null && mon.optInt("e") != 1) {
            v.sound.cry(mon.optInt("s"));
        }
    }

    private int criedFor = -1;

    @Override
    public void shown() {
        if (criedFor != index) {
            criedFor = index;   // (back from a page over it: no cry again)
            cry();
        }
    }

    private Planned planned() {
        return new Planned(v.app.model(), v.app.client != null ? v.app.client.orders.pending()
                : java.util.Collections.<JSONObject>emptyList());
    }

    private JSONObject place() {
        return places.get(index);
    }

    private JSONObject mon() {
        return planned().at(place());
    }

    private static final int TOP_H = 92, TAB_Y = 112, BODY_Y = 136;

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONObject mon = mon();
        banner(c, w, "SUMMARY" + (places.size() > 1 ? "  " + (index + 1) + "/" + places.size() : ""), "");
        if (mon == null) {
            art.window(c, 0, 12, 30, w - 24, 16);
            art.text(c, "It isn't there any more.", 14, 30);
            button(c, "BACK", w - 74, h - 28, 70, true);
            return;
        }
        drawTop(c, w, mon);
        boolean egg = mon.optInt("e") == 1;
        if (egg) {
            art.window(c, 0, 12, BODY_Y + 4, w - 24, 60);
            art.text(c, "The EGG Watch", 16, BODY_Y + 4, GameArt.BLUE_TEXT, GameArt.SHADOW);
            art.text(c, "It's still an EGG. Sounds can", 16, BODY_Y + 22);
            art.text(c, "be heard from inside!", 16, BODY_Y + 38);
        } else {
            int tw = (w - 16) / TABS.length;
            for (int i = 0; i < TABS.length; i++) {
                boolean on = i == tab;
                if (on) {
                    art.window(c, 0, 8 + i * tw + 4, TAB_Y + 4, tw - 8, 14);
                }
                int nw = art.width(TABS[i]);
                art.text(c, TABS[i], 8 + i * tw + (tw - nw) / 2, TAB_Y + 2, on ? GameArt.TEXT : GameArt.WHITE_TEXT,
                        on ? GameArt.SHADOW : 0xFF202850);
            }
            if (tab == 0) {
                drawInfo(c, w, mon);
            } else if (tab == 1) {
                drawSkills(c, w, mon);
            } else {
                drawMoves(c, w, mon);
            }
        }
        // the actions and the arrows
        int by = h - 30, bw = (w - 16) / 4;
        boolean locked = v.app.locked();
        button(c, "USE", 8, by, bw, !locked && !egg && place().has("party"));
        button(c, "GIVE", 8 + bw, by, bw, !locked && !egg);
        button(c, "TAKE", 8 + 2 * bw, by, bw, !locked && mon.optInt("i") != 0);
        button(c, boxes != null ? "MOVE" : "BACK", 8 + 3 * bw, by, bw, boxes == null || !locked);
        if (places.size() > 1) {
            DexPage.chip(c, art, fill, ">", w - 26, 2, true);
            DexPage.chip(c, art, fill, "<", w - 50, 2, true);
        }
    }

    private void drawTop(Canvas c, int w, JSONObject mon) {
        int species = mon.optInt("s");
        boolean egg = mon.optInt("e") == 1;
        art.window(c, 0, 10, 26, w - 20, TOP_H - 12);
        // the picture on its own little stage
        fill.setColor(0xFFE0E8F0);
        c.drawRect(12, 28, 80, 96 - 4, fill);
        if (egg) {
            art.icon(c, MonCard.eggSpecies(art), 30, 44, GameArt.ICON_PLAIN);
        } else {
            art.front(c, species, art.monPal(mon), 14, 28, false);
        }
        int x = 86, y = 22;
        int nw;
        if (renaming != null) {
            // the new name as it's typed, in a box with a cursor
            fill.setColor(0xFFFFFFFF);
            c.drawRect(x - 2, y + 2, x + 84, y + 16, fill);
            fill.setColor(0xFF5878E0);
            c.drawRect(x - 2, y + 15, x + 84, y + 16, fill);
            nw = art.text(c, renaming, x, y);
            if ((System.currentTimeMillis() / 400) % 2 == 0) {
                fill.setColor(GameArt.TEXT);
                c.drawRect(x + nw + 1, y + 4, x + nw + 2, y + 14, fill);
            }
            v.postInvalidateDelayed(400);
        } else {
            nw = art.textHex(c, mon.optString("n"), x, y, GameArt.TEXT, GameArt.SHADOW);
        }
        if (!egg) {
            int g = mon.optInt("g");
            if (g == 0) {
                art.text(c, "♂", x + nw + 4, y, GameArt.BLUE_TEXT, GameArt.SHADOW);
            } else if (g == 254) {
                art.text(c, "♀", x + nw + 4, y, GameArt.RED_TEXT, GameArt.SHADOW);
            }
            int lw = art.text(c, "Lv" + mon.optInt("l"), x, y + 16);
            art.ballIcon(c, mon.optInt("ball"), x + lw + 4, y + 13);
            if (mon.optInt("sh") == 1) {
                chip(c, "SHINY", x + lw + 30, y + 20, 0xFFD89010);
            }
            int dex = art.species(species).optInt("dex");
            art.text(c, String.format(java.util.Locale.US, "No%03d", dex), x, y + 32, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.text(c, art.speciesName(species), x + 40, y + 32, GameArt.GRAY_TEXT, GameArt.SHADOW);
            JSONArray t = art.species(species).optJSONArray("t");
            if (t != null) {
                art.typeIcon(c, t.optInt(0), x, y + 50);
                if (t.optInt(1) != t.optInt(0)) {
                    art.typeIcon(c, t.optInt(1), x + 34, y + 50);
                }
            }
            if (mon.has("hp")) {
                MonCard.hp(c, art, x, y + 72, mon.optInt("hp"), mon.optInt("mhp"));
            }
            // RENAME (the game's SELECT on the summary's INFO): its name typed on the phone
            String rn = renaming != null ? "DONE" : "RENAME";
            chip(c, rn, w - 16 - art.width(rn) - 6, y + 50, renaming != null ? 0xFF38A858 : 0xFF5878E0);
        }
    }

    private void section(Canvas c, int w, int y, String label) {
        art.text(c, label, 14, y, GameArt.BLUE_TEXT, GameArt.SHADOW);
    }

    private void drawInfo(Canvas c, int w, JSONObject mon) {
        int y = BODY_Y;
        int bh = v.contentHeight() - 34 - BODY_Y - 4;
        art.window(c, 0, 12, y + 4, w - 24, bh);
        y += 2;
        // its Trainer
        section(c, w, y, "OT/");
        int ox = art.textHex(c, mon.optString("otn"), 40, y, mon.optInt("otg") == 1 ? GameArt.RED_TEXT : GameArt.BLUE_TEXT,
                GameArt.SHADOW);
        String id = String.format(java.util.Locale.US, "IDNo%05d", mon.optLong("o") & 0xFFFF);
        art.text(c, id, w - 18 - art.width(id), y);
        y += 18;
        // its item
        section(c, w, y, "ITEM");
        int item = mon.optInt("i");
        if (item == 0) {
            art.text(c, "NONE", 60, y);
            y += 18;
        } else {
            art.itemIcon(c, item, 52, y - 4);
            art.text(c, art.name("items", item), 78, y);
            y += 18;
            y += art.paragraph(c, art.item(item).optString("d"), 18, y, w - 40, GameArt.GRAY_TEXT, GameArt.SHADOW) + 2;
        }
        // the Trainer's memo, the game's way: nature, where and when it was met
        section(c, w, y, "TRAINER MEMO");
        y += 16;
        int nat = mon.optInt("nat");
        int nx = art.text(c, art.name("natures", nat), 18, y, GameArt.RED_TEXT, GameArt.SHADOW);
        art.text(c, " nature.", 18 + nx, y);
        y += 14;
        JSONArray st = art.data.optJSONArray("natureStats");
        JSONArray ns = st == null ? null : st.optJSONArray(nat);
        String up = "", down = "";
        for (int i = 0; ns != null && i < 5; i++) {
            if (ns.optInt(i) > 0) {
                up = STAT_NAMES[NATURE_ROW[i]];
            } else if (ns.optInt(i) < 0) {
                down = STAT_NAMES[NATURE_ROW[i]];
            }
        }
        if (!up.isEmpty()) {
            int ux = art.text(c, "+" + up, 18, y, GameArt.RED_TEXT, GameArt.SHADOW);
            art.text(c, "-" + down, 18 + ux + 10, y, GameArt.BLUE_TEXT, GameArt.SHADOW);
        } else {
            art.text(c, "Its stats grow evenly.", 18, y, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        y += 14;
        JSONArray met = mon.optJSONArray("met");
        if (met != null) {
            int loc = met.optInt(0), lvl = met.optInt(1);
            JSONArray secs = art.data.optJSONArray("mapsecs");
            String where = secs == null ? "" : art.plain(GameArt.hexText(secs.optString(loc)));
            String memo;
            if (loc == 0xFF) {
                memo = "Met in a fateful encounter at Lv" + lvl + ".";
            } else if (loc == 0xFE) {
                memo = "Met in a trade.";
            } else if (lvl == 0) {
                memo = "Hatched at " + (where.isEmpty() ? "a faraway place" : where) + ".";
            } else {
                memo = "Met at Lv" + lvl + ", " + (where.isEmpty() ? "a faraway place" : where) + ".";
            }
            for (String l : wrap(memo, w - 40)) {
                art.text(c, l, 18, y);
                y += 14;
            }
        }
        y += 4;
        // how much it likes its Trainer
        section(c, w, y, "FRIENDSHIP");
        int f = mon.optInt("f");
        bar(c, 110, y + 6, w - 140, f, 255, 0xFFF870A8);
        y += 16;
        // (VERDANTURF's friendship rater's words)
        String feel = f >= 255 ? "It adores you! It can't possibly love you any more."
                : f >= 200 ? "It's extremely friendly toward you."
                : f >= 150 ? "It's quite friendly to you."
                : f >= 100 ? "It's warming up to you."
                : f >= 50 ? "It feels neutral toward you."
                : f >= 1 ? "It's not very fond of you." : "It doesn't like you at all.";
        for (String l : wrap(feel, w - 40)) {
            art.text(c, l, 18, y, GameArt.GRAY_TEXT, GameArt.SHADOW);
            y += 14;
        }
        // its page in the POKéDEX
        button(c, "POKéDEX ENTRY", 18, dexButtonY(), w - 36, true);
    }

    private int dexButtonY() {
        return v.contentHeight() - 34 - 4 - 30;
    }

    private void openDex(JSONObject mon) {
        int dex = art.species(mon.optInt("s")).optInt("dex");
        if (dex <= 0) {
            return;
        }
        java.util.ArrayList<Integer> one = new java.util.ArrayList<>();
        one.add(dex);
        v.buzz(false);
        v.showOver(new DexEntryPage(v, one, 0));
    }

    private void bar(Canvas c, int x, int y, int bw, int v, int max, int color) {
        fill.setColor(0xFF404040);
        c.drawRect(x, y, x + bw + 2, y + 5, fill);
        fill.setColor(0xFFD0D8E0);
        c.drawRect(x + 1, y + 1, x + bw + 1, y + 4, fill);
        fill.setColor(color);
        c.drawRect(x + 1, y + 1, x + 1 + Math.max(0, Math.min(bw, max > 0 ? v * bw / max : 0)), y + 4, fill);
    }

    private void drawSkills(Canvas c, int w, JSONObject mon) {
        int y = BODY_Y;
        int bh = v.contentHeight() - 34 - BODY_Y - 4;
        art.window(c, 0, 12, y + 4, w - 24, bh);
        y += 2;
        JSONArray stats = mon.optJSONArray("stats");
        JSONArray st = art.data.optJSONArray("natureStats");
        JSONArray ns = st == null ? null : st.optJSONArray(mon.optInt("nat"));
        int[] mark = new int[6];
        for (int i = 0; ns != null && i < 5; i++) {
            mark[NATURE_ROW[i]] = ns.optInt(i);
        }
        int top = 1;
        for (int i = 0; stats != null && i < 6; i++) {
            top = Math.max(top, stats.optInt(i));
        }
        for (int i = 0; i < 6; i++) {
            int color = mark[i] > 0 ? GameArt.RED_TEXT : mark[i] < 0 ? GameArt.BLUE_TEXT : GameArt.TEXT;
            art.text(c, STAT_NAMES[i], 18, y, color, GameArt.SHADOW);
            String val = stats == null ? "?" : (i == 0 && mon.has("hp") ? mon.optInt("hp") + "/" : "") + stats.optInt(i);
            art.text(c, val, 120 - art.width(val) + 26, y);
            if (stats != null) {
                bar(c, 154, y + 6, w - 184, stats.optInt(i), top, i == 0 ? 0xFF70F8A8 : mark[i] > 0 ? 0xFFF87858
                        : mark[i] < 0 ? 0xFF5890F8 : 0xFFF8C858);
            }
            y += 15;
        }
        y += 4;
        JSONArray xp = mon.optJSONArray("xp");
        if (xp != null) {
            section(c, w, y, "EXP. POINTS");
            String pts = String.valueOf(xp.optLong(0));
            art.text(c, pts, w - 20 - art.width(pts), y);
            y += 15;
            long next = Math.max(0, xp.optLong(2) - xp.optLong(0));
            art.text(c, "NEXT LV.", 18, y);
            String n = String.valueOf(next);
            art.text(c, n, w - 20 - art.width(n), y);
            y += 15;
            long span = Math.max(1, xp.optLong(2) - xp.optLong(1));
            bar(c, 18, y + 2, w - 48, (int) ((xp.optLong(0) - xp.optLong(1)) * 1000 / span), 1000, 0xFF40C8F8);
            y += 12;
        }
        section(c, w, y, "ABILITY");
        art.text(c, art.name("abilities", mon.optInt("ab")), 80, y);
        y += 16;
        JSONArray ad = art.data.optJSONArray("abilityDesc");
        if (ad != null) {
            art.paragraph(c, ad.optString(mon.optInt("ab")), 18, y, w - 40, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
    }

    private int ppMax(int moveId, int bonuses, int slot) {
        int base = art.move(moveId).optInt("pp");
        return base + base * ((bonuses >> (2 * slot)) & 3) / 5;
    }

    private void drawMoves(Canvas c, int w, JSONObject mon) {
        int y = BODY_Y + 4;
        JSONArray moves = mon.optJSONArray("m"), pp = mon.optJSONArray("pp");
        for (int i = 0; i < 4; i++) {
            int mv = moves == null ? 0 : moves.optInt(i);
            art.window(c, 0, 12, y + 4, w - 24, 24);
            if (mv == 0) {
                art.text(c, "-", 52, y + 8, GameArt.GRAY_TEXT, GameArt.SHADOW);
            } else {
                JSONObject m = art.move(mv);
                art.typeIcon(c, m.optInt("t"), 14, y + 8);
                art.text(c, art.name("moves", mv), 52, y + 2);
                String ppText = "PP " + (pp == null ? "?" : pp.optInt(i)) + "/" + ppMax(mv, mon.optInt("ppb"), i);
                art.text(c, ppText, w - 18 - art.width(ppText), y + 14, GameArt.GRAY_TEXT, GameArt.SHADOW);
                if (i == move) {
                    fill.setStyle(Paint.Style.STROKE);
                    fill.setColor(0xFFF85838);
                    c.drawRect(11, y + 3, w - 11, y + 29, fill);
                    fill.setStyle(Paint.Style.FILL);
                }
            }
            y += 32;
        }
        // the chosen move's details
        int mv = moves == null || move < 0 ? 0 : moves.optInt(move);
        int bh = v.contentHeight() - 34 - y - 4;
        art.window(c, 0, 12, y + 4, w - 24, bh);
        if (mv == 0) {
            art.text(c, "Tap a move to see what it does.", 16, y + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return;
        }
        JSONObject m = art.move(mv);
        String pow = m.optInt("p") <= 1 ? "---" : String.valueOf(m.optInt("p"));
        String acc = m.optInt("a") == 0 ? "---" : String.valueOf(m.optInt("a"));
        section(c, w, y + 4, "POWER");
        art.text(c, pow, 70, y + 4);
        section(c, w, y + 20, "ACCURACY");
        art.text(c, acc, 90, y + 20);
        // its kind of damage, the Gen 4 way (each move its own)
        String[] cats = {"PHYSICAL", "SPECIAL", "STATUS"};
        int[] catColors = {0xFFE05838, 0xFF5878E0, 0xFF909090};
        int cat = m.optInt("c");
        if (cat >= 0 && cat < 3) {
            int cw = art.width(cats[cat]) + 10;
            chip(c, cats[cat], w - 20 - cw, y + 10, catColors[cat]);
        }
        art.paragraph(c, m.optString("d"), 16, y + 38, w - 36, GameArt.TEXT, GameArt.SHADOW);
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width(), h = v.contentHeight();
        JSONObject mon = mon();
        if (y < 20) {
            if (x >= w - 26) {
                swipe(1);
            } else if (x >= w - 50) {
                swipe(-1);
            }
            return;
        }
        if (y >= h - 30) {
            int bw = (w - 16) / 4, k = Math.min(3, Math.max(0, (x - 8) / bw)) + 1;
            if (k == 1) {
                if (mon == null || v.app.client == null || mon.optInt("e") == 1) {
                    return;
                }
                if (v.app.locked()) {
                    v.message(Companion.LOCKED);
                } else if (!place().has("party")) {
                    v.message("Items are used on the POKéMON in your party, as in the game.");
                } else {
                    v.showOver(ItemPickPage.forUse(v, place(), mon, this));
                }
                return;
            }
            if (mon == null || v.app.client == null) {
                if (k == 4) {
                    back();
                }
                return;
            }
            boolean locked = v.app.locked();
            if (k == 2 && mon.optInt("e") == 0) {
                if (locked) {
                    v.message(Companion.LOCKED);
                } else {
                    v.showOver(new ItemPickPage(v, place(), mon, this));
                }
            } else if (k == 3 && mon.optInt("i") != 0) {
                if (locked) {
                    v.message(Companion.LOCKED);
                } else {
                    v.app.client.order(Model.item(false, place(), mon, 0));
                    v.buzz(false);
                }
            } else if (k == 4) {
                if (boxes != null && !locked) {
                    boxes.startMove(place());
                }
                back();
            }
            return;
        }
        if (mon == null) {
            return;
        }
        if (mon.optInt("e") == 0 && y >= 22 + 48 && y < 22 + 64 && x >= w - 70) {
            if (renaming != null) {
                finishRename(mon);
            } else {
                startRename(mon);
            }
            return;
        }
        if (mon.optInt("e") == 0 && renaming == null && y >= 22 && y < 40 && x >= 86 && x < w - 20) {
            startRename(mon);
            return;
        }
        if (y >= TAB_Y && y < TAB_Y + 22 && mon.optInt("e") == 0) {
            int t = Math.min(TABS.length - 1, Math.max(0, (x - 8) / ((w - 16) / TABS.length)));
            if (t == 3) {
                v.buzz(false);
                v.showOver(new BallPage(v, place(), this));   // (its POKé BALL: a page of its own)
                return;
            }
            if (t != tab) {
                v.sound.play("se_select");
            }
            tab = t;
            move = -1;
            return;
        }
        if (y < TOP_H + 4 && x < 84) {
            cry();   // (the picture: its cry again)
            return;
        }
        if (tab == 0 && y >= dexButtonY() && y < dexButtonY() + 26 && x >= 18 && x < w - 18) {
            openDex(mon);
            return;
        }
        if (tab == 2 && y >= BODY_Y + 4 && y < BODY_Y + 4 + 4 * 32) {
            move = (y - BODY_Y - 4) / 32;
        }
    }

    private void startRename(JSONObject mon) {
        if (v.app.client == null || v.app.locked()) {
            v.message(v.app.client == null ? "Link with your game to rename it." : Companion.LOCKED);
            return;
        }
        renaming = art.plain(GameArt.hexText(mon.optString("n")));
        v.keyboard(true);
        v.sound.play("se_select");
    }

    private void finishRename(JSONObject mon) {
        String name = renaming == null ? "" : renaming.trim();
        renaming = null;
        v.keyboard(false);
        if (name.isEmpty() || mon == null || name.equals(art.plain(GameArt.hexText(mon.optString("n"))))) {
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "rename").put("at", place()).put("p", mon.optLong("p"))
                    .put("o", mon.optLong("o")).put("name", art.toHex(name)));
        } catch (org.json.JSONException ignored) {
        }
        v.buzz(false);
    }

    @Override
    public void typed(String text) {
        if (renaming == null) {
            return;
        }
        StringBuilder b = new StringBuilder(renaming);
        for (char ch : text.toCharArray()) {
            if (b.length() < 10 && art.hasLetter(ch) && ch != '\n') {
                b.append(ch);
            }
        }
        renaming = b.toString();
    }

    @Override
    public void backspace() {
        if (renaming != null && !renaming.isEmpty()) {
            renaming = renaming.substring(0, renaming.length() - 1);
        }
    }

    @Override
    public void typingDone() {
        if (renaming != null) {
            finishRename(mon());
        }
    }

    @Override
    public void hidden() {
        if (renaming != null) {
            renaming = null;
            v.keyboard(false);
        }
    }

    @Override
    public void swipe(int dir) {
        if (places.size() > 1) {
            index = (index + dir + places.size()) % places.size();
            move = -1;
            cry();
        }
    }
}
