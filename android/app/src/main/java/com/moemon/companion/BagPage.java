package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * The BAG's five pockets: each item's icon, name and count; tap one to read what it does (the game's own words), and
 * GIVE it to a party POKéMON when it can be held.
 */
public final class BagPage extends Page {
    private static final String[] POCKETS = {"ITEMS", "BALLS", "TMs", "BERRIES", "KEY"};
    private static final int ROW = 24, TABS = 40, PANEL = 92;
    private int pocket, chosen = -1;
    private final Paint fill = new Paint();

    public BagPage(GameView v) {
        super(v);
    }

    private JSONArray items() {
        Model m = v.app.model();
        JSONArray bag = m == null ? null : m.snap.optJSONArray("bag");
        JSONArray p = bag == null ? null : bag.optJSONArray(pocket);
        return p != null ? p : new JSONArray();
    }

    private int listH() {
        return v.contentHeight() - TABS - PANEL;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, items().length() * ROW + 8 - listH());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = items();
        c.save();
        c.clipRect(0, TABS, w, TABS + listH());
        for (int i = 0; i < l.length(); i++) {
            int y = TABS + 2 + i * ROW - scroll;
            if (y < TABS - ROW || y > TABS + listH()) {
                continue;
            }
            JSONArray it = l.optJSONArray(i);
            fill.setColor(i == chosen ? 0xFFF8E8B0 : 0xFFF8F8F8);
            c.drawRect(10, y + 2, w - 10, y + ROW - 1, fill);
            art.itemIcon(c, it.optInt(0), 12, y);
            art.text(c, art.name("items", it.optInt(0)), 40, y + 3);
            if (pocket != 4) {
                String n = "x" + it.optInt(1);
                art.text(c, n, w - 16 - art.width(n), y + 3);
            }
        }
        if (l.length() == 0) {
            art.window(c, 0, 12, TABS + 10, w - 24, 16);
            art.text(c, v.app.model() == null ? "Nothing from the game yet." : "This POCKET is empty.", 14, TABS + 10);
        }
        c.restore();
        // the banner and the pockets
        banner(c, w, "BAG", v.app.model() == null ? "" : money());
        fill.setColor(BANNER_DARK);
        c.drawRect(0, 20, w, TABS, fill);
        int tw = w / POCKETS.length;
        for (int i = 0; i < POCKETS.length; i++) {
            int nw = art.width(POCKETS[i]);
            boolean on = i == pocket;
            if (on) {
                art.window(c, 0, i * tw + 4, 24, tw - 8, 12);
            }
            art.text(c, POCKETS[i], i * tw + (tw - nw) / 2, 22, on ? GameArt.TEXT : GameArt.WHITE_TEXT,
                    on ? GameArt.SHADOW : 0xFF202850);
        }
        // the chosen item's words
        int py = TABS + listH() + 4;
        art.window(c, 0, 12, py + 4, w - 24, PANEL - 14);
        if (chosen < 0 || chosen >= l.length()) {
            art.text(c, "Tap an item to read about it.", 16, py + 4, GameArt.GRAY_TEXT, GameArt.SHADOW);
            return;
        }
        int item = l.optJSONArray(chosen).optInt(0);
        art.itemIcon(c, item, 12, py + 2);
        art.text(c, art.name("items", item), 40, py + 4, GameArt.BLUE_TEXT, GameArt.SHADOW);
        boolean hold = art.item(item).optInt("hold") == 1, usable = UseFlow.kind(art, item) != UseFlow.NONE;
        if (hold) {
            button(c, "GIVE", w - 74, py, 64, !v.app.locked());
        }
        if (usable) {
            button(c, "USE", w - (hold ? 138 : 74), py, 64, !v.app.locked());
        }
        art.paragraph(c, art.item(item).optString("d"), 16, py + 24, w - 36, GameArt.TEXT, GameArt.SHADOW);
    }

    private String money() {
        JSONObject s = v.app.model().save();
        return "$" + s.optLong("money");
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width();
        if (y < 20) {
            return;
        }
        if (y < TABS) {
            pocket = Math.min(POCKETS.length - 1, x / (w / POCKETS.length));
            scroll = 0;
            chosen = -1;
            return;
        }
        JSONArray l = items();
        if (y < TABS + listH()) {
            int i = (y + scroll - TABS - 2) / ROW;
            chosen = i >= 0 && i < l.length() ? i : -1;
            return;
        }
        int py = TABS + listH() + 4;
        if (chosen < 0 || chosen >= l.length() || x < w - 138 || y > py + 26) {
            return;
        }
        final int item = l.optJSONArray(chosen).optInt(0);
        boolean holdable = art.item(item).optInt("hold") == 1;
        boolean onUse = UseFlow.kind(art, item) != UseFlow.NONE && (holdable ? x < w - 74 : x >= w - 74);
        if (onUse && v.app.model() != null && v.app.client != null) {
            if (v.app.locked()) {
                v.message(Companion.LOCKED);
                return;
            }
            Menu menu = Menu.make(v, "USE ON...");
            JSONArray party = v.app.model().party();
            for (int k = 0; party != null && k < party.length(); k++) {
                final JSONObject mon = party.optJSONObject(k);
                if (mon == null || mon.optInt("e") == 1) {
                    continue;
                }
                final int slot = k;
                menu.add(art.plain(GameArt.hexText(mon.optString("n"))), () -> UseFlow.use(v, Model.partyPlace(slot), mon, item, null));
            }
            menu.add("CANCEL", null);
            v.showMenu(menu);
            return;
        }
        if (x < w - 74) {
            return;
        }
        Model m = v.app.model();
        if (art.item(item).optInt("hold") != 1 || m == null || v.app.client == null) {
            return;
        }
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        Menu menu = Menu.make(v, "GIVE TO...");
        JSONArray party = m.party();
        for (int k = 0; party != null && k < party.length(); k++) {
            final JSONObject mon = party.optJSONObject(k);
            if (mon == null || mon.optInt("e") == 1) {
                continue;
            }
            final int slot = k;
            menu.add(art.plain(GameArt.hexText(mon.optString("n"))), () -> {
                v.app.client.order(Model.item(true, Model.partyPlace(slot), mon, item));
                v.buzz(false);
                v.app.changed();
            });
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    @Override
    public void swipe(int dir) {
        pocket = (pocket + dir + POCKETS.length) % POCKETS.length;
        scroll = 0;
        chosen = -1;
    }
}
