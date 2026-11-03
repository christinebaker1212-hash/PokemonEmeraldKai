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
    public final TtArt ttArt;                                     // TRIPLE TRIAD's cards (the game's engine and art)
    public final TcgArt tcgArt;                                   // the POKéMON cards (the game's engine and scans)
    public volatile int[] lastTcgPack;                            // the cards of the last booster opened (tcgopen's answer)
    public volatile int lastTcgPackType = -1, lastTcgPackNew;   // (the booster's NEW cards: a bit each)
    public volatile int[] lastPack;                               // the cards of the last pack opened (ttopen's answer)
    public volatile int lastPackType = -1;
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
        this.ttArt = new TtArt(context.getAssets());
        this.tcgArt = new TcgArt(context.getAssets());
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

    // ---------------------------------------------------------------------- TRIPLE TRIAD with friends, by turns

    private java.util.List<JSONObject> ttGames;   // this save's friend duels (the phone's own record)
    private String ttGamesSave = "";
    public volatile java.util.List<JSONObject> ttInvites = new java.util.ArrayList<>();

    /** This save's friend duels, newest first */
    public synchronized java.util.List<JSONObject> ttGames() {
        String save = client == null ? "" : client.saved().id();
        if (ttGames == null || !save.equals(ttGamesSave)) {
            ttGamesSave = save;
            ttGames = new java.util.ArrayList<>();
            JSONObject o = readJson("ttgames_" + save + ".json");
            JSONArray a = o == null ? null : o.optJSONArray("games");
            for (int i = 0; a != null && i < a.length(); i++) {
                ttGames.add(a.optJSONObject(i));
            }
        }
        return ttGames;
    }

    public synchronized void saveTtGames() {
        if (ttGamesSave.isEmpty() || ttGames == null) {
            return;
        }
        JSONArray a = new JSONArray();
        long now = System.currentTimeMillis() / 1000;
        for (JSONObject g : ttGames) {
            if (now - g.optLong("last") < 30L * 86400) {   // (finished ones fade out after a month)
                a.put(g);
            }
        }
        try {
            store.write("ttgames_" + ttGamesSave + ".json", new JSONObject().put("games", a).toString());
        } catch (org.json.JSONException ignored) {
        }
    }

    private JSONObject friendsSnap() {
        Model m = model();
        return m == null ? null : m.snap.optJSONObject("friends");
    }

    private JSONObject friendEntry(String id) {
        JSONObject f = friendsSnap();
        JSONArray list = f == null ? null : f.optJSONArray("list");
        for (int i = 0; list != null && i < list.length(); i++) {
            if (id.equals(list.optJSONObject(i).optString("id"))) {
                return list.optJSONObject(i);
            }
        }
        return null;
    }

    /** Friends' invites and the moves in each game going on, from the server (in the background) */
    public void refreshTtGames() {
        background.execute(this::syncTtGames);
    }

    /** The same, here and now (StepJob: off the screen's thread) */
    public void syncTtGames() {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        final java.util.List<JSONObject> games;
        synchronized (this) {
            games = new java.util.ArrayList<>(ttGames());
        }
        {
            boolean changed = false;
            try {
                java.util.List<JSONObject> inv = com.moemon.companion.core.TtFriends.invites(crypto, f.optString("srv"), f);
                java.util.List<JSONObject> fresh = new java.util.ArrayList<>();
                for (JSONObject i : inv) {
                    boolean known = false;
                    for (JSONObject g : games) {
                        known |= g.optString("g").equals(i.optString("g"));
                    }
                    if (!known && System.currentTimeMillis() / 1000 - i.optLong("at") < com.moemon.companion.core.TtFriends.DAYS * 86400L) {
                        fresh.add(i);
                    }
                }
                ttInvites = fresh;
                for (JSONObject g : games) {
                    String st = TtFriendGame.status(g);
                    if (st.equals("won") || st.equals("lost") || st.equals("draw") || st.equals("declined")) {
                        continue;
                    }
                    JSONObject friend = friendEntry(g.optString("peer"));
                    if (friend == null) {
                        continue;
                    }
                    if (g.has("unsent")) {
                        // (our last move didn't get there: again)
                        int seq = g.optInt("unsent");
                        JSONArray mv = g.getJSONArray("moves").optJSONArray(seq - 1);
                        if (mv != null && com.moemon.companion.core.TtFriends.move(crypto, g.optString("srv", f.optString("srv")),
                                friend, g.optString("g"), seq, mv.optInt(0), mv.optInt(1))) {
                            synchronized (this) {
                                g.remove("unsent");
                            }
                            changed = true;
                        }
                    }
                    JSONObject r = com.moemon.companion.core.TtFriends.read(crypto, g.optString("srv", f.optString("srv")), friend,
                            g.optString("g"));
                    synchronized (this) {
                        int other = 1 - g.optInt("seat");
                        if (r.optInt("declined") == 1 && g.optInt("seat") == 0) {
                            g.put("declined", 1);
                            changed = true;
                        }
                        if (r.opt("hand") instanceof JSONArray && g.getJSONArray("hands").isNull(other)) {
                            g.getJSONArray("hands").put(other, r.getJSONArray("hand"));
                            changed = true;
                        }
                        if (r.getJSONArray("moves").length() > g.getJSONArray("moves").length()) {
                            g.put("moves", r.getJSONArray("moves"));
                            g.put("last", System.currentTimeMillis() / 1000);
                            changed = true;
                        }
                    }
                }
            } catch (Exception ignored) {
            }
            if (changed) {
                saveTtGames();
            }
            main.post(this::changed);
        }
    }

    /** Invites FRIEND to a duel with HAND (the result: null, else why not) */
    public void inviteTt(JSONObject friend, int[] hand, int rules, java.util.function.Consumer<String> done) {
        final JSONObject f = friendsSnap();
        Model m = model();
        if (f == null || m == null) {
            done.accept("Link with your game first.");
            return;
        }
        final String name = m.snap.optJSONObject("save").optString("name");
        background.execute(() -> {
            String why = null;
            try {
                JSONObject g = com.moemon.companion.core.TtFriends.invite(crypto, f.optString("srv"), f, friend, name, hand, rules);
                synchronized (this) {
                    ttGames().add(0, g);
                }
                saveTtGames();
            } catch (IllegalStateException e) {
                why = "Play together in a room once more first: then you can duel by turns.";
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

    public void acceptTt(JSONObject invite, int[] hand, java.util.function.Consumer<JSONObject> done) {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        background.execute(() -> {
            JSONObject g = null;
            try {
                g = com.moemon.companion.core.TtFriends.accept(crypto, f.optString("srv"), invite, hand);
                synchronized (this) {
                    ttGames().add(0, g);
                    ttInvites.remove(invite);
                }
                saveTtGames();
            } catch (Exception ignored) {
            }
            final JSONObject game = g;
            main.post(() -> {
                done.accept(game);
                changed();
            });
        });
    }

    public void declineTt(JSONObject invite) {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        ttInvites.remove(invite);
        background.execute(() -> {
            try {
                com.moemon.companion.core.TtFriends.decline(crypto, f.optString("srv"), invite);
            } catch (Exception ignored) {
            }
            main.post(this::changed);
        });
    }

    /** Our move SEQ in GAME (already played on the phone): to the server; kept to send again if it doesn't get there */
    public void postTtMove(JSONObject game, int slot, int cell) {
        final JSONObject f = friendsSnap();
        final JSONObject friend = friendEntry(game.optString("peer"));
        final int seq;
        synchronized (this) {
            JSONArray moves = game.optJSONArray("moves");
            moves.put(new JSONArray().put(slot).put(cell));
            seq = moves.length();
            try {
                game.put("last", System.currentTimeMillis() / 1000).put("unsent", seq);
            } catch (org.json.JSONException ignored) {
            }
        }
        saveTtGames();
        if (f == null || friend == null) {
            return;
        }
        background.execute(() -> {
            try {
                if (com.moemon.companion.core.TtFriends.move(crypto, game.optString("srv", f.optString("srv")), friend,
                        game.optString("g"), seq, slot, cell)) {
                    synchronized (this) {
                        game.remove("unsent");
                    }
                    saveTtGames();
                }
            } catch (Exception ignored) {
            }
        });
    }

    // ------------------------------------------------------------------ the POKéMON card game with friends, by turns

    private java.util.List<JSONObject> tcgGames;   // this save's friend duels (the phone's own record)
    private String tcgGamesSave = "";
    public volatile java.util.List<JSONObject> tcgInvites = new java.util.ArrayList<>();

    /** This save's POKéMON card duels with friends, newest first */
    public synchronized java.util.List<JSONObject> tcgGames() {
        String save = client == null ? "" : client.saved().id();
        if (tcgGames == null || !save.equals(tcgGamesSave)) {
            tcgGamesSave = save;
            tcgGames = new java.util.ArrayList<>();
            JSONObject o = readJson("tcggames_" + save + ".json");
            JSONArray a = o == null ? null : o.optJSONArray("games");
            for (int i = 0; a != null && i < a.length(); i++) {
                tcgGames.add(a.optJSONObject(i));
            }
        }
        return tcgGames;
    }

    public synchronized void saveTcgGames() {
        if (tcgGamesSave.isEmpty() || tcgGames == null) {
            return;
        }
        JSONArray a = new JSONArray();
        long now = System.currentTimeMillis() / 1000;
        for (JSONObject g : tcgGames) {
            if (now - g.optLong("last") < 30L * 86400) {   // (finished ones fade out after a month)
                a.put(g);
            }
        }
        try {
            store.write("tcggames_" + tcgGamesSave + ".json", new JSONObject().put("games", a).toString());
        } catch (org.json.JSONException ignored) {
        }
    }

    public void refreshTcgGames() {
        background.execute(this::syncTcgGames);
    }

    /** Friends' invites and the moves in each duel going on, from the server (off the screen's thread) */
    public void syncTcgGames() {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        final java.util.List<JSONObject> games;
        synchronized (this) {
            games = new java.util.ArrayList<>(tcgGames());
        }
        boolean changed = false;
        try {
            java.util.List<JSONObject> inv = com.moemon.companion.core.CardDuels.invites(crypto, f.optString("srv"), f, "tcg");
            java.util.List<JSONObject> fresh = new java.util.ArrayList<>();
            for (JSONObject i : inv) {
                boolean known = false;
                for (JSONObject g : games) {
                    known |= g.optString("g").equals(i.optString("g"));
                }
                if (!known && System.currentTimeMillis() / 1000 - i.optLong("at") < com.moemon.companion.core.CardDuels.DAYS * 86400L) {
                    fresh.add(i);
                }
            }
            tcgInvites = fresh;
            for (JSONObject g : games) {
                String st = TcgFriendGame.status(g);
                if (st.equals("won") || st.equals("lost") || st.equals("declined")) {
                    continue;
                }
                JSONObject friend = friendEntry(g.optString("peer"));
                if (friend == null) {
                    continue;
                }
                String srv = g.optString("srv", f.optString("srv"));
                if (g.has("unsent")) {
                    // (our moves that didn't get there: again, in order)
                    boolean all = true;
                    JSONArray moves = g.getJSONArray("moves");
                    for (int seq = g.optInt("unsent"); seq <= moves.length() && all; seq++) {
                        all = com.moemon.companion.core.CardDuels.move(crypto, srv, friend, "tcg", g.optString("g"), seq,
                                moves.optJSONObject(seq - 1));
                    }
                    if (all) {
                        synchronized (this) {
                            g.remove("unsent");
                        }
                        changed = true;
                    }
                }
                JSONObject r = com.moemon.companion.core.CardDuels.read(crypto, srv, friend, "tcg", g.optString("g"));
                synchronized (this) {
                    int other = 1 - g.optInt("seat");
                    if (r.optInt("declined") == 1 && g.optInt("seat") == 0) {
                        g.put("declined", 1);
                        changed = true;
                    }
                    JSONObject zero = r.optJSONObject("zero");
                    if (zero != null && zero.optJSONArray("deck") != null && g.getJSONArray("decks").isNull(other)) {
                        g.getJSONArray("decks").put(other, zero.getJSONArray("deck"));
                        changed = true;
                    }
                    if (r.getJSONArray("moves").length() > g.getJSONArray("moves").length()) {
                        g.put("moves", r.getJSONArray("moves"));
                        g.put("last", System.currentTimeMillis() / 1000);
                        changed = true;
                    }
                }
            }
        } catch (Exception ignored) {
        }
        if (changed) {
            saveTcgGames();
        }
        main.post(this::changed);
    }

    private static JSONArray ints(int[] a) {
        JSONArray out = new JSONArray();
        for (int x : a) {
            out.put(x);
        }
        return out;
    }

    /** Invites FRIEND to a POKéMON card duel with DECK (the result: null, else why not) */
    public void inviteTcg(JSONObject friend, int[] deck, java.util.function.Consumer<String> done) {
        final JSONObject f = friendsSnap();
        Model m = model();
        if (f == null || m == null) {
            done.accept("Link with your game first.");
            return;
        }
        final String name = m.snap.optJSONObject("save").optString("name");
        background.execute(() -> {
            String why = null;
            try {
                JSONObject g = com.moemon.companion.core.CardDuels.invite(crypto, f.optString("srv"), f, friend, name, "tcg",
                        new JSONObject().put("deck", ints(deck)).put("prizes", 6));
                g.put("prizes", 6).put("decks", new JSONArray().put(ints(deck)).put(JSONObject.NULL));
                synchronized (this) {
                    tcgGames().add(0, g);
                }
                saveTcgGames();
            } catch (IllegalStateException e) {
                why = "Play together in a room once more first: then you can duel by turns.";
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

    public void acceptTcg(JSONObject invite, int[] deck, java.util.function.Consumer<JSONObject> done) {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        background.execute(() -> {
            JSONObject g = null;
            try {
                com.moemon.companion.core.CardDuels.accept(crypto, f.optString("srv"), invite, "tcg",
                        new JSONObject().put("deck", ints(deck)));
                JSONObject friend = invite.optJSONObject("friend");
                g = new JSONObject().put("g", invite.optString("g")).put("peer", friend.optString("id"))
                        .put("pn", invite.optString("n")).put("seat", 1).put("seed", invite.optInt("seed"))
                        .put("prizes", invite.optInt("prizes", 6))
                        .put("decks", new JSONArray().put(invite.optJSONArray("deck")).put(ints(deck)))
                        .put("moves", new JSONArray()).put("at", invite.optLong("at"))
                        .put("last", System.currentTimeMillis() / 1000).put("srv", f.optString("srv"));
                synchronized (this) {
                    tcgGames().add(0, g);
                    tcgInvites.remove(invite);
                }
                saveTcgGames();
            } catch (Exception ignored) {
                g = null;
            }
            final JSONObject game = g;
            main.post(() -> {
                done.accept(game);
                changed();
            });
        });
    }

    public void declineTcg(JSONObject invite) {
        final JSONObject f = friendsSnap();
        if (f == null) {
            return;
        }
        tcgInvites.remove(invite);
        background.execute(() -> {
            try {
                com.moemon.companion.core.CardDuels.decline(crypto, f.optString("srv"), invite, "tcg");
            } catch (Exception ignored) {
            }
            main.post(this::changed);
        });
    }

    /** Our action in GAME (already played on the phone): to the server; kept to send again if it doesn't get there */
    public void postTcgMove(JSONObject game, JSONObject action) {
        final JSONObject f = friendsSnap();
        final JSONObject friend = friendEntry(game.optString("peer"));
        final int seq;
        synchronized (this) {
            JSONArray moves = game.optJSONArray("moves");
            moves.put(action);
            seq = moves.length();
            try {
                game.put("last", System.currentTimeMillis() / 1000);
                if (!game.has("unsent")) {
                    game.put("unsent", seq);
                }
            } catch (org.json.JSONException ignored) {
            }
        }
        saveTcgGames();
        if (f == null || friend == null) {
            return;
        }
        background.execute(() -> {
            try {
                JSONArray moves = game.optJSONArray("moves");
                boolean all = true;
                for (int k = game.optInt("unsent", seq); k <= seq && all; k++) {
                    all = com.moemon.companion.core.CardDuels.move(crypto, game.optString("srv", f.optString("srv")), friend,
                            "tcg", game.optString("g"), k, moves.optJSONObject(k - 1));
                }
                if (all) {
                    synchronized (this) {
                        if (game.optInt("unsent") <= seq) {
                            game.remove("unsent");
                        }
                    }
                    saveTcgGames();
                }
            } catch (Exception ignored) {
            }
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
            case "today": return "That's today's prizes from the phone. More tomorrow, or win cards in the game!";
            case "nocards": return "Start TRIPLE TRIAD in your game first: a card seller in a POKéMON CENTER deals your first hand.";
            case "notowned": return "That deck uses cards your game doesn't have (any more).";
            case "deckrules": return "That deck breaks the deck rules: check its stars.";
            case "nopack": return "That pack isn't in your game any more.";
            case "activedeck": return "That's the deck you duel with: choose another one first.";
            case "empty": return "That deck has no cards yet.";
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
            String w = done.optString("why");
            boolean tcg = order != null && order.optString("op").startsWith("tcg");
            toast = w.equals("unknown") ? "The game doesn't know how to do that yet. Restart it to update it."
                    : tcg && w.equals("nocards") ? "Get your first POKéMON card deck in your game first: the CARD CLUB member in a POKéMON MART."
                    : tcg && w.equals("deckrules") ? "That deck breaks the deck rules: 60 cards, a Basic POKéMON, and no more than 4 of a name."
                    : why(w);
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
        } else if (order != null && "ttopen".equals(order.optString("op")) && done.has("cards")) {
            org.json.JSONArray a = done.optJSONArray("cards");
            int[] ids = new int[a.length()];
            for (int i = 0; i < ids.length; i++) {
                ids[i] = a.optInt(i);
            }
            lastPackType = order.optInt("pack");
            lastPack = ids;
        } else if (order != null && "tcgopen".equals(order.optString("op")) && done.has("cards")) {
            org.json.JSONArray a = done.optJSONArray("cards");
            int[] ids = new int[a.length()];
            for (int i = 0; i < ids.length; i++) {
                ids[i] = a.optInt(i);
            }
            lastTcgPackType = order.optInt("pack");
            org.json.JSONArray fresh = done.optJSONArray("new");
            lastTcgPackNew = 0;
            for (int i = 0; fresh != null && i < fresh.length(); i++) {
                lastTcgPackNew |= fresh.optInt(i) << i;
            }
            lastTcgPack = ids;
        } else if (order != null && "tcgduel".equals(order.optString("op")) && done.optInt("prize", -1) >= 0) {
            toast = "Won a " + Tcg.PACK_NAMES[Math.min(Tcg.PACKS - 1, done.optInt("prize"))] + " booster! Open it in CARDS.";
            toastGood = true;
            toastAt = System.currentTimeMillis();
        } else if (order != null && "ttwin".equals(order.optString("op")) && done.has("prize")) {
            org.json.JSONArray packs = ttArt.words == null ? null : ttArt.words.optJSONArray("packs");
            toast = "Won a " + (packs == null ? "" : packs.optString(done.optInt("prize")) + " ") + "pack! Open it in CARDS.";
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
