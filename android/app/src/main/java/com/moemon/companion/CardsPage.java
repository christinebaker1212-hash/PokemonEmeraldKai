package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * CARDS (docs/COMPANION_APP.md "Card games"): TRIPLE TRIAD on the phone with the game's own engine. PLAY a FREE DUEL
 * against a duelist from where you are (their hand fitted to yours, as the game's CARD SCALING does; a win earns a
 * pack, 3 a day) or the DAILY PUZZLE (one a day, a SILVER pack); open the game's unopened PACKS; build DECKS; browse
 * the BINDER. The collection is the game's: changes go to it as orders, like box moves.
 */
public final class CardsPage extends Page {
    private static final String[] TRAINERS = {"YOUNGSTER", "LASS", "HIKER", "BEAUTY", "SAILOR", "PICNICKER", "CAMPER",
            "GENTLEMAN", "LADY", "FISHERMAN", "BIRD KEEPER", "PSYCHIC", "EXPERT", "COLLECTOR", "TRIATHLETE", "SCHOOL KID"};
    static final String[] LEVELS = {"BEGINNER", "CASUAL", "SKILLED", "EXPERT"};
    private final Paint fill = new Paint();
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    private int bottom;
    private volatile JSONObject puzzle;
    private volatile int puzzleDay = -1;
    private boolean puzzleBusy;

    public CardsPage(GameView v) {
        super(v);
    }

    private JSONObject tt() {
        Model m = v.app.model();
        return m == null ? null : m.snap.optJSONObject("tt");
    }

    static int day(JSONObject tt) {
        return tt != null && tt.optInt("day") > 0 ? tt.optInt("day") : (int) java.time.LocalDate.now().toEpochDay();
    }

    static int[] deckOf(JSONObject tt, int i) {
        JSONArray d = tt.optJSONArray("decks");
        JSONArray c = d == null || i < 0 || i >= d.length() ? null : d.optJSONObject(i).optJSONArray("c");
        int[] out = new int[5];
        for (int k = 0; c != null && k < 5; k++) {
            out[k] = c.optInt(k);
        }
        return out;
    }

    static boolean full(int[] deck) {
        for (int id : deck) {
            if (id == 0) {
                return false;
            }
        }
        return true;
    }

