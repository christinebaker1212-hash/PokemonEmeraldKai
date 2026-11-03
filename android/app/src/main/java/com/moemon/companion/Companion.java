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
    public final java.util.Map<Integer, JSONObject> places = new java.util.HashMap<>();  // map section -> "place"
    public JSONObject calls;                                      // MATCH CALL's list ("calls")
    public JSONObject call;                                       // the last call's words ("call")
    public long callAt;                                           // (when it came)
    public JSONObject gear;                                       // the POKéGEAR's last answer ("gear")
    public long gearAt;                                           // (when it came)
    public volatile JSONObject battle;                            // the battle as the player sees it ("battle")
    public final java.util.Map<Long, JSONObject> journals = new java.util.HashMap<>();   // personality -> "journal"
    private com.moemon.companion.core.Walk walk;                 // the POKéMON on a walk with this phone (this save's)
    private String walkSave = "";                                 // (the save it's for)
    public JSONObject lastHome;                                   // the last homecoming ("home" of a done answer)
    public volatile java.util.Map<String, JSONObject> friends = new java.util.HashMap<>();   // save ID -> presence
    public volatile long friendsAt;                               // (when they were read; 0: never)
    public volatile String friendsError = "";
    public volatile java.util.List<JSONObject> offers = new java.util.ArrayList<>();   // friends' offers to this save
    private final java.util.concurrent.ExecutorService background = java.util.concurrent.Executors.newSingleThreadExecutor();
    public long toastAt;
    public boolean toastGood;                                     // (a USE's result, not a refusal)
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
        JSONArray cached = spawnMaps == null ? null : spawnMaps.optJSONArray("maps");
        if (cached != null && cached.length() > 0 && !cached.optJSONObject(0).has("region")) {
            spawnMaps = null;   // (from an older game: asked again, with each place's region)
        }
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

    // ---------------------------------------------------------------------------------------------- friends

    /** Friends' presence again from the co-op server (in the background; FriendsPage, StepJob) */
    public void refreshFriends() {
        Model m = model();
        JSONObject f = m == null ? null : m.snap.optJSONObject("friends");
        if (f == null) {
            return;
        }
        final String server = f.optString("srv");
        final JSONArray list = f.optJSONArray("list");
        background.execute(() -> {
            try {
                friends = com.moemon.companion.core.Friends.fetch(crypto, server, list);
                offers = com.moemon.companion.core.Trades.incoming(crypto, server, f);
                friendsError = "";
            } catch (Exception e) {
                friendsError = "The co-op server didn't answer.";
            }
            friendsAt = System.currentTimeMillis();
            main.post(this::changed);
        });
    }

    /** A trade by mail: the offer into FRIEND's inbox (in the background), then the order to this save's game */
    public void offerTrade(JSONObject friend, JSONObject mon, JSONObject place, int want, java.util.function.Consumer<String> done) {
        Model m = model();
        JSONObject f = m == null ? null : m.snap.optJSONObject("friends");
        if (f == null || client == null) {
            done.accept("Link with your game first.");
            return;
        }
        final String name = m.snap.optJSONObject("save").optString("name");
        background.execute(() -> {
            String why = null;
            try {
                JSONObject order = com.moemon.companion.core.Trades.offer(crypto, f.optString("srv"), f, friend, name, mon, place, want);
                main.post(() -> client.order(order));
            } catch (IllegalStateException e) {
                why = "Play together in a room once more first: then you can trade by mail.";
            } catch (Exception e) {
                why = "The co-op server didn't answer. Try again in a moment.";
            }
            final String w = why;
            main.post(() -> {
                done.accept(w);
                changed();
            });
        });
    }

    /** DECLINE (SIDE 1: a friend's offer) or CANCEL (SIDE 0: ours) straight at the server's escrow */
    public void callOffTrade(String tHex, int side, java.util.function.Consumer<String> done) {
        Model m = model();
        JSONObject f = m == null ? null : m.snap.optJSONObject("friends");
        if (f == null) {
            done.accept("Link with your game first.");
            return;
        }
        background.execute(() -> {
            String why;
            try {
                why = com.moemon.companion.core.Trades.callOff(crypto, f.optString("srv"), f, tHex, side) ? null
                        : "Too late: both POKéMON are already on their way.";
            } catch (Exception e) {
                why = "The co-op server didn't answer. Try again in a moment.";
            }
            if (why == null && side == 0 && client != null) {
                try {
                    final JSONObject o = new JSONObject().put("op", "untrade").put("trade", tHex);
                    main.post(() -> client.order(o));
                } catch (org.json.JSONException ignored) {
                }
            }
            final String w = why;
            main.post(() -> {
                done.accept(w);
                refreshFriends();
            });
        });
    }

    // ------------------------------------------------------------------------------------------------- the walk

    private String walkFile(String save) {
        return "walk_" + save + ".json";
    }

    /** The POKéMON on a walk with this phone for the save in use, or null */
    public synchronized com.moemon.companion.core.Walk walk() {
        String save = client == null ? "" : client.saved().id();
        if (!save.equals(walkSave)) {
            walkSave = save;
            JSONObject o = readJson(walkFile(save));
            walk = o == null ? null : com.moemon.companion.core.Walk.fromJson(o, java.time.ZoneId.systemDefault());
        }
        return walk;
    }

    public synchronized void saveWalk() {
        if (walkSave.isEmpty()) {
            return;
        }
        if (walk == null) {
            new File(context.getFilesDir(), walkFile(walkSave)).delete();
            return;
        }
        try {
            store.write(walkFile(walkSave), walk.toJson().toString());
        } catch (org.json.JSONException ignored) {
        }
    }

    /** A step counter reading (StepCounter, StepJob) */
    public synchronized void walkReading(long counter, long boot, long now) {
        com.moemon.companion.core.Walk w = walk();
        if (w != null) {
            w.reading(counter, boot, now);
            saveWalk();
            main.post(this::changed);
        }
    }

    // The game's walk, from a snapshot or a "walk" answer: a new walk adopted, the one going on refreshed; a snapshot
    // from a game that knows walks ("now") without one: it came home some other way (FORGET PHONES in the game)
    private synchronized void walkFromGame(JSONObject snap, JSONObject fromAnswer) {
        com.moemon.companion.core.Walk w = walk();
        JSONObject g = fromAnswer != null ? fromAnswer : snap == null ? null : snap.optJSONObject("walk");
        if (g != null) {
            if (w == null || w.id() != g.optLong("id")) {
                walk = new com.moemon.companion.core.Walk(g, java.time.ZoneId.systemDefault());
            } else {
                walk.game = g;
            }
            saveWalk();
        } else if (snap != null && snap.has("now") && w != null) {
            walk = null;
            saveWalk();
        }
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
            case "crashed": return "The game closed while doing this, so it was stopped.";
            case "walking": return "Another POKéMON is already out on a walk.";
            case "cantsave": return "The game couldn't save just now. Try again.";
            case "nowalk": return "That walk already ended.";
            case "otherphone": return "That POKéMON is on a walk with another phone.";
            case "trainbox": return "Only POKéMON in the boxes can train.";
            case "slots": return "Every training spot is taken. Earn BADGES for more.";
            case "notparty": return "Items are used on the POKéMON in your party.";
            case "noeffect": return "It won't have any effect.";
            case "cap": return "RARE CANDIES won't raise it past the next BADGE's level.";
            case "knows": return "It already knows that move.";
            case "cantlearn": return "It can't learn that move.";
            case "cantuse": return "That can't be used on a POKéMON.";
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

    /** DEVICE > HIDE ON PC: the game's own POKéTCH is put away while the phone shows it */
    public boolean hideOnPc() {
        return context.getSharedPreferences("moemon", 0).getBoolean("hideOnPc", false);
    }

    public JSONObject watchOn() throws org.json.JSONException {
        return new JSONObject().put("t", "watch").put("on", 1).put("hide", hideOnPc() ? 1 : 0);
    }

    @Override
    public void onStatus(LinkClient.Status status, String why) {
        statusWhy = why;
        if (status != LinkClient.Status.LINKED) {
            battle = null;   // (only while linked: it's live)
        }
        if (status == LinkClient.Status.LINKED && client != null && spawnMaps == null) {
            client.ask("spawnmaps");
        }
        if (status == LinkClient.Status.LINKED && client != null && watching) {
            try {
                client.send(watchOn());   // (linked again: the POKéTCH again)
            } catch (org.json.JSONException ignored) {
            }
        }
        changed();
    }

    @Override
    public void onModel(Model model) {
        walkFromGame(model.snap, null);
        // a game older than the app sends less (stats, places): say so once, the game updates itself on its start
        String game = model.save().optString("game"), app = Updater.version(context);
        if (!game.isEmpty() && Updater.older(game, app) && !game.equals(warnedGame)) {
            warnedGame = game;
            toast = "The game is on " + game + ". Restart it to update to " + app + " so everything shows here.";
            toastGood = false;
            toastAt = System.currentTimeMillis();
        }
        changed();
    }

    private String warnedGame = "";

    @Override
    public void onAnswer(JSONObject order, JSONObject done) {
        if (done.optInt("ok") == 0) {
            // ("unknown": an order the game doesn't know yet: an older game, which updates itself on its start)
            toast = done.optString("why").equals("unknown")
                    ? "The game doesn't know how to do that yet. Restart it to update it." : why(done.optString("why"));
            toastGood = false;
            toastAt = System.currentTimeMillis();
        } else if (order != null && "walk".equals(order.optString("op")) && done.has("walk")) {
            walkFromGame(null, done.optJSONObject("walk"));
            toast = "Off on a walk! Your steps count from now.";
            toastGood = true;
            toastAt = System.currentTimeMillis();
        } else if (order != null && "home".equals(order.optString("op"))) {
            synchronized (this) {
                lastHome = done.optJSONObject("home");
                walk = null;
                saveWalk();
            }
            toast = "Home again! The game has it now.";
            toastGood = true;
            toastAt = System.currentTimeMillis();
        } else if (order != null && "kind".equals(order.optString("op")) && done.has("line")) {
            toast = art.plain(GameArt.hexText(done.optString("line")));
            toastGood = true;
            toastAt = System.currentTimeMillis();
        } else if (order != null && ("collect".equals(order.optString("op")) || "untrain".equals(order.optString("op")))) {
            toast = TrainText.gains(art, done.optJSONArray("gains"));
            toastGood = true;
            toastAt = System.currentTimeMillis();
        } else if (order != null && "use".equals(order.optString("op"))) {
            toast = UseFlow.result(art, order, done, model());
            toastGood = true;
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
        } else if (t.equals("journal")) {
            journals.put(m.optLong("p"), m);
        } else if (t.equals("place")) {
            places.put(m.optInt("sec"), m);
        } else if (t.equals("calls")) {
            calls = m;
        } else if (t.equals("call")) {
            call = m;
            callAt = System.currentTimeMillis();
        } else if (t.equals("gear")) {
            gear = m;
            gearAt = System.currentTimeMillis();
        } else if (t.equals("battle")) {
            battle = m.optInt("on") == 1 ? m : null;
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
