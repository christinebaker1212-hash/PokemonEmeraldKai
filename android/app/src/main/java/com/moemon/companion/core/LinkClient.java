package com.moemon.companion.core;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Keeps one paired save's link up (docs/COMPANION_APP.md "Linking", "Sync model"): tries where the game was last found,
 * else asks the local network; asks for a snapshot, sends the waiting orders again (the game answers each ID once),
 * pings, and asks for a new snapshot whenever the game's "rev" isn't the model's. Away when the game can't be reached:
 * the model stays as last seen and new orders wait. Listener calls come on the link's own thread.
 */
public final class LinkClient {
    public enum Status { LINKING, LINKED, AWAY, REFUSED }

    public interface Listener {
        void onStatus(Status status, String why);
        void onModel(Model model);                          // a new snapshot, or an answer applied
        void onAnswer(JSONObject order, JSONObject done);   // the game did, or couldn't do, an order
        void onMessage(JSONObject message);                 // anything else: "spawnmaps", "spawns", "queued", "state"
    }

    private static final int PING_MS = 2500;

    private final Crypto crypto;
    private final Store store;
    private final SavedLink saved;
    private final Listener listener;
    public final Orders orders;
    private volatile Model model;
    private volatile Link link;
    private volatile Status status = Status.AWAY;
    private final AtomicBoolean running = new AtomicBoolean();
    private Thread thread;
    private volatile boolean snapAsked;

    public LinkClient(Crypto crypto, Store store, SavedLink saved, Listener listener) {
        this.crypto = crypto;
        this.store = store;
        this.saved = saved;
        this.listener = listener;
        this.orders = new Orders(store, saved.id());
        this.model = Model.fromFile(store.read(modelFile()));
    }

    private String modelFile() {
        return "model_" + saved.id() + ".json";
    }

    public Model model() {
        return model;
    }

    public Status status() {
        return status;
    }

    public SavedLink saved() {
        return saved;
    }

    public void start() {
        if (running.compareAndSet(false, true)) {
            thread = new Thread(this::run, "moemon-link");
            thread.setDaemon(true);
            thread.start();
        }
    }

    public void stop() {
        running.set(false);
        Link l = link;
        if (l != null) {
            l.close();
        }
        if (thread != null) {
            thread.interrupt();
        }
    }

    /** Try again now (pull down to resync) */
    public void kick() {
        Link l = link;
        if (l != null) {
            ask("snap");
        } else if (thread != null) {
            thread.interrupt();
        }
    }

    /** A new order: kept until answered, sent now if linked */
    public JSONObject order(JSONObject o) {
        JSONObject sent = orders.add(o);
        send(sent);
        return sent;
    }

    /** A request with no lasting effect ("spawnmaps", {"t":"spawns","map":[g,m]}) */
    public boolean ask(String t) {
        try {
            return send(new JSONObject().put("t", t));
        } catch (JSONException e) {
            return false;
        }
    }

    public boolean send(JSONObject o) {
        Link l = link;
        if (l == null) {
            return false;
        }
        try {
            l.send(o);
            return true;
        } catch (IOException e) {
            l.close();
            return false;
        }
    }

    private void setStatus(Status s, String why) {
        status = s;
        listener.onStatus(s, why);
    }

    private void saveLinks() {
        store.write("link_" + saved.id() + ".json", toJson(saved));
    }

    static String toJson(SavedLink s) {
        try {
            return s.toJson().toString();
        } catch (JSONException e) {
            return "{}";
        }
    }

    /** Where the game is: where it was, else whoever answers on the local network with this save */
    private Link reach() {
        if (saved.host != null && !saved.host.isEmpty()) {
            try {
                return Link.connect(crypto, saved, saved.host, saved.port);
            } catch (Link.Refused r) {
                if (!r.why.equals("othersave")) {   // (another save there now: look for this one)
                    throw new RefusedError(r.why);
                }
            } catch (IOException ignored) {
            }
        }
        List<Link.Found> found = Link.discover(1500);
        for (Link.Found f : found) {
            if (java.util.Arrays.equals(f.saveId, saved.saveId)) {
                if (f.version != Link.VERSION) {
                    throw new RefusedError("version");
                }
                try {
                    Link l = Link.connect(crypto, saved, f.host, f.port);
                    saved.host = f.host;
                    saved.port = f.port;
                    saveLinks();
                    return l;
                } catch (Link.Refused r) {
                    throw new RefusedError(r.why);
                } catch (IOException ignored) {
                }
            }
        }
        return null;
    }

    private static final class RefusedError extends RuntimeException {
        final String why;

        RefusedError(String why) {
            super(why);
            this.why = why;
        }
    }

    private void run() {
        int wait = 1000;
        while (running.get()) {
            setStatus(Status.LINKING, "");
            Link l;
            try {
                l = reach();
            } catch (RefusedError e) {
                setStatus(Status.REFUSED, e.why);
                l = null;
                wait = 30000;
            }
            if (l == null) {
                if (status != Status.REFUSED) {
                    setStatus(Status.AWAY, "");
                }
                try {
                    Thread.sleep(wait);
                } catch (InterruptedException ignored) {
                }
                wait = Math.min(wait * 2, 15000);
                continue;
            }
            wait = 1000;
            link = l;
            setStatus(Status.LINKED, "");
            final Link linked = l;
            Thread pinger = new Thread(() -> ping(linked), "moemon-ping");
            pinger.setDaemon(true);
            try {
                snapAsked = true;
                l.send(new JSONObject().put("t", "snap"));
                for (JSONObject o : orders.pending()) {
                    l.send(o);
                }
                pinger.start();
                while (running.get()) {
                    receive(l.recv());
                }
            } catch (Link.Refused r) {
                setStatus(Status.REFUSED, r.why);
            } catch (IOException | JSONException ignored) {
            } finally {
                link = null;
                l.close();
                pinger.interrupt();
            }
            if (running.get() && status != Status.REFUSED) {
                setStatus(Status.AWAY, "");
            }
        }
    }

    private void ping(Link l) {
        try {
            while (!Thread.currentThread().isInterrupted()) {
                Thread.sleep(PING_MS);
                l.send(new JSONObject().put("t", "ping"));
            }
        } catch (InterruptedException | IOException | JSONException ignored) {
        }
    }

    private void receive(JSONObject o) {
        String t = o.optString("t");
        Model m = model;
        switch (t) {
            case "snap": {
                snapAsked = false;
                Model n = Model.fromSnapshot(o, System.currentTimeMillis());
                if (n.saveId().equals(saved.id())) {
                    model = n;
                    saved.name = n.playerName();
                    saveLinks();
                    store.write(modelFile(), n.toFile());
                    listener.onModel(n);
                }
                break;
            }
            case "done": {
                JSONObject order = orders.answered(o);
                if (m != null && o.optInt("ok") == 1) {
                    m.apply(o);
                    m.asOf = System.currentTimeMillis();
                    store.write(modelFile(), m.toFile());
                    listener.onModel(m);
                }
                listener.onAnswer(order, o);
                break;
            }
            case "state": {
                if (m != null) {
                    m.setPlace(o.optJSONObject("place"));
                }
                if ((m == null || o.optLong("rev") != m.rev) && !snapAsked && orders.pending().isEmpty()) {
                    snapAsked = true;
                    ask("snap");
                }
                listener.onMessage(o);
                break;
            }
            default:
                listener.onMessage(o);
        }
    }
}
