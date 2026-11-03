package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * A POKéMON card duel on the phone, upright: their Bench and Active Pokémon at the top, yours below, your hand along
 * the bottom. The game's own engine plays it (Tcg) and the game's duel board decides what each move asks for
 * (src/tcg/tcg_flow.c): tap a card in your hand or in play to see it up close with what it can do, then answer the
 * move's questions on the table (the spaces that can be chosen glow), in a card picker, or from a list. The other side
 * is the game's AI (a FREE DUEL: a win sends a booster to the game, 3 a day; CARD SCALING learns from every duel) or a
 * friend by turns. What happens is called out as it happens: attacks, coins, damage, Knock Outs, Prize cards.
 */
public final class TcgDuelPage extends Page {
    public static final int MODE_DUEL = 0, MODE_FRIEND = 1;

    public static final class Setup {
        public int mode, seed, prizes = 6, level = 1, myRating, oppRating, seat;
        public int[] mine, theirs;
        public String who = "A CARD PLAYER", deckName = "";
        public JSONObject game;   // (a friend's duel: Companion.tcgGames' record)
    }

    private final Setup s;
    private final int seat;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG), line = new Paint(Paint.ANTI_ALIAS_FLAG);
    private long duel, flow;
    private JSONObject st = new JSONObject(), opts = new JSONObject();
    private int lastSeq;
    // a move in the making: the flow's question (null: none)
    private JSONObject q;
    private final List<Integer> chosen = new ArrayList<>();
    // a card up close: 0 none, 1 in the hand (index), 2 yours in play (slot), 3 theirs in play (slot), 4 a pile
    private int sheet, sheetIndex;
    private JSONArray pile;   // (a pile to look through: a discard pile)
    private String pileTitle = "";
    // what happens, called out in turn
    private final List<Object[]> beats = new ArrayList<>();   // [due ms, kind, text, who, slot, value, card]
    private String caption = "";
    private int captionCard;
    private long captionUntil, busyUntil, aiAt;
    private final List<Object[]> floats = new ArrayList<>();   // [start ms, who, slot, text, color]
    private int coin = -1;
    private long coinAt;
    private boolean ended, reported;
    private String endLine = "";
    private int applied;   // (a friend's duel: the record's moves played here so far)
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    // layout (each draw): where each player's six spaces are
    private final float[][] slotX = new float[2][6], slotY = new float[2][6], slotW = new float[2][6], slotH = new float[2][6];
    private float scale = 1;

    public TcgDuelPage(GameView v, Setup setup) {
        super(v);
        this.s = setup;
        this.seat = setup.mode == MODE_FRIEND ? setup.seat : 0;
        if (s.mode == MODE_FRIEND && s.game != null) {
            duel = TcgFriendGame.replay(s.game);
            applied = TcgFriendGame.moves(s.game).length();
        } else {
            duel = Tcg.start(s.mine, s.theirs, s.seed, s.prizes);
        }
        flow = Tcg.flowNew();
        refresh();
        lastSeq = st.optInt("seq");
        busyUntil = System.currentTimeMillis() + 500;
        say(seat == st.optInt("actor") ? (st.optInt("phase") == Tcg.SETUP ? "Place your POKéMON!" : "YOUR TURN")
                : "", 0, 900);
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public void hidden() {
        if (duel != 0) {
            Tcg.end(duel);
            duel = 0;
        }
        if (flow != 0) {
            Tcg.flowEnd(flow);
            flow = 0;
        }
    }

    private TcgArt ta() {
        return v.app.tcgArt;
    }

    private void refresh() {
        if (duel == 0) {
            return;
        }
        try {
            st = new JSONObject(Tcg.state(duel, seat));
            opts = new JSONObject(Tcg.options(duel, seat));
        } catch (Exception ignored) {
        }
    }

    private JSONObject me() {
        return st.optJSONArray("p") == null ? new JSONObject() : st.optJSONArray("p").optJSONObject(seat);
    }

    private JSONObject them() {
        return st.optJSONArray("p") == null ? new JSONObject() : st.optJSONArray("p").optJSONObject(1 - seat);
    }

    private JSONObject field(int who, int slot) {
        JSONArray p = st.optJSONArray("p");
        JSONArray f = p == null ? null : p.optJSONObject(who).optJSONArray("field");
        return f == null || f.isNull(slot) ? null : f.optJSONObject(slot);
    }

    private boolean idle() {
        return beats.isEmpty() && System.currentTimeMillis() >= busyUntil && aiAt == 0;
    }

    private boolean myMove() {
        return !ended && st.optInt("actor") == seat && idle();
    }

    // ---------------------------------------------------------------------------------------------- the moves

    /** Begin a move (Tcg.FLOW_*) and follow its questions */
    private void begin(int kind, int index) {
        if (!myMove()) {
            return;
        }
        chosen.clear();
        answer(Tcg.flowBegin(flow, duel, seat, kind, index));
    }

    private void answer(String json) {
        try {
            q = new JSONObject(json);
        } catch (Exception e) {
            q = null;
            return;
        }
        sheet = 0;
        chosen.clear();
        scroll = 0;
        switch (q.optString("q")) {
            case "done":
                JSONObject a = q.optJSONObject("action");
                q = null;
                act(a);
                break;
            case "fail":
                v.message(q.optString("error"));
                v.buzz(false);
                q = null;
                break;
            case "menu":
                showChoices();
                break;
            default:
                break;   // (a space on the table, or cards from a pile: drawn by the page)
        }
        v.invalidate();
    }

    private void cancel() {
        q = null;
        chosen.clear();
        v.invalidate();
    }

    private void showChoices() {
        JSONArray menu = q.optJSONArray("menu");
        Menu m = Menu.make(v, q.optString("prompt"));
        for (int i = 0; menu != null && i < menu.length(); i++) {
            JSONArray row = menu.optJSONArray(i);
            final int value = row.optInt(1);
            final boolean ok = row.optInt(2) == 1;
            final String why = row.optString(3);
            m.add(row.optString(0), () -> {
                if (!ok) {
                    v.message(why.isEmpty() ? "That can't be chosen." : why);
                    v.post(this::showChoices);
                    return;
                }
                answer(Tcg.flowMenu(flow, duel, value));
            });
        }
        m.add("CANCEL", this::cancel);
        v.showMenu(m);
    }

    /** An action for the rules: applied here, its events called out (a friend's duel: sent to them too) */
    private void act(JSONObject action) {
        if (duel == 0 || action == null) {
            return;
        }
        int actor = st.optInt("actor");
        try {
            JSONObject r = new JSONObject(Tcg.apply(duel, action.toString()));
            if (r.optInt("ok") != 1) {
                if (actor == seat) {
                    v.message(r.optString("error"));
                    v.buzz(false);
                }
                return;
            }
        } catch (Exception e) {
            return;
        }
        if (actor == seat && s.mode == MODE_FRIEND && s.game != null) {
            applied++;
            v.app.postTcgMove(s.game, action);
        }
        v.buzz(false);
        refresh();
        callOut();
    }

    // the events since the last move, in time
    private void callOut() {
        JSONArray ev;
        try {
            ev = new JSONArray(Tcg.events(duel, lastSeq, seat));
        } catch (Exception e) {
            return;
        }
        lastSeq = st.optInt("seq");
        long t = Math.max(System.currentTimeMillis(), lastDue());
        TcgArt a = ta();
        for (int i = 0; i < ev.length(); i++) {
            JSONArray e = ev.optJSONArray(i);
            int type = e.optInt(1), who = e.optInt(2), slot = e.optInt(3), flags = e.optInt(4), value = e.optInt(5),
                    card = e.optInt(6);
            boolean mine = who == seat;
            String name = card != 0 ? a.name(card) : nameAt(who, slot);
            switch (type) {
                case Tcg.EV_TURN:
                    t = beat(t, "say", mine ? "YOUR TURN" : theirName() + "'S TURN", who, 0, 0, 0, 800);
                    break;
                case Tcg.EV_ATTACK: {
                    JSONObject c = a.card(card);
                    JSONArray atk = c == null ? null : c.optJSONArray("a");
                    String an = atk == null || slot >= atk.length() ? "" : atk.optJSONObject(slot).optString("n");
                    t = beat(t, "say", a.name(card) + ": " + an + "!", who, 0, 0, card, 1000);
                    break;
                }
                case Tcg.EV_COIN:
                    t = beat(t, "coin", value == 1 ? "HEADS!" : "TAILS!", who, 0, value, 0, 750);
                    break;
                case Tcg.EV_DAMAGE:
                    t = beat(t, "float", "-" + value + ((flags & Tcg.DMG_WEAK) != 0 ? "!" : ""), who, slot,
                            (flags & Tcg.DMG_WEAK) != 0 ? 0xFFFF5040 : 0xFFF86830, 0, 420);
                    break;
                case Tcg.EV_HEAL:
                    t = beat(t, "float", "+" + value, who, slot, 0xFF60D860, 0, 380);
                    break;
                case Tcg.EV_NO_DAMAGE:
                    t = beat(t, "float", "NO DAMAGE", who, slot, 0xFFB8C8E8, 0, 450);
                    break;
                case Tcg.EV_STATUS: {
                    String c = value == Tcg.ASLEEP ? "Asleep" : value == Tcg.PARALYZED ? "Paralyzed"
                            : value == Tcg.CONFUSED ? "Confused" : flags > 0 ? "Poisoned" : "";
                    if (!c.isEmpty()) {
                        t = beat(t, "float", c.toUpperCase() + "!", who, slot, 0xFFC890F8, 0, 500);
                    }
                    break;
                }
                case Tcg.EV_KO:
                    t = beat(t, "say", name + " is Knocked Out!", who, slot, 0, card, 1000);
                    break;
                case Tcg.EV_PRIZE:
                    t = beat(t, "say", mine ? "You take a Prize card." : theirName() + " takes a Prize card.", who, 0, 0, 0, 650);
                    break;
                case Tcg.EV_TRAINER:
                    if (!mine) {
                        t = beat(t, "say", theirName() + " plays " + a.name(card) + ".", who, 0, 0, card, 1100);
                    }
                    break;
                case Tcg.EV_PLAY:
                    if (!mine && st.optInt("phase") != Tcg.SETUP) {
                        t = beat(t, "say", a.name(card) + " comes onto the Bench.", who, slot, 0, 0, 600);
                    }
                    break;
                case Tcg.EV_EVOLVE:
                    t = beat(t, "say", "It evolved into " + a.name(card) + "!", who, slot, 0, card, 900);
                    break;
                case Tcg.EV_ATTACH:
                    if (!mine) {
                        t = beat(t, "say", a.name(card) + " is attached to " + nameAt(who, slot) + ".", who, slot, 0, 0, 650);
                    }
                    break;
                case Tcg.EV_RETREAT:
                case Tcg.EV_SWITCH:
                case Tcg.EV_PROMOTE:
                    t = beat(t, "say", nameAt(who, 0) + " is now Active.", who, 0, 0, 0, 600);
                    break;
                case Tcg.EV_RETREAT_FAIL:
                    t = beat(t, "say", "It's Confused and couldn't retreat!", who, 0, 0, 0, 800);
                    break;
                case Tcg.EV_POWER:
                    t = beat(t, "say", nameAt(who, slot) + "'s Pokémon Power!", who, slot, 0, card, 800);
                    break;
                case Tcg.EV_PEEK:
                    if (mine && card != 0) {
                        t = beat(t, "say", "You looked at " + a.name(card) + ".", who, 0, 0, card, 1500);
                    }
                    break;
                case Tcg.EV_DEVOLVE:
                    t = beat(t, "say", "It devolved!", who, slot, 0, 0, 700);
                    break;
                case Tcg.EV_TO_HAND:
                    t = beat(t, "say", "It went back to the hand.", who, 0, 0, 0, 700);
                    break;
                case Tcg.EV_SUDDEN_DEATH:
                    t = beat(t, "say", "SUDDEN DEATH! One Prize card each.", who, 0, 0, 0, 1400);
                    break;
                case Tcg.EV_WIN:
                    t = beat(t, "say", (mine ? "YOU WIN!" : theirName() + " WINS") + "  " + new String[]{"",
                            mine ? "No POKéMON left on their side." : "No POKéMON left on your side.",
                            mine ? "Their deck ran out." : "Your deck ran out.",
                            mine ? "They conceded." : "You conceded."}[Math.min(3, value)], who, 0, 0, 0, 1400);
                    break;
                default:
                    break;
            }
        }
        beats.add(new Object[]{t, "end", "", 0, 0, 0, 0});
    }

    private long lastDue() {
        return beats.isEmpty() ? 0 : (long) beats.get(beats.size() - 1)[0];
    }

    private long beat(long at, String kind, String text, int who, int slot, int value, int card, int ms) {
        beats.add(new Object[]{at, kind, text, who, slot, value, card});
        return at + ms;
    }

    private void say(String text, int card, int ms) {
        caption = text;
        captionCard = card;
        captionUntil = System.currentTimeMillis() + ms;
    }

    private String theirName() {
        String n = s.who;
        int sp = n.indexOf("  ");
        return (sp > 0 ? n.substring(0, sp) : n).toUpperCase();
    }

    private String nameAt(int who, int slot) {
        JSONObject f = field(who, slot);
        return f == null ? "It" : ta().name(f.optInt("id"));
    }

    private void runBeats() {
        long now = System.currentTimeMillis();
        while (!beats.isEmpty() && (long) beats.get(0)[0] <= now) {
            Object[] b = beats.remove(0);
            String kind = (String) b[1];
            switch (kind) {
                case "say":
                    say((String) b[2], (int) b[6], 1100);
                    break;
                case "coin":
                    coin = (int) b[5];
                    coinAt = now;
                    say((String) b[2], 0, 750);
                    v.buzz(false);
                    break;
                case "float":
                    floats.add(new Object[]{now, b[3], b[4], b[2], b[5]});
                    break;
                case "end":
                    busyUntil = now + 250;
                    break;
                default:
                    break;
            }
        }
        // the other side's move: the game's AI after a moment to think, or a friend's from their record
        if (!ended && idle() && st.optInt("actor") >= 0 && st.optInt("actor") != seat) {
            if (s.mode == MODE_DUEL) {
                aiAt = now + 650;
            } else if (s.game != null) {
                JSONArray moves = TcgFriendGame.moves(s.game);
                if (moves.length() > applied) {
                    JSONObject m = moves.optJSONObject(applied++);
                    act(m);
                } else if (now - polledAt > 8000) {
                    polledAt = now;
                    v.app.refreshTcgGames();
                }
            }
        }
        if (aiAt > 0 && now >= aiAt) {
            aiAt = 0;
            try {
                String a = Tcg.ai(duel, s.level);
                if (a != null) {
                    int before = st.optInt("seq");
                    act(new JSONObject(a));
                    if (st.optInt("seq") == before && st.optInt("actor") != seat) {
                        // (the AI's move refused: its turn ends, as on the game's table)
                        act(new JSONObject().put("t", st.optInt("phase") == Tcg.SETUP ? Tcg.A_SETUP_DONE : Tcg.A_PASS)
                                .put("x", new JSONArray()));
                    }
                }
            } catch (Exception ignored) {
            }
        }
        if (!ended && st.optInt("phase") == Tcg.FINISHED && beats.isEmpty()) {
            finish();
        }
    }

    private long polledAt;

    private void finish() {
        ended = true;
        int winner = st.optInt("winner");
        boolean won = winner == seat && !lost;
        v.buzz(true);
        if (reported) {
            return;
        }
        reported = true;
        if (v.app.client == null) {
            endLine = won ? "Link with your game to send it the booster." : "";
            return;
        }
        try {
            JSONObject o = new JSONObject().put("op", "tcgduel").put("won", won ? 1 : 0)
                    .put("kind", s.mode == MODE_FRIEND ? "friend" : "duel");
            if (s.mode == MODE_DUEL) {
                o.put("deck", s.myRating).put("opp", s.oppRating).put("level", s.level);
            }
            v.app.client.order(o);
            JSONObject tcg = v.app.model() == null ? null : v.app.model().snap.optJSONObject("tcg");
            JSONObject today = tcg == null ? null : tcg.optJSONObject("today");
            int left = today == null ? 0 : today.optInt("packsMax") - today.optInt("packs");
            endLine = !won ? "" : left > 0 ? "A booster goes to your game!" : "No more boosters today, but well played!";
        } catch (Exception ignored) {
        }
    }

    // ---------------------------------------------------------------------------------------------- drawing

    @Override
    public void draw(Canvas c, int w, int h) {
        runBeats();
        hits.clear();
        actions.clear();
        long now = System.currentTimeMillis();
        TcgArt a = ta();
        scale = v.pixelScale();
        // the mat: a deep green felt, lighter in the middle, a line between the sides
        fill.setShader(new LinearGradient(0, 0, 0, h, new int[]{0xFF173E2A, 0xFF22583A, 0xFF173E2A}, null,
                Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        // sizes: everything fits the height
        float bw = Math.min((w - 16) / 5f - 3, 44), aw = Math.min(w * 0.3f, 70), hw = Math.min(w / 6f, 46);
        float need = 16 + bw * 1.375f * 2 + aw * 1.375f * 2 + 22 + 12 + hw * 1.375f + 30;
        float k = Math.min(1, (h - 8) / need);
        bw *= k;
        aw *= k;
        hw *= k;
        float bh = bw * 1.375f, ah = aw * 1.375f, hh = hw * 1.375f;
        float y = 16;
        drawTop(c, w);
        // their side: the Bench, then the Active Pokémon (their cards turned to face you, as in the game)
        int them = 1 - seat;
        benchRow(c, w, them, y, bw, bh);
        y += bh + 4;
        activeSpot(c, w, them, y, aw, ah, true);
        float mid = y + ah + 2;
        y = mid + 20;
        activeSpot(c, w, seat, y, aw, ah, false);
        y += ah + 4;
        benchRow(c, w, seat, y, bw, bh);
        y += bh + 6;
        float handY = y;
        drawHand(c, w, handY, hw, hh);
        drawBar(c, w, h);
        // the line between the sides: what's happening, or what to do
        drawMiddle(c, w, mid);
        drawFloats(c, now);
        if (coin >= 0 && now - coinAt < 900) {
            drawCoin(c, w / 2f, mid - 10, now - coinAt);
        }
        if (q != null && "pick".equals(q.optString("q"))) {
            drawPick(c, w, h);
        } else if (sheet != 0) {
            drawSheet(c, w, h);
        }
        if (ended && idle()) {
            drawEnd(c, w, h);
        }
        if (!beats.isEmpty() || aiAt > 0 || now < busyUntil || now < captionUntil || !floats.isEmpty()
                || (q != null && "slot".equals(q.optString("q")))) {
            v.postInvalidateOnAnimation();
        } else if (s.mode == MODE_FRIEND && !ended && st.optInt("actor") != seat) {
            v.postInvalidateDelayed(1000);   // (watching for their move)
        }
    }

    private void drawTop(Canvas c, int w) {
        JSONObject t = them();
        fill.setColor(0xC0081810);
        c.drawRect(0, 0, w, 14, fill);
        art.text(c, s.who, 4, -1, GameArt.WHITE_TEXT, 0xFF000000);
        String right = "HAND " + t.optJSONArray("hand").length() + "  DECK " + t.optInt("deck");
        art.text(c, right, w - 4 - art.width(right), -1, 0xFFB8D8C0, 0xFF000000);
    }

    // a player's six spaces: the Bench row
    private void benchRow(Canvas c, int w, int who, float y, float bw, float bh) {
        float x0 = (w - 5 * bw - 4 * 3) / 2f;
        for (int i = 1; i <= 5; i++) {
            float x = x0 + (i - 1) * (bw + 3);
            spot(c, who, i, x, y, bw, bh, false);
        }
    }

    // the Active spot with the deck, discard pile and Prize cards beside it
    private void activeSpot(Canvas c, int w, int who, float y, float aw, float ah, boolean theirs) {
        float x = (w - aw) / 2f;
        spot(c, who, 0, x, y, aw, ah, true);
        JSONObject p = st.optJSONArray("p") == null ? null : st.optJSONArray("p").optJSONObject(who);
        if (p == null) {
            return;
        }
        float pw = Math.min(aw * 0.36f, 24), ph = pw * 1.375f;
        // Prize cards on the left; the deck above the discard pile on the right
        int prizes = p.optJSONArray("prize").length();
        float px = x - 12 - pw - 6, py = y + (ah - ph) / 2 - 4;
        for (int i = Math.min(prizes, 6) - 1; i >= 0; i--) {
            ta().draw(c, 0, px + (i % 2) * 5, py + (i / 2) * 4, pw, ph, scale, 255);
        }
        tag(c, "PRIZES " + prizes, px - 4, py + ph + 22);
        float dx = x + aw + 12, top = y + 2, low = y + ah - ph - 10;
        if (p.optInt("deck") > 0) {
            ta().draw(c, 0, dx, top, pw, ph, scale, 255);
        } else {
            empty(c, dx, top, pw, ph);
        }
        tag(c, String.valueOf(p.optInt("deck")), dx + pw + 3, top + ph / 2 + 6);
        JSONArray discard = p.optJSONArray("discard");
        if (discard.length() > 0) {
            ta().draw(c, discard.optJSONArray(discard.length() - 1).optInt(1), dx, low, pw, ph, scale, 255);
            final JSONArray d = discard;
            final String title = who == seat ? "YOUR DISCARD PILE" : "THEIR DISCARD PILE";
            hit(dx, low, pw, ph, () -> {
                pile = d;
                pileTitle = title;
                sheet = 4;
            });
        } else {
            empty(c, dx, low, pw, ph);
        }
        tag(c, String.valueOf(discard.length()), dx + pw + 3, low + ph / 2 + 6);
        tag(c, "DISCARD", dx - 6, low + ph + 12);
    }

    private void tag(Canvas c, String t, float x, float y) {
        art.text(c, t, (int) x, (int) y - 12, 0xFFB8D8C0, 0xFF06140C);
    }

    private void empty(Canvas c, float x, float y, float w, float h) {
        line.setStyle(Paint.Style.STROKE);
        line.setStrokeWidth(1);
        line.setColor(0x6090C8A0);
        c.drawRoundRect(new RectF(x, y, x + w, y + h), 3, 3, line);
    }

    // one space: the Pokémon (its damage, conditions and Energy), or an empty space; glowing when it can be chosen
    private void spot(Canvas c, int who, int slot, float x, float y, float w, float h, boolean active) {
        slotX[who][slot] = x;
        slotY[who][slot] = y;
        slotW[who][slot] = w;
        slotH[who][slot] = h;
        JSONObject f = field(who, slot);
        boolean choosable = q != null && "slot".equals(q.optString("q")) && q.optInt("who") == who
                && (q.optInt("mask") >> slot & 1) != 0;
        if (f == null) {
            empty(c, x, y, w, h);
        } else {
            int id = f.optInt("id");
            ta().draw(c, id, x, y, w, h, scale, 255);
            int dmg = f.optInt("dmg"), hp = f.optInt("hp");
            if (dmg > 0) {
                // damage counters as the game shows them: a red badge with the damage
                float r = Math.max(6, w * 0.16f);
                fill.setColor(0xFF801010);
                c.drawCircle(x + w - r * 0.7f, y + r * 0.9f, r, fill);
                fill.setColor(0xFFE03030);
                c.drawCircle(x + w - r * 0.7f, y + r * 0.9f, r * 0.85f, fill);
                String d = String.valueOf(dmg);
                art.text(c, d, (int) (x + w - r * 0.7f - art.width(d) / 2f), (int) (y + r * 0.9f - 7), GameArt.WHITE_TEXT, 0xFF400000);
            }
            if (hp > 0 && id != 0) {
                // the HP left, a bar along the bottom
                float frac = Math.max(0, Math.min(1, (hp - dmg) / (float) hp));
                fill.setColor(0xC0000000);
                c.drawRect(x + 2, y + h - 5, x + w - 2, y + h - 1, fill);
                fill.setColor(frac > 0.5f ? 0xFF58D058 : frac > 0.2f ? 0xFFF0C830 : 0xFFF04830);
                c.drawRect(x + 3, y + h - 4, x + 3 + (w - 6) * frac, y + h - 2, fill);
            }
            // the Energy attached, as orbs along the bottom edge
            JSONArray att = f.optJSONArray("att");
            float r = Math.max(2.5f, w * 0.07f), ex = x + r + 2;
            for (int i = 0; att != null && i < att.length(); i++) {
                int e = att.optJSONArray(i).optInt(1);
                JSONObject ec = ta().card(e);
                if (ec == null || ec.optInt("k") != Tcg.ENERGY) {
                    continue;
                }
                int n = e == 7 ? 2 : 1;
                for (int k = 0; k < n; k++) {
                    ta().orb(c, ec.optInt("t"), ex, y + h - 8 - r, r);
                    ex += r * 1.7f;
                }
            }
            // conditions above the card
            String cond = new String[]{"", "ASLEEP", "PARALYZED", "CONFUSED"}[Math.min(3, f.optInt("cond"))];
            if (f.optInt("poison") > 0) {
                cond = (cond.isEmpty() ? "" : cond + " ") + "POISON";
            }
            if (!cond.isEmpty()) {
                chip(c, cond, (int) x, (int) (y - 6), 0xFF8040B0);
            }
        }
        if (choosable) {
            float pulse = (float) (0.5 + 0.5 * Math.sin(System.currentTimeMillis() / 160.0));
            line.setStyle(Paint.Style.STROKE);
            line.setStrokeWidth(2);
            line.setColor(((int) (140 + 115 * pulse) << 24) | 0xF8E070);
            c.drawRoundRect(new RectF(x - 2, y - 2, x + w + 2, y + h + 2), 4, 4, line);
            line.setStrokeWidth(1);
            hit(x, y, w, h, () -> answer(Tcg.flowSlot(flow, duel, slot)));
            return;
        }
        if (q != null) {
            return;
        }
        if (f != null) {
            hit(x, y, w, h, () -> tapField(who, slot));
        } else if (who == seat && myMove() && opts.optInt("setup") == 1) {
            hit(x, y, w, h, () -> v.message("Tap a Basic POKéMON in your hand to put it here."));
        }
    }

    private void tapField(int who, int slot) {
        // promoting: a Benched Pokémon goes in at a tap
        if (who == seat && slot > 0 && opts.optInt("promote") == 1 && myMove()) {
            begin(Tcg.FLOW_PROMOTE, slot);
            return;
        }
        if (who != seat && st.optInt("phase") == Tcg.SETUP) {
            v.message("Their POKéMON stay face down until the duel begins.");
            return;
        }
        sheet = who == seat ? 2 : 3;
        sheetIndex = slot;
    }

    private void drawHand(Canvas c, int w, float y, float hw, float hh) {
        JSONArray hand = me().optJSONArray("hand"), ho = opts.optJSONArray("hand");
        if (hand == null) {
            return;
        }
        int n = hand.length();
        float step = n <= 1 ? 0 : Math.min(hw + 3, (w - 8 - hw) / (float) (n - 1));
        float x0 = (w - (step * (n - 1) + hw)) / 2f;
        for (int i = 0; i < n; i++) {
            int id = hand.optJSONArray(i).optInt(1);
            JSONObject o = ho == null ? null : ho.optJSONObject(i);
            boolean ok = o != null && o.optInt("ok") == 1 && myMove();
            float x = x0 + i * step, lift = ok ? 4 : 0;
            ta().draw(c, id, x, y - lift, hw, hh, scale, ok || !myMove() ? 255 : 150);
            if (ok) {
                fill.setColor(0xFFF8E070);
                c.drawRect(x + 2, y - lift - 3, x + hw - 2, y - lift - 1, fill);
            }
            final int index = i;
            if (q == null) {
                hit(x, y - 6, i == n - 1 ? hw : step, hh + 6, () -> {
                    sheet = 1;
                    sheetIndex = index;
                });
            }
        }
    }

    private void drawBar(Canvas c, int w, int h) {
        int by = h - 26;
        if (q != null && "slot".equals(q.optString("q"))) {
            button(c, "CANCEL", w - 84, by, 80, true);
            hit(w - 84, by, 80, 24, this::cancel);
            return;
        }
        if (ended) {
            return;
        }
        button(c, "MENU", 4, by, 60, true);
        hit(4, by, 60, 24, this::menu);
        if (opts.optInt("setup") == 1) {
            boolean ok = opts.optInt("ready") == 1 && myMove();
            button(c, "READY", w - 84, by, 80, ok);
            hit(w - 84, by, 80, 24, () -> {
                if (opts.optInt("ready") == 1) {
                    begin(Tcg.FLOW_READY, 0);
                } else {
                    v.message(st.optInt("actor") == seat ? "Choose your Active POKéMON first: tap a Basic POKéMON in your hand."
                            : "Waiting for " + theirName() + ".");
                }
            });
        } else if (opts.optInt("end") == 1 || st.optInt("actor") != seat) {
            boolean ok = opts.optInt("end") == 1 && myMove();
            button(c, "END TURN", w - 84, by, 80, ok);
            hit(w - 84, by, 80, 24, () -> {
                if (!myMove()) {
                    return;
                }
                if (opts.optInt("energyLeft") == 1) {
                    v.showMenu(Menu.confirm(v, "End your turn without attaching Energy?", () -> begin(Tcg.FLOW_PASS, 0)));
                } else {
                    begin(Tcg.FLOW_PASS, 0);
                }
            });
        }
    }

    private void menu() {
        Menu m = Menu.make(v, "DUEL");
        m.add("YOUR DISCARD PILE", () -> {
            pile = me().optJSONArray("discard");
            pileTitle = "YOUR DISCARD PILE";
            sheet = 4;
        });
        m.add("THEIR DISCARD PILE", () -> {
            pile = them().optJSONArray("discard");
            pileTitle = "THEIR DISCARD PILE";
            sheet = 4;
        });
        m.add("CONCEDE", () -> v.showMenu(Menu.confirm(v, s.mode == MODE_FRIEND ? "Concede this duel?"
                : "Concede this duel? You won't lose any cards.", this::concede)));
        m.add("THE LOG", this::showLog);
        m.add("BACK", null);
        v.showMenu(m);
    }

    // The rules take a concession from the player whose turn it is (TcgDuel_Apply's TCG_CONCEDE): on your own turn
    // it goes through them; otherwise (a FREE DUEL only) the duel simply ends as a loss
    private boolean canConcedeNow() {
        int phase = st.optInt("phase");
        return st.optInt("current") == seat && (phase == Tcg.TURN || phase == Tcg.SETUP);
    }

    private void concede() {
        if (ended || duel == 0) {
            return;
        }
        if (canConcedeNow()) {
            try {
                act(new JSONObject(Tcg.flowBegin(flow, duel, seat, Tcg.FLOW_CONCEDE, 0)).optJSONObject("action"));
            } catch (Exception ignored) {
            }
            return;
        }
        if (s.mode == MODE_FRIEND) {
            v.message("You can concede on your turn.");
            return;
        }
        beats.clear();
        aiAt = 0;
        ended = true;
        lost = true;
        finish();
    }

    private boolean lost;

    // the rules' own last lines (what happened, in words)
    private void showLog() {
        JSONArray log = st.optJSONArray("log");
        StringBuilder b = new StringBuilder();
        for (int i = 0; log != null && i < log.length(); i++) {
            b.append(log.optString(i).replace("Player " + (seat + 1), "You").replace("Player " + (2 - seat), theirName()))
                    .append('\n');
        }
        v.showMenu(Menu.message(v, b.length() == 0 ? "Nothing yet." : b.toString().trim()));
    }

    private void drawMiddle(Canvas c, int w, float mid) {
        long now = System.currentTimeMillis();
        String text;
        int color = 0xFFF8F0C8;
        if (q != null && "slot".equals(q.optString("q"))) {
            text = q.optString("prompt");
            color = 0xFFF8E070;
        } else if (now < captionUntil && !caption.isEmpty()) {
            text = caption;
        } else if (ended) {
            text = "";
        } else if (st.optInt("actor") == seat && idle()) {
            text = opts.optInt("setup") == 1 ? "Your Active POKéMON, then up to 5 on the Bench. Then READY."
                    : opts.optInt("promote") == 1 ? "Choose a Benched POKéMON to send out."
                    : "YOUR TURN";
        } else {
            text = s.mode == MODE_FRIEND ? theirName() + "'S TURN" : theirName() + " is thinking...";
            color = 0xFFB8D8C0;
        }
        fill.setColor(0xA0061008);
        c.drawRect(0, mid, w, mid + 18, fill);
        List<String> lines = wrap(text, w - 12);
        String shown = lines.get(0) + (lines.size() > 1 ? " " + lines.get(1) : "");
        if (art.width(shown) > w - 12) {
            shown = lines.get(0);
        }
        art.text(c, shown, (w - art.width(shown)) / 2, (int) mid + 1, color, 0xFF000000);
        if (captionCard != 0 && now < captionUntil && q == null) {
            // the card being played, shown a moment over the table
            float cw = Math.min(w * 0.36f, 90), ch = cw * 1.375f;
            float a = Math.min(1, (captionUntil - now) / 250f);
            ta().draw(c, captionCard, w - cw - 6, mid - ch / 2 + 9, cw, ch, scale, (int) (255 * a));
        }
    }

    private void drawFloats(Canvas c, long now) {
        for (int i = floats.size() - 1; i >= 0; i--) {
            Object[] f = floats.get(i);
            long age = now - (long) f[0];
            if (age > 900) {
                floats.remove(i);
                continue;
            }
            int who = (int) f[1], slot = (int) f[2];
            float x = slotX[who][slot] + slotW[who][slot] / 2, y = slotY[who][slot] + slotH[who][slot] / 2 - age / 30f;
            String t = (String) f[3];
            c.save();
            c.scale(2, 2, x, y);
            art.text(c, t, (int) (x - art.width(t) / 2f), (int) y - 8, (int) f[4], 0xFF000000);
            c.restore();
        }
    }

    private void drawCoin(Canvas c, float cx, float cy, long age) {
        float spin = age < 500 ? (float) Math.abs(Math.cos(age / 60.0)) : 1;
        float r = 14;
        RectF o = new RectF(cx - r * spin, cy - r, cx + r * spin, cy + r);
        fill.setColor(0xFF806010);
        c.drawOval(o, fill);
        o.inset(2 * spin, 2);
        fill.setColor(coin == 1 ? 0xFFF8D040 : 0xFFD8D8E0);
        c.drawOval(o, fill);
        if (age >= 500) {
            String t = coin == 1 ? "H" : "T";
            art.text(c, t, (int) (cx - art.width(t) / 2f), (int) cy - 7, 0xFF503000, 0);
        }
    }

    // ------------------------------------------------------------------------------- a card up close

    private void drawSheet(Canvas c, int w, int h) {
        fill.setColor(0xD0000000);
        c.drawRect(0, 0, w, h, fill);
        hit(0, 0, w, h, () -> sheet = 0);   // (a tap outside closes it)
        if (sheet == 4) {
            drawPileSheet(c, w, h);
            return;
        }
        int id = 0;
        JSONObject f = null;
        JSONArray hand = me().optJSONArray("hand");
        if (sheet == 1) {
            if (hand == null || sheetIndex >= hand.length()) {
                sheet = 0;
                return;
            }
            id = hand.optJSONArray(sheetIndex).optInt(1);
        } else {
            f = field(sheet == 2 ? seat : 1 - seat, sheetIndex);
            if (f == null) {
                sheet = 0;
                return;
            }
            id = f.optInt("id");
        }
        float cw = Math.min(w * 0.62f, (h - 140) / 1.375f), ch = cw * 1.375f;
        float cx = (w - cw) / 2, cy = 10;
        ta().draw(c, id, cx, cy, cw, ch, scale, 255);
        hit(cx, cy, cw, ch, () -> {});   // (the card itself: nothing)
        float y = cy + ch + 6;
        JSONObject card = ta().card(id);
        if (f != null) {
            String info = card == null ? "" : card.optString("n") + (f.optInt("hp") > 0 ? "  HP " + Math.max(0, f.optInt("hp")
                    - f.optInt("dmg")) + "/" + f.optInt("hp") : "");
            art.text(c, info, (w - art.width(info)) / 2, (int) y, GameArt.WHITE_TEXT, 0xFF000000);
            y += 14;
        }
        List<Object[]> buttons = new ArrayList<>();   // [label, enabled, Runnable, why, sub]
        if (sheet == 1) {
            JSONObject o = opts.optJSONArray("hand") == null ? null : opts.optJSONArray("hand").optJSONObject(sheetIndex);
            boolean ok = o != null && o.optInt("ok") == 1 && myMove();
            String why = !myMove() ? (st.optInt("actor") == seat ? "Wait a moment." : "It isn't your turn.")
                    : o == null ? "" : o.optString("why");
            final int index = sheetIndex;
            buttons.add(new Object[]{o == null ? "PLAY" : o.optString("verb"), ok, (Runnable) () -> begin(Tcg.FLOW_HAND, index), why,
                    o != null && o.optInt("use") == 0 && ok ? "(It wouldn't do anything now.)" : null});
        } else if (sheet == 2) {
            if (sheetIndex == 0 && st.optInt("phase") == Tcg.TURN) {
                JSONArray atk = opts.optJSONArray("attacks"), info = card == null ? null : card.optJSONArray("a");
                for (int i = 0; atk != null && i < atk.length(); i++) {
                    JSONObject o = atk.optJSONObject(i);
                    if (o.optInt("power") == 1) {
                        continue;
                    }
                    JSONObject ai = info == null || i >= info.length() ? null : info.optJSONObject(i);
                    String label = o.optString("n") + (ai != null && ai.optInt("dmg") > 0 ? "  " + ai.optInt("dmg") : "");
                    final int which = i;
                    buttons.add(new Object[]{label, o.optInt("ok") == 1 && myMove(), (Runnable) () -> begin(Tcg.FLOW_ATTACK, which),
                            o.optString("why"), ai == null ? null : ai.optString("txt")});
                }
                JSONObject r = opts.optJSONObject("retreat");
                if (r != null) {
                    buttons.add(new Object[]{"RETREAT (" + r.optInt("cost") + " ENERGY)", r.optInt("ok") == 1 && myMove(),
                            (Runnable) () -> begin(Tcg.FLOW_RETREAT, 0), r.optString("why"), null});
                }
            }
            JSONArray powers = opts.optJSONArray("powers");
            for (int i = 0; powers != null && i < powers.length(); i++) {
                JSONObject o = powers.optJSONObject(i);
                if (o.optInt("slot") != sheetIndex) {
                    continue;
                }
                final int slot = sheetIndex;
                buttons.add(new Object[]{"POKéMON POWER: " + o.optString("n"), o.optInt("ok") == 1 && myMove(),
                        (Runnable) () -> begin(Tcg.FLOW_POWER, slot), o.optString("why"),
                        card == null || card.optJSONArray("a") == null || card.optJSONArray("a").length() == 0 ? null
                                : card.optJSONArray("a").optJSONObject(0).optString("txt")});
            }
            JSONArray discard = opts.optJSONArray("discard");
            for (int i = 0; discard != null && i < discard.length(); i++) {
                JSONObject o = discard.optJSONObject(i);
                if (o.optInt("slot") == sheetIndex) {
                    final int slot = sheetIndex;
                    buttons.add(new Object[]{"DISCARD IT", o.optInt("ok") == 1 && myMove(),
                            (Runnable) () -> begin(Tcg.FLOW_DISCARD, slot), "It can't be discarded now.", null});
                }
            }
        }
        for (Object[] b : buttons) {
            final boolean ok = (boolean) b[1];
            final Runnable go = (Runnable) b[2];
            final String why = (String) b[3];
            int bw = Math.min(w - 24, 220), bx = (w - bw) / 2;
            button(c, (String) b[0], bx, (int) y, bw, ok);
            hit(bx, y, bw, 24, () -> {
                if (ok) {
                    sheet = 0;
                    go.run();
                } else {
                    v.message(why == null || why.isEmpty() ? "That can't be done now." : why);
                }
            });
            y += 24;
            if (b[4] != null && !((String) b[4]).isEmpty()) {
                for (String l : wrap((String) b[4], bw - 8)) {
                    if (y > h - 34) {
                        break;
                    }
                    art.text(c, l, bx + 4, (int) y - 2, 0xFFC8D0D8, 0xFF000000);
                    y += 12;
                }
                y += 2;
            }
        }
        button(c, "CLOSE", w / 2 - 40, h - 28, 80, true);
        hit(w / 2 - 40, h - 28, 80, 24, () -> sheet = 0);
    }

    private void drawPileSheet(Canvas c, int w, int h) {
        art.text(c, pileTitle, (w - art.width(pileTitle)) / 2, 4, GameArt.WHITE_TEXT, 0xFF000000);
        int cols = 5;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.375f;
        int n = pile == null ? 0 : pile.length();
        if (n == 0) {
            String t = "Nothing here yet.";
            art.text(c, t, (w - art.width(t)) / 2, 40, 0xFFB8C8D0, 0xFF000000);
        }
        for (int i = 0; i < n; i++) {
            int id = pile.optJSONArray(n - 1 - i).optInt(1);   // (the newest first)
            float x = 8 + (i % cols) * (cw + 4), y = 22 + (i / cols) * (ch + 4) - scroll;
            if (y + ch < 0 || y > h) {
                continue;
            }
            ta().draw(c, id, x, y, cw, ch, scale, 255);
        }
        pileRows = (n + cols - 1) / cols;
        pileRowH = ch + 4;
        button(c, "CLOSE", w / 2 - 40, h - 28, 80, true);
        hit(w / 2 - 40, h - 28, 80, 24, () -> sheet = 0);
    }

    private int pileRows;
    private float pileRowH;

    @Override
    public int scrollMax() {
        if (sheet == 4) {
            return Math.max(0, (int) (22 + pileRows * pileRowH + 40 - v.height()));
        }
        if (q != null && "pick".equals(q.optString("q"))) {
            return Math.max(0, (int) (pickBottom - v.height() + 40));
        }
        return 0;
    }

    // ------------------------------------------------------------------------------- choosing cards

    private float pickBottom;

    private void drawPick(Canvas c, int w, int h) {
        fill.setColor(0xE0081018);
        c.drawRect(0, 0, w, h, fill);
        hit(0, 0, w, h, () -> {});
        List<String> title = wrap(q.optString("prompt"), w - 12);
        int ty = 4 - scroll;
        for (String l : title) {
            art.text(c, l, (w - art.width(l)) / 2, ty, 0xFFF8E070, 0xFF000000);
            ty += 13;
        }
        JSONArray cards = q.optJSONArray("cards");
        int value = q.optInt("value"), min = q.optInt("min"), max = q.optInt("max");
        String need = value > 0 ? "Energy worth " + value + " (" + chosenValue() + " chosen)"
                : min == max ? "Choose " + min + (min == 1 ? " card" : " cards") + " (" + chosen.size() + ")"
                : "Choose " + (min == 0 ? "up to " + max : min + " to " + max) + " (" + chosen.size() + ")";
        if (!q.optString("error").isEmpty()) {
            need = q.optString("error");
        }
        art.text(c, need, (w - art.width(need)) / 2, ty, 0xFFC8D0D8, 0xFF000000);
        ty += 16;
        int cols = 5;
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.375f;
        int n = cards == null ? 0 : cards.length();
        if (n == 0) {
            String t = "There's nothing to choose.";
            art.text(c, t, (w - art.width(t)) / 2, ty + 20, 0xFFB8C8D0, 0xFF000000);
        }
        for (int i = 0; i < n; i++) {
            JSONArray k = cards.optJSONArray(i);
            final int phys = k.optInt(0);
            boolean ok = k.optInt(2) == 1;
            float x = 8 + (i % cols) * (cw + 4), y = ty + (i / cols) * (ch + 4);
            int order = chosen.indexOf(phys);
            ta().draw(c, k.optInt(1), x, y, cw, ch, scale, ok ? 255 : 90);
            if (order >= 0) {
                line.setStyle(Paint.Style.STROKE);
                line.setStrokeWidth(2);
                line.setColor(0xFFF8E070);
                c.drawRoundRect(new RectF(x - 1, y - 1, x + cw + 1, y + ch + 1), 3, 3, line);
                line.setStrokeWidth(1);
                fill.setColor(0xFFF8E070);
                c.drawCircle(x + cw - 6, y + 7, 7, fill);
                String o = String.valueOf(order + 1);
                art.text(c, o, (int) (x + cw - 6 - art.width(o) / 2f), (int) y, 0xFF302000, 0);
            }
            hit(x, y, cw, ch, () -> {
                int at = chosen.indexOf(phys);
                if (at >= 0) {
                    chosen.remove(at);
                } else if (!ok) {
                    v.message("That card can't be chosen here.");
                    return;
                } else if (value == 0 && chosen.size() >= max) {
                    if (max == 1) {
                        chosen.clear();
                        chosen.add(phys);
                    } else {
                        v.message("You've already chosen enough cards.");
                    }
                    return;
                } else {
                    chosen.add(phys);
                }
                v.buzz(false);
            });
        }
        pickBottom = ty + scroll + ((n + cols - 1) / cols) * (ch + 4) + 10;
        boolean done = value > 0 ? chosenValue() >= value : chosen.size() >= min && chosen.size() <= max;
        button(c, "CANCEL", 6, h - 28, 80, true);
        hit(6, h - 28, 80, 24, this::cancel);
        button(c, "DONE", w - 86, h - 28, 80, done);
        hit(w - 86, h - 28, 80, 24, () -> {
            int[] pick = new int[chosen.size()];
            for (int i = 0; i < pick.length; i++) {
                pick[i] = chosen.get(i);
            }
            answer(Tcg.flowPick(flow, duel, pick));
        });
    }

    private int chosenValue() {
        int n = 0;
        for (int p : chosen) {
            n += Tcg.cardOf(duel, q.optInt("owner"), p) == 7 ? 2 : 1;
        }
        return n;
    }

    // ------------------------------------------------------------------------------------------- the end

    private void drawEnd(Canvas c, int w, int h) {
        boolean won = st.optInt("winner") == seat && !lost;
        float cy = h * 0.42f;
        fill.setColor(0xD0081018);
        c.drawRoundRect(new RectF(12, cy - 34, w - 12, cy + 50), 8, 8, fill);
        line.setStyle(Paint.Style.STROKE);
        line.setColor(won ? 0xFFF8E070 : 0xFF8090A8);
        c.drawRoundRect(new RectF(12, cy - 34, w - 12, cy + 50), 8, 8, line);
        String t = won ? "YOU WIN!" : "YOU LOSE";
        c.save();
        c.scale(2, 2, w / 2f, cy - 18);
        art.text(c, t, (w - art.width(t)) / 2, (int) cy - 26, won ? 0xFFF8E070 : 0xFFD0D8E8, 0xFF302000);
        c.restore();
        if (!endLine.isEmpty()) {
            art.text(c, endLine, (w - art.width(endLine)) / 2, (int) cy, GameArt.WHITE_TEXT, 0xFF000000);
        }
        int bw = 80;
        if (s.mode == MODE_DUEL) {
            button(c, "AGAIN", w / 2 - bw - 4, (int) cy + 20, bw, true);
            hit(w / 2 - bw - 4, cy + 20, bw, 24, this::again);
        }
        button(c, "DONE", s.mode == MODE_DUEL ? w / 2 + 4 : w / 2 - bw / 2, (int) cy + 20, bw, true);
        hit(s.mode == MODE_DUEL ? w / 2 + 4 : w / 2 - bw / 2, cy + 20, bw, 24, () -> v.showOver(null));
    }

    private void again() {
        Setup n = new Setup();
        n.mode = s.mode;
        n.mine = s.mine;
        n.theirs = s.theirs;
        n.level = s.level;
        n.prizes = s.prizes;
        n.who = s.who;
        n.deckName = s.deckName;
        n.myRating = s.myRating;
        n.oppRating = s.oppRating;
        n.seed = (int) (System.nanoTime() & 0x7FFFFFFF);
        v.showOver(new TcgDuelPage(v, n));
    }

    // ------------------------------------------------------------------------------- for the screens test

    /** Both sides played by the game's AI until it's this phone's turn proper (at most N moves), the call-outs skipped */
    void autoPlay(int n) {
        for (int i = 0; i < n && duel != 0 && st.optInt("actor") >= 0; i++) {
            if (i > 8 && st.optInt("actor") == seat && st.optInt("phase") == Tcg.TURN) {
                break;
            }
            try {
                act(new JSONObject(Tcg.ai(duel, 3)));
            } catch (Exception e) {
                break;
            }
        }
        beats.clear();
        floats.clear();
        busyUntil = 0;
        captionUntil = 0;
    }

    /** A card in play up close (WHO 0: this phone's side) */
    void inspect(int who, int slot) {
        sheet = who == 0 ? 2 : 3;
        sheetIndex = slot;
    }

    /** A move begun on the first card in the hand whose verb is VERB (ATTACH, PLAY...), as a tap would */
    boolean beginHand(String verb) {
        JSONArray ho = opts.optJSONArray("hand");
        for (int i = 0; ho != null && i < ho.length(); i++) {
            if (ho.optJSONObject(i).optInt("ok") == 1 && ho.optJSONObject(i).optString("verb").equals(verb)) {
                begin(Tcg.FLOW_HAND, i);
                return true;
            }
        }
        return false;
    }

    // ------------------------------------------------------------------------------------------------ touch

    private void hit(float x, float y, float w, float h, Runnable r) {
        hits.add(new int[]{(int) x, (int) y, (int) Math.ceil(w), (int) Math.ceil(h)});
        actions.add(r);
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

    @Override
    public void back() {
        if (q != null) {
            cancel();
            return;
        }
        if (sheet != 0) {
            sheet = 0;
            v.invalidate();
            return;
        }
        if (!ended && st.optInt("phase") != Tcg.FINISHED && s.mode == MODE_DUEL) {
            Menu m = Menu.make(v, "LEAVE THE DUEL? IT COUNTS AS A LOSS.");
            m.add("CONCEDE AND LEAVE", () -> {
                concede();
                v.showOver(null);
            });
            m.add("KEEP PLAYING", null);
            v.showMenu(m);
            return;
        }
        v.showOver(null);
    }
}
