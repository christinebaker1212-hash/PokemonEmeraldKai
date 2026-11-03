package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * Orders for the game (docs/PHONE_LINK.md "Snapshot and orders"): kept in order and on disk until the game answers,
 * sent again after every reconnect (the game answers an ID once and never does it twice). Ones the game couldn't do
 * are kept for the "Couldn't do" list.
 */
public final class Orders {
    private final Store store;
    private final String file;
    private final List<JSONObject> pending = new ArrayList<>();
    private final List<JSONObject> failed = new ArrayList<>();
    private int nextId = 1;

    public Orders(Store store, String saveId) {
        this.store = store;
        this.file = "orders_" + saveId + ".json";
        try {
            String t = store.read(file);
            if (t != null) {
                JSONObject o = new JSONObject(t);
                nextId = o.optInt("next", 1);
                JSONArray p = o.optJSONArray("pending"), f = o.optJSONArray("failed");
                for (int i = 0; p != null && i < p.length(); i++) {
                    pending.add(p.getJSONObject(i));
                }
                for (int i = 0; f != null && i < f.length(); i++) {
                    failed.add(f.getJSONObject(i));
                }
            }
        } catch (JSONException ignored) {
        }
    }

    private void save() {
        try {
            JSONObject o = new JSONObject();
            o.put("next", nextId);
            o.put("pending", new JSONArray(pending));
            o.put("failed", new JSONArray(failed));
            store.write(file, o.toString());
        } catch (JSONException ignored) {
        }
    }

    /** A new order (its ID set here); the caller sends it if linked */
    public synchronized JSONObject add(JSONObject order) {
        try {
            order.put("t", "order");
            order.put("id", nextId++);
        } catch (JSONException ignored) {
        }
        pending.add(order);
        save();
        return order;
    }

    /**
     * The waiting orders to send again on a new link. One sent this often with no answer (the link closing each
     * time: the game closing as it did it?) goes to "Couldn't do" instead, so it can't close the game on every link.
     */
    public synchronized List<JSONObject> toResend() {
        List<JSONObject> out = new ArrayList<>();
        for (JSONObject o : new ArrayList<>(pending)) {
            int tries = o.optInt("tries") + 1;
            try {
                o.put("tries", tries);
            } catch (JSONException ignored) {
            }
            if (tries > 3) {
                pending.remove(o);
                try {
                    o.put("why", "crashed");
                } catch (JSONException ignored) {
                }
                failed.add(o);
            } else {
                out.add(o);
            }
        }
        save();
        return out;
    }

    public synchronized List<JSONObject> pending() {
        return new ArrayList<>(pending);
    }

    public synchronized List<JSONObject> failed() {
        return new ArrayList<>(failed);
    }

    public synchronized void clearFailed() {
        failed.clear();
        save();
    }

    /** The game's answer: the order leaves the waiting list (into "Couldn't do" when it wasn't done) */
    public synchronized JSONObject answered(JSONObject done) {
        int id = done.optInt("id");
        for (int i = 0; i < pending.size(); i++) {
            if (pending.get(i).optInt("id") == id) {
                JSONObject o = pending.remove(i);
                if (done.optInt("ok") == 0) {
                    try {
                        o.put("why", done.optString("why"));
                    } catch (JSONException ignored) {
                    }
                    failed.add(o);
                    while (failed.size() > 50) {
                        failed.remove(0);
                    }
                }
                save();
                return o;
            }
        }
        return null;
    }
}
