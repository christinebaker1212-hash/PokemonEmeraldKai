package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * The game as the app last saw it: the snapshot (docs/PHONE_LINK.md), with the places each answer changed applied.
 * Kept on disk so the app opens on it at once, "as of" its time, away from the game.
 */
public final class Model {
    public JSONObject snap;          // the whole snapshot
    public long asOf;                // when it came (ms)
    public long rev;                 // the game's hash of it ("rev"): a new snapshot is asked for when it differs

    public static Model fromSnapshot(JSONObject snap, long now) {
        Model m = new Model();
        m.snap = snap;
        m.asOf = now;
        m.rev = snap.optLong("rev");
        return m;
    }

    public JSONObject save() {
        JSONObject s = snap.optJSONObject("save");
        return s != null ? s : new JSONObject();
    }

    public String saveId() {
        return save().optString("id");
    }

    /** The player's name, as the game's text bytes in hex */
    public String playerName() {
        return save().optString("name");
    }

    public void setPlace(JSONObject place) {
        if (place != null) {
            try {
                snap.put("place", place);
            } catch (JSONException ignored) {
            }
        }
    }

    public JSONArray party() {
        return snap.optJSONArray("party");
    }

    public JSONArray boxes() {
        return snap.optJSONArray("boxes");
    }

    public JSONObject box(int b) {
        return boxes().optJSONObject(b);
    }

    /** The POKeMON at a place, null for none */
    public JSONObject at(JSONObject place) {
        if (place.has("party")) {
            return party().optJSONObject(place.optInt("party"));
        }
        JSONArray m = box(place.optInt("box")).optJSONArray("m");
        return m.optJSONObject(place.optInt("slot"));
    }

    public JSONObject place() {
        return snap.optJSONObject("place");
    }

    /** The game is in a battle: the PC, moves and items wait (the app doesn't offer them) */
    public boolean inBattle() {
        return place() != null && place().optInt("battle") == 1;
    }

    public boolean pcReachable() {
        return place() != null && place().optInt("pc") == 1;
    }

    /** An answer's changed places (and the BAG) */
    public void apply(JSONObject done) {
        try {
            JSONArray ch = done.optJSONArray("changed");
            for (int i = 0; ch != null && i < ch.length(); i++) {
                JSONObject c = ch.getJSONObject(i);
                if (c.has("party")) {
                    snap.put("party", c.getJSONArray("party"));
                } else {
                    Object mon = c.opt("mon");
                    box(c.getInt("box")).getJSONArray("m").put(c.getInt("slot"), mon == null ? 0 : mon);
                }
            }
            if (done.has("bag")) {
                snap.put("bag", done.getJSONArray("bag"));
            }
            if (done.has("rev")) {
                rev = done.getLong("rev");
                snap.put("rev", rev);
            }
        } catch (JSONException ignored) {
        }
    }

    public String toFile() {
        try {
            JSONObject o = new JSONObject();
            o.put("asOf", asOf);
            o.put("snap", snap);
            return o.toString();
        } catch (JSONException e) {
            return null;
        }
    }

    public static Model fromFile(String text) {
        if (text == null) {
            return null;
        }
        try {
            JSONObject o = new JSONObject(text);
            return fromSnapshot(o.getJSONObject("snap"), o.optLong("asOf"));
        } catch (JSONException e) {
            return null;
        }
    }

    /** A move order from one place to another, with the POKeMON the app expects at each */
    public static JSONObject move(JSONObject from, JSONObject fromMon, JSONObject to, JSONObject toMon) {
        JSONObject o = new JSONObject();
        try {
            o.put("op", "move");
            o.put("from", from);
            o.put("to", to);
            o.put("p", fromMon.optLong("p"));
            o.put("o", fromMon.optLong("o"));
            o.put("tp", toMon == null ? 0 : toMon.optLong("p"));
            o.put("to2", toMon == null ? 0 : toMon.optLong("o"));
        } catch (JSONException ignored) {
        }
        return o;
    }

    public static JSONObject item(boolean give, JSONObject at, JSONObject mon, int item) {
        JSONObject o = new JSONObject();
        try {
            o.put("op", give ? "give" : "take");
            o.put("at", at);
            o.put("p", mon.optLong("p"));
            o.put("o", mon.optLong("o"));
            if (give) {
                o.put("item", item);
            }
        } catch (JSONException ignored) {
        }
        return o;
    }

    /** The POKé BALL page: its ball (another kind costs one from the BAG), color and decoration */
    public static JSONObject ballStyle(JSONObject at, JSONObject mon, int item, int color, int deco, int special) {
        JSONObject o = new JSONObject();
        try {
            o.put("op", "ballstyle");
            o.put("at", at);
            o.put("p", mon.optLong("p"));
            o.put("o", mon.optLong("o"));
            o.put("item", item);
            o.put("color", color);
            o.put("deco", deco);
            o.put("special", special);
        } catch (JSONException ignored) {
        }
        return o;
    }

    /** The POKé BALL page: its entrance / recall effect; at null = every POKéMON in the party; slot >= 0 = MY PRESET */
    public static JSONObject ballFx(JSONObject at, JSONObject mon, BallFx fx, int slot) {
        JSONObject o = new JSONObject();
        try {
            o.put("op", slot >= 0 ? "fxpreset" : at == null ? "fxparty" : "ballfx");
            if (at != null && slot < 0) {
                o.put("at", at);
                o.put("p", mon.optLong("p"));
                o.put("o", mon.optLong("o"));
            }
            if (slot >= 0) {
                o.put("slot", slot);
            }
            o.put("fx", fx.pack());
            o.put("fxs", fx.sound);
        } catch (JSONException ignored) {
        }
        return o;
    }

    public static JSONObject boxPlace(int box, int slot) {
        JSONObject o = new JSONObject();
        try {
            o.put("box", box);
            o.put("slot", slot);
        } catch (JSONException ignored) {
        }
        return o;
    }

    public static JSONObject partyPlace(int slot) {
        JSONObject o = new JSONObject();
        try {
            o.put("party", slot);
        } catch (JSONException ignored) {
        }
        return o;
    }
}
