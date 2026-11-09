package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Breeding;
import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * BREEDING: how a POKéMON's IVs (and egg moves) reach another kind, egg by egg (core/Breeding.java, the game's own
 * rules). FROM is one of the player's POKéMON (its gender and IVs) or any kind; TO any kind, and up to four of its egg
 * moves. The best routes follow, each egg drawn as the father, the mother and what hatches, with every mother that
 * would do (the caught ones in color, first); then where each wanted egg move comes from, and how to keep the IVs.
 * On the POKéDEX's green, its panels and chips (DexPage, DexEntryPage).
 */
public final class BreedPage extends Page {
    private static final int MAIN = 0, PICK_FROM = 1, PICK_TO = 2;
    private static final String[] STATS = {"HP", "ATK", "DEF", "SPA", "SPD", "SPE"};
    private static Breeding sCalc;
    private static JSONObject sCalcData;

    private int mode = MAIN;
    private int from, fromGender = -1, to;
    private JSONObject fromMon;                     // (one of the player's: its IVs, gender, nickname)
    private final List<Integer> wanted = new ArrayList<>();
    private List<Breeding.Route> routes = new ArrayList<>();
    private List<Breeding.MoveSource> sources = new ArrayList<>();
    private boolean dirty = true;
    private boolean mine = true;                    // (the FROM picker: MY POKéMON or every kind)
    private String search = "";
    private boolean typing;
    private int height, savedScroll;
    private final Paint fill = new Paint();
    private final List<int[]> hits = new ArrayList<>();   // {x0, y0, x1, y1, kind, arg} in content coordinates
    private static final int H_BACK = 1, H_FROM = 2, H_TO = 3, H_MALE = 4, H_FEMALE = 5, H_MOVE = 6, H_ROW = 7,
            H_TAB = 8, H_SEARCH = 9, H_CLEAR = 10, H_SWAP = 11;
    private final List<Object> rows = new ArrayList<>();   // (the picker's: JSONObject a POKéMON, Integer a kind)

    public BreedPage(GameView v) {
        super(v);
    }

    /** Opened from a POKéDEX entry: that kind as the target */
    public BreedPage to(int species) {
        to = species;
        dirty = true;
        return this;
    }

    /** Opened from one of the player's POKéMON */
    public BreedPage from(JSONObject mon) {
        pickMon(mon);
        return this;
    }

    private Breeding calc() {
        if (sCalc == null || sCalcData != art.data) {
            sCalcData = art.data;
            sCalc = new Breeding(art.data);
        }
        Model m = v.app.model();
        JSONObject d = m == null ? null : m.snap.optJSONObject("dex");
        sCalc.setCaught(d == null ? null : d.optString("caught"));
        return sCalc;
    }

    private void pickMon(JSONObject mon) {
        fromMon = mon;
        from = mon.optInt("s");
        fromGender = mon.optInt("g");
        dirty = true;
    }

    private String name(int s) {
        return art.plain(art.speciesName(s));
    }

    private String moveName(int m) {
        return art.plain(art.name("moves", m));
    }

    private static String group(int g) {
        return g == Breeding.EG_DITTO ? "DITTO" : g >= 0 && g < DexEntryPage.EGG_GROUPS.length ? DexEntryPage.EGG_GROUPS[g] : "?";
    }

    private void recompute() {
        if (!dirty) {
            return;
        }
        dirty = false;
        Breeding b = calc();
        routes = from > 0 && to > 0 ? b.routes(from, fromGender, to, 3) : new ArrayList<>();
        int lastFather = 0;
        if (!routes.isEmpty()) {
            List<Breeding.Step> st = routes.get(0).steps;
            lastFather = st.get(st.size() - 1).father;
        }
        int[] mv = new int[wanted.size()];
        for (int i = 0; i < mv.length; i++) {
            mv[i] = wanted.get(i);
        }
        sources = to > 0 ? b.moveSources(to, mv, lastFather) : new ArrayList<>();
    }

    // ------------------------------------------------------------------------------------------------- drawing

    @Override
    public int scrollMax() {
        return Math.max(0, height - v.contentHeight());
    }

