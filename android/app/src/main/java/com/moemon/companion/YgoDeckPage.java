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
 * A YU-GI-OH! deck built on the phone (the game checks it as its own deck builder does: ygodeck): the Main Deck's and
 * the Fusion Deck's cards in rows of stacks (tap one to take a copy out), then your collection by kind (tap a card to
 * put a copy in: a Fusion Monster goes to the Fusion Deck), hold any card to see it up close. 40 to 60 cards, up to 15
 * in the Fusion Deck, no more than three of a card or what your Forbidden & Limited list allows. SAVE sends it to the
 * game; USE IT makes it the deck you duel with.
 */
public final class YgoDeckPage extends Page {
    private static final String[] FILTERS = {"ALL", "MONSTER", "SPELL", "TRAP", "FUSION"};
    private final int deck;
    private final List<Integer> cards = new ArrayList<>(), extra = new ArrayList<>();
    private String name;
    private boolean changed, renaming, use;
    private int filter;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>(), holds = new ArrayList<>();
    private int bottom;

    public YgoDeckPage(GameView v, int deck) {
        super(v);
        this.deck = deck;
        JSONObject ygo = CardsPage.ygo(v);
        JSONObject d = ygo == null || ygo.optJSONArray("decks") == null ? null : ygo.optJSONArray("decks").optJSONObject(deck);
        JSONArray m = d == null ? null : d.optJSONArray("m"), x = d == null ? null : d.optJSONArray("x");
        for (int i = 0; m != null && i < m.length(); i++) {
            cards.add(m.optInt(i));
        }
        for (int i = 0; x != null && i < x.length(); i++) {
            extra.add(x.optInt(i));
        }
        name = d == null || d.optString("n").isEmpty() ? "DECK " + (deck + 1) : d.optString("n");
        use = ygo != null && ygo.optInt("active") == deck;
        v.app.ygoArt.engine();
        Ygo.useBanlist(ygo == null ? "" : ygo.optString("list"));
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
        for (int k : extra) {
            n += k == id ? 1 : 0;
        }
        return n;
    }

    private static int[] array(List<Integer> l) {
        int[] out = new int[l.size()];
        for (int i = 0; i < out.length; i++) {
            out[i] = l.get(i);
        }
        return out;
    }

