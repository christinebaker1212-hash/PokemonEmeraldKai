package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.DashPathEffect;
import android.graphics.Paint;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Planned;

import org.json.JSONObject;

/** The party, like the game's POKéMON screen: tap one for its details, GIVE and TAKE */
public final class PartyPage extends Page {
    private static final int ROW = 46;
    private int open = -1;
    private final Paint dash = new Paint();

    public PartyPage(GameView v) {
        super(v);
        dash.setStyle(Paint.Style.STROKE);
        dash.setColor(0xFF2848D8);
        dash.setPathEffect(new DashPathEffect(new float[]{3, 2}, 0));
    }

    private Planned planned() {
        return new Planned(v.app.model(), v.app.client != null ? v.app.client.orders.pending()
                : java.util.Collections.<JSONObject>emptyList());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        Model m = v.app.model();
        if (m == null) {
            art.window(c, 0, 12, 20, w - 24, 16);
            art.text(c, "Nothing from the game yet.", 18, 20);
            return;
        }
        Planned p = planned();
        if (open >= 0 && open < p.party.size()) {
            JSONObject mon = p.party.get(open);
            MonCard.draw(c, art, mon, 10, 10, w - 20);
            int by = 10 + MonCard.HEIGHT + 10, bw = (w - 20) / 3;
            boolean locked = v.app.locked();
            button(c, "GIVE", 10, by, bw, !locked && mon.optInt("e") == 0);
            button(c, "TAKE", 10 + bw, by, bw, !locked && mon.optInt("i") != 0);
            if (locked) {
                int ty = by + 26;
                for (String line : wrap(Companion.LOCKED, w - 24)) {
                    art.text(c, line, 12, ty, 0xFFF8A050, GameArt.WHITE_SHADOW);
                    ty += 16;
                }
            }
            button(c, "BACK", 10 + 2 * bw, by, bw, true);
            return;
        }
        for (int i = 0; i < 6; i++) {
            int y = 6 + i * ROW;
            JSONObject mon = i < p.party.size() ? p.party.get(i) : null;
            art.window(c, 0, 12, y + 4, w - 24, ROW - 10);
            if (mon == null) {
                continue;
            }
            boolean egg = mon.optInt("e") == 1;
            art.icon(c, egg ? MonCard.eggSpecies(art) : mon.optInt("s"), 8, y + 2, GameArt.ICON_PLAIN);
            art.textHex(c, mon.optString("n"), 42, y + 2, GameArt.TEXT, GameArt.SHADOW);
            if (!egg) {
                art.text(c, "Lv" + mon.optInt("l"), 42, y + 18);
                MonCard.hp(c, art, w - 130, y + 24, mon.optInt("hp"), mon.optInt("mhp"));
            }
            if (mon.optInt("i") != 0) {
                art.itemIcon(c, mon.optInt("i"), w - 40, y + 2);
            }
            if (p.isDashed(Model.partyPlace(i)) || p.itemPending(Model.partyPlace(i))) {
                c.drawRect(13, y + 5, w - 13, y + ROW - 7, dash);
            }
        }
    }

    @Override
    public void tap(int x, int y) {
        Planned p = planned();
        if (open >= 0) {
            if (open >= p.party.size()) {
                open = -1;
                return;
            }
            JSONObject mon = p.party.get(open);
            int by = 10 + MonCard.HEIGHT + 10, bw = (v.width() - 20) / 3;
            if (y >= by && y < by + 24) {
                int k = (x - 10) / bw;
                JSONObject place = Model.partyPlace(open);
                if (k < 2 && v.app.locked()) {
                    v.message(Companion.LOCKED);
                    return;
                }
                if (k == 0 && mon.optInt("e") == 0) {
                    v.showOver(new ItemPickPage(v, place, mon));
                } else if (k == 1 && mon.optInt("i") != 0 && v.app.client != null) {
                    v.app.client.order(Model.item(false, place, mon, 0));
                    v.buzz(false);
                } else if (k == 2) {
                    open = -1;
                }
            }
            return;
        }
        int i = (y - 6) / ROW;
        if (i >= 0 && i < p.party.size()) {
            open = i;
        }
    }

    @Override
    public void back() {
        open = -1;
    }

    @Override
    public void shown() {
        open = -1;
    }
}