    private int panel(Canvas c, int x, int y, int w, int h, String title) {
        fill.setColor(DexPage.BORDER);
        c.drawRect(x + 1, y, x + w - 1, y + h, fill);
        c.drawRect(x, y + 1, x + w, y + h - 1, fill);
        fill.setColor(0xFFF8F8F0);
        c.drawRect(x + 1, y + 1, x + w - 1, y + h - 1, fill);
        fill.setColor(DexPage.YELLOW);
        c.drawRect(x + 1, y + 1, x + w - 1, y + 16, fill);
        fill.setColor(DexPage.BORDER_LIGHT);
        c.drawRect(x + 1, y + 16, x + w - 1, y + 17, fill);
        art.text(c, title, x + 6, y - 1, GameArt.BLUE_TEXT, GameArt.SHADOW);
        return y + 20;
    }

    private void hit(int x0, int y0, int x1, int y1, int kind, int arg) {
        hits.add(new int[]{x0, y0, x1, y1, kind, arg});
    }

    /** Small words wrapped to MAX wide from (x, y); returns the y under them */
    private int words(Canvas c, String s, int x, int y, int max, int color) {
        for (String line : art.wrapSmall(s, max)) {
            art.small(c, line, x, y, color, GameArt.SHADOW);
            y += 11;
        }
        return y;
    }

    private void gender(Canvas c, int g, int x, int y) {
        if (g == 0) {
            art.text(c, "♂", x, y, GameArt.BLUE_TEXT, GameArt.SHADOW);
        } else if (g == Breeding.FEMALE_ONLY) {
            art.text(c, "♀", x, y, GameArt.RED_TEXT, GameArt.SHADOW);
        }
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        DexPage.stripes(c, fill, w, h + 8, scroll);
        c.save();
        c.translate(0, -scroll);
        int y = mode == MAIN ? drawMain(c, w) : drawPicker(c, w);
        height = y + 8;
        c.restore();
    }

    private int drawMain(Canvas c, int w) {
        recompute();
        Breeding b = calc();
        DexPage.chip(c, art, fill, "BACK", 6, 4, false);
        hit(0, 0, 60, 22, H_BACK, 0);
        art.text(c, "BREEDING", w - 8 - art.width("BREEDING"), 2, 0xFFFFFFFF, 0xFF000000);
        if (from > 0 && to > 0) {   // (FROM and TO the other way round)
            int sx = 66;
            DexPage.chip(c, art, fill, "SWAP", sx, 4, false);
            hit(sx, 0, sx + art.width("SWAP") + 12, 22, H_SWAP, 0);
        }
        int y = 26, half = (w - 18) / 2;
        // FROM and TO, side by side
        y = slot(c, 6, y, half, "FROM", from, true);
        slot(c, 12 + half, 26, w - 18 - half, "TO", to, false);
        y += 6;
        // the target's egg moves: up to four to plan for
        if (to > 0) {
            int[] em = b.eggMoves(to);
            int rowsH = 0, x = 12, lines = 1;
            for (int m : em) {
                int bw = art.width(moveName(m)) + 12;
                if (x + bw > w - 12) {
                    x = 12;
                    lines++;
                }
                x += bw + 4;
            }
            rowsH = em.length == 0 ? 14 : lines * 19;
            int top = panel(c, 6, y, w - 12, 34 + rowsH, "EGG MOVES (" + wanted.size() + "/4)");
            if (em.length == 0) {
                art.small(c, name(to) + " has no egg moves.", 12, top, GameArt.GRAY_TEXT, GameArt.SHADOW);
            } else {
                art.small(c, "Tap up to four to plan where they come from.", 12, top, GameArt.GRAY_TEXT, GameArt.SHADOW);
                x = 12;
                int ry = top + 13;
                for (int m : em) {
                    String nm = moveName(m);
                    int bw = art.width(nm) + 12;
                    if (x + bw > w - 12) {
                        x = 12;
                        ry += 19;
                    }
                    DexPage.chip(c, art, fill, nm, x, ry, wanted.contains(m), bw);
                    hit(x, ry, x + bw, ry + 16, H_MOVE, m);
                    x += bw + 4;
                }
            }
            y += 34 + rowsH + 6;
        }
        if (from <= 0 || to <= 0) {
            y = note(c, w, y, "Choose the POKéMON whose IVs you want to pass on (FROM) and the kind that should get them (TO).");
            return tips(c, w, y);
        }
        if (routes.isEmpty()) {
            y = note(c, w, y, why(b));
            return tips(c, w, y);
        }
        for (int i = 0; i < routes.size(); i++) {
            y = route(c, w, y, routes.get(i), i);
        }
        if (!sources.isEmpty()) {
            y = movePlan(c, w, y);
        }
        return tips(c, w, y);
    }

