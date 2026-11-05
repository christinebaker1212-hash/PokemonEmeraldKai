package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.DashPathEffect;
import android.graphics.Paint;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Planned;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The PC: a box drawn on its own wallpaper, the party under it, the chosen POKeMON's details. Drag a POKeMON (or hold
 * it, then drag) to move it; swipe or tap the arrows for the next box; tap the box's name for all 40. Orders the
 * game hasn't answered yet show where they'll end, with a dashed outline.
 */
public final class BoxesPage extends Page {
    private static final int WP_W = 160, WP_H = 144, CELL = 24, PARTY_CELL = 36;
    private int box = -1;
    private JSONObject selected;                  // the place chosen (details shown)
    private JSONObject held;                      // the place picked up (dragging, or MOVE mode)
    private JSONObject heldMon;
    private boolean moveMode;
    private int dragX, dragY;
    private final Paint dash = new Paint(), cursor = new Paint(), shade = new Paint();
    // ALL BOXES: every box at once, the party pinned on top; drag across boxes, or SELECT several and send them
    private boolean grid, picking;
    private final java.util.LinkedHashSet<String> picks = new java.util.LinkedHashSet<>();
    private final java.util.Map<String, JSONObject> pickPlaces = new java.util.HashMap<>();
    private int gridBottom;
    private final Paint fill = new Paint(), pickPaint = new Paint();
    private static final int GRID_TOP = 60, GRID_GAP = 6;

    public BoxesPage(GameView v) {
        super(v);
        dash.setStyle(Paint.Style.STROKE);
        dash.setStrokeWidth(1);
        dash.setColor(0xFFF8F8F8);
        dash.setPathEffect(new DashPathEffect(new float[]{3, 2}, 0));
        cursor.setStyle(Paint.Style.STROKE);
        cursor.setStrokeWidth(2);
        cursor.setColor(0xFFF85838);
        shade.setColor(0x90303030);
        pickPaint.setStyle(Paint.Style.STROKE);
        pickPaint.setStrokeWidth(2);
        pickPaint.setColor(0xFF40C8F8);
    }

    // the box as large as the screen allows in whole screen pixels (1.5x on a 1080-wide phone)
    private float bk() {
        int sc = v.pixelScale();
        float k = (float) Math.floor((v.width() - 28) * sc / (float) WP_W) / sc;
        return Math.max(1, Math.min(k, 2));
    }

    private int wpW() {
        return Math.round(WP_W * bk());
    }

    private int wpH() {
        return Math.round(WP_H * bk());
    }

    private float cell() {
        return CELL * bk();
    }

    private int wx(int w) {
        return (w - wpW()) / 2;
    }

    // (the wallpaper's top: under the picker's banner when picking)
    private int WY() {
        return onPick != null ? 24 : 4;
    }

    // ------------------------------------------------------------------------------- picking one (walks, trades)
    /** What a pick does with the POKéMON chosen */
    public interface Pick {
        void picked(JSONObject mon, JSONObject place);
    }

    /** Why a POKéMON can't be chosen (null: it can) */
    public interface Why {
        String why(JSONObject mon, JSONObject place);
    }

    private String pickTitle;
    private Pick onPick;
    private Why pickWhy;
    private boolean pickParty;

    /**
     * The boxes as a picker, over the tabs: the same wallpaper, swipes and party row; the ones that can't be chosen
     * dimmed; a tap shows the card with CHOOSE (and why not, for a dimmed one). PARTY: whether the party row counts.
     */
    public static BoxesPage picker(GameView v, String title, boolean party, Why why, Pick pick) {
        BoxesPage b = new BoxesPage(v);
        b.pickTitle = title;
        b.pickParty = party;
        b.pickWhy = why;
        b.onPick = pick;
        return b;
    }

    boolean isPicker() {
        return onPick != null;
    }

    private String whyNot(JSONObject mon, JSONObject place) {
        if (mon == null) {
            return "";
        }
        if (place.has("party") && !pickParty) {
            return "Only a POKéMON in a BOX can.";
        }
        return pickWhy == null ? null : pickWhy.why(mon, place);
    }

    private int boxCount(Model m) {
        JSONArray b = m == null ? null : m.boxes();
        return b == null ? 0 : b.length();
    }

