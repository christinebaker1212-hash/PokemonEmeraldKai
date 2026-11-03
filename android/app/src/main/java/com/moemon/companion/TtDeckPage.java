package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.RectF;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * A TRIPLE TRIAD deck on the phone: its five cards on top (tap one to take it out), your cards below (strongest
 * first; tap one to put it in), RENAME, USE IT and SAVE. The game's deck rules hold (one of each card, one five-star
 * card, at most two of four stars or more), and SAVE sends it to the game, which checks it again.
 */
public final class TtDeckPage extends Page {
    private final int deck;
    private final int[] cards;
    private String name;
    private boolean use, changed;
    private final Paint fill = new Paint(), line = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    private List<int[]> mine;   // [id, count], strongest first
    private int bottom;
    private boolean renaming;

    public TtDeckPage(GameView v, int deck) {
        super(v);
        this.deck = deck;
        JSONObject tt = tt();
        cards = CardsPage.deckOf(tt, deck);
        JSONArray d = tt == null ? null : tt.optJSONArray("decks");
        name = d == null ? "DECK" : d.optJSONObject(deck).optString("n");
        use = tt != null && tt.optInt("active") == deck;
        line.setStyle(Paint.Style.STROKE);
    }

    private JSONObject tt() {
        Model m = v.app.model();
        return m == null ? null : m.snap.optJSONObject("tt");
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.height());
    }

    private List<int[]> mine() {
        if (mine == null) {
            mine = new ArrayList<>();
            JSONObject tt = tt();
            JSONArray o = tt == null ? null : tt.optJSONArray("owned");
            for (int i = 0; o != null && i < o.length(); i++) {
                mine.add(new int[]{o.optJSONArray(i).optInt(0), o.optJSONArray(i).optInt(1)});
            }
            final TtArt ta = v.app.ttArt;
            mine.sort((a, b) -> {
                JSONObject ca = ta.card(a[0]), cb = ta.card(b[0]);
                int r = Integer.compare(cb == null ? 0 : cb.optInt("r"), ca == null ? 0 : ca.optInt("r"));
                return r != 0 ? r : Integer.compare(a[0], b[0]);
            });
        }
        return mine;
    }

    /** The game's deck rules (src/tt/tt_common.c Tt_DeckFits): why ID can't go in SLOT, or null */
    private String fits(int slot, int id) {
        TtArt ta = v.app.ttArt;
        JSONObject c = ta.card(id);
        int four = 0, five = 0;
        for (int i = 0; i < 5; i++) {
            if (i == slot || cards[i] == 0) {
                continue;
            }
            if (cards[i] == id) {
                return "A deck holds only one of each card.";
            }
            JSONObject o = ta.card(cards[i]);
            int st = o == null ? 0 : o.optInt("st");
            four += st >= 4 ? 1 : 0;
            five += st == 5 ? 1 : 0;
        }
        int st = c == null ? 0 : c.optInt("st");
        if (st == 5 && five > 0) {
            return "A deck holds only one five-star card.";
        }
        if (st >= 4 && four >= 2) {
            return "A deck holds at most two cards of four stars or more.";
        }
        return null;
    }

    private void tapArea(float x, float y, float w, float h, Runnable r) {
        hits.add(new int[]{(int) x, (int) y, (int) w, (int) h});
        actions.add(r);
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        TtArt ta = v.app.ttArt;
        fill.setColor(0xFF14284A);
        c.drawRect(0, 0, w, h, fill);
        int y = 4 - scroll;
        // the deck
        art.window(c, 0, 8, y + 2, w - 16, 18);
        art.text(c, name + (renaming ? "_" : ""), 14, y + 4, GameArt.TEXT, GameArt.SHADOW);
        String power = "POWER " + Tt.handPower(cards);
        art.text(c, power, w - 14 - art.width(power), y + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
        tapArea(8, y + 2, w / 2f, 18, this::rename);
        y += 26;
        float dw = (w - 16 - 4 * 4) / 5f, dh = dw * TtArt.H / TtArt.W;
        for (int i = 0; i < 5; i++) {
            float x = 8 + i * (dw + 4);
            if (cards[i] != 0) {
                ta.draw(c, cards[i], TtArt.SEAT_PLAYER, null, x, y, dw, dh, 1, 255);
            } else {
                fill.setColor(0xFF203A68);
                c.drawRoundRect(new RectF(x, y, x + dw, y + dh), 4, 4, fill);
                line.setColor(0xFF5078B8);
                c.drawRoundRect(new RectF(x, y, x + dw, y + dh), 4, 4, line);
            }
            final int slot = i;
            tapArea(x, y, dw, dh, () -> {
                if (cards[slot] != 0) {
                    cards[slot] = 0;
                    changed = true;
                    v.buzz(false);
                }
            });
        }
        y += (int) dh + 8;
        // the buttons
        int bw = (w - 16 - 8) / 3;
        button(c, use ? "IN USE" : "USE IT", 8, y, bw, !use);
        tapArea(8, y, bw, 24, () -> {
            use = true;
            changed = true;
        });
        button(c, "CANCEL", 12 + bw, y, bw, true);
        tapArea(12 + bw, y, bw, 24, () -> v.showOver(null));
        boolean ready = CardsPage.full(cards);
        button(c, "SAVE", 16 + bw * 2, y, bw, ready && changed);
        tapArea(16 + bw * 2, y, bw, 24, this::save);
        y += 30;
        // your cards
        art.text(c, "YOUR CARDS: TAP ONE TO ADD IT", 10, y, 0xFFB8C8E8, 0xFF000000);
        y += 16;
        int cols = 5;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * TtArt.H / TtArt.W;
        List<int[]> list = mine();
        for (int k = 0; k < list.size(); k++) {
            int id = list.get(k)[0];
            float x = 8 + (k % cols) * (cw + 4), cy = y + (k / cols) * (ch + 4);
            if (cy > h || cy + ch < 0) {
                continue;
            }
            boolean inDeck = false;
            for (int d : cards) {
                inDeck |= d == id;
            }
            ta.draw(c, id, TtArt.SEAT_NONE, null, x, cy, cw, ch, 1, inDeck ? 80 : 255);
            final int card = id;
            tapArea(x, cy, cw, ch, () -> add(card));
        }
        bottom = y + scroll + ((list.size() + cols - 1) / cols) * (int) (ch + 4) + 12;
    }

    private void add(int id) {
        int slot = -1;
        for (int i = 0; i < 5 && slot < 0; i++) {
            if (cards[i] == 0) {
                slot = i;
            }
        }
        if (slot < 0) {
            v.message("The deck is full: tap one of its cards to take it out first.");
            return;
        }
        String why = fits(slot, id);
        if (why != null) {
            v.message(why);
            return;
        }
        cards[slot] = id;
        changed = true;
        v.buzz(false);
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
            if (name.length() < 16 && (Character.isLetterOrDigit(ch) || ch == ' ' || ch == '-' || ch == '.')) {
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
        if (!CardsPage.full(cards)) {
            v.message("A deck needs five cards.");
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
            v.app.client.order(new JSONObject().put("op", "ttdeck").put("deck", deck).put("cards", a).put("name", name)
                    .put("active", use ? 1 : 0));
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
    public void back() {
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
