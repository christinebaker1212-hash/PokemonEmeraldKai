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
    private final Page back;   // where GIVE came from (shown again after), or null for the tabs
    private boolean use;       // USE (items used on a POKéMON) rather than GIVE (items it can hold)

    public static ItemPickPage forUse(GameView v, JSONObject place, JSONObject mon, Page back) {
        ItemPickPage p = new ItemPickPage(v, place, mon, back);
        p.use = true;
        return p;
    }

    public ItemPickPage(GameView v, JSONObject place, JSONObject mon) {
        this(v, place, mon, null);
    }

    public ItemPickPage(GameView v, JSONObject place, JSONObject mon, Page back) {
        super(v);
        this.place = place;
        this.mon = mon;
        this.back = back;
    }

    @Override
    public void back() {
        v.showOver(back);
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
                if (it != null && (use ? UseFlow.kind(art, it.optInt(0)) != UseFlow.NONE : art.item(it.optInt(0)).optInt("hold") == 1)) {
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
        List<int[]> l = items();
        if (l.isEmpty()) {
            art.window(c, 0, 12, 30, w - 24, 16);
            art.text(c, use ? "Nothing in the BAG can be used on it." : "Nothing in the BAG can be held.", 14, 30);
        }
        for (int i = 0; i < l.size(); i++) {
            int y = 26 + i * ROW - scroll;
            if (y < 20 - ROW || y > h) {
                continue;
            }
            art.window(c, 0, 12, y + 4, w - 24, ROW - 8);
            art.itemIcon(c, l.get(i)[0], 10, y);
            art.text(c, art.name("items", l.get(i)[0]), 36, y + 4);
            String n = "x" + l.get(i)[1];
            art.text(c, n, w - 16 - art.width(n), y + 4);
        }
        banner(c, w, (use ? "USE ON " : "GIVE TO ") + art.plain(GameArt.hexText(mon.optString("n"))), "BACK");
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20) {
            if (x > v.width() - 60) {
                back();
            }
            return;
        }
        List<int[]> l = items();
        int i = (y + scroll - 26) / ROW;
        if (y + scroll >= 26 && i >= 0 && i < l.size() && v.app.locked()) {
            v.message(Companion.LOCKED);
            v.showOver(null);
            return;
        }
        if (use && y + scroll >= 26 && i >= 0 && i < l.size()) {
            UseFlow.use(v, place, mon, l.get(i)[0], back);
            return;
        }
        if (y + scroll >= 26 && i >= 0 && i < l.size() && v.app.client != null) {
            v.app.client.order(Model.item(true, place, mon, l.get(i)[0]));
            v.buzz(false);
            v.showOver(back);
            v.app.changed();
        }
    }
}