    private int slot(Canvas c, int x, int y, int sw, String title, int s, boolean isFrom) {
        int top = panel(c, x, y, sw, 76, title);
        hit(x, y, x + sw, y + 76, isFrom ? H_FROM : H_TO, 0);
        if (s <= 0) {
            art.small(c, "TAP TO CHOOSE", x + 8, top + 18, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 76;
        }
        art.icon(c, s, x + 2, top - 2, GameArt.ICON_PLAIN);
        String nm = isFrom && fromMon != null ? art.plain(GameArt.hexText(fromMon.optString("n"))) : name(s);
        int nw = art.text(c, nm, x + 34, top - 1, GameArt.TEXT, GameArt.SHADOW);
        if (isFrom) {
            gender(c, fromGender < 0 ? (calc().canBeMale(s) ? 0 : calc().gender(s)) : fromGender, x + 36 + nw, top - 1);
        }
        int[] g = calc().groups(s);
        art.small(c, group(g[0]) + (g.length > 1 ? " / " + group(g[1]) : ""), x + 34, top + 13, GameArt.GRAY_TEXT, GameArt.SHADOW);
        if (isFrom && fromMon != null && fromMon.has("iv")) {
            long iv = fromMon.optLong("iv");
            int perfect = 0;
            StringBuilder sb = new StringBuilder();
            for (int k = 0; k < 6; k++) {
                int one = (int) ((iv >> (5 * k)) & 31);
                perfect += one == 31 ? 1 : 0;
                sb.append(k > 0 ? " " : "").append(one);
            }
            art.small(c, "IVs " + sb, x + 6, top + 30, perfect == 6 ? GameArt.RED_TEXT : GameArt.TEXT, GameArt.SHADOW);
            art.small(c, perfect + " PERFECT", x + 6, top + 41, GameArt.GRAY_TEXT, GameArt.SHADOW);
        } else if (isFrom) {
            // any of the kind: its gender to choose (when it can be either)
            Breeding b = calc();
            int gx = x + 6;
            if (b.canBeMale(s)) {
                DexPage.chip(c, art, fill, "♂", gx, top + 32, fromGender != Breeding.FEMALE_ONLY, 22);
                hit(gx, top + 30, gx + 22, top + 50, H_MALE, 0);
                gx += 26;
            }
            if (b.canBeFemale(s)) {
                DexPage.chip(c, art, fill, "♀", gx, top + 32, fromGender == Breeding.FEMALE_ONLY || !b.canBeMale(s), 22);
                hit(gx, top + 30, gx + 22, top + 50, H_FEMALE, 0);
            }
        } else {
            art.small(c, b4(calc().gender(s)), x + 6, top + 32, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
        return y + 76;
    }

    /** A kind's genders, as words */
    private static String b4(int g) {
        return g == Breeding.GENDERLESS ? "NO GENDER" : g == Breeding.MALE_ONLY ? "MALES ONLY" : g == Breeding.FEMALE_ONLY
                ? "FEMALES ONLY" : "MALE OR FEMALE";
    }

    private String why(Breeding b) {
        switch (b.why) {
            case Breeding.FROM_NO_EGGS: return name(from) + " can't breed: its egg group is UNDISCOVERED.";
            case Breeding.TO_NO_EGG: return name(to) + " never hatches from an egg.";
            case Breeding.TO_ONLY_OWN: return name(to) + " breeds only with DITTO (" + b4(b.gender(to)) + "), so its eggs"
                    + " are always its own kind: no other kind can pass it IVs. Breed the IVs up with DITTO and "
                    + name(to) + "s of its own.";
            case Breeding.FROM_GENDERLESS: return name(from) + " has no gender: it breeds only with DITTO, and its eggs are"
                    + " always " + name(from) + ". Its IVs can't move to another kind.";
            default: return "No chain of eggs leads from " + name(from) + " to " + name(to) + ".";
        }
    }

    private int note(Canvas c, int w, int y, String text) {
        List<String> lines = art.wrapSmall(text, w - 28);
        int top = panel(c, 6, y, w - 12, 26 + lines.size() * 11, "HOW");
        words(c, text, 12, top - 2, w - 28, GameArt.TEXT);
        return y + 26 + lines.size() * 11 + 6;
    }

    /** One route: its eggs, each as father + mother -> egg, the words, and the mothers that would do */
    private int route(Canvas c, int w, int y, Breeding.Route r, int index) {
        Breeding b = calc();
        int eggs = r.steps.size();
        String title = (index == 0 ? "BEST ROUTE" : "OR") + ": " + eggs + (eggs == 1 ? " EGG" : " EGGS");
        // (laid out first, to know its height)
        List<List<String>> texts = new ArrayList<>();
        int per = Math.max(4, (w - 40) / 26);
        int hgt = 24;
        for (int i = 0; i < eggs; i++) {
            List<String> t = art.wrapSmall(stepText(b, r.steps.get(i), i == eggs - 1, i > 0 ? r.steps.get(i - 1).hatch : 0), w - 30);
            texts.add(t);
            int nm = r.steps.get(i).mothers.size();
            hgt += 36 + t.size() * 11 + (nm > 1 ? 14 + ((Math.min(nm, per * 2) + per - 1) / per) * 24 : 0) + 6;
        }
        int top = panel(c, 6, y, w - 12, hgt, title);
        int sy = top;
        for (int i = 0; i < eggs; i++) {
            Breeding.Step st = r.steps.get(i);
            art.small(c, "EGG " + (i + 1), 12, sy + 8, GameArt.GRAY_TEXT, GameArt.SHADOW);
            int ix = 46;
            if (st.herOwn) {
                art.icon(c, from, ix, sy, GameArt.ICON_PLAIN);
                gender(c, Breeding.FEMALE_ONLY, ix + 28, sy + 14);
                art.text(c, "+ ANY ♂", ix + 40, sy + 6, GameArt.TEXT, GameArt.SHADOW);
                ix += 40 + art.width("+ ANY ♂") + 6;
            } else {
                art.icon(c, st.father, ix, sy, GameArt.ICON_PLAIN);
                if (st.group != Breeding.EG_DITTO || !b.isDitto(st.father)) {
                    gender(c, 0, ix + 28, sy + 14);
                }
                art.text(c, "+", ix + 38, sy + 6, GameArt.TEXT, GameArt.SHADOW);
                art.icon(c, st.mother, ix + 46, sy, b.caught(st.mother) || st.group == Breeding.EG_DITTO
                        ? GameArt.ICON_PLAIN : GameArt.ICON_GRAY);
                if (!b.isDitto(st.mother)) {
                    gender(c, Breeding.FEMALE_ONLY, ix + 74, sy + 14);
                }
                ix += 88;
            }
            arrow(c, ix, sy + 15);
            art.icon(c, st.hatch, ix + 14, sy, GameArt.ICON_PLAIN);
            sy += 34;
            for (String line : texts.get(i)) {
                art.small(c, line, 14, sy, GameArt.TEXT, GameArt.SHADOW);
                sy += 11;
            }
            if (st.mothers.size() > 1) {
                art.small(c, "MOTHERS THAT DO (COLOR: CAUGHT)", 14, sy + 1, GameArt.GRAY_TEXT, GameArt.SHADOW);
                sy += 13;
                int shown = Math.min(st.mothers.size(), per * 2);
                for (int k = 0; k < shown; k++) {
                    int m = st.mothers.get(k);
                    art.iconAt(c, m, 14 + (k % per) * 26, sy + (k / per) * 24 - 4, 28, b.caught(m) ? GameArt.ICON_PLAIN : GameArt.ICON_GRAY);
                }
                sy += ((shown + per - 1) / per) * 24;
                if (st.mothers.size() > shown) {
                    String more = "+" + (st.mothers.size() - shown) + " MORE";
                    art.small(c, more, w - 18 - art.smallWidth(more), sy - 12, GameArt.GRAY_TEXT, GameArt.SHADOW);
                }
            }
            sy += 6;
            if (i < eggs - 1) {
                fill.setColor(0xFFD8D8C8);
                c.drawRect(12, sy - 3, w - 12, sy - 2, fill);
            }
        }
        return y + hgt + 6;
    }

    /** A small arrow pointing right, its middle at (x, y) */
    private void arrow(Canvas c, int x, int y) {
        fill.setColor(GameArt.TEXT);
        c.drawRect(x, y - 1, x + 7, y + 1, fill);
        for (int k = 0; k < 4; k++) {
            c.drawRect(x + 6 + k, y - 4 + k, x + 7 + k, y + 4 - k, fill);
        }
    }

    private String stepText(Breeding b, Breeding.Step st, boolean last, int before) {
        String keep = last ? "Keep the one with the best IVs." : "Keep a ♂ with the best IVs: he carries them on.";
        if (st.herOwn) {
            int[] g = b.groups(from);
            return "Your ♀ " + name(from) + " with any ♂ of the " + group(g[0]) + (g.length > 1 ? " or " + group(g[1]) : "")
                    + " group, or DITTO: her eggs are " + name(st.hatch) + ". " + keep;
        }
        if (st.group == Breeding.EG_DITTO) {
            if (b.isDitto(st.father)) {
                return "DITTO with any " + name(st.mother) + ": the egg is " + name(st.hatch) + ". " + keep;
            }
            return name(st.father) + " with a ♀ of its own kind or DITTO: the egg is " + name(st.hatch) + ". " + keep;
        }
        String dad = name(st.father) + (before > 0 && st.father != before ? " (the " + name(before) + ", evolved)" : "");
        return "♂ " + dad + " with a ♀ " + name(st.mother) + " (both " + group(st.group) + "): the egg is "
                + name(st.hatch) + (b.family(st.hatch) != b.family(st.mother) ? " (half its eggs)" : "") + ". " + keep;
    }

    /** Where each wanted egg move comes from, in the order to breed them onto the last mother */
    private int movePlan(Canvas c, int w, int y) {
        Breeding b = calc();
        List<String> lines = new ArrayList<>();
        List<Breeding.Step> st = routes.isEmpty() ? null : routes.get(0).steps;
        int lastFather = st == null ? 0 : st.get(st.size() - 1).father;
        boolean herOwn = st != null && st.get(st.size() - 1).herOwn;   // (the player's ♀ lays the last egg herself)
        int round = 0;
        boolean starred = false;
        for (Breeding.MoveSource ms : sources) {
            String mv = moveName(ms.move);
            if (herOwn) {
                StringBuilder f = new StringBuilder();
                List<Integer> list = ms.fathers.isEmpty() && ms.bridge != 0 ? java.util.Collections.singletonList(ms.bridge) : ms.fathers;
                for (int k = 0; k < Math.min(4, list.size()); k++) {
                    f.append(k > 0 ? ", " : "").append(name(list.get(k))).append(b.caught(list.get(k)) ? "*" : "");
                    starred |= b.caught(list.get(k));
                }
                lines.add(list.isEmpty() ? mv + ": no father here can pass it on." : mv + ": her mate brings it: a ♂ that knows it ("
                        + f + (list.size() > 4 ? " or " + (list.size() - 4) + " more" : "") + ")"
                        + (ms.fathers.isEmpty() ? ", who gets it as an egg move first" : "") + ".");
                continue;
            }
            if (ms.lastFatherKnows) {
                lines.add(mv + ": the last father (" + name(lastFather) + ") learns it "
                        + (b.learnsByLevel(lastFather, ms.move) ? "by leveling up" : "from a TM")
                        + ". Have him know it at the last egg.");
            } else if (!ms.fathers.isEmpty()) {
                round++;
                StringBuilder f = new StringBuilder();
                for (int k = 0; k < Math.min(4, ms.fathers.size()); k++) {
                    f.append(k > 0 ? ", " : "").append(name(ms.fathers.get(k))).append(b.caught(ms.fathers.get(k)) ? "*" : "");
                    starred |= b.caught(ms.fathers.get(k));
                }
                lines.add(mv + ": first, a ♂ that knows it (" + f + (ms.fathers.size() > 4 ? " or " + (ms.fathers.size() - 4)
                        + " more" : "") + ") with a ♀ " + name(to) + ". Her daughter knows it" + (round > 1
                        ? " and the moves before" : "") + ": she is the next mother.");
            } else if (ms.bridge != 0) {
                round++;
                lines.add(mv + ": two eggs. A ♂ " + name(ms.bridgeFathers.get(0)) + " that knows it with a ♀ "
                        + name(ms.bridge) + "; her son with a ♀ " + name(to) + ". Her daughter is the next mother.");
            } else {
                lines.add(mv + ": no father here can pass it on.");
            }
        }
        if (herOwn && sources.size() > 1) {
            lines.add("One ♂ can bring them all: one that knows them, or one bred to know them first.");
        }
        if (round > 0) {
            lines.add("Then breed the IV route's last father with that daughter: the egg has her egg moves and his IVs.");
        }
        if (starred) {
            lines.add("* caught");
        }
        int hgt = 24;
        List<List<String>> wrapped = new ArrayList<>();
        for (String l : lines) {
            List<String> wl = art.wrapSmall(l, w - 30);
            wrapped.add(wl);
            hgt += wl.size() * 11 + 4;
        }
        int top = panel(c, 6, y, w - 12, hgt, "EGG MOVES PLAN");
        int ly = top - 2;
        for (List<String> wl : wrapped) {
            for (String l : wl) {
                art.small(c, l, 12, ly, GameArt.TEXT, GameArt.SHADOW);
                ly += 11;
            }
            ly += 4;
        }
        return y + hgt + 6;
    }

    private int tips(Canvas c, int w, int y) {
        String t = "DESTINY KNOT: a parent holding it passes 5 of the two parents' 12 IVs (3 without). A POWER item "
                + "makes one of them its stat, from its holder. EVERSTONE: the holder's Nature. Each egg, keep the "
                + "hatchling with the most perfect IVs and breed it again with its best parent: a 6 IV father and a "
                + "wild mother take a few eggs to reach 5 or 6. Egg moves come from either parent; moves from TMs "
                + "don't pass.";
        List<String> lines = art.wrapSmall(t, w - 28);
        int top = panel(c, 6, y, w - 12, 24 + lines.size() * 11, "TIPS");
        words(c, t, 12, top - 2, w - 28, GameArt.TEXT);
        return y + 24 + lines.size() * 11 + 6;
    }

    // ------------------------------------------------------------------------------------------------- pickers

    private void buildRows() {
        rows.clear();
        String q = search.trim().toUpperCase(Locale.US);
        Breeding b = calc();
        if (mode == PICK_FROM && mine) {
            Model m = v.app.model();
            List<JSONObject> all = new ArrayList<>();
            if (m != null) {
                JSONArray party = m.party();
                for (int i = 0; party != null && i < party.length(); i++) {
                    all.add(party.optJSONObject(i));
                }
                JSONArray boxes = m.boxes();
                for (int bx = 0; boxes != null && bx < boxes.length(); bx++) {
                    JSONArray ms = boxes.optJSONObject(bx) == null ? null : boxes.optJSONObject(bx).optJSONArray("m");
                    for (int i = 0; ms != null && i < ms.length(); i++) {
                        all.add(ms.optJSONObject(i));
                    }
                }
            }
            for (JSONObject mon : all) {
                if (mon == null || mon.optInt("s") <= 0 || mon.optInt("e") == 1 || !b.isReal(mon.optInt("s"))) {
                    continue;
                }
                String nm = name(mon.optInt("s")) + " " + art.plain(GameArt.hexText(mon.optString("n")));
                if (!q.isEmpty() && !nm.toUpperCase(Locale.US).contains(q)) {
                    continue;
                }
                rows.add(mon);
            }
            java.util.Collections.sort(rows, (x, y) -> perfect((JSONObject) y) - perfect((JSONObject) x));
            return;
        }
        for (int s = 1; s < b.count(); s++) {
            if (!b.isReal(s) || (mode == PICK_TO && !b.breeds(s) && b.family(s) == s && !b.isDitto(s))) {
                continue;
            }
            String nm = name(s);
            boolean num = q.matches("[0-9]+") && String.valueOf(b.dexOf(s)).startsWith(q);
            if (!q.isEmpty() && !num && !nm.toUpperCase(Locale.US).contains(q)) {
                continue;
            }
            rows.add(s);
        }
        java.util.Collections.sort(rows, (x, y) -> b.dexOf((Integer) x) - b.dexOf((Integer) y));
    }

    private static int perfect(JSONObject mon) {
        long iv = mon.optLong("iv");
        int n = 0;
        for (int k = 0; k < 6; k++) {
            n += ((iv >> (5 * k)) & 31) == 31 ? 1 : 0;
        }
        return mon.has("iv") ? n : -1;
    }

    private int drawPicker(Canvas c, int w) {
        buildRows();
        Breeding b = calc();
        DexPage.chip(c, art, fill, "BACK", 6, 4, false);
        hit(0, 0, 60, 22, H_BACK, 0);
        String title = mode == PICK_FROM ? "FROM: WHOSE IVs" : "TO: WHICH KIND";
        art.text(c, title, w - 8 - art.width(title), 2, 0xFFFFFFFF, 0xFF000000);
        int y = 26;
        if (mode == PICK_FROM) {
            int tx = 6;
            tx += DexPage.chip(c, art, fill, "MY POKéMON", tx, y, mine) + 4;
            hit(6, y, tx, y + 16, H_TAB, 1);
            int t2 = tx;
            tx += DexPage.chip(c, art, fill, "ANY KIND", tx, y, !mine) + 4;
            hit(t2, y, tx, y + 16, H_TAB, 0);
            y += 22;
        }
        // the search bar (DexPage's)
        fill.setColor(DexPage.BORDER);
        c.drawRect(6, y, w - 6, y + 20, fill);
        fill.setColor(typing ? 0xFFFFFFFF : 0xFFF0F0E8);
        c.drawRect(7, y + 1, w - 7, y + 19, fill);
        if (search.isEmpty() && !typing) {
            art.text(c, "Search: a name or a number", 12, y + 2, GameArt.GRAY_TEXT, GameArt.SHADOW);
        } else {
            int tw = art.text(c, search, 12, y + 2);
            if (typing && (System.currentTimeMillis() / 500) % 2 == 0) {
                fill.setColor(GameArt.TEXT);
                c.drawRect(13 + tw, y + 4, 14 + tw, y + 16, fill);
            }
            if (typing) {
                v.postInvalidateDelayed(500);
            }
        }
        if (!search.isEmpty()) {
            art.text(c, "x", w - 18, y + 2, GameArt.RED_TEXT, GameArt.SHADOW);
            hit(w - 30, y, w, y + 20, H_CLEAR, 0);
            hit(0, y, w - 30, y + 20, H_SEARCH, 0);
        } else {
            hit(0, y, w, y + 20, H_SEARCH, 0);
        }
        y += 26;
        if (rows.isEmpty()) {
            art.window(c, 0, 14, y, w - 28, 16);
            art.text(c, mode == PICK_FROM && mine && v.app.model() == null ? "Nothing from the game yet."
                    : "No POKéMON match.", 16, y, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return y + 30;
        }
        int rh = 36, first = Math.max(0, (scroll - y) / rh - 1), last = Math.min(rows.size(), (scroll - y + v.contentHeight()) / rh + 2);
        for (int i = first; i < last; i++) {
            Object o = rows.get(i);
            int ry = y + i * rh;
            art.window(c, 0, 10, ry + 2, w - 20, rh - 10);
            if (o instanceof JSONObject) {
                JSONObject mon = (JSONObject) o;
                int s = mon.optInt("s");
                art.icon(c, s, 10, ry, GameArt.ICON_PLAIN);
                int nw = art.textHex(c, mon.optString("n"), 44, ry + 1, GameArt.TEXT, GameArt.SHADOW);
                gender(c, mon.optInt("g"), 46 + nw, ry + 1);
                int p = perfect(mon);
                String ivs = p < 0 ? "" : p + " PERFECT IV" + (p == 1 ? "" : "s");
                art.small(c, ivs, w - 18 - art.smallWidth(ivs), ry + 4, p == 6 ? GameArt.RED_TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
                String nick = art.plain(GameArt.hexText(mon.optString("n"))), sub = "Lv" + mon.optInt("l");
                if (!nick.equals(name(s))) {
                    sub += "  " + name(s);   // (a nickname: its kind too)
                }
                art.small(c, sub, 44, ry + 15, GameArt.GRAY_TEXT, GameArt.SHADOW);
            } else {
                int s = (Integer) o;
                art.icon(c, s, 10, ry, b.caught(s) ? GameArt.ICON_PLAIN : GameArt.ICON_GRAY);
                art.text(c, String.format(Locale.US, "No%03d  ", b.dexOf(s)) + name(s), 44, ry + 1, GameArt.TEXT, GameArt.SHADOW);
                int[] g = b.groups(s);
                String eg = group(g[0]) + (g.length > 1 ? " / " + group(g[1]) : "");
                art.small(c, eg, 44, ry + 15, GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
        }
        for (int i = 0; i < rows.size(); i++) {
            hit(10, y + i * rh, w - 10, y + i * rh + rh - 4, H_ROW, i);
        }
        return y + rows.size() * rh;
    }

    // ------------------------------------------------------------------------------------------------- input

    @Override
    public void tap(int x, int y) {
        int cy = y + scroll;
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (x >= r[0] && x < r[2] && cy >= r[1] && cy < r[3]) {
                act(r[4], r[5]);
                return;
            }
        }
        if (typing) {
            typing = false;
            v.keyboard(false);
        }
    }

    private void open(int m) {
        if (mode == MAIN) {
            savedScroll = scroll;
        }
        mode = m;
        search = "";
        scroll = 0;
        v.buzz(false);
    }

    private void close() {
        if (typing) {
            typing = false;
            v.keyboard(false);
        }
        mode = MAIN;
        scroll = savedScroll;
    }

    private void act(int kind, int arg) {
        Breeding b = calc();
        switch (kind) {
            case H_BACK:
                back();
                return;
            case H_FROM:
                open(PICK_FROM);
                mine = v.app.model() != null;
                return;
            case H_TO:
                open(PICK_TO);
                return;
            case H_SWAP: {
                int t = from;
                from = to;
                to = t;
                fromMon = null;
                fromGender = -1;
                wanted.clear();
                dirty = true;
                v.buzz(false);
                return;
            }
            case H_MALE:
                fromGender = 0;
                dirty = true;
                return;
            case H_FEMALE:
                fromGender = Breeding.FEMALE_ONLY;
                dirty = true;
                return;
            case H_MOVE:
                if (wanted.contains(arg)) {
                    wanted.remove(Integer.valueOf(arg));
                } else if (wanted.size() < 4) {
                    wanted.add(arg);
                }
                dirty = true;
                v.buzz(false);
                return;
            case H_TAB:
                mine = arg == 1;
                scroll = 0;
                return;
            case H_SEARCH:
                typing = true;
                v.keyboard(true);
                return;
            case H_CLEAR:
                search = "";
                scroll = 0;
                return;
            case H_ROW: {
                Object o = arg < rows.size() ? rows.get(arg) : null;
                if (o instanceof JSONObject) {
                    pickMon((JSONObject) o);
                } else if (o != null && mode == PICK_FROM) {
                    from = (Integer) o;
                    fromMon = null;
                    fromGender = b.canBeMale(from) ? 0 : b.gender(from);
                    dirty = true;
                } else if (o != null) {
                    if (to == 0 || b.family(to) != b.family((Integer) o)) {
                        wanted.clear();
                    }
                    to = (Integer) o;
                    dirty = true;
                }
                v.buzz(false);
                close();
                return;
            }
            default:
        }
    }

    @Override
    public void back() {
        if (mode != MAIN) {
            close();
            return;
        }
        super.back();
    }

    @Override
    public void typed(String text) {
        String t = text.toUpperCase(Locale.US).replaceAll("[^A-Z0-9 .'-]", "");
        if (search.length() + t.length() <= 12) {
            search += t;
            scroll = 0;
        }
    }

    @Override
    public void backspace() {
        if (!search.isEmpty()) {
            search = search.substring(0, search.length() - 1);
        }
    }

    @Override
    public void typingDone() {
        typing = false;
    }

    @Override
    public void hidden() {
        if (typing) {
            typing = false;
            v.keyboard(false);
        }
    }
}
