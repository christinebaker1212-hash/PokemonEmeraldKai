package com.moemon.companion;

import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

import com.moemon.companion.core.Hex;
import com.moemon.companion.core.Link;
import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;
import com.moemon.companion.core.Pairing;
import com.moemon.companion.core.SavedLink;
import com.moemon.companion.core.Store;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * The app's state: the paired saves (a SAVE FILE picker, like the game's), the link to the one in use, what came from
 * the game (the model, spawn tables), and the last answers. Screens read it and redraw when it calls changed().
 */
public final class Companion implements LinkClient.Listener {
    private static Companion sInstance;

    public final Context context;
    public final GameArt art;
    public final Store store;
    public final NativeCrypto crypto = new NativeCrypto();
    public final List<SavedLink> saves = new ArrayList<>();
    public LinkClient client;
    public String statusWhy = "";
    public JSONObject spawnMaps;                                  // "spawnmaps"
    public final java.util.Map<String, JSONObject> spawns = new java.util.HashMap<>();   // "g,m" -> "spawns"
    public String toast = "";                                     // the last answer worth a word
    public final java.util.Map<Integer, JSONObject> where = new java.util.HashMap<>();   // species -> "where"
    public volatile JSONObject watchFrame;                        // the POKéTCH's last LCD from the game
    public volatile boolean watching;                             // the DEVICE page shows it
    public long toastAt;
    private final Handler main = new Handler(Looper.getMainLooper());
    private final List<Runnable> watchers = new ArrayList<>();
    private boolean resumed;

    public static synchronized Companion get(Context c) throws Exception {
        if (sInstance == null) {
            sInstance = new Companion(c.getApplicationContext());
        }
        return sInstance;
    }

    /** Tests only: a fresh app next time (its link stopped) */
    static synchronized void resetForTests() {
        if (sInstance != null && sInstance.client != null) {
            sInstance.client.stop();
        }
        sInstance = null;
    }

