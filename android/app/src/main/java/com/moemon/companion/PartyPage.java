package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.DashPathEffect;
import android.graphics.Paint;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Planned;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * The party like the game's POKéMON screen: each one's icon, name, level, HP and condition, held item; tap one for
 * its SUMMARY (GIVE and TAKE there).
 */
public final class PartyPage extends Page {
    private static final int ROW = 50, TOP = 24;
    private final Paint dash = new Paint(), fill = new Paint();

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

    /** The game's status conditions (STATUS1: sleep turns 0-7, then POISON, BURN, FREEZE, PARALYSIS, TOXIC) */
    static String status(JSONObject mon) {
        int st = mon.optInt("st");
        if (mon.has("hp") && mon.optInt("hp") == 0) {
            return "FNT";
        }
        if ((st & 7) != 0) {
            return "SLP";
        }
        if ((st & (1 << 3)) != 0 || (st & (1 << 7)) != 0) {
            return "PSN";
        }
        if ((st & (1 << 4)) != 0) {
            return "BRN";
        }
        if ((st & (1 << 5)) != 0) {
            return "FRZ";
        }
        if ((st & (1 << 6)) != 0) {
            return "PAR";
        }
        return null;
    }

    static int statusColor(String s) {
        switch (s) {
            case "FNT": return 0xFF606060;
            case "SLP": return 0xFF8870A8;
            case "PSN": return 0xFFB048B0;
            case "BRN": return 0xFFE05830;
            case "FRZ": return 0xFF50A8D8;
            default: return 0xFFC8A800;
        }
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        Model m = v.app.model();
        banner(c, w, "PARTY", m == null ? "" : m.pcReachable() ? "PC IN REACH" : "NO PC HERE");
        if (m == null) {
            art.window(c, 0, 12, TOP + 8, w - 24, 16);
            art.text(c, "Nothing from the game yet.", 18, TOP + 8);
            return;
        }
        Planned p = planned();
        for (int i = 0; i < 6; i++) {
            int y = TOP + i * ROW;
            JSONObject mon = i < p.party.size() ? p.party.get(i) : null;
            // (the lead in the game's highlight, the rest in white)
            fill.setColor(mon == null ? 0x40FFFFFF : i == 0 ? 0xFFF8E8B0 : 0xFFF8F8F8);
            c.drawRect(10, y + 4, w - 10, y + ROW - 2, fill);
            fill.setColor(0xFF384868);
            c.drawRect(10, y + ROW - 3, w - 10, y + ROW - 2, fill);
            if (mon == null) {
                continue;
            }
            boolean egg = mon.optInt("e") == 1;
            art.icon(c, egg ? MonCard.eggSpecies(art) : mon.optInt("s"), 12, y + 8, GameArt.ICON_PLAIN);
            int nw = art.textHex(c, mon.optString("n"), 48, y + 4, GameArt.TEXT, GameArt.SHADOW);
            if (!egg) {
                int g = mon.optInt("g");
                if (g == 0) {
                    art.text(c, "♂", 48 + nw + 3, y + 4, GameArt.BLUE_TEXT, GameArt.SHADOW);
                } else if (g == 254) {
                    art.text(c, "♀", 48 + nw + 3, y + 4, GameArt.RED_TEXT, GameArt.SHADOW);
                }
                art.text(c, "Lv" + mon.optInt("l"), 48, y + 22);
                String s = status(mon);
                if (s != null) {
                    chip(c, s, 84, y + 28, statusColor(s));
                }
                MonCard.hp(c, art, w - 132, y + 30, mon.optInt("hp"), mon.optInt("mhp"));
            }
            if (mon.optInt("i") != 0) {
                art.itemIcon(c, mon.optInt("i"), w - 40, y + 4);
            }
            if (p.isDashed(Model.partyPlace(i)) || p.itemPending(Model.partyPlace(i))) {
                c.drawRect(11, y + 5, w - 11, y + ROW - 4, dash);
            }
        }
        int y = TOP + 6 * ROW + 4;
        art.text(c, "Tap a POKéMON for its SUMMARY.", 12, y, GameArt.WHITE_TEXT, 0xFF305060);
    }

    @Override
    public void tap(int x, int y) {
        Planned p = planned();
        int i = (y - TOP) / ROW;
        if (y >= TOP && i >= 0 && i < p.party.size()) {
            List<JSONObject> places = new ArrayList<>();
            for (int k = 0; k < p.party.size(); k++) {
                places.add(Model.partyPlace(k));
            }
            v.showOver(new SummaryPage(v, places, i, null));
        }
    }
}
