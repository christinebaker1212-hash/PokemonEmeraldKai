package com.moemon.companion;

import android.graphics.Canvas;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/** GIVE: the BAG's items a POKeMON can hold; tap one to give it */
public final class ItemPickPage extends Page {
    private static final int ROW = 26;
    private final JSONObject place, mon;

    public ItemPickPage(GameView v, JSONObject place, JSONObject mon) {
        super(v);
        this.place = place;
        this.mon = mon;
    }

    /** [item, count] for every holdable item in the BAG, pocket by pocket */
    private List<int[]> items() {
        List<int[]> out = new ArrayList<>();
        Model m = v.app.model();
        JSONArray bag = m == null ? null : m.snap.optJSONArray("bag");
        for (int p = 0; bag != null && p < bag.length(); p++) {
            JSONArray pocket = bag.optJSONArray(p);
            for (int i = 0; pocket != null && i < pocket.length(); i++) {
                JSONArray it = pocket.optJSONArray(i);
                if (it != null && art.item(it.optInt(0)).optInt("hold") == 1) {
                    out.add(new int[]{it.optInt(0), it.optInt(1)});
                }
            }
        }
        return out;
    }

    @Override
    public int scrollMax() {
        return Math.max(0, items().size() * ROW + 40 - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        art.text(c, "GIVE AN ITEM TO", 8, 4 - scroll, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        art.textHex(c, mon.optString("n"), 8 + art.width("GIVE AN ITEM TO ") , 4 - scroll, 0xFFF8E070, GameArt.WHITE_SHADOW);
        List<int[]> l = items();
        if (l.isEmpty()) {
            art.window(c, 0, 12, 30, w - 24, 16);
            art.text(c, "Nothing in the BAG can be held.", 14, 30);
        }
        for (int i = 0; i < l.size(); i++) {
            int y = 26 + i * ROW - scroll;
            if (y < -ROW || y > h) {
                continue;
            }
            art.window(c, 0, 12, y + 4, w - 24, ROW - 8);
            art.itemIcon(c, l.get(i)[0], 10, y);
            art.text(c, art.name("items", l.get(i)[0]), 36, y + 4);
            String n = "x" + l.get(i)[1];
            art.text(c, n, w - 16 - art.width(n), y + 4);
        }
    }

    @Override
    public void tap(int x, int y) {
        List<int[]> l = items();
        int i = (y + scroll - 26) / ROW;
        if (y + scroll >= 26 && i >= 0 && i < l.size() && v.app.client != null) {
            v.app.client.order(Model.item(true, place, mon, l.get(i)[0]));
            v.buzz(false);
            v.showOver(null);
            v.app.changed();
        }
    }
}
