package com.moemon.companion;

import android.graphics.Canvas;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

/** The BAG's five pockets; an item that can be held can be given to a party POKéMON from here */
public final class BagPage extends Page {
    private static final String[] POCKETS = {"ITEMS", "BALLS", "TMs", "BERRIES", "KEY"};
    private static final int ROW = 26, TABS = 22;
    private int pocket;

    public BagPage(GameView v) {
        super(v);
    }

    private JSONArray items() {
        Model m = v.app.model();
        JSONArray bag = m == null ? null : m.snap.optJSONArray("bag");
        JSONArray p = bag == null ? null : bag.optJSONArray(pocket);
        return p != null ? p : new JSONArray();
    }

    @Override
    public int scrollMax() {
        return Math.max(0, items().length() * ROW + TABS + 8 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONArray l = items();
        for (int i = 0; i < l.length(); i++) {
            int y = TABS + 4 + i * ROW - scroll;
            if (y < TABS - ROW || y > h) {
                continue;
            }
            JSONArray it = l.optJSONArray(i);
            art.window(c, 0, 12, y + 4, w - 24, ROW - 8);
            art.itemIcon(c, it.optInt(0), 10, y);
            art.text(c, art.name("items", it.optInt(0)), 36, y + 4);
            if (pocket != 4) {
                String n = "x" + it.optInt(1);
                art.text(c, n, w - 16 - art.width(n), y + 4);
            }
        }
        if (l.length() == 0) {
            art.window(c, 0, 12, TABS + 10, w - 24, 16);
            art.text(c, v.app.model() == null ? "Nothing from the game yet." : "This POCKET is empty.", 14, TABS + 10);
        }
        // the pockets, over the list
        android.graphics.Paint bar = new android.graphics.Paint();
        bar.setColor(0xFF70B8C8);
        c.drawRect(0, 0, w, TABS, bar);
        int tw = w / POCKETS.length;
        for (int i = 0; i < POCKETS.length; i++) {
            int nw = art.width(POCKETS[i]);
            boolean on = i == pocket;
            if (on) {
                art.window(c, 0, i * tw + 4, 4, tw - 8, 12);
            }
            art.text(c, POCKETS[i], i * tw + (tw - nw) / 2, 2, on ? GameArt.TEXT : GameArt.WHITE_TEXT,
                    on ? GameArt.SHADOW : GameArt.WHITE_SHADOW);
        }
    }

    @Override
    public void tap(int x, int y) {
        if (y < TABS) {
            pocket = Math.min(POCKETS.length - 1, x / (v.width() / POCKETS.length));
            scroll = 0;
            return;
        }
        JSONArray l = items();
        int i = (y + scroll - TABS - 4) / ROW;
        if (i < 0 || i >= l.length()) {
            return;
        }
        final int item = l.optJSONArray(i).optInt(0);
        Model m = v.app.model();
        if (art.item(item).optInt("hold") != 1 || m == null || v.app.client == null) {
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
    }
}
