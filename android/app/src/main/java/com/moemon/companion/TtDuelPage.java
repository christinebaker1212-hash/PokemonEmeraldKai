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
 * A game of TRIPLE TRIAD on the phone, upright: the other side's hand along the top, the 3x3 board, yours along the
 * bottom. Tap a card then a square (or drag it there). The game's own engine plays it (Tt), the cards it takes turn
 * over in waves (SAME / PLUS / COMBO called out as in the game), and the game's AI answers. A FREE DUEL's win earns a
 * pack (the game counts today's prizes); the DAILY PUZZLE starts three cards from the end against the best defense.
 */
public final class TtDuelPage extends Page {
    public static final int MODE_DUEL = 0, MODE_PUZZLE = 1, MODE_FRIEND = 2;

    /** What a game is: the rules, both hands, who starts, and (a puzzle) the plays before the player's turn */
    public static final class Setup {
        public int mode, rules, seed, first, level, region;
        public int[] mine = new int[5], theirs = new int[5];
        public String who = "A DUELIST", title = "", place = "";
        public boolean harder;
        public JSONArray moves;   // (a puzzle: replayed before the player's turn)
        public JSONObject game;   // (a friend's duel: Companion.ttGames' record, its moves replayed and added to)
        public Runnable onEnd;    // (the caller's: a friend's game sends its move, etc.)
    }

    private final Setup s;
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG), line = new Paint(Paint.ANTI_ALIAS_FLAG);
    private long duel;
    private JSONObject st;
    // what the board shows while the events play out
    private final int[] showCard = new int[9], showSeat = new int[9];
    private final long[] flipAt = new long[9];
    private final int[] flipFrom = new int[9];
    private final List<Object[]> queue = new ArrayList<>();   // [due ms, kind, ...]
    private long animEnd;
    private String banner = "";
    private long bannerAt;
    private int selected = -1, dragSlot = -1, aiSlot, aiCell;
    private long aiAt, polledAt;
    private int applied;   // (a friend's duel: the game's moves played here so far)
    private float dragX, dragY;
    private boolean prizeSent, ended;
    private String endLine = "";
    private final List<int[]> hits = new ArrayList<>();
    private final List<Runnable> actions = new ArrayList<>();
    // layout (set each draw)
    private float bx, by, cw, ch, hx, hy, hw, hh, ox, oy, ow, oh;
    private final float[] handX = new float[5];

    public TtDuelPage(GameView v, Setup setup) {
        super(v);
        this.s = setup;
        if (s.game != null) {
            duel = TtFriendGame.replay(s.game);
            applied = s.game.optJSONArray("moves").length();
        } else {
            duel = Tt.start(s.rules, 0, s.seed, s.mine, s.theirs, s.first);
        }
        if (s.moves != null && s.game == null) {
            for (int i = 0; i < s.moves.length(); i++) {
                Tt.play(duel, s.moves.optJSONArray(i).optInt(0), s.moves.optJSONArray(i).optInt(1));
            }
        }
        refresh();
        for (int i = 0; i < 9; i++) {
            JSONArray cell = st.optJSONArray("cells").optJSONArray(i);
            showCard[i] = cell.optInt(0);
            showSeat[i] = cell.optInt(1);
        }
        queue.add(new Object[]{System.currentTimeMillis() + 600L, "think"});
        animEnd = System.currentTimeMillis() + 600;
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public void hidden() {
        if (duel != 0) {
            Tt.end(duel);
            duel = 0;
        }
    }

    private void refresh() {
        try {
            st = new JSONObject(Tt.state(duel, 0));
        } catch (Exception e) {
            st = new JSONObject();
        }
    }

    private boolean myTurn() {
        return st.optInt("over") == 0 && st.optInt("toMove") == 0 && System.currentTimeMillis() >= animEnd;
    }

    // -------------------------------------------------------------------------------------------- the moves

    private void myPlay(int slot, int cell) {
        boolean mine = st.optInt("toMove") == 0;
        if (play(slot, cell) && s.game != null && mine) {
            applied++;
            v.app.postTtMove(s.game, slot, cell);
        }
    }

    private boolean play(int slot, int cell) {
        if (duel == 0) {
            return false;
        }
        String ev = Tt.play(duel, slot, cell);
        if (ev == null) {
            v.buzz(false);
            return false;
        }
        selected = -1;
        v.buzz(false);
        animate(ev);
        return true;
    }

    // the events in time: the card goes down, then each wave of captures turns over (a call-out first for SAME / PLUS)
    private void animate(String json) {
        long t = Math.max(System.currentTimeMillis(), animEnd);
        try {
            JSONArray ev = new JSONArray(json);
            int wave = -1;
            for (int i = 0; i < ev.length(); i++) {
                JSONObject e = ev.getJSONObject(i);
                switch (e.optInt("k")) {
                    case Tt.EV_PLACE:
                        queue.add(new Object[]{t, "place", e.optInt("cell"), e.optInt("card"), e.optInt("seat")});
                        t += 260;
                        break;
                    case Tt.EV_RULE:
                        int by = e.optInt("by");
                        queue.add(new Object[]{t, "banner", by == 1 ? "SAME!" : by == 2 ? "PLUS!" : by == 3 ? "COMBO!" : "SAME WALL!"});
                        t += 650;
                        break;
                    case Tt.EV_FLIP:
                        if (e.optInt("wave") != wave) {
                            wave = e.optInt("wave");
                            t += 120;
                        }
                        queue.add(new Object[]{t, "flip", e.optInt("cell"), e.optInt("seat")});
                        break;
                    case Tt.EV_ROUND:
                        t += 500;
                        queue.add(new Object[]{t, "banner", "SUDDEN DEATH!"});
                        t += 900;
                        queue.add(new Object[]{t, "sync"});
                        break;
                    default:
                        break;
                }
            }
        } catch (Exception ignored) {
        }
        t += 420;
        queue.add(new Object[]{t, "sync"});
        queue.add(new Object[]{t, "think"});
        animEnd = t;
        refresh();
        v.invalidate();
    }

    private void runQueue() {
        long now = System.currentTimeMillis();
        if (aiAt > 0 && now >= aiAt) {
            aiAt = 0;
            animEnd = 0;
            play(aiSlot, aiCell);
        }
        if (s.game != null && duel != 0 && now >= animEnd && queue.isEmpty() && st.optInt("over") == 0) {
            JSONArray moves = s.game.optJSONArray("moves");
            if (moves != null && moves.length() > applied && st.optInt("toMove") == 1) {
                JSONArray m = moves.optJSONArray(applied++);
                play(m.optInt(0), m.optInt(1));
            } else if (st.optInt("toMove") == 1 && now - polledAt > 8000) {
                polledAt = now;
                v.app.refreshTtGames();   // (their move comes in from the server)
            }
        }
        while (!queue.isEmpty() && (long) queue.get(0)[0] <= now) {
            Object[] q = queue.remove(0);
            switch ((String) q[1]) {
                case "place":
                    showCard[(int) q[2]] = (int) q[3];
                    showSeat[(int) q[2]] = (int) q[4];
                    flipAt[(int) q[2]] = 0;
                    break;
                case "flip": {
                    int cell = (int) q[2];
                    flipFrom[cell] = showSeat[cell];
                    showSeat[cell] = flipFrom[cell] == 0 ? 1 : 0;
                    flipAt[cell] = now;
                    v.buzz(false);
                    break;
                }
                case "banner":
                    banner = (String) q[2];
                    bannerAt = now;
                    break;
                case "sync":
                    for (int i = 0; i < 9; i++) {
                        JSONArray cell = st.optJSONArray("cells").optJSONArray(i);
                        showCard[i] = cell.optInt(0);
                        showSeat[i] = cell.optInt(1);
                    }
                    break;
                case "think":
                    think();
                    break;
                default:
                    break;
            }
        }
    }

    // the other side's move (the game's AI, or a puzzle's best defense), or the end
    private void think() {
        if (st.optInt("over") != 0) {
            finish();
            return;
        }
        if (st.optInt("toMove") != 1 || s.mode == MODE_FRIEND) {
            return;
        }
        int[] p = s.mode == MODE_PUZZLE ? Tt.best(duel) : Tt.ai(duel, s.level, s.seed ^ st.optInt("turn") * 7919);
        if (p != null) {
            aiSlot = p[0];
            aiCell = p[1];
            aiAt = System.currentTimeMillis() + 450;   // (a moment to think)
            animEnd = aiAt;
            v.invalidate();
        }
    }

    private void finish() {
        if (ended) {
            return;
        }
        ended = true;
        int winner = st.optInt("winner");
        JSONArray score = st.optJSONArray("score");
        boolean perfect = score != null && score.optInt(0) >= 9;
        v.buzz(true);
        if (winner == 0) {
            banner = s.mode == MODE_PUZZLE ? "SOLVED!" : "YOU WIN!";
        } else if (winner == 2) {
            banner = "DRAW";
        } else {
            banner = s.mode == MODE_PUZZLE ? "NOT THIS TIME" : "YOU LOSE";
        }
        bannerAt = System.currentTimeMillis() + 100000;   // (stays)
        if (winner == 0 && s.mode != MODE_FRIEND) {
            JSONObject tt = tt();
            JSONObject today = tt == null ? null : tt.optJSONObject("today");
            boolean puzzle = s.mode == MODE_PUZZLE;
            int left = today == null ? 0 : puzzle ? today.optInt("puzzlesMax") - today.optInt("puzzles")
                    : today.optInt("packsMax") - today.optInt("packs");
            if (left > 0 && v.app.client != null) {
                try {
                    v.app.client.order(new JSONObject().put("op", "ttwin").put("kind", puzzle ? "puzzle" : "duel")
                            .put("harder", s.harder ? 1 : 0).put("perfect", perfect ? 1 : 0));
                    prizeSent = true;
                    endLine = puzzle || s.harder || perfect ? "A SILVER pack goes to your game!" : "A BRONZE pack goes to your game!";
                } catch (org.json.JSONException ignored) {
                }
            } else {
                endLine = puzzle ? "Today's puzzle prize is already yours." : "No more prizes today, but well played!";
            }
        } else if (s.mode == MODE_PUZZLE && winner != 0) {
            endLine = "There's a way to win from here. Try again!";
        }
        if (s.onEnd != null) {
            s.onEnd.run();
        }
    }

    private JSONObject tt() {
        com.moemon.companion.core.Model m = v.app.model();
        return m == null ? null : m.snap.optJSONObject("tt");
    }

    // ----------------------------------------------------------------------------------------------- drawing

    @Override
    public void draw(Canvas c, int w, int h) {
        runQueue();
        hits.clear();
        actions.clear();
        long now = System.currentTimeMillis();
        TtArt tt = v.app.ttArt;
        // the table: deep blue felt with a soft light in the middle, as on the DS
        fill.setShader(new LinearGradient(0, 0, 0, h, 0xFF14284A, 0xFF0A1428, Shader.TileMode.CLAMP));
        c.drawRect(0, 0, w, h, fill);
        fill.setShader(null);
        // their hand along the top
        JSONArray hands = st.optJSONArray("hands");
        JSONArray theirs = hands == null ? null : hands.optJSONArray(1), mine = hands == null ? null : hands.optJSONArray(0);
        ow = Math.min((w - 20) / 5f, 44);
        oh = ow * TtArt.H / TtArt.W;
        hw = Math.min((w - 20) / 5f, 56);
        hh = hw * TtArt.H / TtArt.W;
        // the table centered top to bottom: their hand, the board, the score line, your hand
        float boardMax = (w - 24) / 3f;
        float room = h - 18 - oh - 10 - 34 - hh - 16;
        cw = Math.min(boardMax, (room - 8) / 3f * TtArt.W / TtArt.H);
        ch = cw * TtArt.H / TtArt.W;
        float total = oh + 10 + ch * 3 + 8 + 34 + hh;
        oy = Math.max(18, 18 + (h - 18 - 6 - total) / 2);
        int leftThem = 0;
        for (int i = 0; theirs != null && i < 5; i++) {
            leftThem += theirs.optJSONArray(i).optInt(1) == 1 ? 0 : 1;
        }
        ox = (w - ow * leftThem - 2 * Math.max(0, leftThem - 1)) / 2f;
        art.text(c, s.who, 6, 2, GameArt.WHITE_TEXT, 0xFF000000);
        String rules = tt.rulesText(s.rules);
        art.text(c, rules, w - 6 - Math.min(art.width(rules), w / 2), 2, 0xFFB8C8E8, 0xFF000000);
        for (int i = 0, n = 0; theirs != null && i < 5; i++) {
            JSONArray k = theirs.optJSONArray(i);
            if (k.optInt(1) == 1) {
                continue;
            }
            tt.draw(c, k.optInt(0), TtArt.SEAT_OTHER, null, ox + n++ * (ow + 2), oy, ow, oh, 1, 255);
        }
        // the board
        bx = (w - cw * 3 - 8) / 2f;
        by = oy + oh + 10;
        fill.setColor(0xFF0A1830);
        c.drawRoundRect(new RectF(bx - 6, by - 6, bx + cw * 3 + 14, by + ch * 3 + 14), 6, 6, fill);
        line.setStyle(Paint.Style.STROKE);
        line.setStrokeWidth(1);
        line.setColor(0xFF4870B0);
        c.drawRoundRect(new RectF(bx - 6, by - 6, bx + cw * 3 + 14, by + ch * 3 + 14), 6, 6, line);
        JSONArray cells = st.optJSONArray("cells");
        for (int i = 0; i < 9; i++) {
            float x = bx + (i % 3) * (cw + 4), y = by + (i / 3) * (ch + 4);
            fill.setColor(0xFF183058);
            c.drawRoundRect(new RectF(x, y, x + cw, y + ch), 4, 4, fill);
            int tile = cells == null ? 0 : cells.optJSONArray(i).optInt(3);
            if (tile > 0 && showCard[i] == 0) {
                art.text(c, tt.words == null ? "" : tt.words.optJSONArray("types").optString(tile), (int) x + 4, (int) (y + ch / 2 - 6),
                        0xFF6890C8, 0xFF000000);
            }
            if (showCard[i] != 0) {
                float flip = 1;
                int seat = showSeat[i];
                if (flipAt[i] > 0 && now - flipAt[i] < 320) {
                    float k = (now - flipAt[i]) / 320f;
                    flip = (float) Math.abs(Math.cos(k * Math.PI));
                    seat = k < 0.5f ? flipFrom[i] : showSeat[i];
                }
                int elem = cells == null ? 0 : cells.optJSONArray(i).optInt(4);
                int[] mods = elem == 0 ? null : new int[]{elem, elem, elem, elem};
                tt.draw(c, showCard[i], seat, mods, x, y, cw, ch, Math.max(0.04f, flip), 255);
            } else if (myTurn() && (selected >= 0 || dragSlot >= 0)) {
                line.setColor(0x8098C0FF);
                c.drawRoundRect(new RectF(x + 2, y + 2, x + cw - 2, y + ch - 2), 4, 4, line);
            }
            final int cell = i;
            hits.add(new int[]{(int) x, (int) y, (int) cw, (int) ch});
            actions.add(() -> {
                if (selected >= 0 && myTurn() && showCard[cell] == 0) {
                    myPlay(selected, cell);
                }
            });
        }
        // the score
        JSONArray score = st.optJSONArray("score");
        drawScore(c, bx - 4, by + ch * 3 + 18, score == null ? 5 : score.optInt(0), 0);
        drawScore(c, bx + cw * 3 + 8 - 28, by + ch * 3 + 18, score == null ? 5 : score.optInt(1), 1);
        String turn = st.optInt("over") != 0 ? "" : now < animEnd ? "" : st.optInt("toMove") == 0 ? "YOUR TURN"
                : s.mode == MODE_FRIEND ? "THEIR TURN" : "THINKING...";
        art.text(c, turn, (w - art.width(turn)) / 2, (int) (by + ch * 3 + 18), 0xFFF8E070, 0xFF000000);
        // your hand
        hy = by + ch * 3 + 8 + 34;
        int leftMine = 0;
        for (int i = 0; mine != null && i < 5; i++) {
            leftMine += mine.optJSONArray(i).optInt(1) == 1 ? 0 : 1;
        }
        hx = (w - hw * leftMine - 2 * Math.max(0, leftMine - 1)) / 2f;
        int can = st.optInt("over") == 0 ? Tt.playable(duel) : 0;
        for (int i = 0, n = 0; mine != null && i < 5; i++) {
            JSONArray k = mine.optJSONArray(i);
            if (k.optInt(1) == 1) {
                continue;
            }
            float x = hx + n++ * (hw + 2);
            handX[i] = x;
            if (i == dragSlot) {
                continue;
            }
            boolean playable = (can >> i & 1) != 0 && myTurn();
            float lift = i == selected ? 10 : 0;
            tt.draw(c, k.optInt(0), TtArt.SEAT_PLAYER, null, x, hy - lift, hw, hh, 1,
                    playable || st.optInt("toMove") != 0 ? 255 : 150);
            if (i == selected) {
                line.setColor(0xFFF8E070);
                line.setStrokeWidth(2);
                c.drawRoundRect(new RectF(x - 1, hy - lift - 1, x + hw + 1, hy - lift + hh + 1), 4, 4, line);
                line.setStrokeWidth(1);
            }
            final int slot = i;
            hits.add(new int[]{(int) x, (int) (hy - 12), (int) hw, (int) (hh + 12)});
            actions.add(() -> {
                if (myTurn() && (Tt.playable(duel) >> slot & 1) != 0) {
                    selected = selected == slot ? -1 : slot;
                    v.buzz(false);
                }
            });
        }
        if (dragSlot >= 0 && mine != null) {
            tt.draw(c, mine.optJSONArray(dragSlot).optInt(0), TtArt.SEAT_PLAYER, null, dragX - hw * 0.6f, dragY - hh * 0.6f,
                    hw * 1.2f, hh * 1.2f, 1, 235);
        }
        // the call-outs and the end
        if (!banner.isEmpty() && (now - bannerAt < 700 || bannerAt > now)) {
            drawBanner(c, w, by + ch * 1.5f, banner);
        }
        if (ended && now >= animEnd) {
            drawEnd(c, w, h);
        }
        if (now < animEnd || aiAt > 0 || !queue.isEmpty() || anyFlipping(now)) {
            v.postInvalidateOnAnimation();
        } else if (s.game != null && st.optInt("over") == 0 && st.optInt("toMove") == 1) {
            v.postInvalidateDelayed(1000);   // (watching for their move)
        }
    }

    private boolean anyFlipping(long now) {
        for (long f : flipAt) {
            if (f > 0 && now - f < 320) {
                return true;
            }
        }
        return false;
    }

    private void drawScore(Canvas c, float x, float y, int n, int seat) {
        fill.setColor(v.app.ttArt.color(seat, 2));
        c.drawRoundRect(new RectF(x, y - 2, x + 28, y + 14), 4, 4, fill);
        fill.setColor(v.app.ttArt.color(seat, 0));
        c.drawRoundRect(new RectF(x + 1, y - 1, x + 27, y + 13), 3, 3, fill);
        String t = String.valueOf(n);
        art.text(c, t, (int) (x + 14 - art.width(t) / 2f), (int) y - 1, GameArt.WHITE_TEXT, 0xFF000000);
    }

    private void drawBanner(Canvas c, int w, float cy, String text) {
        int tw = art.width(text);
        float bw = tw * 2 + 32;
        fill.setColor(0xD0101828);
        c.drawRoundRect(new RectF((w - bw) / 2, cy - 18, (w + bw) / 2, cy + 18), 8, 8, fill);
        line.setColor(0xFFF8E070);
        c.drawRoundRect(new RectF((w - bw) / 2, cy - 18, (w + bw) / 2, cy + 18), 8, 8, line);
        c.save();
        c.scale(2, 2, w / 2f, cy);
        art.text(c, text, (w - tw) / 2, (int) cy - 8, 0xFFF8E070, 0xFF503010);
        c.restore();
    }

    private void drawEnd(Canvas c, int w, int h) {
        float y = by + ch * 3 - 40;
        if (!endLine.isEmpty()) {
            int tw = art.width(endLine);
            art.window(c, 0, (w - tw) / 2 - 8, (int) y - 4, tw + 16, 16);
            art.text(c, endLine, (w - tw) / 2, (int) y - 4);
        }
        int bw = 80;
        if (s.mode == MODE_DUEL || s.mode == MODE_PUZZLE) {
            button(c, s.mode == MODE_PUZZLE ? "TRY AGAIN" : "AGAIN", w / 2 - bw - 4, (int) y + 16, bw, true);
            hits.add(new int[]{w / 2 - bw - 4, (int) y + 16, bw, 24});
            actions.add(this::again);
        }
        button(c, "DONE", w / 2 + 4, (int) y + 16, bw, true);
        hits.add(new int[]{w / 2 + 4, (int) y + 16, bw, 24});
        actions.add(() -> v.showOver(null));
    }

    private void again() {
        Setup n = new Setup();
        n.mode = s.mode;
        n.rules = s.rules;
        n.level = s.level;
        n.mine = s.mine;
        n.theirs = s.theirs;
        n.who = s.who;
        n.title = s.title;
        n.harder = s.harder;
        n.moves = s.moves;
        n.seed = s.mode == MODE_PUZZLE ? s.seed : (int) (System.nanoTime() & 0x7FFFFFFF);
        n.first = s.mode == MODE_PUZZLE ? s.first : n.seed & 1;
        v.showOver(new TtDuelPage(v, n));
    }

    // ------------------------------------------------------------------------------------------------- touch

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
        selected = -1;
    }

    @Override
    public boolean dragStart(int x, int y) {
        if (!myTurn()) {
            return false;
        }
        for (int i = 0; i < 5; i++) {
            if (in(x, y, (int) handX[i], (int) (hy - 12), (int) hw, (int) (hh + 12)) && (Tt.playable(duel) >> i & 1) != 0) {
                dragSlot = i;
                selected = -1;
                dragX = x;
                dragY = y;
                v.buzz(false);
                return true;
            }
        }
        return false;
    }

    @Override
    public void dragMove(int x, int y) {
        dragX = x;
        dragY = y;
        v.invalidate();
    }

    @Override
    public void dragEnd(int x, int y) {
        int slot = dragSlot;
        dragSlot = -1;
        for (int i = 0; i < 9; i++) {
            float cx = bx + (i % 3) * (cw + 4), cy = by + (i / 3) * (ch + 4);
            if (x >= cx && x < cx + cw && y >= cy && y < cy + ch && showCard[i] == 0) {
                myPlay(slot, i);
                return;
            }
        }
        v.invalidate();
    }

    @Override
    public void back() {
        if (!ended && st.optInt("over") == 0 && s.mode == MODE_DUEL) {
            Menu m = Menu.make(v, "LEAVE THE GAME?");
            m.add("LEAVE (NO PRIZE)", () -> v.showOver(null));
            m.add("KEEP PLAYING", null);
            v.showMenu(m);
            return;
        }
        v.showOver(null);
    }
}