    private int box(Model m) {
        if (box < 0 && m != null) {
            box = m.snap.optInt("box");
        }
        return Math.max(0, Math.min(box, Math.max(0, boxCount(m) - 1)));
    }

    public void showBox(int b) {
        box = b;
        if (!moveMode) {
            selected = null;   // (moving one: it stays in hand for the box chosen)
        }
        v.invalidate();
    }

    private Planned planned() {
        Model m = v.app.model();
        return new Planned(m, v.app.client != null ? v.app.client.orders.pending() : java.util.Collections.emptyList());
    }

    private int partyY() {
        return WY() + wpH() + 6;
    }

    private int cardY() {
        return partyY() + PARTY_CELL + 22;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        Model m = v.app.model();
        if (m == null) {
            art.window(c, 0, 12, 20, w - 24, 48);
            art.text(c, v.app.client == null ? "Pair a game from the MENU." : "Waiting for the game...", 18, 22);
            art.text(c, "OPTIONS > CO-OP > PHONE LINK", 18, 40, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return;
        }
        Planned p = planned();
        if (grid) {
            drawGrid(c, w, h, m, p);
            return;
        }
        if (onPick != null) {
            banner(c, w, pickTitle, "CANCEL");
        }
        int b = box(m), x0 = wx(w);
        JSONObject bx = m.box(b);
        c.save();
        c.translate(x0, WY());
        c.scale(bk(), bk());
        art.wallpaper(c, bx.optInt("w"), 0, 0);
        c.restore();
        // the box's name on the wallpaper's banner, the arrows beside it
        byte[] name = GameArt.hexText(bx.optString("n"));
        int nw = art.width(name);
        int bannerY = WY() + (int) (8 * bk()) - 6;
        art.text(c, name, x0 + (wpW() - nw) / 2, bannerY, GameArt.WHITE_TEXT, 0xFF383838);
        art.text(c, "<", x0 - 12, bannerY, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        art.text(c, ">", x0 + wpW() + 4, bannerY, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        for (int s = 0; s < 30; s++) {
            JSONObject place = Model.boxPlace(b, s);
            // (the PC's own places: icons centered at 20 + 24 x column, 28 + 24 x row on the wallpaper, scaled)
            float cx = x0 + 8 * bk() + (s % 6) * cell(), cy = WY() + 16 * bk() + (s / 6) * cell();
            drawSlotF(c, p, place, cx, cy, cell());
        }
        // the party
        boolean pc = m.pcReachable();
        int py = partyY(), px0 = (w - 6 * PARTY_CELL) / 2;
        art.text(c, pc ? "PARTY" : "PARTY: no PC here", px0, py - 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        if (onPick == null) {
            chipButton(c, "ALL BOXES", w - 8 - art.width("ALL BOXES") - 10, py - 7);
        }
        for (int i = 0; i < 6; i++) {
            int cx = px0 + i * PARTY_CELL, cy = py + 16;
            art.window(c, 0, cx + 2, cy + 2, PARTY_CELL - 4, PARTY_CELL - 4);
            drawSlot(c, p, Model.partyPlace(i), cx + 2, cy + 2, PARTY_CELL - 4);
            if (!pc) {
                c.drawRect(cx + 2, cy + 2, cx + PARTY_CELL - 2, cy + PARTY_CELL - 2, shade);
            }
        }
        // the chosen POKeMON: a short card, its SUMMARY a tap away
        int cy = cardY();
        JSONObject mon = selected == null ? null : p.at(selected);
        if (mon == null) {
            art.window(c, 0, 12, cy + 4, w - 24, 48);
            int ty = cy + 4;
            for (String line : wrap(onPick != null ? "Tap the POKéMON to choose. Swipe, or tap the BOX's name, for another BOX."
                    : "Tap a POKéMON to see it. Hold it, or drag it, to move it. Swipe for the next BOX.", w - 32)) {
                art.text(c, line, 14, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
                ty += 16;
            }
        } else {
            drawCard(c, w, cy, mon);
        }
        // the one being dragged, under the finger
        if (held != null && heldMon != null && !moveMode) {
            art.icon(c, heldMon.optInt("e") == 1 ? MonCard.eggSpecies(art) : heldMon.optInt("s"), dragX - 16, dragY - 26,
                    GameArt.ICON_PLAIN);
        }
    }

    private static final int CARD_H = 52;

    private void drawCard(Canvas c, int w, int cy, JSONObject mon) {
        art.window(c, 0, 12, cy + 4, w - 24, CARD_H);
        boolean egg = mon.optInt("e") == 1;
        int species = mon.optInt("s");
        art.icon(c, egg ? MonCard.eggSpecies(art) : species, 12, cy + 6, GameArt.ICON_PLAIN);
        int nw = art.textHex(c, mon.optString("n"), 46, cy + 2, GameArt.TEXT, GameArt.SHADOW);
        if (!egg) {
            int g = mon.optInt("g");
            if (g == 0) {
                art.text(c, "♂", 46 + nw + 3, cy + 2, GameArt.BLUE_TEXT, GameArt.SHADOW);
            } else if (g == 254) {
                art.text(c, "♀", 46 + nw + 3, cy + 2, GameArt.RED_TEXT, GameArt.SHADOW);
            }
            String lv = "Lv" + mon.optInt("l");
            art.text(c, lv, w - 18 - art.width(lv), cy + 2);
            org.json.JSONArray t = art.species(species).optJSONArray("t");
            if (t != null) {
                art.typeIcon(c, t.optInt(0), 46, cy + 22);
                if (t.optInt(1) != t.optInt(0)) {
                    art.typeIcon(c, t.optInt(1), 82, cy + 22);
                }
            }
            if (mon.optInt("sh") == 1) {
                chip(c, "SHINY", 120, cy + 24, 0xFFD89010);
            }
            if (mon.optInt("off") == 1) {
                chip(c, "ON OFFER", mon.optInt("sh") == 1 ? 160 : 120, cy + 24, 0xFFE07820);
            }
            int item = mon.optInt("i");
            if (item != 0) {
                art.itemIcon(c, item, w - 40, cy + 22);
            }
        }
        boolean locked = v.app.locked();
        if (locked) {
            moveMode = false;
            held = null;
        }
        int by = cy + CARD_H + 10, bw = (w - 20) / 4;
        if (onPick != null) {
            // picking: CHOOSE, or why it can't be
            String no = whyNot(mon, selected);
            button(c, "CHOOSE", 10, by, w - 20, no == null);
            int ty = by + 26;
            for (String line : wrap(no == null ? "" : no, w - 24)) {
                art.text(c, line, 12, ty, 0xFFF8A050, GameArt.WHITE_SHADOW);
                ty += 16;
            }
            return;
        }
        button(c, "SUMMARY", 10, by, bw, true);
        button(c, moveMode ? "CANCEL" : "MOVE", 10 + bw, by, bw, !locked);
        button(c, "GIVE", 10 + 2 * bw, by, bw, !locked && !egg);
        button(c, "TAKE", 10 + 3 * bw, by, bw, !locked && mon.optInt("i") != 0);
        int ty = by + 26;
        if (locked) {
            for (String line : wrap(Companion.LOCKED, w - 24)) {
                art.text(c, line, 12, ty, 0xFFF8A050, GameArt.WHITE_SHADOW);
                ty += 16;
            }
        } else if (moveMode) {
            for (String line : wrap("Tap where it should go. Swipe, or tap the arrows or the BOX's name, for another BOX.",
                    w - 24)) {
                art.text(c, line, 12, ty, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
                ty += 16;
            }
        }
    }

    /** MOVE from the summary: the next tap on a place moves it there */
    public void startMove(JSONObject place) {
        Planned p = planned();
        if (p.at(place) != null) {
            selected = place;
            held = place;
            heldMon = p.at(place);
            moveMode = true;
        }
    }

    /** The places the summary pages through: this box's POKéMON, or the party */
    private java.util.List<JSONObject> around(JSONObject place) {
        java.util.List<JSONObject> l = new java.util.ArrayList<>();
        Planned p = planned();
        if (place.has("party")) {
            for (int i = 0; i < p.party.size(); i++) {
                l.add(Model.partyPlace(i));
            }
        } else {
            for (int s = 0; s < 30; s++) {
                JSONObject q = Model.boxPlace(place.optInt("box"), s);
                if (p.at(q) != null) {
                    l.add(q);
                }
            }
        }
        return l;
    }

    // a box slot at the box's scale: the icon scaled with it (the PC's 32-pixel icon in a 24-pixel place)
    private void drawSlotF(Canvas c, Planned p, JSONObject place, float x, float y, float size) {
        float k = size / CELL;
        if (k == 1) {
            drawSlot(c, p, place, (int) x, (int) y, (int) size);
            return;
        }
        c.save();
        c.translate(x, y);
        c.scale(k, k);
        drawSlot(c, p, place, 0, 0, CELL);
        c.restore();
    }

    private void drawSlot(Canvas c, Planned p, JSONObject place, int x, int y, int size) {
        JSONObject mon = p.at(place);
        boolean isHeld = held != null && Planned.key(held).equals(Planned.key(place)) && !moveMode;
        if (mon != null && !isHeld) {
            int sp = mon.optInt("e") == 1 ? MonCard.eggSpecies(art) : mon.optInt("s");
            art.icon(c, sp, x + (size - 32) / 2, y + (size - 32) / 2, GameArt.ICON_PLAIN);
            if (mon.optInt("i") != 0) {
                Paint dot = cursor;
                dot.setStyle(Paint.Style.FILL);
                dot.setColor(0xFFF8D030);
                c.drawRect(x + size - 7, y + size - 7, x + size - 3, y + size - 3, dot);
                dot.setStyle(Paint.Style.STROKE);
                dot.setColor(0xFFF85838);
            }
            if (mon.optInt("off") == 1) {
                offerMark(c, x + 1, y + 1);
            }
            if (onPick != null && whyNot(mon, place) != null) {
                c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, shade);   // (can't be chosen here)
            }
        }
        if (p.isDashed(place) || p.itemPending(place)) {
            c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, dash);
        }
        if ((selected != null && Planned.key(selected).equals(Planned.key(place)))
                || (moveMode && held != null && Planned.key(held).equals(Planned.key(place)))) {
            c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, cursor);
        }
    }

    /** The place under (x, y), or null */
    private JSONObject placeAt(int x, int y) {
        Model m = v.app.model();
        if (m == null) {
            return null;
        }
        if (grid) {
            return gridPlaceAt(x, y, m);
        }
        int w = v.width();
        float x0 = wx(w) + 8 * bk(), y0 = WY() + 16 * bk(), cl = cell();
        if (x >= x0 && x < x0 + 6 * cl && y >= y0 && y < y0 + 5 * cl) {
            return Model.boxPlace(box(m), (int) ((y - y0) / cl) * 6 + (int) ((x - x0) / cl));
        }
        int px0 = (w - 6 * PARTY_CELL) / 2, py = partyY() + 16;
        if (x >= px0 && x < px0 + 6 * PARTY_CELL && y >= py && y < py + PARTY_CELL) {
            return Model.partyPlace((x - px0) / PARTY_CELL);
        }
        return null;
    }

    @Override
    public void tap(int x, int y) {
        Model m = v.app.model();
        if (m == null) {
            return;
        }
        int w = v.width(), x0 = wx(w);
        if (grid) {
            tapGrid(x, y, m);
            return;
        }
        if (onPick != null) {
            tapPick(x, y, m);
            return;
        }
        if (y >= partyY() - 7 && y < partyY() + 10 && x >= w - 8 - art.width("ALL BOXES") - 10) {
            setGrid(true);
            return;
        }
        if (y >= WY() && y < WY() + 16 * bk() + 4) {
            if (x < x0 + 10) {
                swipe(-1);
            } else if (x >= x0 + wpW() - 10) {
                swipe(1);
            } else {
                v.showOver(new BoxListPage(v, this));
            }
            return;
        }
        JSONObject place = placeAt(x, y);
        Planned p = planned();
        if (moveMode && held != null) {
            if (place != null) {
                drop(held, place, p);
            }
            moveMode = false;
            held = null;
            return;
        }
        if (place != null) {
            selected = p.at(place) != null ? place : null;
            return;
        }
        JSONObject mon = selected == null ? null : p.at(selected);
        int by = cardY() + CARD_H + 10, bw = (w - 20) / 4;
        if (mon != null && y >= cardY() && y < by) {
            openSummary(selected);
            return;
        }
        if (mon != null && y >= by && y < by + 24) {
            int k = Math.min(3, Math.max(0, (x - 10) / bw));
            if (k == 0) {
                openSummary(selected);
                return;
            }
            if (v.app.locked()) {
                v.message(Companion.LOCKED);
                return;
            }
            if (k == 1) {
                moveMode = !moveMode;
                held = moveMode ? selected : null;
                heldMon = moveMode ? mon : null;
            } else if (k == 2 && mon.optInt("e") == 0) {
                v.showOver(new ItemPickPage(v, selected, mon));
            } else if (k == 3 && mon.optInt("i") != 0) {
                order(Model.item(false, selected, mon, 0));
            }
        }
    }

    private void tapPick(int x, int y, Model m) {
        int w = v.width(), x0 = wx(w);
        if (y < 20) {
            if (x > w - 60) {
                back();
            }
            return;
        }
        if (y >= WY() && y < WY() + 16 * bk() + 4) {
            if (x < x0 + 10) {
                swipe(-1);
            } else if (x >= x0 + wpW() - 10) {
                swipe(1);
            } else {
                v.showOver(new BoxListPage(v, this));
            }
            return;
        }
        Planned p = planned();
        JSONObject place = placeAt(x, y);
        if (place != null) {
            selected = p.at(place) != null ? place : null;
            v.invalidate();
            return;
        }
        JSONObject mon = selected == null ? null : p.at(selected);
        int by = cardY() + CARD_H + 10;
        if (mon != null && y >= by && y < by + 24 && whyNot(mon, selected) == null) {
            JSONObject chosen = selected;
            v.buzz(false);
            v.showOver(null);
            onPick.picked(mon, chosen);
        }
    }

    private void openSummary(JSONObject place) {
        java.util.List<JSONObject> l = around(place);
        int i = 0;
        for (int k = 0; k < l.size(); k++) {
            if (Planned.key(l.get(k)).equals(Planned.key(place))) {
                i = k;
            }
        }
        if (!l.isEmpty()) {
            v.showOver(new SummaryPage(v, l, i, this));
        }
    }

    private void order(JSONObject o) {
        if (v.app.client != null) {
            v.app.client.order(o);
            v.buzz(false);
            v.app.changed();
        }
    }

    /** A move from one place to another as the game will check it: the POKeMON expected at both */
    private void drop(JSONObject from, JSONObject to, Planned p) {
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        JSONObject a = p.at(from), b = p.at(to);
        if (a == null || Planned.key(from).equals(Planned.key(to))) {
            return;
        }
        Model m = v.app.model();
        if ((from.has("party") || to.has("party")) && (m == null || !m.pcReachable())) {
            v.message(Companion.why("nopc"));
            return;
        }
        order(Model.move(from, a, to, b));
        selected = to.has("party") && b == null ? null : to;
    }

    @Override
    public boolean longPress(int x, int y) {
        if (v.app.locked() || onPick != null) {
            return false;   // (in a battle: nothing moves)
        }
        JSONObject place = placeAt(x, y);
        Planned p = planned();
        if (place == null || p.at(place) == null) {
            return false;
        }
        held = place;
        heldMon = p.at(place);
        selected = place;
        moveMode = false;
        dragX = x;
        dragY = y;
        v.buzz(false);
        return true;
    }

    @Override
    public boolean dragStart(int x, int y) {
        return !grid && longPress(x, y);   // (ALL BOXES: a swipe scrolls; hold to pick one up)
    }

    private boolean inArrow;   // (a drag over a box's arrow turns the box once until it leaves)

    @Override
    public void dragMove(int x, int y) {
        dragX = x;
        dragY = y;
        if (grid) {
            v.invalidate();   // (near the top or bottom: the boxes scroll under it, drawGrid)
            return;
        }
        // like the game's PC: a POKéMON carried to the box's edge turns to the next box
        int x0 = wx(v.width());
        boolean arrow = held != null && y >= WY() && y < WY() + wpH() && (x < x0 + 4 || x >= x0 + wpW() - 4);
        if (arrow && !inArrow) {
            int dir = x < x0 + 4 ? -1 : 1;
            Model m = v.app.model();
            int n = boxCount(m);
            if (n > 0) {
                box = (box(m) + dir + n) % n;
                v.buzz(false);
            }
        }
        inArrow = arrow;
    }

    @Override
    public void dragEnd(int x, int y) {
        inArrow = false;
        if (held != null) {
            JSONObject to = placeAt(x, y);
            if (to != null) {
                drop(held, to, planned());
            }
            v.buzz(false);
        }
        held = null;
        heldMon = null;
    }

    @Override
    public void swipe(int dir) {
        if (grid) {
            return;
        }
        Model m = v.app.model();
        int n = boxCount(m);
        if (n > 0) {
            box = (box(m) + dir + n) % n;
            if (!moveMode) {
                selected = null;   // (moving one: it stays in hand)
            }
        }
    }

    @Override
    public void shown() {
        moveMode = false;
        held = null;
    }

    @Override
    public int scrollMax() {
        return grid ? Math.max(0, gridBottom - v.contentHeight()) : 0;
    }

    // ------------------------------------------------------------------------------------------- ALL BOXES

    private void chipButton(Canvas c, String label, int x, int y) {
        int tw = art.width(label);
        art.window(c, 0, x, y + 2, tw + 10, 12);
        art.text(c, label, x + 5, y, GameArt.TEXT, GameArt.SHADOW);
    }

    private void setGrid(boolean on) {
        grid = on;
        picking = false;
        picks.clear();
        pickPlaces.clear();
        selected = null;
        moveMode = false;
        scroll = 0;
        if (on) {
            // (the box that was open, near the top)
            Model m = v.app.model();
            scroll = Math.max(0, gridBoxY(box(m), v.width()) - GRID_TOP - 6);
        }
        v.invalidate();
    }

    private int gridCols(int w) {
        return Math.max(2, Math.min(4, w / 132));
    }

    private float gridBoxW(int w) {
        int cols = gridCols(w);
        return (w - 12 - (cols - 1) * GRID_GAP) / (float) cols;
    }

    // (the PC's own places scaled: 24-pixel cells from 8 in on a 160-wide wallpaper, under its 16-pixel banner)
    private float gridK(int w) {
        return gridBoxW(w) / 160f;
    }

    private float gridCell(int w) {
        return 24 * gridK(w);
    }

    private float gridRow(int w) {
        return 122 * gridK(w) / 5f;   // (five rows spread evenly in the framed inside)
    }

    private int gridBoxH(int w) {
        return (int) (14 + 122 * gridK(w) + 3);
    }

    // where box B starts, in the scrolled list (before the scroll)
    private int gridBoxY(int b, int w) {
        return GRID_TOP + 4 + (b / gridCols(w)) * (gridBoxH(w) + GRID_GAP);
    }

    private float gridBoxX(int b, int w) {
        return 6 + (b % gridCols(w)) * (gridBoxW(w) + GRID_GAP);
    }

    private void drawGrid(Canvas c, int w, int h, Model m, Planned p) {
        int n = boxCount(m);
        float bw = gridBoxW(w), cell = gridCell(w);
        int bh = gridBoxH(w), icon = cell >= 16 ? 16 : (int) cell;
        // (dragging near the top or bottom: the list scrolls)
        if (held != null && !moveMode) {
            if (dragY > h - 28) {
                scrollTo(scroll + 6);
                v.postInvalidateOnAnimation();
            } else if (dragY < GRID_TOP + 18 && dragY > GRID_TOP - 30 && scroll > 0) {
                scrollTo(scroll - 6);
                v.postInvalidateOnAnimation();
            }
        }
        c.save();
        c.clipRect(0, GRID_TOP, w, h);
        for (int b = 0; b < n; b++) {
            float x = gridBoxX(b, w);
            int y = gridBoxY(b, w) - scroll;
            if (y + bh < GRID_TOP || y > h) {
                continue;
            }
            JSONObject bx = m.box(b);
            art.wallpaperAt(c, bx.optInt("w"), x, y + 14, bw, 122 * gridK(w));
            fill.setColor(b == box(m) ? 0xF0284878 : 0xE0182838);
            c.drawRect(x, y, x + bw, y + 14, fill);
            art.textHex(c, bx.optString("n"), (int) x + 3, y, GameArt.WHITE_TEXT, 0xFF101010);
            if (picking && !picks.isEmpty()) {
                art.text(c, "+", (int) (x + bw - 9), y, 0xFF80E0FF, 0xFF101010);
            }
            for (int s = 0; s < 30; s++) {
                JSONObject place = Model.boxPlace(b, s);
                float cx = x + 8 * gridK(w) + (s % 6) * cell, cy = y + 14 + (s / 6) * gridRow(w) + (gridRow(w) - cell) / 2;
                drawMini(c, p, place, cx, cy, cell, icon);
            }
        }
        gridBottom = n == 0 ? GRID_TOP : gridBoxY(n - 1, w) + bh + 10;
        c.restore();
        // pinned on top: the views, SELECT, the party
        fill.setColor(Page.BANNER_DARK);
        c.drawRect(0, 0, w, GRID_TOP, fill);
        chipButton(c, "ONE BOX", 6, 2);
        String sel = picking ? (picks.isEmpty() ? "PICK SOME" : "SEND " + picks.size() + ": TAP A BOX") : "SELECT";
        chipButton(c, picking ? "DONE" : "SELECT", w - 6 - art.width(picking ? "DONE" : "SELECT") - 10, 2);
        if (picking) {
            art.text(c, sel, 6 + art.width("ONE BOX") + 18, 2, 0xFF80E0FF, 0xFF101010);
        }
        boolean pc = m.pcReachable();
        int pcell = Math.min(34, (w - 12) / 6), px0 = (w - 6 * pcell) / 2;
        for (int i = 0; i < 6; i++) {
            int cx = px0 + i * pcell, cy = 21;
            art.window(c, 0, cx + 1, cy + 1, pcell - 2, pcell - 2);
            drawMini(c, p, Model.partyPlace(i), cx + 1, cy + 1, pcell - 2, 32);
            if (!pc) {
                c.drawRect(cx + 1, cy + 1, cx + pcell - 1, cy + pcell - 1, shade);
            }
        }
        // the one carried, under the finger
        if (held != null && heldMon != null && !moveMode) {
            art.icon(c, heldMon.optInt("e") == 1 ? MonCard.eggSpecies(art) : heldMon.optInt("s"), dragX - 16, dragY - 26,
                    GameArt.ICON_PLAIN);
        }
        // the one chosen: name, level and SUMMARY at the bottom
        JSONObject mon = selected == null || picking ? null : p.at(selected);
        if (mon != null && held == null) {
            int by = h - 30;
            art.window(c, 0, 6, by, w - 12, 24);
            art.iconAt(c, mon.optInt("e") == 1 ? MonCard.eggSpecies(art) : mon.optInt("s"), 8, by + 2, 20, GameArt.ICON_PLAIN);
            art.textHex(c, mon.optString("n"), 32, by + 4, GameArt.TEXT, GameArt.SHADOW);
            String lv = mon.optInt("e") == 1 ? "EGG" : "Lv" + mon.optInt("l");
            art.text(c, lv, w - 92 - art.width(lv), by + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            chipButton(c, "SUMMARY", w - 78, by + 4);
        }
    }

    private void drawMini(Canvas c, Planned p, JSONObject place, float x, float y, float size, int icon) {
        JSONObject mon = p.at(place);
        String k = Planned.key(place);
        boolean isHeld = held != null && Planned.key(held).equals(k) && !moveMode;
        if (mon != null && !isHeld) {
            int sp = mon.optInt("e") == 1 ? MonCard.eggSpecies(art) : mon.optInt("s");
            art.iconAt(c, sp, x + (size - icon) / 2f, y + (size - icon) / 2f - (icon >= 32 ? 2 : 1), icon, GameArt.ICON_PLAIN);
            if (mon.optInt("off") == 1) {
                offerMark(c, x + 1, y + 1);
            }
        }
        if (p.isDashed(place) || p.itemPending(place)) {
            c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, dash);
        }
        if (picks.contains(k)) {
            c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, pickPaint);
        } else if (selected != null && Planned.key(selected).equals(k) && !picking) {
            c.drawRect(x + 1, y + 1, x + size - 1, y + size - 1, cursor);
        }
    }

    /** ON OFFER for a trade by mail: a small orange tag in the slot's corner */
    void offerMark(Canvas c, float x, float y) {
        fill.setColor(0xFF803010);
        c.drawRect(x, y, x + 7, y + 6, fill);
        fill.setColor(0xFFF89838);
        c.drawRect(x + 1, y + 1, x + 6, y + 5, fill);
        fill.setColor(0xFFFFFFFF);
        c.drawRect(x + 2, y + 2, x + 5, y + 3, fill);
    }

    private JSONObject gridPlaceAt(int x, int y, Model m) {
        int w = v.width();
        int pcell = Math.min(34, (w - 12) / 6), px0 = (w - 6 * pcell) / 2;
        if (y >= 21 && y < 21 + pcell && x >= px0 && x < px0 + 6 * pcell) {
            return Model.partyPlace((x - px0) / pcell);
        }
        if (y < GRID_TOP) {
            return null;
        }
        float bw = gridBoxW(w), cell = gridCell(w);
        int bh = gridBoxH(w);
        for (int b = 0; b < boxCount(m); b++) {
            float bx = gridBoxX(b, w);
            int by = gridBoxY(b, w) - scroll;
            float gx = bx + 8 * gridK(w);
            if (x >= gx && x < gx + 6 * cell && y >= by + 14 && y < by + 14 + 5 * gridRow(w)) {
                int col = (int) ((x - gx) / cell), row = (int) ((y - by - 14) / gridRow(w));
                if (col < 6 && row < 5) {
                    return Model.boxPlace(b, row * 6 + col);
                }
            }
        }
        return null;
    }

    // the box whose name strip is at (x, y), or -1
    private int gridHeaderAt(int x, int y, Model m) {
        int w = v.width();
        float bw = gridBoxW(w);
        for (int b = 0; b < boxCount(m); b++) {
            float bx = gridBoxX(b, w);
            int by = gridBoxY(b, w) - scroll;
            if (y >= GRID_TOP && x >= bx && x < bx + bw && y >= by && y < by + 14) {
                return b;
            }
        }
        return -1;
    }

    private void tapGrid(int x, int y, Model m) {
        int w = v.width();
        if (y < 18) {
            if (x < 6 + art.width("ONE BOX") + 12) {
                setGrid(false);
            } else if (x > w - 6 - art.width(picking ? "DONE" : "SELECT") - 12) {
                picking = !picking;
                picks.clear();
                pickPlaces.clear();
            }
            v.invalidate();
            return;
        }
        Planned p = planned();
        JSONObject mon = selected == null || picking ? null : p.at(selected);
        if (mon != null && y >= v.contentHeight() - 30) {
            openSummary(selected);
            return;
        }
        int header = gridHeaderAt(x, y, m);
        if (header >= 0) {
            if (picking && !picks.isEmpty()) {
                sendPicks(header, p);
            } else {
                setGrid(false);
                showBox(header);
            }
            return;
        }
        JSONObject place = placeAt(x, y);
        if (place == null) {
            return;
        }
        if (picking) {
            String k = Planned.key(place);
            if (p.at(place) == null) {
                return;
            }
            if (!picks.remove(k)) {
                picks.add(k);
                pickPlaces.put(k, place);
            }
            v.buzz(false);
        } else {
            selected = p.at(place) != null ? place : null;
        }
        v.invalidate();
    }

    // the POKéMON picked, each to the next empty place in box B, in the order picked
    private void sendPicks(int b, Planned p) {
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        java.util.List<JSONObject> empty = new java.util.ArrayList<>();
        for (int s = 0; s < 30; s++) {
            JSONObject q = Model.boxPlace(b, s);
            if (p.at(q) == null) {
                empty.add(q);
            }
        }
        int moved = 0, skipped = 0;
        Model m = v.app.model();
        for (String k : picks) {
            JSONObject from = pickPlaces.get(k), mon = from == null ? null : p.at(from);
            if (mon == null || (from.has("box") && from.optInt("box") == b)) {
                continue;
            }
            if (empty.isEmpty()) {
                skipped++;
                continue;
            }
            if (from.has("party") && (m == null || !m.pcReachable())) {
                skipped++;
                continue;
            }
            JSONObject to = empty.remove(0);
            order(Model.move(from, mon, to, null));
            moved++;
        }
        picks.clear();
        pickPlaces.clear();
        picking = false;
        if (skipped > 0) {
            v.message(moved + " moved. " + skipped + (empty.isEmpty() ? " didn't fit: that box is full." : " stay: the party moves only near a PC."));
        }
        v.invalidate();
    }
}
