package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * The boxes and party as they'll be once the waiting orders are done (docs/COMPANION_APP.md: pending orders show with
 * a dashed outline): the model with each waiting move played the way the game plays it (DoMove in mp_phone.c: a swap
 * when the place is taken, a party that closes up), and the places they touch. The game still checks each order; an
 * order it refuses simply stops showing.
 */
public final class Planned {
    public final List<JSONObject> party = new ArrayList<>();
    public final List<List<JSONObject>> boxes = new ArrayList<>();
    private final Set<String> dashed = new HashSet<>();
    private final Set<String> heldItemChanges = new HashSet<>();

    public Planned(Model m, List<JSONObject> pending) {
        if (m == null) {
            return;
        }
        JSONArray p = m.party();
        for (int i = 0; p != null && i < p.length(); i++) {
            party.add(p.optJSONObject(i));
        }
        JSONArray bs = m.boxes();
        for (int b = 0; bs != null && b < bs.length(); b++) {
            JSONArray slots = bs.optJSONObject(b).optJSONArray("m");
            List<JSONObject> l = new ArrayList<>();
            for (int s = 0; slots != null && s < slots.length(); s++) {
                l.add(slots.optJSONObject(s));
            }
            boxes.add(l);
        }
        for (JSONObject o : pending) {
            play(o);
        }
    }

    public static String key(JSONObject place) {
        return place.has("party") ? "p" + place.optInt("party") : place.optInt("box") + "/" + place.optInt("slot");
    }

    public boolean isDashed(JSONObject place) {
        return dashed.contains(key(place));
    }

    public boolean itemPending(JSONObject place) {
        return heldItemChanges.contains(key(place));
    }

    public JSONObject at(JSONObject place) {
        if (place.has("party")) {
            int i = place.optInt("party");
            return i < party.size() ? party.get(i) : null;
        }
        int b = place.optInt("box"), s = place.optInt("slot");
        return b < boxes.size() && s < boxes.get(b).size() ? boxes.get(b).get(s) : null;
    }

    private void put(JSONObject place, JSONObject mon) {
        if (place.has("party")) {
            int i = place.optInt("party");
            if (mon == null) {
                if (i < party.size()) {
                    party.remove(i);   // (the party closes up)
                }
            } else if (i < party.size()) {
                party.set(i, mon);
            } else {
                party.add(mon);
            }
        } else {
            boxes.get(place.optInt("box")).set(place.optInt("slot"), mon);
        }
    }

    private static JSONObject partyPlace(int i) {
        return Model.partyPlace(i);
    }

    private static boolean same(JSONObject mon, long p, long o) {
        return mon != null && mon.optLong("p") == p && mon.optLong("o") == o;
    }

    private void play(JSONObject o) {
        String op = o.optString("op");
        try {
            if (op.equals("move")) {
                JSONObject from = o.getJSONObject("from"), to = o.getJSONObject("to");
                JSONObject a = at(from), b = at(to);
                if (!same(a, o.optLong("p"), o.optLong("o"))) {
                    return;   // (the game will refuse it)
                }
                if (o.optLong("tp") == 0 ? b != null : !same(b, o.optLong("tp"), o.optLong("to2"))) {
                    return;
                }
                if (from.has("party") && to.has("party")) {
                    if (b == null) {
                        to = partyPlace(party.size() - 1);   // (to the end: a party has no gaps)
                        b = at(to);
                    }
                    put(to, a);
                    put(from, b);
                } else if (from.has("party") && b == null) {
                    // (out of the party into an empty box place: the party closes up after)
                    put(to, a);
                    put(from, null);
                } else if (to.has("party") && b == null) {
                    if (party.size() >= 6) {
                        return;
                    }
                    to = partyPlace(party.size());
                    put(from, null);
                    put(to, a);
                } else {
                    put(from, b);
                    put(to, a);
                }
                dashed.add(key(from));
                dashed.add(key(to));
            } else if (op.equals("give") || op.equals("take")) {
                JSONObject at = o.getJSONObject("at");
                JSONObject mon = at(at);
                if (!same(mon, o.optLong("p"), o.optLong("o"))) {
                    return;
                }
                JSONObject copy = new JSONObject(mon.toString());
                copy.put("i", op.equals("give") ? o.optInt("item") : 0);
                put(at, copy);
                heldItemChanges.add(key(at));
            }
        } catch (Exception ignored) {
        }
    }
}