    static int owned(JSONObject tt, int id) {
        JSONArray o = tt.optJSONArray("owned");
        for (int i = 0; o != null && i < o.length(); i++) {
            if (o.optJSONArray(i).optInt(0) == id) {
                return o.optJSONArray(i).optInt(1);
            }
        }
        return 0;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.contentHeight());
    }

    private void tapArea(int x, int y, int w, int h, Runnable r) {
        hits.add(new int[]{x, y, w, h});
        actions.add(r);
    }

    private int heading(Canvas c, int w, int y, String title, String right) {
        fill.setColor(0xFF203868);
        c.drawRect(8, y + 2, w - 8, y + 18, fill);
        art.text(c, title, 14, y + 3, GameArt.WHITE_TEXT, 0xFF101830);
        if (right != null) {
            art.text(c, right, w - 14 - art.width(right), y + 3, 0xFFF8D030, 0xFF101830);
        }
        return y + 22;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        JSONObject tt = tt();
        TtArt ta = v.app.ttArt;
        int y = 24 - scroll;
        if (v.app.lastPack != null) {
            int[] cards = v.app.lastPack;
            int type = v.app.lastPackType;
            v.app.lastPack = null;
            v.showOver(new TtPackPage(v, type, cards));
        }
        if (tt == null || tt.optInt("on") == 0) {
            art.window(c, 0, 12, y + 4, w - 24, 72);
            int ty = y + 6;
            for (String l : wrap(v.app.model() == null ? "Link with your game to play TRIPLE TRIAD here."
                    : "Start TRIPLE TRIAD in your game first: a card seller in a POKéMON CENTER deals your first hand. "
                    + "Then your cards, decks and packs show here.", w - 32)) {
                art.text(c, l, 16, ty);
                ty += 14;
            }
            bottom = ty + scroll + 16;
            banner(c, w, "CARDS", "TRIPLE TRIAD");
            return;
        }
        int total = ta.maxId(), have = tt.optJSONArray("owned") == null ? 0 : tt.optJSONArray("owned").length();
        JSONObject today = tt.optJSONObject("today");
        JSONArray regions = ta.words == null ? null : ta.words.optJSONArray("regions");
        String region = regions == null ? "" : regions.optJSONArray(tt.optInt("region")).optString(0);
        // PLAY
        y = heading(c, w, y, "PLAY", null);
        int packsLeft = today == null ? 0 : today.optInt("packsMax") - today.optInt("packs");
        int[] deck = deckOf(tt, tt.optInt("active"));
        y = playCard(c, w, y, "FREE DUEL", "A duelist from " + region + ". Rules: " + ta.rulesText(rules(tt)) + ".",
                packsLeft > 0 ? "Win: a pack (" + packsLeft + " left today)" : "Prizes done for today: play for fun!",
                full(deck) ? "DUEL" : "DECK?", this::freeDuel);
        boolean solved = today != null && today.optInt("puzzles") >= today.optInt("puzzlesMax");
        y = playCard(c, w, y, "DAILY PUZZLE", "Three cards left. Find the play that wins against the best defense.",
                solved ? "Solved today! A new one tomorrow." : "Solve it: a SILVER pack",
                puzzleBusy ? "..." : "PLAY", this::dailyPuzzle);
        // VS FRIENDS
        y = drawFriends(c, w, y, tt);
        // PACKS
        JSONArray packs = tt.optJSONArray("packs");
        int unopened = 0;
        for (int i = 0; packs != null && i < packs.length(); i++) {
            unopened += packs.optInt(i);
        }
        if (unopened > 0) {
            y = heading(c, w, y, "PACKS", unopened + " TO OPEN");
            JSONArray names = ta.words == null ? null : ta.words.optJSONArray("packs");
            for (int i = 0; packs != null && i < packs.length(); i++) {
                if (packs.optInt(i) == 0) {
                    continue;
                }
                art.window(c, 0, 12, y + 4, w - 24, 26);
                ta.draw(c, 0, TtArt.SEAT_NONE, null, 16, y + 6, 18, 22, 1, 255);
                art.text(c, (names == null ? "" : names.optString(i)) + " PACK  x" + packs.optInt(i), 40, y + 10);
                final int pack = i;
                button(c, "OPEN", w - 76, y + 5, 60, v.app.client != null);
                tapArea(w - 76, y + 5, 60, 24, () -> openPack(pack));
                y += 32;
            }
        }
        // DECKS
        y = heading(c, w, y, "DECKS", null);
        JSONArray decks = tt.optJSONArray("decks");
        float mw = Math.min(30, (w - 120) / 5f), mh = mw * TtArt.H / TtArt.W;
        for (int i = 0; decks != null && i < decks.length(); i++) {
            JSONObject d = decks.optJSONObject(i);
            int[] cards = deckOf(tt, i);
            boolean active = i == tt.optInt("active");
            int rh = (int) mh + 10;
            art.window(c, 0, 12, y + 4, w - 24, rh);
            art.text(c, d.optString("n"), 18, y + 6, active ? GameArt.BLUE_TEXT : GameArt.TEXT, GameArt.SHADOW);
            if (active) {
                art.text(c, "IN USE", 18, y + 20, GameArt.GRAY_TEXT, GameArt.SHADOW);
            } else if (!full(cards)) {
                art.text(c, "EMPTY", 18, y + 20, GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
            for (int k = 0; k < 5; k++) {
                float x = w - 18 - (5 - k) * (mw + 2);
                if (cards[k] != 0) {
                    ta.draw(c, cards[k], TtArt.SEAT_PLAYER, null, x, y + 8, mw, mh, 1, 255);
                } else {
                    fill.setColor(0xFFD0D8E0);
                    c.drawRect(x, y + 8, x + mw, y + 8 + mh, fill);
                }
            }
            final int deckNo = i;
            tapArea(12, y + 4, w - 24, rh, () -> v.showOver(new TtDeckPage(v, deckNo)));
            y += rh + 6;
        }
        // BINDER
        y = heading(c, w, y, "BINDER", have + "/" + total);
        int cols = 6;
        float bw = (w - 24 - (cols - 1) * 3) / (float) cols, bh = bw * TtArt.H / TtArt.W;
        JSONArray owned = tt.optJSONArray("owned");
        java.util.Set<Integer> seen = new java.util.HashSet<>();
        JSONArray sa = tt.optJSONArray("seen");
        for (int i = 0; sa != null && i < sa.length(); i++) {
            seen.add(sa.optInt(i));
        }
        int n = 0;
        for (int id = 1; id <= total; id++) {
            if (ta.card(id) == null) {
                continue;
            }
            int count = owned(tt, id);
            float x = 12 + (n % cols) * (bw + 3), cy = y + 4 + (n / cols) * (bh + 3);
            n++;
            if (cy + bh < 0 || cy > h) {
                continue;   // (only what's on screen is drawn)
            }
            if (count > 0) {
                ta.draw(c, id, TtArt.SEAT_NONE, null, x, cy, bw, bh, 1, 255);
                if (count > 1) {
                    art.text(c, "x" + count, (int) x + 2, (int) (cy + bh - 13), GameArt.WHITE_TEXT, 0xFF000000);
                }
            } else if (seen.contains(id)) {
                ta.draw(c, id, TtArt.SEAT_NONE, null, x, cy, bw, bh, 1, 90);
            } else {
                ta.draw(c, 0, TtArt.SEAT_NONE, null, x, cy, bw, bh, 1, 140);
            }
            final int card = id, cnt = count;
            final boolean known = count > 0 || seen.contains(id);
            tapArea((int) x, (int) cy, (int) bw, (int) bh, () -> {
                if (known) {
                    v.showOver(new TtCardPage(v, card, cnt));
                } else {
                    v.message("A card you haven't come across yet.");
                }
            });
        }
        y += 4 + ((n + cols - 1) / cols) * (bh + 3);
        bottom = y + scroll + 12;
        banner(c, w, "CARDS", "TRIPLE TRIAD");
    }

    @Override
    public void shown() {
        v.app.refreshTtGames();
        polledAt = System.currentTimeMillis();
    }

    private long polledAt;
    private static final int[] FRIEND_RULES = {
            Tt.bit(Tt.RULE_OPEN), Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME), Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_PLUS),
            Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME) | Tt.bit(Tt.RULE_PLUS),
            Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME) | Tt.bit(Tt.RULE_PLUS) | Tt.bit(Tt.RULE_ELEMENTAL),
            Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_REVERSE) | Tt.bit(Tt.RULE_FALLEN_ACE)};

    private String nameOf(String hex) {
        return art.plain(GameArt.hexText(hex));
    }

    private static String statusLine(String st, String peer) {
        switch (st) {
            case "invited": return "Waiting for " + peer + " to answer.";
            case "declined": return peer + " turned it down.";
            case "yourturn": return "YOUR TURN!";
            case "theirturn": return peer + "'s turn.";
            case "won": return "You won!";
            case "lost": return peer + " won.";
            default: return "A draw.";
        }
    }

    private int drawFriends(Canvas c, int w, int y, JSONObject tt) {
        java.util.List<JSONObject> invites = v.app.ttInvites, games = v.app.ttGames();
        int yourTurn = 0;
        for (JSONObject g : games) {
            yourTurn += "yourturn".equals(TtFriendGame.status(g)) ? 1 : 0;
        }
        y = heading(c, w, y, "VS FRIENDS", invites.size() + yourTurn > 0 ? (invites.size() + yourTurn) + " WAITING" : null);
        TtArt ta = v.app.ttArt;
        for (JSONObject inv : invites) {
            String peer = nameOf(inv.optString("n"));
            java.util.List<String> lines = wrap("Rules: " + ta.rulesText(inv.optInt("rules")) + ".", w - 40);
            int ch = 22 + 14 * lines.size() + 28;
            art.window(c, 0, 12, y + 4, w - 24, ch);
            art.text(c, peer + " challenges you!", 18, y + 6, GameArt.BLUE_TEXT, GameArt.SHADOW);
            int ty = y + 20;
            for (String l : lines) {
                art.text(c, l, 18, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
                ty += 14;
            }
            final JSONObject invite = inv;
            button(c, "ACCEPT", w - 148, ty, 64, true);
            tapArea(w - 148, ty, 64, 24, () -> acceptInvite(invite));
            button(c, "DECLINE", w - 80, ty, 64, true);
            tapArea(w - 80, ty, 64, 24, () -> v.app.declineTt(invite));
            y += ch + 6;
        }
        for (JSONObject g : games) {
            String st = TtFriendGame.status(g), peer = nameOf(g.optString("pn"));
            boolean claim = TtFriendGame.canClaim(g);
            art.window(c, 0, 12, y + 4, w - 24, 38);
            art.text(c, "VS " + peer, 18, y + 6, GameArt.TEXT, GameArt.SHADOW);
            art.text(c, claim ? peer + " hasn't played in 3 days." : statusLine(st, peer), 18, y + 20,
                    "yourturn".equals(st) ? 0xFFC07818 : GameArt.GRAY_TEXT, GameArt.SHADOW);
            final JSONObject game = g;
            if (claim) {
                button(c, "CLAIM WIN", w - 96, y + 10, 80, true);
                tapArea(w - 96, y + 10, 80, 24, () -> {
                    try {
                        game.put("forfeit", 1);
                    } catch (org.json.JSONException ignored) {
                    }
                    v.app.saveTtGames();
                });
            } else if (!"invited".equals(st) && !"declined".equals(st)) {
                button(c, "yourturn".equals(st) ? "PLAY" : "SEE", w - 76, y + 10, 60, true);
                tapArea(w - 76, y + 10, 60, 24, () -> openGame(game));
            }
            y += 44;
        }
        button(c, "CHALLENGE A FRIEND", 12, y + 2, w - 24, true);
        tapArea(12, y + 2, w - 24, 24, this::challenge);
        if (System.currentTimeMillis() - polledAt > 15000) {
            polledAt = System.currentTimeMillis();
            v.app.refreshTtGames();
        }
        return y + 30;
    }

    private void challenge() {
        JSONObject tt = tt();
        int[] deck = deckOf(tt, tt.optInt("active"));
        if (!full(deck)) {
            v.message("Build a deck of five first: tap a deck below.");
            return;
        }
        Model m = v.app.model();
        JSONObject fs = m == null ? null : m.snap.optJSONObject("friends");
        JSONArray list = fs == null ? null : fs.optJSONArray("list");
        Menu menu = Menu.make(v, "CHALLENGE WHO?");
        int n = 0;
        for (int i = 0; list != null && i < list.length(); i++) {
            JSONObject f = list.optJSONObject(i);
            if (!f.has("pk")) {
                continue;
            }
            n++;
            menu.add(nameOf(f.optString("n")), () -> {
                Menu r = Menu.make(v, "WHICH RULES?");
                TtArt ta = v.app.ttArt;
                for (int rules : FRIEND_RULES) {
                    r.add(ta.rulesText(rules), () -> v.app.inviteTt(f, deck, rules, why -> v.message(why != null ? why
                            : "Challenge sent! " + nameOf(f.optString("n")) + " answers from their phone.")));
                }
                r.add("CANCEL", null);
                v.showMenu(r);
            });
        }
        if (n == 0) {
            v.message("Friends are the players you've been in a co-op room with. Play together once, and you can duel them here by turns.");
            return;
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void acceptInvite(JSONObject invite) {
        JSONObject tt = tt();
        int[] deck = deckOf(tt, tt.optInt("active"));
        if (!full(deck)) {
            v.message("Build a deck of five first: tap a deck below.");
            return;
        }
        v.app.acceptTt(invite, deck, game -> {
            if (game == null) {
                v.message("The co-op server didn't answer. Try again in a moment.");
            } else {
                openGame(game);
            }
        });
    }

    private void openGame(JSONObject g) {
        TtDuelPage.Setup s = new TtDuelPage.Setup();
        s.mode = TtDuelPage.MODE_FRIEND;
        s.game = g;
        s.rules = g.optInt("rules");
        s.seed = g.optInt("seed");
        s.first = TtFriendGame.localFirst(g);
        s.mine = TtFriendGame.hand(g, g.optInt("seat"));
        s.theirs = TtFriendGame.hand(g, 1 - g.optInt("seat"));
        s.who = "VS " + nameOf(g.optString("pn"));
        v.showOver(new TtDuelPage(v, s));
    }

    private int playCard(Canvas c, int w, int y, String title, String text, String prize, String label, Runnable go) {
        List<String> lines = wrap(text, w - 112);
        int ch = Math.max(46, 22 + 14 * lines.size() + 14);
        art.window(c, 0, 12, y + 4, w - 24, ch);
        art.text(c, title, 18, y + 6, GameArt.BLUE_TEXT, GameArt.SHADOW);
        int ty = y + 20;
        for (String l : lines) {
            art.text(c, l, 18, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
            ty += 14;
        }
        art.text(c, prize, 18, ty, 0xFFC07818, GameArt.SHADOW);
        button(c, label, w - 84, y + 6 + (ch - 24) / 2, 68, true);
        tapArea(w - 84, y + 6 + (ch - 24) / 2, 68, 24, go);
        return y + ch + 6;
    }

    // the region's rules as the phone plays them (no RANDOM: your deck is what you play; trades never happen here)
    static int rules(JSONObject tt) {
        return tt.optInt("rules") & ~Tt.bit(Tt.RULE_RANDOM);
    }

    private void freeDuel() {
        JSONObject tt = tt();
        int[] deck = deckOf(tt, tt.optInt("active"));
        if (!full(deck)) {
            v.message("Build a deck of five first: tap a deck below.");
            return;
        }
        int seed = (int) (System.nanoTime() & 0x7FFFFFFF), rules = rules(tt);
        int mine = Tt.handPower(deck) + tt.optInt("skill");
        try {
            JSONObject o = new JSONObject(Tt.opponent(tt.optInt("region"), seed, tt.optInt("tier"), day(tt), mine, rules));
            TtDuelPage.Setup s = new TtDuelPage.Setup();
            s.mode = TtDuelPage.MODE_DUEL;
            s.rules = rules;
            s.seed = seed;
            s.first = seed >> 3 & 1;
            s.level = o.optInt("level");
            s.mine = deck;
            JSONArray h = o.getJSONArray("hand");
            for (int i = 0; i < 5; i++) {
                s.theirs[i] = h.getInt(i);
            }
            s.harder = o.optInt("power") + o.optInt("levelRating") > mine + o.optInt("playerRating") + 40;
            s.who = TRAINERS[(seed >> 5) % TRAINERS.length] + "  " + LEVELS[Math.max(0, Math.min(3, o.optInt("level")))];
            v.showOver(new TtDuelPage(v, s));
        } catch (Exception e) {
            v.message("The cards couldn't be dealt. Try again.");
        }
    }

    private void dailyPuzzle() {
        JSONObject tt = tt();
        final int day = day(tt);
        if (puzzle != null && puzzleDay == day) {
            startPuzzle(puzzle);
            return;
        }
        if (puzzleBusy) {
            return;
        }
        puzzleBusy = true;
        new Thread(() -> {
            String p = Tt.puzzle(day);
            v.post(() -> {
                puzzleBusy = false;
                try {
                    puzzle = new JSONObject(p);
                    puzzleDay = day;
                    startPuzzle(puzzle);
                } catch (Exception e) {
                    v.message("No puzzle today. Try a FREE DUEL!");
                }
            });
        }, "tt-puzzle").start();
        v.invalidate();
    }

    private void startPuzzle(JSONObject p) {
        TtDuelPage.Setup s = new TtDuelPage.Setup();
        s.mode = TtDuelPage.MODE_PUZZLE;
        s.rules = p.optInt("rules");
        s.seed = p.optInt("seed");
        s.first = 0;
        s.moves = p.optJSONArray("moves");
        for (int i = 0; i < 5; i++) {
            s.mine[i] = p.optJSONArray("hand0").optInt(i);
            s.theirs[i] = p.optJSONArray("hand1").optInt(i);
        }
        s.who = "DAILY PUZZLE";
        v.showOver(new TtDuelPage(v, s));
    }

    private void openPack(int pack) {
        if (v.app.client == null) {
            v.message("Link with your game to open packs: the game deals the cards.");
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "ttopen").put("pack", pack));
            v.buzz(true);
        } catch (org.json.JSONException ignored) {
        }
    }

    @Override
    public void tap(int x, int y) {
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                actions.get(i).run();
                v.invalidate();
                return;
            }
        }
    }
}
