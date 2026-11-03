// The app's link core (android/app/src/main/java/com/moemon/companion/core) against a headless game, on a desktop JVM
// with the game's own C crypto (NativeCrypto through a host build of link_jni.c). Run by android/coretest/run.sh.
//
// Pairs by the secret, links with LinkClient, moves a POKeMON and checks the model and the game's rev agree with no
// new snapshot, a stale order is refused, an order made away is sent on the next link, a change made in the game by
// hand (the script's boxput) brings a new snapshot by itself, and the model is back from the disk at once.
import com.moemon.companion.NativeCrypto;
import com.moemon.companion.core.*;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

public class CoreTest {
    static final int PORT = 47900 + (int) (ProcessHandle.current().pid() % 90);
    static final LinkedBlockingQueue<String> gameLines = new LinkedBlockingQueue<>();

    static void fail(String why) {
        System.out.println("CORE TEST FAILED: " + why);
        System.exit(1);
    }

    static final class DirStore implements Store {
        final File dir;

        DirStore(File dir) {
            this.dir = dir;
        }

        public String read(String name) {
            try {
                return new String(Files.readAllBytes(new File(dir, name).toPath()), StandardCharsets.UTF_8);
            } catch (Exception e) {
                return null;
            }
        }

        public void write(String name, String text) {
            try {
                Files.write(new File(dir, name).toPath(), text.getBytes(StandardCharsets.UTF_8));
            } catch (Exception e) {
                fail("can't write " + name);
            }
        }
    }

    static final class Events implements LinkClient.Listener {
        final LinkedBlockingQueue<Object[]> q = new LinkedBlockingQueue<>();
        int snaps;

        public void onStatus(LinkClient.Status s, String why) {
            q.add(new Object[]{"status", s, why});
        }

        public void onModel(Model m) {
            q.add(new Object[]{"model", m});
        }

        public void onAnswer(JSONObject order, JSONObject done) {
            q.add(new Object[]{"answer", order, done});
        }

        public void onMessage(JSONObject o) {
            q.add(new Object[]{"msg", o});
        }

        Object[] next(String kind, int seconds) throws InterruptedException {
            long end = System.currentTimeMillis() + seconds * 1000L;
            while (System.currentTimeMillis() < end) {
                Object[] e = q.poll(200, TimeUnit.MILLISECONDS);
                if (e != null && e[0].equals(kind)) {
                    return e;
                }
            }
            fail("no " + kind + " in " + seconds + " s");
            return null;
        }

        LinkClient.Status waitStatus(LinkClient.Status want, int seconds) throws InterruptedException {
            long end = System.currentTimeMillis() + seconds * 1000L;
            while (System.currentTimeMillis() < end) {
                Object[] e = q.poll(200, TimeUnit.MILLISECONDS);
                if (e != null && e[0].equals("status") && e[1] == want) {
                    return want;
                }
            }
            fail("never " + want);
            return null;
        }
    }

