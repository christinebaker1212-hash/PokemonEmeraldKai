package com.moemon.companion;

import com.moemon.companion.core.Model;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * USE: an item on a party POKéMON, as the BAG's USE does it in the game (the game carries it out with its own effects
 * and rules: mp_phone.c DoUse). The app only asks what the game would ask: which move for a PP item, which move to
 * forget for a TM when it knows four.
 */
public final class UseFlow {
    // the item's use on a POKéMON (data.json's "use": mp_phone.c / party_menu.c MP_PhoneItemUse)
    public static final int NONE = 0, PP_ONE = 5, PP_UP = 6, TM = 8;

    private UseFlow() {}

    public static int kind(GameArt art, int item) {
        return art.item(item).optInt("use");
    }

    /** Start using item on the party POKéMON at place (back: the page to show after) */
    public static void use(GameView v, JSONObject place, JSONObject mon, int item, Page back) {
        if (v.app.client == null) {
            return;
        }
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        if (!place.has("party")) {
            v.message("Items are used on the POKéMON in your party, as in the game.");
            return;
        }
        GameArt art = v.art;
        int kind = kind(art, item);
        JSONArray moves = mon.optJSONArray("m");
        int known = 0;
        for (int i = 0; moves != null && i < 4; i++) {
            known += moves.optInt(i) != 0 ? 1 : 0;
        }
        if (kind == PP_ONE || kind == PP_UP || (kind == TM && known == 4)) {
            Menu m = Menu.make(v, kind == TM ? "FORGET WHICH MOVE?" : "WHICH MOVE?");
            for (int i = 0; moves != null && i < 4; i++) {
                if (moves.optInt(i) == 0) {
                    continue;
                }
                final int slot = i;
                m.add(art.plain(art.name("moves", moves.optInt(i))), () -> send(v, place, mon, item, slot, back));
            }
            m.add("CANCEL", null);
            v.showMenu(m);
            return;
        }
        send(v, place, mon, item, -1, back);
    }

    private static void send(GameView v, JSONObject place, JSONObject mon, int item, int move, Page back) {
        JSONObject o = Model.item(true, place, mon, item);
        try {
            o.put("op", "use");
            if (move >= 0) {
                o.put("move", move);
            }
        } catch (JSONException ignored) {
        }
        v.app.client.order(o);
        v.buzz(false);
        v.showOver(back);
        v.app.changed();
    }

    /** What came of a USE, in the game's way of saying it */
    public static String result(GameArt art, JSONObject order, JSONObject done, Model m) {
        String name = "It";
        JSONObject at = order.optJSONObject("at");
        if (m != null && at != null) {
            JSONObject mon = m.at(at);
            if (mon != null) {
                name = art.plain(GameArt.hexText(mon.optString("n")));
            }
        }
        StringBuilder s = new StringBuilder();
        JSONArray learned = done.optJSONArray("learned"), wants = done.optJSONArray("wants");
        for (int i = 0; learned != null && i < learned.length(); i++) {
            s.append(name).append(" learned ").append(art.plain(art.name("moves", learned.optInt(i)))).append("! ");
        }
        for (int i = 0; wants != null && i < wants.length(); i++) {
            s.append(name).append(" wants to learn ").append(art.plain(art.name("moves", wants.optInt(i))))
                    .append(". Teach it in the game. ");
        }
        if (done.optInt("evolve") == 1) {
            s.append("What? ").append(name).append(" is evolving! Watch the game!");
        }
        if (s.length() == 0) {
            s.append("Used ").append(art.plain(art.name("items", order.optInt("item")))).append(" on ").append(name).append(".");
        }
        return s.toString().trim();
    }
}
