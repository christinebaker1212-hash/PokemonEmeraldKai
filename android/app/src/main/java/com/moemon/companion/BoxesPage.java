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
    }

    private int wx(int w) {
        return (w - WP_W) / 2;
    }

    private static final int WY = 4;

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
        selected = null;
        v.invalidate();
    }

    private Planned planned() {
        Model m = v.app.model();
        return new Planned(m, v.app.client != null ? v.app.client.orders.pending() : java.util.Collections.emptyList());
    }

    private int partyY() {
        return WY + WP_H + 6;
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
        int b = box(m), x0 = wx(w);
        JSONObject bx = m.box(b);
        art.wallpaper(c, bx.optInt("w"), x0, WY);
        // the box's name on the wallpaper's banner, the arrows beside it
        byte[] name = GameArt.hexText(bx.optString("n"));
        int nw = art.width(name);
        art.text(c, name, x0 + (WP_W - nw) / 2, WY + 2, GameArt.WHITE_TEXT, 0xFF383838);
        art.text(c, "<", x0 - 14, WY + 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        art.text(c, ">", x0 + WP_W + 6, WY + 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        for (int s = 0; s < 30; s++) {
            JSONObject place = Model.boxPlace(b, s);
            // (the PC's own places: icons centered at 20 + 24 x column, 28 + 24 x row on the wallpaper)
            int cx = x0 + 8 + (s % 6) * CELL, cy = WY + 16 + (s / 6) * CELL;
            drawSlot(c, p, place, cx, cy, CELL);
        }
        // the party
        boolean pc = m.pcReachable();
        int py = partyY(), px0 = (w - 6 * PARTY_CELL) / 2;
        art.text(c, pc ? "PARTY" : "PARTY: no PC here", px0, py - 2, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
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
            for (String line : wrap("Tap a POKéMON to see it. Hold it, or drag it, to move it. Swipe for the next BOX.",
                    w - 32)) {
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
            art.text(c, "Tap where it should go.", 12, ty, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
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
        int w = v.width(), x0 = wx(w) + 8, y0 = WY + 16;
        if (x >= x0 && x < x0 + 6 * CELL && y >= y0 && y < y0 + 5 * CELL) {
            return Model.boxPlace(box(m), (y - y0) / CELL * 6 + (x - x0) / CELL);
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
        if (y >= WY && y < WY + 20) {
            if (x < x0) {
                swipe(-1);
            } else if (x >= x0 + WP_W) {
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
        if (v.app.locked()) {
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
        return longPress(x, y);
    }

    @Override
    public void dragMove(int x, int y) {
        dragX = x;
        dragY = y;
    }

    @Override
    public void dragEnd(int x, int y) {
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
        Model m = v.app.model();
        int n = boxCount(m);
        if (n > 0) {
            box = (box(m) + dir + n) % n;
            selected = null;
        }
    }

    @Override
    public void shown() {
        moveMode = false;
        held = null;
    }
}
