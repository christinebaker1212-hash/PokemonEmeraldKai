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

    private static final int GAP = 6;

    private int cardW(int w) {
        return (w - 2 * 10 - GAP) / 2;
    }

    private int cardH(int h) {
        return Math.max(96, Math.min(132, (h - TOP - 26 - 2 * GAP) / 3));
    }

    /**
     * The six places as cards in two columns (the game's party screen): the POKéMON's picture on a stand in its type's
     * color, its name and gender, level, types, HP and condition, the item it holds; the lead's card in gold
     */
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
        int cw = cardW(w), ch = cardH(h);
        for (int i = 0; i < 6; i++) {
            int x = 10 + (i % 2) * (cw + GAP), y = TOP + 4 + (i / 2) * (ch + GAP);
            JSONObject mon = i < p.party.size() ? p.party.get(i) : null;
            boolean down = mon != null && pressed(x, y, cw, ch);
            // the card: a dark edge, a face, the lead's in gold
            fill.setColor(0xFF283848);
            c.drawRect(x + 1, y, x + cw - 1, y + ch, fill);
            c.drawRect(x, y + 1, x + cw, y + ch - 1, fill);
            fill.setColor(mon == null ? 0xFF8CB8C8 : down ? 0xFFDCE6F4 : i == 0 ? 0xFFF8ECC0 : 0xFFF8F8F8);
            c.drawRect(x + 1, y + 1, x + cw - 1, y + ch - 1, fill);
            if (mon == null) {
                String e = "EMPTY";
                art.small(c, e, x + (cw - art.smallWidth(e)) / 2, y + ch / 2 - 8, 0xFF5C8898, 0);
                continue;
            }
            boolean egg = mon.optInt("e") == 1;
            int species = mon.optInt("s");
            // the stand: an oval in its first type's color under the picture
            org.json.JSONArray types = art.species(species).optJSONArray("t");
            int tc = egg || types == null ? 0xFFB8C0C8 : art.typeColor(types.optInt(0));
            int pic = Math.min(64, ch - 46);
            int px = x + (cw - pic) / 2, py = y + 2;
            fill.setColor((tc & 0x00FFFFFF) | 0x50000000);
            c.drawOval(px + 4, py + pic - 10, px + pic - 4, py + pic + 2, fill);
            if (egg) {
                art.icon(c, MonCard.eggSpecies(art), x + (cw - 32) / 2, py + pic - 34, GameArt.ICON_PLAIN);
            } else {
                art.front(c, species, art.monPal(mon), px, py, pic, false);
            }
            if (mon.optInt("i") != 0) {
                art.itemIcon(c, mon.optInt("i"), x + cw - 25, y + 2);
            }
            if (i == 0) {
                art.small(c, "LEAD", x + 4, y, 0xFFB07818, 0);
            }
            int ty = y + pic + 2;
            int nw = art.textHex(c, mon.optString("n"), x + 5, ty, GameArt.TEXT, GameArt.SHADOW);
            if (!egg) {
                int g = mon.optInt("g");
                if (g == 0) {
                    art.text(c, "♂", x + 5 + nw + 2, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
                } else if (g == 254) {
                    art.text(c, "♀", x + 5 + nw + 2, ty, GameArt.RED_TEXT, GameArt.SHADOW);
                }
                String lv = "Lv" + mon.optInt("l");
                art.text(c, lv, x + cw - 5 - art.width(lv), ty);
                String st = status(mon);
                if (st != null) {
                    chip(c, st, x + 5, ty + 18, statusColor(st));
                } else if (types != null) {
                    art.typeIcon(c, types.optInt(0), x + 5, ty + 16);
                    if (types.optInt(1) != types.optInt(0)) {
                        art.typeIcon(c, types.optInt(1), x + 39, ty + 16);
                    }
                }
                if (mon.has("hp")) {
                    hpBar(c, x + 5, ty + 34, cw - 10, mon.optInt("hp"), mon.optInt("mhp"));
                }
            } else {
                art.small(c, "It will hatch soon.", x + 5, ty + 16, GameArt.GRAY_TEXT, 0);
            }
            if (p.isDashed(Model.partyPlace(i)) || p.itemPending(Model.partyPlace(i))) {
                c.drawRect(x + 2, y + 2, x + cw - 2, y + ch - 2, dash);
            }
        }
        int y = TOP + 4 + 3 * (ch + GAP);
        art.small(c, "Tap a POKéMON for its SUMMARY.", 12, y, GameArt.WHITE_TEXT, 0xFF305060);
    }

    /** A slim HP bar with its numbers under it, right-aligned */
    private void hpBar(Canvas c, int x, int y, int w, int hp, int max) {
        fill.setColor(0xFF404850);
        c.drawRect(x, y, x + w, y + 5, fill);
        fill.setColor(0xFFE8ECF0);
        c.drawRect(x + 1, y + 1, x + w - 1, y + 4, fill);
        int bw = max > 0 ? (w - 2) * hp / max : 0;
        fill.setColor(hp * 5 <= max ? 0xFFF85838 : hp * 2 <= max ? 0xFFF8C030 : 0xFF58D068);
        c.drawRect(x + 1, y + 1, x + 1 + bw, y + 4, fill);
        String n = hp + "/" + max;
        art.small(c, n, x + w - art.smallWidth(n), y + 2, GameArt.GRAY_TEXT, 0);
    }

    @Override
    public void tap(int x, int y) {
        Planned p = planned();
        int w = v.width(), cw = cardW(w), ch = cardH(v.contentHeight());
        int col = x < 10 + cw + GAP / 2 ? 0 : 1, row = (y - TOP - 4) / (ch + GAP);
        int i = row * 2 + col;
        if (y >= TOP + 4 && row < 3 && i >= 0 && i < p.party.size()) {
            List<JSONObject> places = new ArrayList<>();
            for (int k = 0; k < p.party.size(); k++) {
                places.add(Model.partyPlace(k));
            }
            v.showOver(new SummaryPage(v, places, i, null));
        }
    }
}
