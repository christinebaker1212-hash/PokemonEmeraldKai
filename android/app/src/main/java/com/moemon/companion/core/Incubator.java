package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.time.LocalDate;
import java.time.ZoneId;
import java.util.List;

/**
 * The EGG INCUBATOR (docs/PHONE_LINK.md "The EGG INCUBATOR"; the game's side mp_phone_walk.c): up to 12 EGGS out of
 * the save on this phone, each counting the steps walked since it went in. Android-free, so the desktop test runs it.
 * The steps are kept as the walk keeps them (a Walk used only as a step ledger: readings spread over the days between
 * them, at most 10,000 a day); each EGG remembers the ledger's total when it went in, so its steps are the total since.
 * An EGG is ready when its steps cover its cycles (its Friendship, 32 steps each); "hatchhome" brings any of them back
 * to a box the player chose, and the game checks the steps against its own clock.
 */
public final class Incubator {
    public JSONObject game = new JSONObject();      // the game's "incubator": max, cycle, day, now, eggs [{id, sent, mon}]
    private Walk ledger;                             // (steps since the first EGG went in)
    private JSONObject offsets = new JSONObject();   // EGG id -> the ledger's total when it went in
    private final ZoneId zone;

    public Incubator(ZoneId zone) {
        this.zone = zone;
    }

    public int max() {
        return game.optInt("max", 12);
    }

    public int stepsACycle() {
        return game.optInt("cycle", 32);
    }

    public JSONArray eggs() {
        JSONArray e = game.optJSONArray("eggs");
        return e == null ? new JSONArray() : e;
    }

    public int count() {
        return eggs().length();
    }

    public boolean empty() {
        return count() == 0;
    }

    private long total() {
        return ledger == null ? 0 : ledger.steps();
    }

    /** The game's list (a snapshot's or an answer's): new EGGS count from now, ones gone home are forgotten */
    public void fromGame(JSONObject g) {
        if (g == null) {
            return;
        }
        game = g;
        JSONArray e = eggs();
        try {
            if (e.length() == 0) {
                ledger = null;
                offsets = new JSONObject();
                return;
            }
            if (ledger == null) {
                long sent = Long.MAX_VALUE;
                for (int i = 0; i < e.length(); i++) {
                    sent = Math.min(sent, e.optJSONObject(i).optLong("sent"));
                }
                ledger = new Walk(new JSONObject().put("sent", sent)
                        .put("rules", new JSONArray().put(g.optInt("day", 10000))), zone);
            }
            JSONObject keep = new JSONObject();
            for (int i = 0; i < e.length(); i++) {
                String id = String.valueOf(e.optJSONObject(i).optLong("id"));
                keep.put(id, offsets.has(id) ? offsets.optLong(id) : total());
            }
            offsets = keep;
        } catch (JSONException ignored) {
        }
    }

    /** A step counter reading (as Walk.reading) */
    public void reading(long counter, long boot, long now) {
        if (ledger != null) {
            ledger.reading(counter, boot, now);
        }
    }

    /** Steps added to a day by hand (the tests' simulated walks) */
    public void add(LocalDate day, long steps) {
        if (ledger != null) {
            ledger.add(day, steps);
        }
    }

    public long steps(JSONObject egg) {
        String id = String.valueOf(egg.optLong("id"));
        return Math.max(0, total() - offsets.optLong(id, total()));
    }

    /** The steps it needs to hatch: its cycles (the EGG's Friendship) of 32 */
    public long need(JSONObject egg) {
        JSONObject mon = egg.optJSONObject("mon");
        return (long) (mon == null ? 0 : mon.optInt("f")) * stepsACycle();
    }

    public boolean ready(JSONObject egg) {
        return steps(egg) >= need(egg);
    }

    public int readyCount() {
        int n = 0;
        JSONArray e = eggs();
        for (int i = 0; i < e.length(); i++) {
            n += ready(e.optJSONObject(i)) ? 1 : 0;
        }
        return n;
    }

    public JSONObject egg(long id) {
        JSONArray e = eggs();
        for (int i = 0; i < e.length(); i++) {
            if (e.optJSONObject(i).optLong("id") == id) {
                return e.optJSONObject(i);
            }
        }
        return null;
    }

    /** The homecoming order for these EGGS to BOX (the game checks the box has room and every step) */
    public JSONObject homeOrder(List<Long> ids, int box) throws JSONException {
        JSONArray list = new JSONArray();
        for (long id : ids) {
            JSONObject e = egg(id);
            if (e != null) {
                list.put(new JSONArray().put(id).put(steps(e)));
            }
        }
        return new JSONObject().put("op", "hatchhome").put("box", box).put("eggs", list);
    }

    // ------------------------------------------------------------------------------------------------ disk

    public JSONObject toJson() throws JSONException {
        JSONObject o = new JSONObject().put("game", game).put("offsets", offsets);
        if (ledger != null) {
            o.put("ledger", ledger.toJson());
        }
        return o;
    }

    public static Incubator fromJson(JSONObject o, ZoneId zone) {
        Incubator inc = new Incubator(zone);
        if (o == null) {
            return inc;
        }
        inc.game = o.optJSONObject("game") != null ? o.optJSONObject("game") : new JSONObject();
        inc.offsets = o.optJSONObject("offsets") != null ? o.optJSONObject("offsets") : new JSONObject();
        if (o.optJSONObject("ledger") != null) {
            inc.ledger = Walk.fromJson(o.optJSONObject("ledger"), zone);
        }
        return inc;
    }
}
