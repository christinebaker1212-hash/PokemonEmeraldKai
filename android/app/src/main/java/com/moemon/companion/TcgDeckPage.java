package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.TreeMap;

/**
 * A POKéMON card deck built on the phone (the game checks it as its own deck builder does: tcgdeck): the deck's cards
 * in a row of stacks (tap one to take a copy out), then your collection by kind (tap a card to put a copy in; basic
 * Energy never runs out), hold any card to see it up close. Sixty cards, a Basic POKéMON, no more than four of a
 * name. SAVE sends it to the game; USE IT makes it the deck you duel with.
 */
public final class TcgDeckPage extends Page {
    private static final String[] FILTERS = {"ALL", "POKéMON", "TRAINER", "ENERGY"};
    private final int deck;
    private final List<Integer> cards = new ArrayList<>();
    private String name;
    private boolean changed, renaming, use;
    private int filter, type = -1;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>(), holds = new ArrayList<>();
    private int bottom;

    public TcgDeckPage(GameView v, int deck) {
        super(v);
        this.deck = deck;
        JSONObject tcg = CardsPage.tcg(v);
        JSONObject d = tcg == null || tcg.optJSONArray("decks") == null ? null : tcg.optJSONArray("decks").optJSONObject(deck);
        JSONArray c = d == null ? null : d.optJSONArray("c");
        for (int i = 0; c != null && i < c.length(); i++) {
            if (c.optInt(i) != 0) {
                cards.add(c.optInt(i));
            }
        }
        name = d == null || d.optString("n").isEmpty() ? "DECK " + (deck + 1) : d.optString("n");
        use = tcg != null && tcg.optInt("active") == deck;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.height() + 30);
    }

    private int inDeck(int id) {
        int n = 0;
        for (int k : cards) {
            n += k == id ? 1 : 0;
        }
        return n;
    }

    private int sameName(int id) {
        String n = v.app.tcgArt.name(id);
        int count = 0;
        for (int k : cards) {
            count += v.app.tcgArt.name(k).equals(n) ? 1 : 0;
        }
        return count;
    }

    // the deck's words: what the rules say about it now
    private String verdict() {
        int[] d = new int[Tcg.DECK];
        if (cards.size() != Tcg.DECK) {
            return cards.size() + "/60 cards";
        }
        for (int i = 0; i < Tcg.DECK; i++) {
            d[i] = cards.get(i);
        }
        String why = Tcg.validate(d, CardsPage.ownedArray(CardsPage.tcg(v)));
        return why == null ? "60/60: ready to duel!" : why;
    }

    private void hit(float x, float y, float w, float h, Runnable tap, Runnable hold) {
        hits.add(new int[]{(int) x, (int) y, (int) w, (int) h});
        actions.add(tap);
        holds.add(hold);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        holds.clear();
        background(c, w, h, scroll);
        TcgArt ta = v.app.tcgArt;
        JSONObject tcg = CardsPage.tcg(v);
        int y = 22 - scroll;
        // the name, the count, what the rules say
        art.window(c, 0, 8, y, w - 16, 40);
        String shown = renaming ? name + ((System.currentTimeMillis() / 400) % 2 == 0 ? "_" : " ") : name;
        art.text(c, shown, 14, y + 2, renaming ? GameArt.BLUE_TEXT : GameArt.TEXT, GameArt.SHADOW);
        if (!renaming) {
            hit(8, y, w - 100, 20, this::rename, null);
        }
        String verdict = verdict();
        boolean ready = verdict.startsWith("60/60");
        art.text(c, verdict.length() > 40 ? verdict.substring(0, 40) : verdict, 14, y + 18,
                ready ? 0xFF308030 : 0xFFC05010, GameArt.SHADOW);
        if (renaming) {
            v.postInvalidateDelayed(400);
        }
        y += 44;
        int bw = (w - 16) / 3;
        button(c, "SAVE", 8, y, bw, changed && cards.size() == Tcg.DECK);
        hit(8, y, bw, 24, this::save, null);
        button(c, use ? "IN USE" : "USE IT", 8 + bw, y, bw, !use);
        hit(8 + bw, y, bw, 24, () -> {
            if (!use) {
                use = true;
                changed = true;
                if (cards.size() == Tcg.DECK) {
                    save();
                }
            }
        }, null);
        button(c, "EMPTY", 8 + bw * 2, y, bw, !cards.isEmpty());
        hit(8 + bw * 2, y, bw, 24, () -> {
            if (!cards.isEmpty()) {
                v.showMenu(Menu.confirm(v, "Take every card out of this deck?", () -> {
                    cards.clear();
                    changed = true;
                    v.invalidate();
                }));
            }
        }, null);
        y += 28;
        // the deck: a stack per card, sorted as the game's builder does (POKéMON by type, Trainers, Energy)
        heading(c, w, y, "IN THE DECK", cards.size() + "/60");
        y += 20;
        TreeMap<Long, Integer> stacks = new TreeMap<>();
        for (int id : cards) {
            stacks.put(sortKey(id), id);
        }
        int cols = 6;
        float mw = (w - 16 - (cols - 1) * 3) / (float) cols, mh = mw * 1.375f;
        int n = 0;
        for (int id : stacks.values()) {
            float x = 8 + (n % cols) * (mw + 3), cy = y + (n / cols) * (mh + 3);
            n++;
            if (cy + mh > -10 && cy < h + 10) {
                ta.draw(c, id, x, cy, mw, mh, v.pixelScale(), 255);
                badge(c, "x" + inDeck(id), x + mw - 2, cy + mh - 12);
            }
            final int card = id;
            hit(x, cy, mw, mh, () -> {
                cards.remove(Integer.valueOf(card));
                changed = true;
                v.buzz(false);
            }, () -> v.showOver(new TcgCardPage(v, card, CardsPage.owned(tcg, card), this)));
        }
        if (n == 0) {
            art.text(c, "Tap cards below to put them in.", 12, y, GameArt.WHITE_TEXT, 0xFF203040);
            n = 1;
            y += 14 - (int) (mh + 3);
        }
        y += (int) (((n + cols - 1) / cols) * (mh + 3)) + 6;
        // the collection, by kind (POKéMON by type too)
        heading(c, w, y, "YOUR CARDS", null);
        y += 20;
        int fw = (w - 16) / FILTERS.length;
        for (int i = 0; i < FILTERS.length; i++) {
            boolean on = filter == i;
            fill.setColor(on ? 0xFF3868A8 : 0xFFD8E0E8);
            c.drawRoundRect(new RectF(8 + i * fw + 1, y, 8 + (i + 1) * fw - 1, y + 16), 4, 4, fill);
            String t = FILTERS[i];
            art.text(c, t, 8 + i * fw + (fw - art.width(t)) / 2, y, on ? GameArt.WHITE_TEXT : GameArt.TEXT, on ? 0xFF102040 : GameArt.SHADOW);
            final int f = i;
            hit(8 + i * fw, y, fw, 16, () -> {
                filter = f;
                type = -1;
            }, null);
        }
        y += 20;
        if (filter == 1) {
            float r = 6, ox = 14;
            for (int t = 0; t < 7; t++) {
                ta.orb(c, t, ox + t * 18, y + 7, type == t ? 8 : r);
                final int tt = t;
                hit(ox + t * 18 - 8, y - 2, 16, 18, () -> type = type == tt ? -1 : tt, null);
            }
            y += 18;
        }
        cols = 5;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.375f;
        n = 0;
        for (int id = 1; id <= ta.maxId(); id++) {
            JSONObject k = ta.card(id);
            int owned = CardsPage.owned(tcg, id), basic = id <= 6 ? 1 : 0;
            if (k == null || (owned == 0 && basic == 0)) {
                continue;
            }
            int kind = k.optInt("k");
            if ((filter == 1 && kind != Tcg.POKEMON) || (filter == 2 && kind != Tcg.TRAINER) || (filter == 3 && kind != Tcg.ENERGY)
                    || (type >= 0 && k.optInt("t") != type)) {
                continue;
            }
            float x = 8 + (n % cols) * (cw + 4), cy = y + (n / cols) * (ch + 4);
            n++;
            int left = basic == 1 ? 99 : owned - inDeck(id);
            if (cy + ch > -10 && cy < h + 10) {
                ta.draw(c, id, x, cy, cw, ch, v.pixelScale(), left > 0 ? 255 : 110);
                badge(c, basic == 1 ? "FREE" : left + "/" + owned, x + cw - 2, cy + ch - 12);
            }
            final int card = id, have = owned;
            final boolean free = basic == 1;
            hit(x, cy, cw, ch, () -> add(card, free, have), () -> v.showOver(new TcgCardPage(v, card, have, this)));
        }
        y += (int) (((n + cols - 1) / cols) * (ch + 4));
        bottom = y + scroll + 10;
        banner(c, w, "DECK " + (deck + 1), cards.size() + "/60");
    }

    private void add(int id, boolean free, int owned) {
        if (cards.size() >= Tcg.DECK) {
            v.message("The deck has 60 cards: take one out first.");
            return;
        }
        if (!free && inDeck(id) >= owned) {
            v.message("You've put in every copy you have.");
            return;
        }
        if (id > 6 && sameName(id) >= 4) {
            v.message("No more than four cards with the same name.");
            return;
        }
        cards.add(id);
        changed = true;
        v.buzz(false);
    }

    // POKéMON by type then evolution line order, then Trainers, then Energy (the game's builder's order)
    private long sortKey(int id) {
        JSONObject k = v.app.tcgArt.card(id);
        int kind = k == null ? 3 : k.optInt("k");
        long group = kind == Tcg.POKEMON ? 0 : kind == Tcg.TRAINER ? 1 : 2;
        return group << 40 | (long) (k == null ? 0 : k.optInt("t")) << 32 | id;
    }

    private void badge(Canvas c, String t, float right, float y) {
        int tw = art.width(t) + 4;
        fill.setColor(0xE0102030);
        c.drawRoundRect(new RectF(right - tw, y, right, y + 11), 3, 3, fill);
        art.text(c, t, (int) right - tw + 2, (int) y - 3, GameArt.WHITE_TEXT, 0);
    }

    private void heading(Canvas c, int w, int y, String title, String right) {
        sectionBar(c, 8, w - 8, y + 2, 16, title, right);
    }

    private void rename() {
        renaming = true;
        v.keyboard(true);
    }

    @Override
    public void typed(String text) {
        if (!renaming) {
            return;
        }
        for (char ch : text.toUpperCase(java.util.Locale.ROOT).toCharArray()) {
            if (name.length() < 20 && (Character.isLetterOrDigit(ch) || ch == ' ' || ch == '-' || ch == '.' || ch == '\'')) {
                name += ch;
                changed = true;
            }
        }
        v.invalidate();
    }

    @Override
    public void backspace() {
        if (renaming && !name.isEmpty()) {
            name = name.substring(0, name.length() - 1);
            changed = true;
            v.invalidate();
        }
    }

    @Override
    public void typingDone() {
        renaming = false;
        if (name.trim().isEmpty()) {
            name = "DECK " + (deck + 1);
        }
        v.keyboard(false);
    }

    private void save() {
        if (cards.size() != Tcg.DECK) {
            v.message("A deck needs exactly 60 cards (" + cards.size() + " now).");
            return;
        }
        String why = verdict();
        if (!why.startsWith("60/60")) {
            v.message(why);
            return;
        }
        if (v.app.client == null) {
            v.message("Link with your game first.");
            return;
        }
        try {
            JSONArray a = new JSONArray();
            for (int id : cards) {
                a.put(id);
            }
            v.app.client.order(new JSONObject().put("op", "tcgdeck").put("deck", deck).put("cards", a).put("name", name)
                    .put("active", use ? 1 : 0));
            changed = false;
            v.buzz(true);
            v.showOver(null);
        } catch (org.json.JSONException ignored) {
        }
    }

    @Override
    public void tap(int x, int y) {
        if (renaming) {
            typingDone();
        }
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                actions.get(i).run();
                v.invalidate();
                return;
            }
        }
    }

    @Override
    public boolean longPress(int x, int y) {
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (in(x, y, r[0], r[1], r[2], r[3]) && holds.get(i) != null) {
                holds.get(i).run();
                return true;
            }
        }
        return false;
    }

    @Override
    public void back() {
        if (renaming) {
            typingDone();
            return;
        }
        if (changed) {
            Menu m = Menu.make(v, "LEAVE WITHOUT SAVING?");
            m.add("LEAVE", () -> v.showOver(null));
            m.add("KEEP EDITING", null);
            v.showMenu(m);
            return;
        }
        v.showOver(null);
    }

    @Override
    public boolean swapsWords() {
        return false;   // (the card games' own words: the POKéMON card game is the POKéMON card game)
    }
}