    private Companion(Context context) throws Exception {
        this.context = context;
        // (if the app ever closes on its own, why: shown the next time it opens)
        final Thread.UncaughtExceptionHandler was = Thread.getDefaultUncaughtExceptionHandler();
        final File crash = new File(context.getFilesDir(), "crash.txt");
        Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
            try (java.io.PrintWriter w = new java.io.PrintWriter(new FileOutputStream(crash))) {
                w.println(Updater.version(context) + " " + t.getName());
                e.printStackTrace(w);
            } catch (Exception ignored) {
            }
            if (was != null) {
                was.uncaughtException(t, e);
            }
        });
        this.art = new GameArt(context.getAssets());
        final File dir = context.getFilesDir();
        this.store = new Store() {
            public String read(String name) {
                File f = new File(dir, name);
                if (!f.exists()) {
                    return null;
                }
                try (FileInputStream in = new FileInputStream(f)) {
                    byte[] b = new byte[(int) f.length()];
                    int n = 0;
                    while (n < b.length) {
                        int r = in.read(b, n, b.length - n);
                        if (r < 0) {
                            break;
                        }
                        n += r;
                    }
                    return new String(b, 0, n, StandardCharsets.UTF_8);
                } catch (Exception e) {
                    return null;
                }
            }

            public void write(String name, String text) {
                // (a new file, then renamed over the old: a crash never leaves half a file)
                File tmp = new File(dir, name + ".tmp");
                try (FileOutputStream out = new FileOutputStream(tmp)) {
                    out.write(text.getBytes(StandardCharsets.UTF_8));
                    out.getFD().sync();
                } catch (Exception e) {
                    return;
                }
                tmp.renameTo(new File(dir, name));
            }
        };
        loadSaves();
    }

    // ---------------------------------------------------------------------------------------------- the saves

    private void loadSaves() {
        saves.clear();
        String current = null;
        try {
            String t = store.read("saves.json");
            if (t != null) {
                JSONObject o = new JSONObject(t);
                JSONArray a = o.optJSONArray("saves");
                for (int i = 0; a != null && i < a.length(); i++) {
                    SavedLink s = SavedLink.fromJson(a.getJSONObject(i));
                    if (s != null) {
                        saves.add(s);
                    }
                }
                current = o.optString("current", null);
            }
        } catch (Exception ignored) {
        }
        for (SavedLink s : saves) {
            if (s.id().equals(current)) {
                use(s);
            }
        }
        if (client == null && !saves.isEmpty()) {
            use(saves.get(0));
        }
    }

    public void writeSaves() {
        try {
            JSONObject o = new JSONObject();
            JSONArray a = new JSONArray();
            for (SavedLink s : saves) {
                a.put(s.toJson());
            }
            o.put("saves", a);
            if (client != null) {
                o.put("current", client.saved().id());
            }
            store.write("saves.json", o.toString());
        } catch (Exception ignored) {
        }
    }

    /** Switch to a paired save */
    public void use(SavedLink s) {
        if (client != null) {
            client.stop();
        }
        client = new LinkClient(crypto, store, s, this);
        spawnMaps = readJson("spawnmaps_" + s.id() + ".json");
        spawns.clear();
        writeSaves();
        if (resumed) {
            client.start();
        }
        changed();
    }

    public void forget(SavedLink s) {
        saves.remove(s);
        if (client != null && client.saved() == s) {
            client.stop();
            client = null;
            if (!saves.isEmpty()) {
                use(saves.get(0));
            }
        }
        writeSaves();
        changed();
    }

    /** What closed the app last time (once), or null */
    public String lastCrash() {
        File f = new File(context.getFilesDir(), "crash.txt");
        if (!f.exists()) {
            return null;
        }
        String t = store.read("crash.txt");
        f.delete();
        return t;
    }

    /** Linked to a game in a battle: nothing in the boxes, party or BAG can change from the app */
    public boolean locked() {
        Model m = model();
        return client != null && client.status() == LinkClient.Status.LINKED && m != null && m.inBattle();
    }

    public static final String LOCKED = "In a battle! The PC, the party and the BAG wait until it's over.";

    public Model model() {
        return client != null ? client.model() : null;
    }

    public void resume() {
        resumed = true;
        if (client != null) {
            client.start();
        }
    }

    public void pause() {
        resumed = false;
        if (client != null) {
            client.stop();
        }
    }

    // --------------------------------------------------------------------------------------------- pairing

    public interface Result {
        void done(String error);   // null: paired
    }

    /** Pair by the QR code's text, or by the 6 letters with the games found on this network; off the main thread */
    public void pair(final String qrText, final String code, final Result result) {
        new Thread(() -> {
            String error = null;
            try {
                SavedLink got = null;
                String name = Build.MODEL == null ? "PHONE" : Build.MODEL;
                if (qrText != null) {
                    Pairing p = Pairing.parse(qrText);
                    if (p == null) {
                        throw new Exception("That isn't the game's code.");
                    }
                    if (p.version != Link.VERSION) {
                        throw new Exception("version");
                    }
                    try {
                        got = Link.pair(crypto, p.host, p.port, p.saveId, p.secretRaw, null, name);
                    } catch (java.io.IOException e) {
                        // (the address in the code may be one the phone can't reach: look on the network)
                        for (Link.Found f : Link.discover(2000)) {
                            if (java.util.Arrays.equals(f.saveId, p.saveId)) {
                                got = Link.pair(crypto, f.host, f.port, p.saveId, p.secretRaw, null, name);
                                break;
                            }
                        }
                        if (got == null) {
                            throw e;
                        }
                    }
                } else {
                    List<Link.Found> found = Link.discover(2500);
                    if (found.isEmpty()) {
                        throw new Exception("No game found on this network.");
                    }
                    for (Link.Found f : found) {
                        try {
                            got = Link.pair(crypto, f.host, f.port, f.saveId, null, code, name);
                            break;
                        } catch (Link.Refused r) {
                            error = r.why;
                        }
                    }
                    if (got == null) {
                        throw new Exception(error != null ? error : "wrongcode");
                    }
                }
                error = null;
                final SavedLink paired = got;
                main.post(() -> {
                    for (SavedLink s : new ArrayList<>(saves)) {
                        if (s.id().equals(paired.id())) {
                            saves.remove(s);
                        }
                    }
                    saves.add(0, paired);
                    use(paired);
                });
            } catch (Exception e) {
                error = e.getMessage() == null ? "error" : e.getMessage();
            }
            final String err = error;
            main.post(() -> result.done(err));
        }, "moemon-pair").start();
    }

    /** The game's reasons in the app's words */
    public static String why(String why) {
        switch (why == null ? "" : why) {
            case "version": return "The game and the app need the same update.";
            case "othersave": return "That game has another save loaded.";
            case "unknown": return "The game forgot this phone. Pair again.";
            case "notpairing": return "Open OPTIONS > CO-OP > PHONE LINK in the game.";
            case "wrongcode": return "Those letters weren't right.";
            case "gone": return "That POKéMON isn't there any more.";
            case "changed": return "That place changed in the game.";
            case "same": return "It's already there.";
            case "nopc": return "The party can't reach the PC here.";
            case "last": return "That's your last POKéMON!";
            case "full": return "There's no room.";
            case "egg": return "An EGG can't hold an item.";
            case "mail": return "MAIL can't be moved from here.";
            case "noitem": return "There's none of that item left.";
            case "cantheld": return "That item can't be held.";
            case "bagfull": return "The BAG is full.";
            case "nothing": return "It isn't holding anything.";
            case "busy": return "The game is busy. Try again.";
            default: return why == null || why.isEmpty() ? "It couldn't be done." : why;
        }
    }

    // ----------------------------------------------------------------------------------------- link events

    public void watch(Runnable r) {
        watchers.add(r);
    }

    public void unwatch(Runnable r) {
        watchers.remove(r);
    }

    public void changed() {
        if (Looper.myLooper() == Looper.getMainLooper()) {
            for (Runnable r : new ArrayList<>(watchers)) {
                r.run();
            }
        } else {
            main.post(this::changed);
        }
    }

    private JSONObject readJson(String name) {
        try {
            String t = store.read(name);
            return t == null ? null : new JSONObject(t);
        } catch (Exception e) {
            return null;
        }
    }

    @Override
    public void onStatus(LinkClient.Status status, String why) {
        statusWhy = why;
        if (status == LinkClient.Status.LINKED && client != null && spawnMaps == null) {
            client.ask("spawnmaps");
        }
        if (status == LinkClient.Status.LINKED && client != null && watching) {
            try {
                client.send(new JSONObject().put("t", "watch").put("on", 1));   // (linked again: the POKéTCH again)
            } catch (org.json.JSONException ignored) {
            }
        }
        changed();
    }

    @Override
    public void onModel(Model model) {
        changed();
    }

    @Override
    public void onAnswer(JSONObject order, JSONObject done) {
        if (done.optInt("ok") == 0) {
            toast = why(done.optString("why"));
            toastAt = System.currentTimeMillis();
        }
        changed();
    }

    @Override
    public void onMessage(JSONObject m) {
        String t = m.optString("t");
        if (client == null) {
            return;
        }
        if (t.equals("where")) {
            where.put(m.optInt("s"), m);
        } else if (t.equals("watch")) {
            watchFrame = m.optInt("none") == 1 ? null : m;
        } else if (t.equals("linkerror")) {
            String e = client.lastError == null ? "" : client.lastError;
            store.write("linkerror.txt", e);
            toast = "LINK ERROR: " + firstLine(e);
            toastAt = System.currentTimeMillis();
        } else if (t.equals("spawnmaps")) {
            spawnMaps = m;
            store.write("spawnmaps_" + client.saved().id() + ".json", m.toString());
        } else if (t.equals("spawns")) {
            JSONArray map = m.optJSONArray("map");
            if (map != null) {
                spawns.put(map.optInt(0) + "," + map.optInt(1), m);
            }
        } else {
            return;
        }
        changed();
    }

    static String firstLine(String s) {
        String l = s.split("\n")[0].trim();
        return l.replaceAll("[^A-Za-z0-9 .:_()]", " ");
    }

    public static String hex(byte[] b) {
        return Hex.of(b);
    }
}
