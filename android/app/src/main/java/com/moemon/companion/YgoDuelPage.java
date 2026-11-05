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
 * A YU-GI-OH! duel on the phone, upright: their Spell & Trap and Monster Zones at the top, turned to face them as on
 * the game's table, yours below, your hand along the bottom, Life Points in the middle. The game's own engine runs it
 * (EDOPro through the game's bridge: Ygo) and asks one question at a time: in the Main and Battle Phases the cards that
 * can do something glow (tap one to see it up close with what it can do), the phases are buttons; the rest (targets,
 * Tributes, chains, yes / no, positions, zones, totals, the order of cards, a name to declare) open a picker, a list or
 * a question. Chances to chain while nothing is happening are passed for you (MENU: STOP AT EVERY CHANCE to keep
 * them). The other side is the game's AI (a FREE DUEL: a win sends a booster to your game, 3 a day) or a friend by
 * turns: a duel is its decks, its seed and its answers.
 */
public final class YgoDuelPage extends Page {
    public static final int MODE_DUEL = 0, MODE_FRIEND = 1;

    public static final class Setup {
        public int mode, seed, first, level = 1, myRating, oppRating, seat;
        public int[] main0, extra0, main1, extra1;   // (seat 0's and seat 1's decks)
        public String who = "A DUELIST", deckName = "";
        public JSONObject game;   // (a friend's duel: Companion's record)
    }

    private static final String[] VERBS = {"Choose", "Tribute", "Target", "Attack", "Discard", "Destroy", "Summon",
            "Add to your hand", "Send to the Graveyard", "Banish", "Equip to", "Fusion Materials", "Fusion Summon",
            "Ritual Summon", "Use the effect?", "Chain", "In order", "Call it", "Summon or Set", "Choose",
            "Return to the Deck", "Return to the hand", "Change position", "Take control", "Declare"};
    private static boolean sStopAlways;

    private final Setup s;
    private final int seat;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG), line = new Paint(Paint.ANTI_ALIAS_FLAG);
    private long duel;
    private JSONObject st = new JSONObject(), q = new JSONObject();
    private JSONArray objs = new JSONArray();
    private int lastSeq;
    // a choice in the making (CARDS / SUM / SORT / COUNTER / NAME: picks; a sheet for a card up close)
    private final List<Integer> picks = new ArrayList<>();
    private int[] amounts = new int[0];
    private int sheetObj = -1;    // a card up close (an object), -1 none
    private JSONArray pile;       // a pile looked through (objects)
    private String pileTitle = "";
    private boolean picking;      // the picker is open for the question
    // what happens, called out in turn
    private final List<Object[]> beats = new ArrayList<>();   // [due, kind, text, player, obj, value, card]
    private String caption = "";
    private int captionCard;
    private long captionUntil, busyUntil, aiAt, polledAt;
    private final List<Object[]> floats = new ArrayList<>();   // [start, player (LP: -1 - player) / obj, text, color]
    private boolean ended, reported, lost;
    private String endLine = "";
    private int applied;
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    private final float[] objX = new float[200], objY = new float[200], objW = new float[200], objH = new float[200];
    private float scale = 1, lpY;
    // the motion (CardFx; places by object number): the table shows the state before an answer while its events play
    private final CardFx fx = new CardFx();
    private JSONObject shownSt;
    private JSONArray shownObjs;
    private final RectF[][][] zoneR = new RectF[2][2][6];
    private final RectF[] deckR = new RectF[2], graveR = new RectF[2], handR = new RectF[2];
    private int coin = -1;
    private long coinAt;

    public YgoDuelPage(GameView v, Setup setup) {
        super(v);
        this.s = setup;
        this.seat = setup.mode == MODE_FRIEND ? setup.seat : 0;
        v.app.ygoArt.engine();
        if (s.mode == MODE_FRIEND && s.game != null) {
            duel = YgoFriendGame.replay(s.game);
            applied = YgoFriendGame.moves(s.game).length();
        } else {
            duel = Ygo.start(s.main0, s.extra0, s.main1, s.extra1, s.seed, s.first);
        }
        refresh();
        shownSt = st;
        shownObjs = objs;
        lastSeq = st.optInt("seq");
        busyUntil = System.currentTimeMillis() + 500;
    }

    @Override
    public void shown() {
        // the duel's song as the game's table plays it (in shown: a rematch's page comes before the last one's hidden)
        if (!ended) {
            v.sound.music(s.mode == MODE_FRIEND ? "ygo:mus_rg_union_room" : "ygo:mus_vs_trainer");
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
            Ygo.end(duel);
            duel = 0;
        }
    }

    private YgoArt ya() {
        return v.app.ygoArt;
    }

    private void refresh() {
        if (duel == 0) {
            return;
        }
        try {
            st = new JSONObject(Ygo.state(duel, seat));
            objs = st.optJSONArray("objs") == null ? new JSONArray() : st.optJSONArray("objs");
            q = new JSONObject(Ygo.query(duel));
        } catch (Exception ignored) {
        }
    }

    private JSONObject player(int p) {
        JSONArray a = st.optJSONArray("players");
        return a == null ? new JSONObject() : a.optJSONObject(p);
    }

    private JSONObject obj(int o) {
        return o < 0 || o >= objs.length() ? null : objs.optJSONObject(o);
    }

    private boolean idle() {
        return beats.isEmpty() && System.currentTimeMillis() >= busyUntil && aiAt == 0;
    }

    private boolean mine() {
        return !ended && st.optInt("over") == 0 && q.optInt("type") != Ygo.Q_NONE && q.optInt("player") == seat && idle();
    }

    // -------------------------------------------------------------------------------------------- answering

    private void answer(int[] p, int zone) {
        if (duel == 0) {
            return;
        }
        int who = q.optInt("player");
        try {
            JSONObject r = new JSONObject(Ygo.answer(duel, p, zone));
            if (r.optInt("ok") != 1) {
                if (who == seat) {
                    v.sound.play("se_failure");
                    v.message(r.optString("error"));
                    v.buzz(false);
                }
                return;
            }
        } catch (Exception e) {
            return;
        }
        if (who == seat && s.mode == MODE_FRIEND && s.game != null) {
            applied++;
            try {
                JSONArray a = new JSONArray();
                for (int x : p) {
                    a.put(x);
                }
                v.app.postYgoMove(s.game, new JSONObject().put("p", a).put("z", zone));
            } catch (org.json.JSONException ignored) {
            }
        }
        picks.clear();
        picking = false;
        sheetObj = -1;
        refresh();
        callOut();
        v.invalidate();
    }

    private void answer(int pick) {
        answer(new int[]{pick}, 0);
    }

    private int[] picked() {
        int[] p = new int[picks.size()];
        for (int i = 0; i < p.length; i++) {
            p[i] = picks.get(i);
        }
        return p;
    }

    // a question that isn't about the table opens its own window when it comes
    private void askIfNeeded() {
        if (!mine() || picking || sheetObj >= 0 || pile != null) {
            return;
        }
        JSONArray items = q.optJSONArray("items");
        int type = q.optInt("type");
        switch (type) {
            case Ygo.Q_CHAIN:
                if (q.optInt("quiet") == 1 && !sStopAlways) {
                    answer(new int[0], 0);   // (a quiet moment: passed, as the game's table does unless L is held)
                    return;
                }
                picking = true;
                picks.clear();
                break;
            case Ygo.Q_YESNO: {
                String t = q.optString("text");
                if (t.isEmpty()) {
                    t = q.optInt("card") != 0 ? "Use the effect of " + ya().name(q.optInt("card")) + "?" : "Yes or no?";
                }
                picking = true;
                Menu m = Menu.make(v, t);
                m.add("YES", () -> answer(1));
                m.add("NO", () -> answer(0));
                v.showMenu(m);
                break;
            }
            case Ygo.Q_OPTION:
            case Ygo.Q_POSITION:
            case Ygo.Q_ZONE: {
                if (q.optInt("max", 1) > 1 && type != Ygo.Q_POSITION) {
                    picking = true;   // (several: the picker)
                    picks.clear();
                    break;
                }
                picking = true;
                Menu m = Menu.make(v, prompt());
                for (int i = 0; items != null && i < items.length(); i++) {
                    final int k = i;
                    m.add(itemLabel(type, items.optJSONObject(i)), () -> answer(k));
                }
                if (q.optInt("min") == 0) {
                    m.add("NONE", () -> answer(new int[0], 0));
                }
                v.showMenu(m);
                break;
            }
            case Ygo.Q_CARDS:
            case Ygo.Q_SUM:
            case Ygo.Q_SORT:
            case Ygo.Q_NAME:
                picking = true;
                picks.clear();
                for (int i = 0; items != null && i < items.length(); i++) {
                    if ((items.optJSONObject(i).optInt("f") & Ygo.IF_MUST) != 0 && type == Ygo.Q_SUM) {
                        picks.add(i);
                    }
                }
                break;
            case Ygo.Q_COUNTER:
                picking = true;
                amounts = new int[items == null ? 0 : items.length()];
                break;
            default:
                break;
        }
    }

    private String itemLabel(int type, JSONObject it) {
        if (type == Ygo.Q_POSITION) {
            int p = it.optInt("v");
            return p == Ygo.POS_ATTACK ? "ATTACK POSITION" : p == Ygo.POS_DEFENSE ? "DEFENSE POSITION"
                    : p == Ygo.POS_FACEDOWN_DEFENSE ? "FACE-DOWN DEFENSE" : "FACE-DOWN ATTACK";
        }
        if (type == Ygo.Q_ZONE) {
            int f = it.optInt("f"), seq = it.optInt("v");
            return ((f & 2) != 0 ? "THEIR " : "YOUR ") + ((f & 1) != 0 ? (seq == 5 ? "FIELD ZONE" : "SPELL & TRAP ZONE " + (seq + 1))
                    : "MONSTER ZONE " + (seq + 1));
        }
        String t = it.optString("t");
        return t.isEmpty() ? "OPTION " + (it.optInt("v") + 1) : t;
    }

    private String prompt() {
        int type = q.optInt("type"), pr = q.optInt("prompt");
        String verb = pr >= 0 && pr < VERBS.length ? VERBS[pr] : "Choose";
        String text = q.optString("text");
        if (pr == 0 && type != Ygo.Q_ZONE && !text.isEmpty()) {
            if (text.startsWith("Select ")) {
                text = text.substring(7);
            }
            return Character.toUpperCase(text.charAt(0)) + text.substring(1);
        }
        if (type == Ygo.Q_SUM) {
            return verb + ": a total of " + q.optInt("sum") + (q.optInt("mode") == 1 ? " or more" : "");
        }
        if (type == Ygo.Q_ZONE) {
            return "Choose a zone";
        }
        if (pr == 3) {
            return "Attack which?";
        }
        int min = q.optInt("min"), max = q.optInt("max");
        if (max > 1) {
            return verb + ": " + (min == max ? String.valueOf(max) : min + " to " + max);
        }
        return verb + (min == 0 ? " (or none)" : "");
    }

    // the Main / Battle Phase: what each object can do now (item indices), and the phase buttons
    private List<Integer> actionsOf(int o) {
        List<Integer> out = new ArrayList<>();
        int type = q.optInt("type");
        if (!mine() || (type != Ygo.Q_IDLE && type != Ygo.Q_BATTLE)) {
            return out;
        }
        JSONArray items = q.optJSONArray("items");
        for (int i = 0; items != null && i < items.length(); i++) {
            if (items.optJSONObject(i).optInt("o") == o && o >= 0) {
                out.add(i);
            }
        }
        return out;
    }

    private String kindLabel(JSONObject it) {
        JSONObject c = ya().card(it.optInt("c"));
        switch (it.optInt("k")) {
            case Ygo.ACT_SUMMON: return c != null && c.optInt("lv") >= 5 ? "TRIBUTE SUMMON" : "NORMAL SUMMON";
            case Ygo.ACT_SET_MONSTER:
            case Ygo.ACT_SET_SPELL: return "SET";
            case Ygo.ACT_FLIP_SUMMON: return "FLIP SUMMON";
            case Ygo.ACT_POSITION: return "CHANGE POSITION";
            case Ygo.ACT_ACTIVATE: return YgoArt.monster(c) ? "USE EFFECT" : "ACTIVATE";
            case Ygo.ACT_SPECIAL: return "SPECIAL SUMMON";
            case Ygo.ACT_ATTACK: return "ATTACK";
            case Ygo.ACT_TO_BATTLE: return "BATTLE";
            case Ygo.ACT_TO_MAIN2: return "MAIN 2";
            case Ygo.ACT_END_TURN: return "END TURN";
            default: return "?";
        }
    }

    // -------------------------------------------------------------------------------------- the other side

    private void runBeats() {
        long now = System.currentTimeMillis();
        while (!beats.isEmpty() && (long) beats.get(0)[0] <= now) {
            Object[] b = beats.remove(0);
            switch ((String) b[1]) {
                case "se":
                    v.sound.cue((String) b[2]);
                    break;
                case "say":
                    caption = (String) b[2];
                    captionCard = (int) b[6];
                    captionUntil = now + 1100;
                    break;
                case "float":
                    floats.add(new Object[]{now, b[4], b[2], b[5]});
                    break;
                case "fx":
                    startFx((String) b[2], (int) b[3], (int) b[4], (int) b[5], (int) b[6], now);
                    break;
                case "end":
                    busyUntil = now + 200;
                    shownSt = st;   // (the table catches up with the duel)
                    shownObjs = objs;
                    fx.settle();
                    break;
                default:
                    break;
            }
        }
        if (!ended && idle() && st.optInt("over") == 0 && q.optInt("type") != Ygo.Q_NONE && q.optInt("player") != seat) {
            if (s.mode == MODE_DUEL) {
                aiAt = now + (q.optInt("type") == Ygo.Q_CHAIN ? 250 : 550);
            } else if (s.game != null) {
                JSONArray moves = YgoFriendGame.moves(s.game);
                if (moves.length() > applied) {
                    JSONObject m = moves.optJSONObject(applied++);
                    if (m.has("concede")) {
                        Ygo.concede(duel, m.optInt("concede"));
                        refresh();
                        callOut();
                    } else {
                        answer(ints(m.optJSONArray("p")), m.optInt("z"));
                    }
                } else if (now - polledAt > 8000) {
                    polledAt = now;
                    v.app.refreshYgoGames();
                }
            }
        }
        if (aiAt > 0 && now >= aiAt) {
            aiAt = 0;
            try {
                String a = Ygo.ai(duel, s.level);
                if (a != null) {
                    JSONObject o = new JSONObject(a);
                    answer(ints(o.optJSONArray("p")), o.optInt("z"));
                }
            } catch (Exception ignored) {
            }
        }
        if (!ended && st.optInt("over") == 1 && beats.isEmpty()) {
            finish();
        }
        askIfNeeded();
    }

    static int[] ints(JSONArray a) {
        return Ygo.ints(a);
    }

    private String theirName() {
        String n = s.who;
        int sp = n.indexOf("  ");
        return (sp > 0 ? n.substring(0, sp) : n).toUpperCase();
    }

    private String cardName(int card) {
        return card == 0 ? "A card" : ya().name(card);
    }

    // what happened since the last answer, in time
    private void callOut() {
        JSONArray ev;
        try {
            ev = new JSONArray(Ygo.events(duel, lastSeq, seat));
        } catch (Exception e) {
            return;
        }
        lastSeq = st.optInt("seq");
        long t = Math.max(System.currentTimeMillis(), beats.isEmpty() ? 0 : (long) beats.get(beats.size() - 1)[0]);
        for (int i = 0; i < ev.length(); i++) {
            JSONArray e = ev.optJSONArray(i);
            int type = e.optInt(1), who = e.optInt(2), o = e.optInt(3), target = e.optInt(4), card = e.optInt(5),
                    value = e.optInt(7);
            boolean me = who == seat;
            if (type == Ygo.EV_ATTACK) {
                se(t + 320, "se_m_mega_kick");   // (the blow, as the card slams home)
            }
            se(t, sound(type, me, e.optInt(6), value));
            switch (type) {
                case Ygo.EV_TURN:
                    fxBeat(t, "turn", who, -1, 0, 0);
                    t = beat(t, "say", me ? "YOUR TURN" : theirName() + "'S TURN", who, -1, 0, 0, 900);
                    break;
                case Ygo.EV_DRAW:
                    fxBeat(t, "draw", who, o, 0, card);
                    t += draws++ < 6 ? 150 : 40;
                    break;
                case Ygo.EV_TO_GRAVE:
                    fxBeat(t, "grave", who, o, 0, card);
                    break;
                case Ygo.EV_PHASE:
                    if (value == 3) {
                        t = beat(t, "say", "BATTLE PHASE", who, -1, 0, 0, 500);
                    }
                    break;
                case Ygo.EV_SUMMON:
                    fxBeat(t, "summon", who, o, value, card);
                    t = beat(t, "say", cardName(card) + (value >= 4 ? " is Special Summoned!" : " is summoned!"), who, o, 0, card, 800);
                    break;
                case Ygo.EV_SET:
                    fxBeat(t, "set", who, o, 0, card);
                    if (me) {
                        t += 380;
                    }
                    if (!me) {
                        t = beat(t, "say", theirName() + " sets a card.", who, o, 0, 0, 500);
                    }
                    break;
                case Ygo.EV_FLIP:
                    fxBeat(t, "flip", who, o, 0, card);
                    t = beat(t, "say", cardName(card) + " is flipped face-up!", who, o, 0, card, 700);
                    break;
                case Ygo.EV_ACTIVATE:
                    fxBeat(t, "activate", who, o, value, card);
                    t = beat(t, "say", cardName(card) + (value > 1 ? " (Chain " + value + ")" : "") + "!", who, o, 0, card, 1000);
                    break;
                case Ygo.EV_NEGATE:
                    fxBeat(t, "negate", who, o, 0, card);
                    t = beat(t, "say", cardName(card) + " is negated!", who, o, 0, card, 800);
                    break;
                case Ygo.EV_ATTACK:
                    fxBeat(t + 150, "attack", who, o, target, card);
                    t = beat(t, "say", cardName(card) + (target < 0 ? " attacks directly!" : " attacks " + cardName(cardOfObj(target)) + "!"),
                            who, o, 0, 0, 900);
                    break;
                case Ygo.EV_DESTROY:
                    fxBeat(t, "destroy", who, o, 0, card);
                    t = beat(t, "say", cardName(card) + " is destroyed!", who, o, 0, card, 700);
                    break;
                case Ygo.EV_DAMAGE:
                    fxBeat(t, "damage", who, o, value, 0);
                    t = beat(t, "float", "-" + value, who, -1 - who, 0xFFF86030, 0, 700);
                    break;
                case Ygo.EV_RECOVER:
                    t = beat(t, "float", "+" + value, who, -1 - who, 0xFF60D860, 0, 600);
                    break;
                case Ygo.EV_COIN:
                    fxBeat(t, "coin", who, -1, value, 0);
                    t = beat(t, "say", value == 1 ? "HEADS!" : "TAILS!", who, -1, 0, 0, 700);
                    break;
                case Ygo.EV_DICE:
                    t = beat(t, "say", "The die shows " + value + "!", who, -1, 0, 0, 700);
                    break;
                case Ygo.EV_LOOK:
                    if (me && card != 0) {
                        t = beat(t, "say", "You see: " + cardName(card), who, o, 0, card, 1300);
                    }
                    break;
                case Ygo.EV_WIN:
                    fxBeat(t, "win", who, -1, me ? 1 : 0, 0);
                    t = beat(t, "say", (me ? "YOU WIN!" : theirName() + " WINS") + new String[]{"  Life Points: 0.", "  A Deck ran out.",
                            "  EXODIA!", "  A concession.", ""}[Math.min(4, Math.max(0, value))], who, -1, 0, 0, 1400);
                    break;
                default:
                    break;
            }
        }
        beats.add(new Object[]{t, "end", "", 0, -1, 0, 0});
        draws = 0;
    }

    private int draws;   // (a run of draws: quick after the first few)

    private void fxBeat(long at, String kind, int who, int o, int value, int card) {
        beats.add(new Object[]{at, "fx", kind, who, o, value, card});
    }

    // where object O is drawn now (CardFx's places: who 0, slot = the object)
    private RectF objRect(int who, int o) {
        if (o < 0 || o >= objX.length || objW[o] <= 0) {
            return null;
        }
        return new RectF(objX[o], objY[o], objX[o] + objW[o], objY[o] + objH[o]);
    }

    // where object O will be once the table catches up (its zone in the duel as it is now)
    private RectF destOf(int o) {
        JSONObject x = obj(o);
        if (x == null) {
            return null;
        }
        int loc = x.optInt("loc"), ctl = x.optInt("ctl"), sq = x.optInt("seq");
        if ((loc == Ygo.LOC_MZONE || loc == Ygo.LOC_SZONE) && ctl >= 0 && ctl < 2 && sq >= 0 && sq < 6) {
            return zoneR[ctl][loc == Ygo.LOC_MZONE ? 1 : 0][sq];
        }
        if (loc == Ygo.LOC_GRAVE && ctl >= 0 && ctl < 2) {
            return graveR[ctl];
        }
        return null;
    }

    private void startFx(String kind, int who, int o, int value, int card, long now) {
        boolean me = who == seat;
        int w = Math.max(0, Math.min(1, who));
        RectF at = objRect(0, o), hand = handR[w], deck = deckR[w], grave = graveR[w], dest = destOf(o);
        switch (kind) {
            case "turn":
                fx.banner(now, 1000, me ? "YOUR TURN" : theirName() + "'S TURN", me ? 0x3048B0 : 0xB03048);
                break;
            case "draw":
                if (deck != null && hand != null) {
                    fx.fly(now, 340, card, !me || card == 0, false, deck, hand, false);
                }
                break;
            case "summon":
            case "set": {
                RectF from = at != null ? at : hand;
                if (from != null && dest != null) {
                    boolean down = kind.equals("set");
                    fx.fly(now, 380, card, down || card == 0, !me, from, dest, true);
                    if (!down) {
                        fx.flash(now + 360, 160, value >= 4 ? 0xF8E0F8 : 0xFFFFFF);
                        fx.sparkle(now + 360, 500, 0, o, value >= 4 ? 0xD890F8 : 0xF8F0B0);
                    }
                }
                break;
            }
            case "flip":
                fx.pop(now, 320, 0, o);
                fx.glow(now, 500, 0, o, 0xFFFFFF);
                break;
            case "activate":
                fx.glow(now, 800, 0, o, 0xF8D050);
                fx.ring(now, 700, 0, o, 0xF8D050);
                break;
            case "negate":
                fx.shake(now, 420, 0, o, 0x404060);
                fx.ring(now, 500, 0, o, 0xF84848);
                break;
            case "attack":
                fx.lunge(now, 500, 0, o, me ? -1 : 1);
                if (value >= 0) {
                    fx.shake(now + 220, 380, 0, value, 0xF83020);
                }
                break;
            case "destroy":
                fx.shake(now, 300, 0, o, 0xF83020);
                fx.fade(now + 200, 600, 0, o);
                break;
            case "grave":
                if (at != null && grave != null && !at.equals(grave)) {
                    fx.fly(now, 360, card, card == 0, false, at, grave, false);
                }
                break;
            case "damage":
                if (value >= 1500) {
                    fx.flash(now, 160, 0xF84020);   // (a heavy blow only: small ones say so with their number)
                }
                break;
            case "coin":
                coin = value;
                coinAt = now;
                break;
            case "win":
                fx.flash(now, 400, value == 1 ? 0xFFF8C0 : 0x400000);
                break;
            default:
                break;
        }
    }

    private boolean turnMine;

    /** The sound the game's duel table plays for an event (ygo_board.c Present), null for none */
    private String sound(int type, boolean me, int flags, int value) {
        switch (type) {
            case Ygo.EV_TURN:
                turnMine = me;
                return me ? "se_ball_tray_enter" : "se_ball_tray_exit";
            case Ygo.EV_PHASE:
                return value < 6 ? (turnMine ? "se_ball_tray_enter" : "se_ball_tray_exit") : null;
            case Ygo.EV_DRAW:
            case Ygo.EV_SET:
            case Ygo.EV_FLIP:
            case Ygo.EV_TO_HAND:
                return "se_rg_card_flip";
            case Ygo.EV_SUMMON:
                return "se_m_teleport";
            case Ygo.EV_POSITION:
                return "se_m_harden";
            case Ygo.EV_ACTIVATE:
                return "se_use_item";
            case Ygo.EV_NEGATE:
                return "se_m_barrier";
            case Ygo.EV_ATTACK:
                return "se_m_double_team";
            case Ygo.EV_DESTROY:
                return "se_faint";
            case Ygo.EV_TO_GRAVE:
                return (flags & 8) != 0 ? "se_m_detect" : null;   // (a Tribute)
            case Ygo.EV_DAMAGE:
                return value >= 2000 ? "se_super_effective" : "se_m_pay_day";
            case Ygo.EV_RECOVER:
                return "se_m_morning_sun";
            case Ygo.EV_STAT:
                return value >= 0 ? "se_m_swords_dance" : "se_m_leer";
            case Ygo.EV_COUNTER:
                return "se_m_minimize";
            case Ygo.EV_EQUIP:
                return "se_m_reflect";
            case Ygo.EV_COIN:
                return "se_m_pay_day";
            case Ygo.EV_DICE:
            case Ygo.EV_SHUFFLE:
                return "se_ball_tray_enter";
            case Ygo.EV_TARGET:
                return "se_m_lock_on";
            case Ygo.EV_CONTROL:
                return "se_m_bind";
            case Ygo.EV_WIN:
                return me ? "!ygo:mus_victory_trainer" : "!";   // (the win's song; a loss: the music stops)
            default:
                return null;
        }
    }

    private void se(long at, String sound) {
        if (sound != null) {
            beats.add(new Object[]{at, "se", sound, 0, -1, 0, 0});
        }
    }

    private int cardOfObj(int o) {
        JSONObject x = obj(o);
        return x == null ? 0 : x.optInt("c");
    }

    private long beat(long at, String kind, String text, int who, int o, int value, int card, int ms) {
        beats.add(new Object[]{at, kind, text, who, o, value, card});
        return at + ms;
    }

    private void finish() {
        ended = true;
        boolean won = st.optInt("winner") == seat && !lost;
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
            JSONObject o = new JSONObject().put("op", "ygoduel").put("won", won ? 1 : 0)
                    .put("kind", s.mode == MODE_FRIEND ? "friend" : "duel");
            if (s.mode == MODE_DUEL) {
                o.put("deck", s.myRating).put("opp", s.oppRating).put("level", s.level);
            }
            v.app.client.order(o);
            JSONObject ygo = v.app.model() == null ? null : v.app.model().snap.optJSONObject("ygo");
            JSONObject today = ygo == null ? null : ygo.optJSONObject("today");
            int left = today == null ? 0 : today.optInt("packsMax") - today.optInt("packs");
            endLine = !won ? "" : left > 0 ? "A booster goes to your game!" : "No more boosters today, but well played!";
        } catch (Exception ignored) {
        }
    }

    // ---------------------------------------------------------------------------------------------- drawing

    @Override
    public void draw(Canvas c, int w, int h) {
        runBeats();
        JSONObject truthSt = st;
        JSONArray truthObjs = objs;
        if (shownSt != null) {
            st = shownSt;   // (the table as shown: an answer's events play on the state before it)
            objs = shownObjs;
        }
        try {
            drawTable(c, w, h);
        } finally {
            st = truthSt;
            objs = truthObjs;
        }
    }

    private void drawTable(Canvas c, int w, int h) {
        hits.clear();
        actions.clear();
        long now = System.currentTimeMillis();
        scale = v.pixelScale();
        fill.setColor(0xFF000000);   // (opaque: a faded card's alpha would wash the mat)
        // the table: a deep blue-violet duel mat, a line between the sides
        fill.setShader(new LinearGradient(0, 0, 0, h, new int[]{0xFF1A1838, 0xFF2A2858, 0xFF1A1838}, null, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        // seven columns: the five zones and a pile at each end
        float cw = Math.min((w - 8 - 6 * 3) / 7f, 46), hw = Math.min(w / 5.6f, 56);
        float need = 16 + 4 * (cw * 1.46f + 3) + 24 + 12 + hw * 1.46f + 30;
        float k = Math.min(1, (h - 8) / need);
        cw *= k;
        hw *= k;
        float chh = cw * 1.46f, hh = hw * 1.46f;
        int them = 1 - seat;
        drawTop(c, w);
        // the table centered between the top line and the hand, the hand along the bottom
        float table = 4 * (chh + 3) + 24, handY = h - 34 - hh;
        float y = Math.max(16, 16 + (handY - 8 - 16 - table) / 2);
        // their side, turned: the Spell & Trap row at the far edge, then the monsters
        zoneRow(c, w, them, false, y, cw, chh, true);
        y += chh + 3;
        zoneRow(c, w, them, true, y, cw, chh, true);
        y += chh + 2;
        lpY = y;
        drawMiddle(c, w, y);
        y += 22;
        zoneRow(c, w, seat, true, y, cw, chh, false);
        y += chh + 3;
        zoneRow(c, w, seat, false, y, cw, chh, false);
        drawHand(c, w, Math.max(y + chh + 6, handY), hw, hh);
        drawBar(c, w, h);
        handR[1 - seat] = new RectF(w - hw - 6, -hh * 0.6f, w - 6, hh * 0.4f);
        fx.drawOver(c, w, h, now, this::objRect, (cc, id, down, turned, x, yy, cw2, ch2, al) ->
                cardAt(cc, down ? 0 : id, x, yy, cw2, ch2, turned, al, false), art);
        if (coin >= 0 && now - coinAt < 1100) {
            fx.drawCoin(c, w / 2f, lpY + 10, now - coinAt, coin == 1, art);
        }
        drawFloats(c, now);
        if (sheetObj >= 0) {
            drawSheet(c, w, h);
        } else if (picking && mine() && needsPicker()) {
            drawPicker(c, w, h);
        } else if (pile != null) {
            drawPile(c, w, h);
        }
        if (ended && idle()) {
            drawEnd(c, w, h);
        }
        if (!beats.isEmpty() || aiAt > 0 || now < busyUntil || now < captionUntil || !floats.isEmpty() || fx.moving(now)) {
            v.postInvalidateOnAnimation();
        } else if (s.mode == MODE_FRIEND && !ended && q.optInt("player") != seat) {
            v.postInvalidateDelayed(1000);
        }
    }

    private boolean needsPicker() {
        int type = q.optInt("type");
        return type == Ygo.Q_CARDS || type == Ygo.Q_SUM || type == Ygo.Q_SORT || type == Ygo.Q_CHAIN || type == Ygo.Q_COUNTER
                || type == Ygo.Q_NAME || ((type == Ygo.Q_OPTION || type == Ygo.Q_ZONE) && q.optInt("max", 1) > 1);
    }

    private void drawTop(Canvas c, int w) {
        JSONObject t = player(1 - seat);
        fill.setColor(0xC0080818);
        c.drawRect(0, 0, w, 14, fill);
        art.text(c, s.who, 4, -1, GameArt.WHITE_TEXT, 0xFF000000);
        String right = "HAND " + (t.optJSONArray("hand") == null ? 0 : t.optJSONArray("hand").length()) + "  DECK " + t.optInt("deck");
        art.text(c, right, w - 4 - art.width(right), -1, 0xFFC0C8E8, 0xFF000000);
    }

    // a row of five zones (monsters or Spells & Traps) with the piles at its ends
    private void zoneRow(Canvas c, int w, int who, boolean monsters, float y, float cw, float ch, boolean turned) {
        JSONObject p = player(who);
        JSONArray zones = p.optJSONArray(monsters ? "monsters" : "spells");
        float x0 = (w - 5 * cw - 4 * 3) / 2f;
        for (int i = 0; i < 5; i++) {
            // (their zones are numbered from their left: ours from the left too, theirs mirrored)
            int z = turned ? 4 - i : i;
            float x = x0 + i * (cw + 3);
            int o = zones == null ? -1 : zones.optInt(z, -1);
            zoneR[who][monsters ? 1 : 0][z] = new RectF(x, y, x + cw, y + ch);
            slot(c, o, x, y, cw, ch, turned);
        }
        float px = turned ? x0 - cw - 3 : x0 + 5 * (cw + 3), ox = turned ? x0 + 5 * (cw + 3) : x0 - cw - 3;
        if (monsters) {
            // the Graveyard beside the monsters, the Extra Deck on the other side
            JSONArray grave = p.optJSONArray("grave");
            graveR[who] = new RectF(px, y, px + cw, y + ch);
            pileSpot(c, grave, px, y, cw, ch, turned, who == seat ? "YOUR GRAVEYARD" : "THEIR GRAVEYARD", "GY");
            int extra = p.optInt("extraCount");
            if (extra > 0) {
                cardAt(c, 0, ox, y, cw, ch, turned, 255, false);
                count(c, extra, ox, y, cw, ch);
                if (who == seat) {
                    final JSONArray mine = p.optJSONArray("extra");
                    hit(ox, y, cw, ch, () -> {
                        pile = mine;
                        pileTitle = "YOUR EXTRA DECK";
                    });
                }
            }
        } else {
            // the Field Spell zone and the Deck
            int field = zones == null ? -1 : zones.optInt(5, -1);
            zoneR[who][0][5] = new RectF(ox, y, ox + cw, y + ch);
            deckR[who] = new RectF(px, y, px + cw, y + ch);
            slot(c, field, ox, y, cw, ch, turned);
            if (p.optInt("deck") > 0) {
                cardAt(c, 0, px, y, cw, ch, turned, 255, false);
                count(c, p.optInt("deck"), px, y, cw, ch);
            }
        }
    }

    private void pileSpot(Canvas c, JSONArray list, float x, float y, float cw, float ch, boolean turned, String title, String tag) {
        if (list != null && list.length() > 0) {
            int top = list.optInt(list.length() - 1);
            cardAt(c, cardOfObj(top), x, y, cw, ch, turned, 255, false);
            count(c, list.length(), x, y, cw, ch);
            hit(x, y, cw, ch, () -> {
                pile = list;
                pileTitle = title;
            });
        } else {
            line.setStyle(Paint.Style.STROKE);
            line.setColor(0x506878B8);
            c.drawRoundRect(new RectF(x, y, x + cw, y + ch), 3, 3, line);
            art.text(c, tag, (int) (x + cw / 2 - art.width(tag) / 2f), (int) (y + ch / 2 - 7), 0x806878B8, 0);
        }
    }

    private void count(Canvas c, int n, float x, float y, float cw, float ch) {
        String t = String.valueOf(n);
        int tw = art.width(t) + 4;
        fill.setColor(0xE0101828);
        c.drawRoundRect(new RectF(x + cw - tw, y + ch - 11, x + cw, y + ch), 3, 3, fill);
        art.text(c, t, (int) (x + cw - tw + 2), (int) (y + ch - 14), GameArt.WHITE_TEXT, 0);
    }

    // a card: face-down ones show their back (the owner sees the card faintly through it), defense position sideways
    private void cardAt(Canvas c, int card, float x, float y, float cw, float ch, boolean turned, int alpha, boolean sideways) {
        c.save();
        float cx = x + cw / 2, cy = y + ch / 2;
        if (sideways) {
            // (a card in defense position lies sideways, made smaller to fit the zone's width)
            float k = cw / ch;
            c.rotate(turned ? 270 : 90, cx, cy);
            ya().draw(c, card, cx - cw * k / 2, cy - ch * k / 2, cw * k, ch * k, scale, alpha);
        } else {
            if (turned) {
                c.rotate(180, cx, cy);
            }
            ya().draw(c, card, x, y, cw, ch, scale, alpha);
        }
        c.restore();
    }

    private void slot(Canvas c, int o, float x, float y, float cw, float ch, boolean turned) {
        JSONObject x0 = obj(o);
        if (x0 == null) {
            line.setStyle(Paint.Style.STROKE);
            line.setStrokeWidth(1);
            line.setColor(0x506878B8);
            c.drawRoundRect(new RectF(x, y, x + cw, y + ch), 3, 3, line);
            return;
        }
        objX[o] = x;
        objY[o] = y;
        objW[o] = cw;
        objH[o] = ch;
        boolean up = x0.optInt("up") == 1, defense = x0.optInt("loc") == Ygo.LOC_MZONE && x0.optInt("ap") == 0;
        int card = up ? x0.optInt("c") : 0;
        long now = System.currentTimeMillis();
        int falpha = fx.slotAlpha(0, o, now), ftint = fx.slotTint(0, o, now);
        float fsc = fx.slotScale(0, o, now);
        c.save();
        c.translate(fx.slotDx(0, o, now, cw), fx.slotDy(0, o, now, ch));
        c.scale(fsc, fsc, x + cw / 2, y + ch / 2);
        cardAt(c, card, x, y, cw, ch, turned, falpha, defense);
        if (ftint != 0) {
            fill.setColor(ftint);
            c.drawRoundRect(new RectF(x, y, x + cw, y + ch), 3, 3, fill);
        }
        c.restore();
        if (falpha == 0) {
            return;
        }
        if (!up && x0.optInt("c") != 0) {
            // your own face-down card: its name faintly over the back
            fill.setColor(0x90000000);
            c.drawRect(x, y + ch - 12, x + cw, y + ch, fill);
            String n = ya().name(x0.optInt("c"));
            while (art.width(n) > cw && n.length() > 2) {
                n = n.substring(0, n.length() - 1);
            }
            art.text(c, n, (int) x + 1, (int) (y + ch - 15), 0xFFC0C8E8, 0);
        }
        if (up && x0.optInt("loc") == Ygo.LOC_MZONE) {
            // ATK / DEF under the card
            String stat = x0.optInt("at") + "/" + x0.optInt("df");
            fill.setColor(0xD0000000);
            c.drawRect(x - 2, y + ch - 10, x + cw + 2, y + ch + 1, fill);
            int sw = art.width(stat);
            c.save();
            float k2 = Math.min(1, (cw + 4) / (float) sw);
            c.scale(k2, 1, x + cw / 2, y);
            art.text(c, stat, (int) (x + cw / 2 - sw / 2f), (int) (y + ch - 13), x0.optInt("ap") == 1 ? 0xFFF8D870 : 0xFF88C8F8, 0);
            c.restore();
        }
        if (x0.optInt("cn") > 0) {
            fill.setColor(0xFF5070E0);
            c.drawCircle(x + 6, y + 6, 6, fill);
            String n = String.valueOf(x0.optInt("cn"));
            art.text(c, n, (int) (x + 6 - art.width(n) / 2f), (int) y - 1, GameArt.WHITE_TEXT, 0);
        }
        List<Integer> can = actionsOf(o);
        boolean target = picking && mine() && q.optInt("type") == Ygo.Q_CARDS && isItem(o);
        if (!can.isEmpty() || target) {
            float pulse = (float) (0.5 + 0.5 * Math.sin(System.currentTimeMillis() / 160.0));
            line.setStyle(Paint.Style.STROKE);
            line.setStrokeWidth(2);
            line.setColor(((int) (140 + 115 * pulse) << 24) | (target ? 0xF86060 : 0xF8E070));
            c.drawRoundRect(new RectF(x - 2, y - 2, x + cw + 2, y + ch + 2), 4, 4, line);
            line.setStrokeWidth(1);
            v.postInvalidateOnAnimation();
        }
        final int obj = o;
        hit(x, y, cw, ch, () -> sheetObj = obj);
    }

    private boolean isItem(int o) {
        JSONArray items = q.optJSONArray("items");
        for (int i = 0; items != null && i < items.length(); i++) {
            if (items.optJSONObject(i).optInt("o") == o) {
                return true;
            }
        }
        return false;
    }

    private void drawMiddle(Canvas c, int w, float y) {
        long now = System.currentTimeMillis();
        fill.setColor(0xB0080818);
        c.drawRect(0, y, w, y + 20, fill);
        JSONObject me = player(seat), them = player(1 - seat);
        String a = "LP " + me.optInt("lp"), b = "LP " + them.optInt("lp");
        art.text(c, a, 4, (int) y + 2, 0xFF88D0F8, 0xFF000000);
        art.text(c, b, w - 4 - art.width(b), (int) y + 2, 0xFFF89888, 0xFF000000);
        String text;
        int color = 0xFFF8F0C8;
        if (now < captionUntil && !caption.isEmpty()) {
            text = caption;
        } else if (ended) {
            text = "";
        } else if (mine()) {
            int type = q.optInt("type");
            text = type == Ygo.Q_IDLE ? Ygo.PHASES[Math.min(5, st.optInt("phase"))] + ": YOUR MOVE"
                    : type == Ygo.Q_BATTLE ? "BATTLE: ATTACK OR GO ON" : prompt();
            color = 0xFFF8E070;
        } else {
            text = s.mode == MODE_FRIEND ? theirName() + "'S MOVE" : theirName() + " is thinking...";
            color = 0xFFC0C8E8;
        }
        int room = w - art.width(a) - art.width(b) - 20;
        List<String> lines = wrap(text, room);
        String shown = lines.get(0);
        art.text(c, shown, (w - art.width(shown)) / 2, (int) y + 2, color, 0xFF000000);
        if (captionCard != 0 && now < captionUntil) {
            float cw = Math.min(w * 0.3f, 80), ch = cw * 1.46f;
            float al = Math.min(1, (captionUntil - now) / 250f);
            ya().draw(c, captionCard, w - cw - 6, y + 10 - ch / 2, cw, ch, scale, (int) (255 * al));
        }
    }

    private void drawHand(Canvas c, int w, float y, float hw, float hh) {
        JSONArray hand = player(seat).optJSONArray("hand");
        if (hand == null) {
            return;
        }
        int n = hand.length();
        float step = n <= 1 ? 0 : Math.min(hw + 3, (w - 8 - hw) / (float) (n - 1));
        float x0 = (w - (step * (n - 1) + hw)) / 2f;
        float ex = n == 0 ? (w - hw) / 2f : Math.min(x0 + step * n, w - hw - 4);
        handR[seat] = new RectF(ex, y, ex + hw, y + hh);
        for (int i = 0; i < n; i++) {
            int o = hand.optInt(i);
            boolean can = !actionsOf(o).isEmpty() || (picking && mine() && q.optInt("type") == Ygo.Q_CARDS && isItem(o));
            float x = x0 + i * step, lift = can ? 4 : 0;
            objX[o] = x;
            objY[o] = y - lift;
            objW[o] = hw;
            objH[o] = hh;
            ya().draw(c, cardOfObj(o), x, y - lift, hw, hh, scale, can || !mine() ? 255 : 170);
            if (can) {
                fill.setColor(0xFFF8E070);
                c.drawRect(x + 2, y - lift - 3, x + hw - 2, y - lift - 1, fill);
            }
            final int obj = o;
            hit(x, y - 6, i == n - 1 ? hw : step, hh + 6, () -> sheetObj = obj);
        }
    }

    private void drawBar(Canvas c, int w, int h) {
        int by = h - 26;
        if (ended) {
            return;
        }
        button(c, "MENU", 4, by, 60, true);
        hit(4, by, 60, 24, this::menu);
        int type = q.optInt("type");
        if (mine() && picking && !needsPicker()) {
            // (a question's window closed without an answer: open it again)
            button(c, "ANSWER", w - 84, by, 80, true);
            hit(w - 84, by, 80, 24, () -> picking = false);
            return;
        }
        if (mine() && (type == Ygo.Q_IDLE || type == Ygo.Q_BATTLE)) {
            JSONArray items = q.optJSONArray("items");
            int bx = w - 4;
            for (int i = items == null ? -1 : items.length() - 1; i >= 0; i--) {
                JSONObject it = items.optJSONObject(i);
                int kd = it.optInt("k");
                if (kd != Ygo.ACT_TO_BATTLE && kd != Ygo.ACT_TO_MAIN2 && kd != Ygo.ACT_END_TURN) {
                    continue;
                }
                String label = kindLabel(it);
                int bw = art.width(label) + 20;
                bx -= bw;
                button(c, label, bx, by, bw, true);
                final int pick = i;
                hit(bx, by, bw, 24, () -> answer(pick));
                bx -= 2;
            }
        }
    }

    private void menu() {
        Menu m = Menu.make(v, "DUEL");
        m.add("YOUR GRAVEYARD", () -> {
            pile = player(seat).optJSONArray("grave");
            pileTitle = "YOUR GRAVEYARD";
        });
        m.add("THEIR GRAVEYARD", () -> {
            pile = player(1 - seat).optJSONArray("grave");
            pileTitle = "THEIR GRAVEYARD";
        });
        m.add("BANISHED CARDS", () -> {
            JSONArray all = new JSONArray();
            for (int p = 0; p < 2; p++) {
                JSONArray b = player(p).optJSONArray("banished");
                for (int i = 0; b != null && i < b.length(); i++) {
                    all.put(b.optInt(i));
                }
            }
            pile = all;
            pileTitle = "BANISHED CARDS";
        });
        m.add(sStopAlways ? "PASS QUIET CHANCES" : "STOP AT EVERY CHANCE", () -> sStopAlways = !sStopAlways);
        m.add("CONCEDE", () -> v.showMenu(Menu.confirm(v, s.mode == MODE_FRIEND ? "Concede this duel?"
                : "Concede this duel? You won't lose any cards.", this::concede)));
        m.add("BACK", null);
        v.showMenu(m);
    }

    private void concede() {
        if (ended || duel == 0) {
            return;
        }
        if (s.mode == MODE_FRIEND && !mine()) {
            v.message("You can concede when it's your move.");
            return;
        }
        Ygo.concede(duel, seat);
        if (s.mode == MODE_FRIEND && s.game != null) {
            // (a move of its own: the friend's phone concedes for this side too)
            try {
                applied++;
                v.app.postYgoMove(s.game, new JSONObject().put("concede", seat));
            } catch (org.json.JSONException ignored) {
            }
        }
        lost = true;
        refresh();
        callOut();
    }

    private void drawFloats(Canvas c, long now) {
        for (int i = floats.size() - 1; i >= 0; i--) {
            Object[] f = floats.get(i);
            long age = now - (long) f[0];
            if (age > 900) {
                floats.remove(i);
                continue;
            }
            int where = (int) f[1];
            float x, y;
            if (where < 0) {
                int p = -1 - where;
                x = p == seat ? 30 : v.width() - 40;
                y = lpY - age / 30f;
            } else {
                x = objX[where] + objW[where] / 2;
                y = objY[where] + objH[where] / 2 - age / 30f;
            }
            String t = (String) f[2];
            c.save();
            c.scale(2, 2, x, y);
            art.text(c, t, (int) (x - art.width(t) / 2f), (int) y - 8, (int) f[3], 0xFF000000);
            c.restore();
        }
    }

    // ------------------------------------------------------------------------------------- a card up close

    private void drawSheet(Canvas c, int w, int h) {
        fill.setColor(0xD8000000);
        c.drawRect(0, 0, w, h, fill);
        hit(0, 0, w, h, () -> sheetObj = -1);
        JSONObject o = obj(sheetObj);
        if (o == null) {
            sheetObj = -1;
            return;
        }
        int card = o.optInt("c");
        float cw = Math.min(w * 0.42f, 120), ch = cw * 1.46f;
        float cx = 10, y = 10;
        ya().draw(c, card, cx, y, cw, ch, scale, 255);
        hit(cx, y, cw, ch, () -> {});
        JSONObject k = ya().card(card);
        float tx = cx + cw + 8;
        int ty = (int) y;
        if (k == null) {
            art.text(c, "A face-down card.", (int) tx, ty, GameArt.WHITE_TEXT, 0xFF000000);
        } else {
            for (String l : wrap(k.optString("n"), (int) (w - tx - 8))) {
                art.text(c, l, (int) tx, ty, 0xFFF8E070, 0xFF000000);
                ty += 13;
            }
            for (String l : wrap(YgoArt.kind(k), (int) (w - tx - 8))) {
                art.text(c, l, (int) tx, ty, 0xFFC0C8E8, 0xFF000000);
                ty += 12;
            }
            if (YgoArt.monster(k)) {
                String stat = "ATK " + (o.optInt("loc") == Ygo.LOC_MZONE ? o.optInt("at") : k.optInt("atk")) + "  DEF "
                        + (o.optInt("loc") == Ygo.LOC_MZONE ? o.optInt("df") : k.optInt("def"));
                art.text(c, stat, (int) tx, ty, GameArt.WHITE_TEXT, 0xFF000000);
                ty += 14;
            }
        }
        y = Math.max(ty, y + ch) + 6;
        if (k != null) {
            for (String l : wrap(k.optString("t").replace("\n", " "), w - 20)) {
                if (y > h - 120) {
                    break;
                }
                art.text(c, l, 10, (int) y, 0xFFE0E4F0, 0xFF000000);
                y += 12;
            }
            y += 6;
        }
        // what it can do now
        JSONArray items = q.optJSONArray("items");
        boolean choosing = picking && mine() && q.optInt("type") == Ygo.Q_CARDS;
        for (int i : actionsOf(sheetObj)) {
            JSONObject it = items.optJSONObject(i);
            String label = kindLabel(it);
            if (it.optInt("k") == Ygo.ACT_ACTIVATE && !it.optString("t").isEmpty()) {
                label = label + ": " + it.optString("t");
            }
            int bw = w - 24;
            List<String> ls = wrap(label, bw - 12);
            button(c, ls.get(0), 12, (int) y, bw, true);
            final int pick = i;
            hit(12, y, bw, 24, () -> answer(pick));
            y += 26;
        }
        if (choosing && isItem(sheetObj)) {
            button(c, picks.contains(itemOf(sheetObj)) ? "DON'T CHOOSE IT" : "CHOOSE IT", 12, (int) y, w - 24, true);
            hit(12, y, w - 24, 24, () -> {
                togglePick(itemOf(sheetObj));
                sheetObj = -1;
            });
        }
        button(c, "CLOSE", w / 2 - 40, h - 28, 80, true);
        hit(w / 2 - 40, h - 28, 80, 24, () -> sheetObj = -1);
    }

    private int itemOf(int o) {
        JSONArray items = q.optJSONArray("items");
        for (int i = 0; items != null && i < items.length(); i++) {
            if (items.optJSONObject(i).optInt("o") == o) {
                return i;
            }
        }
        return -1;
    }

    private void togglePick(int i) {
        if (i < 0) {
            return;
        }
        int at = picks.indexOf(i);
        if (at >= 0) {
            picks.remove(at);
        } else {
            picks.add(i);
        }
        v.buzz(false);
    }

    private void drawPile(Canvas c, int w, int h) {
        fill.setColor(0xE0080818);
        c.drawRect(0, 0, w, h, fill);
        hit(0, 0, w, h, () -> {});
        art.text(c, pileTitle, (w - art.width(pileTitle)) / 2, 4, GameArt.WHITE_TEXT, 0xFF000000);
        int cols = 5, n = pile.length();
        float cw = (w - 16 - (cols - 1) * 4) / (float) cols, ch = cw * 1.46f;
        if (n == 0) {
            String t = "Nothing here yet.";
            art.text(c, t, (w - art.width(t)) / 2, 40, 0xFFB8C8D0, 0xFF000000);
        }
        for (int i = 0; i < n; i++) {
            int o = pile.optInt(n - 1 - i);
            float x = 8 + (i % cols) * (cw + 4), y = 22 + (i / cols) * (ch + 4) - scroll;
            if (y + ch < 0 || y > h) {
                continue;
            }
            ya().draw(c, cardOfObj(o), x, y, cw, ch, scale, 255);
            final int obj = o;
            hit(x, y, cw, ch, () -> {
                pile = null;
                sheetObj = obj;
            });
        }
        pileBottom = 22 + ((n + cols - 1) / cols) * (ch + 4);
        button(c, "CLOSE", w / 2 - 40, h - 28, 80, true);
        hit(w / 2 - 40, h - 28, 80, 24, () -> {
            pile = null;
            scroll = 0;
        });
    }

    private float pileBottom, pickBottom;

    @Override
    public int scrollMax() {
        if (pile != null) {
            return Math.max(0, (int) (pileBottom + 40 - v.height()));
        }
        if (picking && needsPicker()) {
            return Math.max(0, (int) (pickBottom + 40 - v.height()));
        }
        return 0;
    }

    // ------------------------------------------------------------------------------------- choosing cards

    private void drawPicker(Canvas c, int w, int h) {
        fill.setColor(0xE8080818);
        c.drawRect(0, 0, w, h, fill);
        hit(0, 0, w, h, () -> {});
        int type = q.optInt("type");
        JSONArray items = q.optJSONArray("items");
        int ty = 4 - scroll;
        String title = type == Ygo.Q_CHAIN ? "Chain an effect?" : type == Ygo.Q_SORT ? "Put them in order" : prompt();
        for (String l : wrap(title, w - 12)) {
            art.text(c, l, (w - art.width(l)) / 2, ty, 0xFFF8E070, 0xFF000000);
            ty += 13;
        }
        if (type == Ygo.Q_SUM) {
            int sum = 0;
            for (int p : picks) {
                sum += items.optJSONObject(p).optInt("p") & 0xFFFF;
            }
            String t = "Chosen: " + sum + " / " + q.optInt("sum");
            art.text(c, t, (w - art.width(t)) / 2, ty, 0xFFC8D0D8, 0xFF000000);
            ty += 14;
        }
        if (type == Ygo.Q_NAME) {
            drawNames(c, w, h, ty);
            return;
        }
        int cols = 4, n = items == null ? 0 : items.length();
        float cw = (w - 16 - (cols - 1) * 6) / (float) cols, ch = cw * 1.46f;
        for (int i = 0; i < n; i++) {
            JSONObject it = items.optJSONObject(i);
            float x = 8 + (i % cols) * (cw + 6), y = ty + (i / cols) * (ch + 30);
            int card = it.optInt("c");
            boolean byWords = type == Ygo.Q_OPTION || type == Ygo.Q_ZONE;
            if (byWords) {
                art.window(c, 0, (int) x, (int) y, (int) cw, (int) ch);
                int ly = (int) y + 4;
                for (String l : wrap(itemLabel(type, it), (int) cw - 8)) {
                    art.text(c, l, (int) x + 4, ly, GameArt.TEXT, GameArt.SHADOW);
                    ly += 12;
                }
            } else {
                ya().draw(c, card, x, y, cw, ch, scale, 255);
            }
            int order = picks.indexOf(i);
            if (type == Ygo.Q_COUNTER) {
                String t = amounts[i] + " / " + (it.optInt("p") & 0xFFFF);
                art.text(c, t, (int) (x + cw / 2 - art.width(t) / 2f), (int) (y + ch + 2), GameArt.WHITE_TEXT, 0);
                final int k = i;
                hit(x, y, cw / 2, ch, () -> amounts[k] = Math.max(0, amounts[k] - 1));
                hit(x + cw / 2, y, cw / 2, ch, () -> amounts[k] = Math.min(it.optInt("p") & 0xFFFF, amounts[k] + 1));
                continue;
            }
            if (order >= 0) {
                line.setStyle(Paint.Style.STROKE);
                line.setStrokeWidth(2);
                line.setColor(0xFFF8E070);
                c.drawRoundRect(new RectF(x - 1, y - 1, x + cw + 1, y + ch + 1), 3, 3, line);
                line.setStrokeWidth(1);
                fill.setColor(0xFFF8E070);
                c.drawCircle(x + cw - 7, y + 8, 7, fill);
                String num = String.valueOf(order + 1);
                art.text(c, num, (int) (x + cw - 7 - art.width(num) / 2f), (int) y + 1, 0xFF302000, 0);
            }
            // under it: whose and where it is, or the effect's words
            String sub = it.optString("t");
            if (sub.isEmpty()) {
                JSONObject o = obj(it.optInt("o"));
                sub = o == null ? "" : (o.optInt("ctl") == seat ? "YOUR " : "THEIR ") + where(o.optInt("loc"));
            }
            List<String> sl = wrap(sub, (int) cw + 4);
            for (int li = 0; li < Math.min(2, sl.size()); li++) {
                art.text(c, sl.get(li), (int) x, (int) (y + ch + 1 + li * 11), 0xFFC0C8E8, 0);
            }
            final int k = i;
            hit(x, y, cw, ch, () -> {
                if (type == Ygo.Q_CHAIN) {
                    answer(k);
                    return;
                }
                if ((it.optInt("f") & Ygo.IF_MUST) != 0 && picks.contains(k)) {
                    return;   // (it always counts)
                }
                if (!picks.contains(k) && picks.size() >= q.optInt("max", 99) && type != Ygo.Q_SUM && type != Ygo.Q_SORT) {
                    if (q.optInt("max") == 1) {
                        picks.clear();
                    } else {
                        v.message("You've already chosen enough.");
                        return;
                    }
                }
                togglePick(k);
            });
            hit(x, y + ch, cw, 22, () -> {
                if (card != 0) {
                    JSONObject o = obj(it.optInt("o"));
                    if (o != null) {
                        picking = true;
                        sheetObj = it.optInt("o");
                    }
                }
            });
        }
        pickBottom = ty + scroll + ((n + cols - 1) / cols) * (ch + 30) + 10;
        int[] answerNow = type == Ygo.Q_COUNTER ? amounts.clone() : picked();
        boolean ok = type == Ygo.Q_CHAIN || Ygo.check(duel, answerNow, 0);
        if (type == Ygo.Q_CHAIN) {
            button(c, "DON'T CHAIN", w / 2 - 50, h - 28, 100, true);
            hit(w / 2 - 50, h - 28, 100, 24, () -> answer(new int[0], 0));
            return;
        }
        if (q.optInt("cancel") == 1 || q.optInt("min") == 0) {
            button(c, "NONE", 6, h - 28, 70, true);
            hit(6, h - 28, 70, 24, () -> answer(new int[0], 0));
        }
        button(c, "DONE", w - 86, h - 28, 80, ok);
        hit(w - 86, h - 28, 80, 24, () -> {
            if (Ygo.check(duel, answerNow, 0)) {
                answer(answerNow, 0);
            } else {
                v.message(type == Ygo.Q_SUM ? "Those don't add up to the total." : "Choose the cards it asks for.");
            }
        });
    }

    private static String where(int loc) {
        switch (loc) {
            case Ygo.LOC_HAND: return "HAND";
            case Ygo.LOC_MZONE: return "MONSTER";
            case Ygo.LOC_SZONE: return "SPELL/TRAP";
            case Ygo.LOC_GRAVE: return "GRAVEYARD";
            case Ygo.LOC_BANISHED: return "BANISHED";
            case Ygo.LOC_EXTRA: return "EXTRA DECK";
            case Ygo.LOC_DECK: return "DECK";
            default: return "";
        }
    }

    // a card name to declare: every name it can be, A to Z (the picks: its index, low byte first)
    private void drawNames(Canvas c, int w, int h, int ty) {
        int[] names = Ygo.names(duel);
        List<int[]> rows = new ArrayList<>();
        for (int i = 0; i < names.length; i++) {
            rows.add(new int[]{i, names[i]});
        }
        rows.sort((a, b) -> ya().name(a[1]).compareToIgnoreCase(ya().name(b[1])));
        int y = ty;
        for (int[] r : rows) {
            if (y > -14 && y < h - 30) {
                art.text(c, ya().name(r[1]), 12, y, GameArt.WHITE_TEXT, 0xFF000000);
                final int index = r[0];
                hit(0, y, w, 14, () -> answer(new int[]{index & 255, index >> 8}, 0));
            }
            y += 15;
        }
        pickBottom = y + scroll;
    }

    // ------------------------------------------------------------------------------------------- the end

    private void drawEnd(Canvas c, int w, int h) {
        boolean won = st.optInt("winner") == seat && !lost;
        float cy = h * 0.42f;
        fill.setColor(0xD8080818);
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
        int dx = s.mode == MODE_DUEL ? w / 2 + 4 : w / 2 - bw / 2;
        button(c, "DONE", dx, (int) cy + 20, bw, true);
        hit(dx, cy + 20, bw, 24, () -> v.showOver(null));
    }

    private void again() {
        Setup n = new Setup();
        n.mode = s.mode;
        n.main0 = s.main0;
        n.extra0 = s.extra0;
        n.main1 = s.main1;
        n.extra1 = s.extra1;
        n.level = s.level;
        n.who = s.who;
        n.deckName = s.deckName;
        n.myRating = s.myRating;
        n.oppRating = s.oppRating;
        n.seed = (int) (System.nanoTime() & 0x7FFFFFFF);
        n.first = n.seed >> 4 & 1;
        v.showOver(new YgoDuelPage(v, n));
    }

    // ---------------------------------------------------------------------------------- for the screens test

    /** Both sides by the game's AI for N answers (or until it's this phone's question in a Main Phase), call-outs skipped */
    void autoPlay(int n) {
        for (int i = 0; i < n && duel != 0 && q.optInt("type") != Ygo.Q_NONE; i++) {
            if (i > 10 && q.optInt("player") == seat && q.optInt("type") == Ygo.Q_IDLE) {
                break;
            }
            try {
                JSONObject o = new JSONObject(Ygo.ai(duel, 3));
                answer(ints(o.optJSONArray("p")), o.optInt("z"));
            } catch (Exception e) {
                break;
            }
        }
        beats.clear();
        floats.clear();
        busyUntil = 0;
        captionUntil = 0;
        shownSt = st;
        shownObjs = objs;
        fx.clear();
    }

    /** Tests: one answer by the AI for whoever is asked, its events played in real time */
    void aiMoveNow() {
        try {
            JSONObject o = new JSONObject(Ygo.ai(duel, 3));
            answer(ints(o.optJSONArray("p")), o.optInt("z"));
        } catch (Exception ignored) {
        }
    }

    /** Tests: the motion and beats still going */
    boolean animating() {
        return !beats.isEmpty() || fx.moving(System.currentTimeMillis());
    }

    void inspect(int obj) {
        sheetObj = obj;
    }

    /** The first card in the hand with something to do, up close */
    boolean inspectHand() {
        JSONArray hand = player(seat).optJSONArray("hand");
        for (int i = 0; hand != null && i < hand.length(); i++) {
            if (!actionsOf(hand.optInt(i)).isEmpty()) {
                sheetObj = hand.optInt(i);
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
        if (sheetObj >= 0) {
            sheetObj = -1;
            v.invalidate();
            return;
        }
        if (pile != null) {
            pile = null;
            scroll = 0;
            v.invalidate();
            return;
        }
        if (!ended && st.optInt("over") == 0 && s.mode == MODE_DUEL) {
            Menu m = Menu.make(v, "LEAVE THE DUEL? IT COUNTS AS A LOSS.");
            m.add("CONCEDE AND LEAVE", () -> {
                concede();
                finish();
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
