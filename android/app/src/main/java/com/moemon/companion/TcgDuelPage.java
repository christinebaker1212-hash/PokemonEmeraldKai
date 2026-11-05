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
    // the motion (CardFx): the table shows the state before a move while that move's events play, as the game's does
    private final CardFx fx = new CardFx();
    private JSONObject shown;
    private final RectF[] deckR = new RectF[2], prizeR = new RectF[2], handR = new RectF[2];

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
        shown = st;
        lastSeq = st.optInt("seq");
        busyUntil = System.currentTimeMillis() + 500;
        say(seat == st.optInt("actor") ? (st.optInt("phase") == Tcg.SETUP ? "Place your POKéMON!" : "YOUR TURN")
                : "", 0, 900);
    }

    @Override
    public void shown() {
        // the duel's song as the game's table plays it (in shown: a rematch's page comes before the last one's hidden)
        if (!ended) {
            v.sound.music(s.mode == MODE_FRIEND ? "tcg:mus_rg_union_room" : "tcg:mus_vs_trainer");
        }
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public void hidden() {
        v.sound.music(null);
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
                v.sound.play("se_failure");
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
                    v.sound.play("se_failure");
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
            se(t, sound(type, mine, flags, value, card, slot));
            switch (type) {
                case Tcg.EV_TURN:
                    fxBeat(t, "turn", who, 0, 0, 0);
                    t = beat(t, "say", mine ? "YOUR TURN" : theirName() + "'S TURN", who, 0, 0, 0, 900);
                    break;
                case Tcg.EV_DRAW:
                    fxBeat(t, "draw", who, 0, value, card);
                    t += draws++ < 8 ? 160 : 40;
                    break;
                case Tcg.EV_ATTACK: {
                    JSONObject c = a.card(card);
                    JSONArray atk = c == null ? null : c.optJSONArray("a");
                    String an = atk == null || slot >= atk.length() ? "" : atk.optJSONObject(slot).optString("n");
                    fxBeat(t + 250, "attack", who, 0, 0, card);
                    t = beat(t, "say", a.name(card) + ": " + an + "!", who, 0, 0, card, 1000);
                    break;
                }
                case Tcg.EV_COIN:
                    t = beat(t, "coin", value == 1 ? "HEADS!" : "TAILS!", who, 0, value, 0, 750);
                    break;
                case Tcg.EV_DAMAGE:
                    fxBeat(t, (flags & Tcg.DMG_WEAK) != 0 ? "weak" : "damage", who, slot, value, 0);
                    t = beat(t, "float", "-" + value + ((flags & Tcg.DMG_WEAK) != 0 ? "!" : ""), who, slot,
                            (flags & Tcg.DMG_WEAK) != 0 ? 0xFFFF5040 : 0xFFF86830, 0, 420);
                    break;
                case Tcg.EV_HEAL:
                    fxBeat(t, "heal", who, slot, value, 0);
                    t = beat(t, "float", "+" + value, who, slot, 0xFF60D860, 0, 380);
                    break;
                case Tcg.EV_NO_DAMAGE:
                    fxBeat(t, "shield", who, slot, 0, 0);
                    t = beat(t, "float", "NO DAMAGE", who, slot, 0xFFB8C8E8, 0, 450);
                    break;
                case Tcg.EV_STATUS: {
                    String c = value == Tcg.ASLEEP ? "Asleep" : value == Tcg.PARALYZED ? "Paralyzed"
                            : value == Tcg.CONFUSED ? "Confused" : flags > 0 ? "Poisoned" : "";
                    if (!c.isEmpty()) {
                        fxBeat(t, "status", who, slot, value, 0);
                        t = beat(t, "float", c.toUpperCase() + "!", who, slot, 0xFFC890F8, 0, 500);
                    }
                    break;
                }
                case Tcg.EV_KO:
                    fxBeat(t, "ko", who, slot, 0, card);
                    t = beat(t, "say", name + " is Knocked Out!", who, slot, 0, card, 1000);
                    break;
                case Tcg.EV_PRIZE:
                    fxBeat(t, "prize", who, 0, 0, 0);
                    t = beat(t, "say", mine ? "You take a Prize card." : theirName() + " takes a Prize card.", who, 0, 0, 0, 650);
                    break;
                case Tcg.EV_TRAINER:
                    if (!mine) {
                        t = beat(t, "say", theirName() + " plays " + a.name(card) + ".", who, 0, 0, card, 1100);
                    }
                    break;
                case Tcg.EV_PLAY:
                    if (st.optInt("phase") != Tcg.SETUP || mine) {
                        fxBeat(t, "play", who, slot, 0, card);
                        if (mine) {
                            t += 380;
                        }
                    }
                    if (!mine && st.optInt("phase") != Tcg.SETUP) {
                        t = beat(t, "say", a.name(card) + " comes onto the Bench.", who, slot, 0, 0, 600);
                    }
                    break;
                case Tcg.EV_EVOLVE:
                    fxBeat(t, "evolve", who, slot, 0, card);
                    t = beat(t, "say", "It evolved into " + a.name(card) + "!", who, slot, 0, card, 900);
                    break;
                case Tcg.EV_ATTACH:
                    fxBeat(t, "attach", who, slot, 0, card);
                    if (mine) {
                        t += 420;
                    }
                    if (!mine) {
                        t = beat(t, "say", a.name(card) + " is attached to " + nameAt(who, slot) + ".", who, slot, 0, 0, 650);
                    }
                    break;
                case Tcg.EV_RETREAT:
                case Tcg.EV_SWITCH:
                case Tcg.EV_PROMOTE:
                    fxBeat(t, "switch", who, 0, 0, 0);
                    t = beat(t, "say", nameAt(who, 0) + " is now Active.", who, 0, 0, 0, 600);
                    break;
                case Tcg.EV_RETREAT_FAIL:
                    t = beat(t, "say", "It's Confused and couldn't retreat!", who, 0, 0, 0, 800);
                    break;
                case Tcg.EV_POWER:
                    fxBeat(t, "power", who, slot, 0, card);
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
                    fxBeat(t, "win", who, 0, mine ? 1 : 0, 0);
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
        draws = 0;
    }

    private int draws;   // (a run of draws: quick after the first few)

    // a motion at AT (no time of its own: it plays alongside the beat it belongs to)
    private void fxBeat(long at, String kind, int who, int slot, int value, int card) {
        beats.add(new Object[]{at, "fx", kind, who, slot, value, card});
    }

    private RectF slotRect(int who, int slot) {
        if (who < 0 || who > 1 || slot < 0 || slot > 5 || slotW[who][slot] <= 0) {
            return null;
        }
        return new RectF(slotX[who][slot], slotY[who][slot], slotX[who][slot] + slotW[who][slot], slotY[who][slot] + slotH[who][slot]);
    }

    private static RectF shrink(RectF r, float k) {
        float w = r.width() * k, h = r.height() * k;
        return new RectF(r.centerX() - w / 2, r.centerY() - h / 2, r.centerX() + w / 2, r.centerY() + h / 2);
    }

    // a motion beat: what it moves, from where to where (the places as last drawn)
    private void startFx(String kind, int who, int slot, int value, int card, long now) {
        boolean mine = who == seat;
        RectF at = slotRect(who, slot), hand = handR[who], deck = deckR[who], prize = prizeR[who];
        switch (kind) {
            case "turn":
                fx.banner(now, 1000, mine ? "YOUR TURN" : theirName() + "'S TURN", mine ? 0x2858B8 : 0xB83838);
                break;
            case "draw":
                if (deck != null && hand != null) {
                    fx.fly(now, 360, card, !mine || card == 0, !mine, deck, hand, false);
                }
                break;
            case "play":
                if (hand != null && at != null) {
                    fx.fly(now, 380, card, false, !mine, hand, at, true);
                    fx.pop(now + 380, 260, who, slot);
                }
                break;
            case "attach":
                if (hand != null && at != null) {
                    fx.fly(now, 400, card, false, !mine, hand, shrink(at, 0.55f), false);
                    fx.sparkle(now + 380, 500, who, slot, 0xF8E070);
                }
                break;
            case "evolve":
                fx.glow(now, 800, who, slot, 0xFFFFFF);
                fx.sparkle(now + 200, 700, who, slot, 0xF8F8F8);
                fx.pop(now + 400, 300, who, slot);
                break;
            case "attack":
                fx.lunge(now, 480, who, slot, mine ? -1 : 1);
                break;
            case "damage":
                fx.shake(now, 420, who, slot, 0xF83020);
                break;
            case "weak":
                fx.shake(now, 520, who, slot, 0xF83020);
                fx.flash(now, 180, 0xFFF0F0);
                break;
            case "heal":
                fx.sparkle(now, 600, who, slot, 0x60E060);
                break;
            case "shield":
                fx.glow(now, 500, who, slot, 0x88B0F8);
                break;
            case "status":
                fx.ring(now, 600, who, slot, value == Tcg.ASLEEP ? 0x8890F8 : value == Tcg.PARALYZED ? 0xF8E040
                        : value == Tcg.CONFUSED ? 0xF890D0 : 0xB060E0);
                break;
            case "ko":
                fx.fade(now, 750, who, slot);
                break;
            case "prize":
                if (prize != null && hand != null) {
                    fx.fly(now, 450, 0, true, !mine, prize, hand, false);
                }
                break;
            case "switch":
                fx.pop(now, 300, who, 0);
                break;
            case "power":
                fx.glow(now, 650, who, slot, 0xF8E070);
                break;
            case "win":
                fx.flash(now, 400, value == 1 ? 0xFFF8C0 : 0x400000);
                break;
            default:
                break;
        }
    }

    /** The sound the game's duel board plays for an event (tcg_board.c BoardEvent), null for none */
    private String sound(int type, boolean mine, int flags, int value, int card, int slot) {
        switch (type) {
            case Tcg.EV_TURN:
                return mine ? "se_ball_tray_enter" : "se_ball_tray_exit";
            case Tcg.EV_ATTACK:
                return "#" + Tcg.attackSound(card, slot);
            case Tcg.EV_COIN:
                return "se_m_pay_day";
            case Tcg.EV_DAMAGE:
                return (flags & Tcg.DMG_POISON) != 0 ? "se_field_poison" : (flags & Tcg.DMG_WEAK) != 0 ? "se_super_effective"
                        : (flags & Tcg.DMG_RESIST) != 0 ? "se_not_effective" : "se_effective";
            case Tcg.EV_HEAL:
                return "se_use_item";
            case Tcg.EV_STATUS:
                if (flags > 0 && value == 0) {
                    return "se_m_toxic";
                }
                return value < 4 ? new String[]{"se_use_item", "se_m_sing", "se_m_thunder_wave", "se_m_confuse_ray"}[value] : null;
            case Tcg.EV_NO_DAMAGE:
                return "se_m_barrier";
            case Tcg.EV_KO:
                return "se_faint";
            case Tcg.EV_TRAINER:
            case Tcg.EV_PEEK:
            case Tcg.EV_PLAY:
            case Tcg.EV_PROMOTE:
            case Tcg.EV_RETREAT:
            case Tcg.EV_SWITCH:
                return "se_rg_card_flip";
            case Tcg.EV_POWER:
                return "se_m_detect";
            case Tcg.EV_RETREAT_FAIL:
                return "se_failure";
            case Tcg.EV_EVOLVE:
            case Tcg.EV_MORPH:
                return "se_shiny";
            case Tcg.EV_ATTACH:
                return "se_ball_tray_ball";
            case Tcg.EV_PRIZE:
                return mine ? "se_rg_card_flip" : "se_rg_card_flipping";
            case Tcg.EV_SUDDEN_DEATH:
                return "se_bang";
            case Tcg.EV_WIN:
                return mine ? "!tcg:mus_victory_trainer" : "!";   // (the win's song; a loss: the music stops)
            case Tcg.EV_DEVOLVE:
                return "se_m_teleport";
            case Tcg.EV_TO_HAND:
                return "se_m_gust";
            case Tcg.EV_DECK_CHANGED:
                return value != 0 ? "se_m_flame_wheel" : "se_rg_card_flipping";
            default:
                return null;
        }
    }

    private void se(long at, String sound) {
        if (sound != null) {
            beats.add(new Object[]{at, "se", sound, 0, 0, 0, 0});
        }
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
                case "se":
                    v.sound.cue((String) b[2]);
                    break;
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
                case "fx":
                    startFx((String) b[2], (int) b[3], (int) b[4], (int) b[5], (int) b[6], now);
                    break;
                case "end":
                    busyUntil = now + 250;
                    shown = st;   // (the table catches up with the rules)
                    fx.settle();
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
        JSONObject truth = st;
        if (shown != null) {
            st = shown;   // (the table as shown: a move's events play on the state before it)
        }
        try {
            drawTable(c, w, h);
        } finally {
            st = truth;
        }
    }

    private void drawTable(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        long now = System.currentTimeMillis();
        TcgArt a = ta();
        scale = v.pixelScale();
        fill.setColor(0xFF000000);   // (opaque: a faded card's alpha would wash the mat)
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
        fx.drawOver(c, w, h, now, this::slotRect, (cc, id, down, turned, x, yy, cw, ch, al) -> {
            if (down) {
                ta().draw(cc, 0, x, yy, cw, ch, scale, al);   // (a card's back)
            } else {
                card(cc, id, x, yy, cw, ch, turned, al);
            }
        }, art);
        if (coin >= 0 && now - coinAt < 1100) {
            fx.drawCoin(c, w / 2f, mid - 10, now - coinAt, coin == 1, art);
        }
        if (q != null && "pick".equals(q.optString("q"))) {
            drawPick(c, w, h);
        } else if (sheet != 0) {
            drawSheet(c, w, h);
        }
        if (ended && idle()) {
            drawEnd(c, w, h);
        }
        if (!beats.isEmpty() || aiAt > 0 || now < busyUntil || now < captionUntil || !floats.isEmpty() || fx.moving(now)
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
        // Prize cards on the left, the deck and discard pile on the right (the deck nearer its owner); the other side
        // the other way round, as across a real table
        boolean turned = who != seat;
        int prizes = p.optJSONArray("prize").length();
        float left = x - 12 - pw - 6, right = x + aw + 12;
        float px = turned ? right : left, py = y + (ah - ph) / 2 - 4;
        for (int i = Math.min(prizes, 6) - 1; i >= 0; i--) {
            card(c, 0, px + (i % 2) * 5, py + (i / 2) * 4, pw, ph, turned);
        }
        prizeR[who] = new RectF(px, py, px + pw, py + ph);
        tag(c, "PRIZES " + prizes, px - 4, py + ph + 22);
        float dx = turned ? left : right, near = y + ah - ph - 10, far = y + 2;
        float deckY, pileY;
        if (turned) {
            // (theirs: the deck at the top, nearer them; the discard pile below it)
            deckY = far;
            pileY = near;
        } else {
            // (yours: the discard pile above, the deck below, nearer you)
            deckY = near;
            pileY = far;
        }
        // the counts beside the piles, on the outer side
        float countX = turned ? dx - 4 : dx + pw + 3;
        deckR[who] = new RectF(dx, deckY, dx + pw, deckY + ph);
        if (p.optInt("deck") > 0) {
            card(c, 0, dx, deckY, pw, ph, turned);
        } else {
            empty(c, dx, deckY, pw, ph);
        }
        String dn = String.valueOf(p.optInt("deck"));
        tag(c, dn, turned ? countX - art.width(dn) : countX, deckY + ph / 2 + 6);
        JSONArray discard = p.optJSONArray("discard");
        if (discard.length() > 0) {
            card(c, discard.optJSONArray(discard.length() - 1).optInt(1), dx, pileY, pw, ph, turned);
            final JSONArray d = discard;
            final String title = who == seat ? "YOUR DISCARD PILE" : "THEIR DISCARD PILE";
            hit(dx, pileY, pw, ph, () -> {
                pile = d;
                pileTitle = title;
                sheet = 4;
            });
        } else {
            empty(c, dx, pileY, pw, ph);
        }
        String pn = String.valueOf(discard.length());
        tag(c, pn, turned ? countX - art.width(pn) : countX, pileY + ph / 2 + 6);
    }

    // a card on the table: the other side's turned to face them
    private void card(Canvas c, int id, float x, float y, float w, float h, boolean turned) {
        card(c, id, x, y, w, h, turned, 255);
    }

    private void card(Canvas c, int id, float x, float y, float w, float h, boolean turned, int alpha) {
        if (turned) {
            ta().drawTurned(c, id, x, y, w, h, scale, alpha);
        } else {
            ta().draw(c, id, x, y, w, h, scale, alpha);
        }
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
        long now = System.currentTimeMillis();
        float fdx = fx.slotDx(who, slot, now, w), fdy = fx.slotDy(who, slot, now, h), fsc = fx.slotScale(who, slot, now);
        int falpha = fx.slotAlpha(who, slot, now), ftint = fx.slotTint(who, slot, now);
        if (f == null) {
            empty(c, x, y, w, h);
        } else if (falpha > 0) {
            int id = f.optInt("id");
            c.save();
            c.translate(fdx, fdy);
            c.scale(fsc, fsc, x + w / 2, y + h / 2);
            card(c, id, x, y, w, h, who != seat, falpha);
            if (ftint != 0) {
                fill.setColor(ftint);
                c.drawRoundRect(new RectF(x, y, x + w, y + h), 3, 3, fill);
            }
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
            c.restore();
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
        // (where cards fly to and from: the hand's end; theirs, the corner their hand is counted in)
        float ex = n == 0 ? (w - hw) / 2f : x0 + step * n;
        handR[seat] = new RectF(Math.min(ex, w - hw - 4), y, Math.min(ex, w - hw - 4) + hw, y + hh);
        handR[1 - seat] = new RectF(w - hw - 6, -hh * 0.6f, w - 6, hh * 0.4f);
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
        shown = st;
        fx.clear();
    }

    /** Tests: one move by the AI for whoever is to move, its events played in real time */
    void aiMoveNow() {
        try {
            act(new JSONObject(Tcg.ai(duel, 3)));
        } catch (Exception ignored) {
        }
    }

    /** Tests: the motion and beats still going */
    boolean animating() {
        return !beats.isEmpty() || fx.moving(System.currentTimeMillis());
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
                v.sound.play("se_select");
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

    @Override
    public boolean swapsWords() {
        return false;   // (the card games' own words: the POKéMON card game is the POKéMON card game)
    }
}