    static Process startGame(File root, File work) throws Exception {
        List<String> lines = new ArrayList<>();
        lines.add("pressuntil A field 5000");
        lines.add("wait 30");
        for (int pos = 0; pos < 10; pos++) {
            lines.add("mp boxput 1 " + pos + " " + (25 + pos) + " " + (5 + pos));
        }
        lines.add("mp additem 139 20");
        lines.add("mp additem 200 3");
        lines.add("mp additem 4 30");
        lines.add("mp phone pair");
        lines.add("wait 2400");   // (40 s at MOEMON_TEST_FPS=60: after steps 1-3)
        lines.add("mp boxput 7 4 133 30");   // (a change "by hand": the app must notice it by itself)
        lines.add("mp phone");
        lines.add("wait 2000000");
        lines.add("quit");
        File script = new File(work, "a.txt");
        Files.write(script.toPath(), String.join("\n", lines).getBytes());
        ProcessBuilder pb = new ProcessBuilder(new File(root, "pokeemerald64").getPath());
        pb.directory(root);
        pb.redirectErrorStream(true);
        java.util.Map<String, String> env = pb.environment();
        env.put("SDL_VIDEODRIVER", "dummy");
        env.put("SDL_AUDIODRIVER", "dummy");
        env.put("MOEMON_TEST", script.getPath());
        env.put("MOEMON_SHOTDIR", work.getPath());
        env.put("MOEMON_DEBUG_QUICKSTART", "1");
        env.put("MOEMON_SKIP_COOP_SETUP", "1");
        env.put("MOEMON_COOP_CONFIG", new File(work, "a.ini").getPath());
        env.put("MOEMON_CONTROLS_CONFIG", new File(work, "a_controls.ini").getPath());
        env.put("MOEMON_SAVE", new File(work, "a.sav").getPath());
        env.put("MOEMON_PHONE_PORT", String.valueOf(PORT));
        env.put("MOEMON_TEST_FPS", "60");
        env.put("MOEMON_DEBUG_PARTY", "25:20,1:20,4:20");
        Process p = pb.start();
        Thread t = new Thread(() -> {
            try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String line;
                while ((line = r.readLine()) != null) {
                    gameLines.add(line);
                }
            } catch (Exception ignored) {
            }
        });
        t.setDaemon(true);
        t.start();
        return p;
    }

    static String waitGame(String needle, int seconds) throws InterruptedException {
        long end = System.currentTimeMillis() + seconds * 1000L;
        while (System.currentTimeMillis() < end) {
            String l = gameLines.poll(200, TimeUnit.MILLISECONDS);
            if (l != null && l.contains(needle)) {
                return l;
            }
        }
        fail("the game never said \"" + needle + "\"");
        return null;
    }

    static String field(String line, String key) {
        String[] f = line.trim().split("\\s+");
        for (int i = 0; i + 1 < f.length; i++) {
            if (f[i].equals(key)) {
                return f[i + 1];
            }
        }
        return null;
    }

    static JSONObject findMon(Model m, int box, boolean occupied) {
        JSONArray s = m.box(box).optJSONArray("m");
        for (int i = 0; i < s.length(); i++) {
            if ((s.optJSONObject(i) != null) == occupied) {
                return Model.boxPlace(box, i);
            }
        }
        return null;
    }

    public static void main(String[] args) throws Exception {
        File root = new File(args[0]), work = Files.createTempDirectory("coretest").toFile();
        File appDir = new File(work, "app");
        appDir.mkdirs();
        NativeCrypto crypto = new NativeCrypto();
        if (NativeCrypto.linkVersion() != Link.VERSION) {
            fail("Link.VERSION isn't PHONE_LINK_VERSION");
        }
        Process game = startGame(root, work);
        try {
            String line = waitGame("[test] phone: save", 180);
            String save = field(line, "save"), secret = field(line, "secret");
            Pairing qr = Pairing.parse("MOEPHONE:1:127.0.0.1:" + PORT + ":" + save + ":" + secret + ":" + field(line, "code"));
            if (qr == null) {
                fail("the QR text didn't parse");
            }
            SavedLink saved = Link.pair(crypto, qr.host, qr.port, qr.saveId, qr.secretRaw, null, "CORETEST");
            System.out.println("paired: " + saved.id());

            DirStore store = new DirStore(appDir);
            Events ev = new Events();
            LinkClient c = new LinkClient(crypto, store, saved, ev);
            if (c.model() != null) {
                fail("a model before any snapshot");
            }
            c.start();
            ev.waitStatus(LinkClient.Status.LINKED, 20);
            Model m = (Model) ev.next("model", 20)[1];
            if (!m.saveId().equals(saved.id())) {
                fail("the snapshot's save isn't the paired one");
            }
            System.out.println("snapshot: party " + m.party().length() + ", rev " + m.rev);
            String dump = System.getenv("SNAP_OUT");   // (the app's screen tests draw a real snapshot)

            // 1. A move: the model applies the answer and its rev is the game's (no new snapshot needed)
            JSONObject from = findMon(m, 0, true), to = findMon(m, 3, false);
            JSONObject mon = m.at(from);
            c.order(Model.move(from, mon, to, null));
            Object[] a = ev.next("answer", 20);
            JSONObject done = (JSONObject) a[2];
            if (done.optInt("ok") != 1) {
                fail("the move wasn't done: " + done);
            }
            Model after = c.model();
            if (after.at(from) != null || after.at(to) == null || after.at(to).optLong("p") != mon.optLong("p")) {
                fail("the model didn't follow the move");
            }
            JSONObject st = (JSONObject) ev.next("msg", 10)[1];
            while (!"state".equals(st.optString("t"))) {
                st = (JSONObject) ev.next("msg", 10)[1];
            }
            if (st.optLong("rev") != after.rev) {
                fail("the model's rev " + after.rev + " isn't the game's " + st.optLong("rev"));
            }
            System.out.println("move: done, rev agrees");

            // 2. The same move again: the POKeMON isn't there any more
            c.order(Model.move(from, mon, to, null));
            done = (JSONObject) ev.next("answer", 20)[2];
            if (done.optInt("ok") != 0 || c.orders.failed().isEmpty()) {
                fail("a stale order was done: " + done);
            }
            System.out.println("stale: refused (" + done.optString("why") + ")");

            // 3. Away: an order made with no link waits on disk, then goes on the next link
            c.stop();
            LinkClient c2 = new LinkClient(crypto, store, saved, ev);
            if (c2.model() == null || c2.model().rev != after.rev) {
                fail("the model didn't come back from the disk");
            }
            JSONObject back = c2.order(Model.move(to, mon, from, null));
            if (c2.orders.pending().size() != 1) {
                fail("the order made away isn't waiting");
            }
            ev.q.clear();
            c2.start();
            done = (JSONObject) ev.next("answer", 30)[2];
            if (done.optInt("id") != back.optInt("id") || done.optInt("ok") != 1 || !c2.orders.pending().isEmpty()) {
                fail("the order made away wasn't done on the link: " + done);
            }
            System.out.println("away order: done on the next link");

            // 4. A change in the game by hand: a new snapshot comes by itself
            waitGame("[test] phone: save", 180);
            long end = System.currentTimeMillis() + 20000;
            boolean seen = false;
            while (!seen && System.currentTimeMillis() < end) {
                Object[] e = ev.q.poll(200, TimeUnit.MILLISECONDS);
                if (e != null && e[0].equals("model")) {
                    JSONObject x = ((Model) e[1]).box(6).optJSONArray("m").optJSONObject(4);
                    seen = x != null && x.optInt("s") == 133;
                }
            }
            if (!seen) {
                fail("the change made in the game never reached the model");
            }
            System.out.println("change in the game: a new snapshot by itself");

            // 5. Spawns
            c2.ask("spawnmaps");
            Object[] e;
            do {
                e = ev.next("msg", 10);
            } while (!"spawnmaps".equals(((JSONObject) e[1]).optString("t")));
            JSONArray maps = ((JSONObject) e[1]).optJSONArray("maps");
            if (dump != null) {
                Files.write(new File(dump, "snap.json").toPath(), c2.model().toFile().getBytes(StandardCharsets.UTF_8));
                Files.write(new File(dump, "spawnmaps.json").toPath(), e[1].toString().getBytes(StandardCharsets.UTF_8));
                JSONObject first = maps.getJSONObject(0);
                c2.send(new JSONObject().put("t", "spawns").put("map", first.getJSONArray("map")));
                do {
                    e = ev.next("msg", 10);
                } while (!"spawns".equals(((JSONObject) e[1]).optString("t")));
                Files.write(new File(dump, "spawns.json").toPath(), e[1].toString().getBytes(StandardCharsets.UTF_8));
            }
            if (maps == null || maps.length() < 50) {
                fail("too few maps with wild POKeMON");
            }
            System.out.println("spawn maps: " + maps.length());
            c2.stop();
            System.out.println("CORE TEST PASSED");
        } finally {
            game.destroy();
        }
        System.exit(0);
    }
}