    // the deck's words: what the rules say about it now
    private String verdict() {
        String why = Ygo.validate(array(cards), array(extra), CardsPage.ygoOwned(CardsPage.ygo(v)));
        return why == null ? cards.size() + " + " + extra.size() + ": ready to duel!" : why;
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
        YgoArt ya = v.app.ygoArt;
        JSONObject ygo = CardsPage.ygo(v);
        int[] owned = CardsPage.ygoOwned(ygo);
        int y = 22 - scroll;
        art.window(c, 0, 8, y, w - 16, 40);
        String shown = renaming ? name + ((System.currentTimeMillis() / 400) % 2 == 0 ? "_" : " ") : name;
        art.text(c, shown, 14, y + 2, renaming ? GameArt.BLUE_TEXT : GameArt.TEXT, GameArt.SHADOW);
        if (!renaming) {
            hit(8, y, w - 100, 20, this::rename, null);
        }
        String verdict = verdict();
        boolean ready = verdict.endsWith("ready to duel!");
        java.util.List<String> vl = wrap(verdict, w - 32);
        art.text(c, vl.get(0), 14, y + 18, ready ? 0xFF308030 : 0xFFC05010, GameArt.SHADOW);
        if (renaming) {
            v.postInvalidateDelayed(400);
        }
        y += 44;
        int bw = (w - 16) / 3;
        button(c, "SAVE", 8, y, bw, changed && ready);
        hit(8, y, bw, 24, this::save, null);
        button(c, use ? "IN USE" : "USE IT", 8 + bw, y, bw, !use);
        hit(8 + bw, y, bw, 24, () -> {
            if (!use) {
                use = true;
                changed = true;
                if (verdict().endsWith("ready to duel!")) {
                    save();
                }
            }
        }, null);
        button(c, "EMPTY", 8 + bw * 2, y, bw, !cards.isEmpty() || !extra.isEmpty());
        hit(8 + bw * 2, y, bw, 24, () -> {
            if (!cards.isEmpty() || !extra.isEmpty()) {
                v.showMenu(Menu.confirm(v, "Take every card out of this deck?", () -> {
                    cards.clear();
                    extra.clear();
                    changed = true;
                    v.invalidate();
                }));
            }
        }, null);
        y += 28;
        y = stacks(c, w, h, y, "MAIN DECK", cards, cards.size() + "/40-60", owned);
        y = stacks(c, w, h, y, "FUSION DECK", extra, extra.size() + "/15", owned);
        // the collection, by kind
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
            hit(8 + i * fw, y, fw, 16, () -> filter = f, null);
        }
        y += 20;
        int cols = 5, n = 0;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.46f;
        for (int id = 1; id <= ya.maxId() && id < owned.length; id++) {
            JSONObject k = ya.card(id);
            int have = owned[id];
            if (k == null || have == 0 || k.optInt("ok") == 0) {
                continue;
            }
            int f = k.optInt("f");
            if ((filter == 1 && !(YgoArt.monster(k) && f != Ygo.FRAME_FUSION)) || (filter == 2 && f != Ygo.FRAME_SPELL)
                    || (filter == 3 && f != Ygo.FRAME_TRAP) || (filter == 4 && f != Ygo.FRAME_FUSION)) {
                continue;
            }
            float x = 8 + (n % cols) * (cw + 4), cy = y + (n / cols) * (ch + 4);
            n++;
            int left = have - inDeck(id);
            if (cy + ch > -10 && cy < h + 10) {
                ya.draw(c, id, x, cy, cw, ch, v.pixelScale(), left > 0 ? 255 : 110);
                badge(c, left + "/" + have, x + cw - 2, cy + ch - 12);
            }
            final int card = id;
            hit(x, cy, cw, ch, () -> add(card, have), () -> v.showOver(new YgoCardPage(v, card, have, this)));
        }
        y += (int) (((n + cols - 1) / cols) * (ch + 4));
        bottom = y + scroll + 10;
        banner(c, w, "DECK " + (deck + 1), cards.size() + " + " + extra.size());
    }

    // a deck part as stacks (a card and how many): tap one to take a copy out
    private int stacks(Canvas c, int w, int h, int y, String title, List<Integer> part, String count, int[] owned) {
        heading(c, w, y, title, count);
        y += 20;
        java.util.TreeMap<Long, Integer> sorted = new java.util.TreeMap<>();
        for (int id : part) {
            JSONObject k = v.app.ygoArt.card(id);
            int f = k == null ? 9 : k.optInt("f");
            sorted.put((long) (f <= Ygo.FRAME_RITUAL ? 0 : f) << 32 | id, id);
        }
        int cols = 6, n = 0;
        float mw = (w - 16 - (cols - 1) * 3) / (float) cols, mh = mw * 1.46f;
        for (int id : sorted.values()) {
            float x = 8 + (n % cols) * (mw + 3), cy = y + (n / cols) * (mh + 3);
            n++;
            if (cy + mh > -10 && cy < h + 10) {
                v.app.ygoArt.draw(c, id, x, cy, mw, mh, v.pixelScale(), 255);
                int copies = 0;
                for (int k : part) {
                    copies += k == id ? 1 : 0;
                }
                badge(c, "x" + copies, x + mw - 2, cy + mh - 12);
            }
            final int card = id;
            hit(x, cy, mw, mh, () -> {
                part.remove(Integer.valueOf(card));
                changed = true;
                v.buzz(false);
            }, () -> v.showOver(new YgoCardPage(v, card, id < owned.length ? owned[id] : 0, this)));
        }
        if (n == 0) {
            art.text(c, "Tap cards below to put them in.", 12, y, GameArt.WHITE_TEXT, 0xFF203040);
            return y + 18;
        }
        return y + (int) (((n + cols - 1) / cols) * (mh + 3)) + 6;
    }

    private void add(int id, int owned) {
        JSONObject k = v.app.ygoArt.card(id);
        boolean fusion = YgoArt.extraDeck(k);
        List<Integer> part = fusion ? extra : cards;
        if (part.size() >= (fusion ? Ygo.EXTRA_MAX : Ygo.DECK_MAX)) {
            v.message(fusion ? "The Fusion Deck holds 15 cards." : "The Main Deck holds 60 cards.");
            return;
        }
        if (inDeck(id) >= owned) {
            v.message("You've put in every copy you have.");
            return;
        }
        int limit = k == null ? 3 : k.optInt("lim", 3);
        if (inDeck(id) >= limit) {
            v.message(limit == 0 ? "That card is Forbidden." : "No more than " + limit + (limit == 1 ? " copy" : " copies") + " of that card.");
            return;
        }
        part.add(id);
        changed = true;
        v.buzz(false);
    }

    private void badge(Canvas c, String t, float right, float y) {
        int tw = art.width(t) + 4;
        fill.setColor(0xE0102030);
        c.drawRoundRect(new RectF(right - tw, y, right, y + 11), 3, 3, fill);
        art.text(c, t, (int) right - tw + 2, (int) y - 3, GameArt.WHITE_TEXT, 0);
    }

    private void heading(Canvas c, int w, int y, String title, String right) {
        fill.setColor(0xFF203868);
        c.drawRect(8, y + 2, w - 8, y + 18, fill);
        art.text(c, title, 14, y + 3, GameArt.WHITE_TEXT, 0xFF101830);
        if (right != null) {
            art.text(c, right, w - 14 - art.width(right), y + 3, 0xFFF8D030, 0xFF101830);
        }
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
        String why = verdict();
        if (!why.endsWith("ready to duel!")) {
            v.message(why);
            return;
        }
        if (v.app.client == null) {
            v.message("Link with your game first.");
            return;
        }
        try {
            JSONArray m = new JSONArray(), x = new JSONArray();
            for (int id : cards) {
                m.put(id);
            }
            for (int id : extra) {
                x.put(id);
            }
            v.app.client.order(new JSONObject().put("op", "ygodeck").put("deck", deck).put("main", m).put("extra", x)
                    .put("name", name).put("active", use ? 1 : 0));
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
}
