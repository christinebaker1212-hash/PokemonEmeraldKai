import com.moemon.companion.NativeCrypto;
import com.moemon.companion.core.Link;
import com.moemon.companion.core.Model;
import com.moemon.companion.core.Pairing;
import com.moemon.companion.core.SavedLink;
import com.moemon.companion.core.Trades;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

/**
 * Trades by mail between two games and their phones, everything through a lagging line (docs/PHONE_LINK.md "Trades by
 * mail"; the roadmap's gate: "trades pass two-client tests at lag"):
 *   1. ALICE's phone offers a POKéMON to BOB (into his inbox; her game keeps it ON OFFER); BOB's phone reads the offer
 *      and accepts with one of his; the games deposit and swap through the server's escrow: each ends with the other's
 *      POKéMON, exactly once, nothing lost or copied, and the trade scene plays in both.
 *   2. ALICE offers another; BOB declines: hers stays home and isn't ON OFFER any more.
 *   3. BOB accepts an offer, then ALICE's game is closed and opened again before it answers: it picks up and the
 *      trade still completes.
 */
public class TradeTest {
    static final NativeCrypto crypto = new NativeCrypto();
    static int idSeq;

    static void fail(String why) {
        System.out.println("TRADE TEST FAILED: " + why);
        System.exit(1);
    }

    static int freePort() throws Exception {
        try (ServerSocket s = new ServerSocket(0)) {
            return s.getLocalPort();
        }
    }

    static final class Game {
        final String name;
        final File dir;
        final int phonePort;
        final LinkedBlockingQueue<String> lines = new LinkedBlockingQueue<>();
        final List<String> log = new ArrayList<>();
        Process p;
        SavedLink saved;

        Game(File work, String name) throws Exception {
            this.name = name;
            this.dir = new File(work, name);
            dir.mkdirs();
            phonePort = freePort();
        }

        void start(File root, String auto, int lobby, String warp, boolean fresh, String... script) throws Exception {
            File s = new File(dir, "s" + System.nanoTime() + ".txt");
            StringBuilder b = new StringBuilder("pressuntil A field 5000\nwait 30\n");
            for (String l : script) {
                b.append(l).append('\n');
            }
            b.append("wait 600\nmash A 1000000 40\nquit\n");   // (A now and then: the trade scene's last page)
            Files.write(s.toPath(), b.toString().getBytes());
            ProcessBuilder pb = new ProcessBuilder(new File(root, "pokeemerald64").getPath()).directory(root).redirectErrorStream(true);
            java.util.Map<String, String> env = pb.environment();
            env.put("SDL_VIDEODRIVER", "dummy");
            env.put("SDL_AUDIODRIVER", "dummy");
            env.put("MOEMON_TEST", s.getPath());
            env.put("MOEMON_SHOTDIR", dir.getPath());
            if (fresh) {
                env.put("MOEMON_DEBUG_QUICKSTART", "1");
                env.put("MOEMON_DEBUG_NAME", name);
                env.put("MOEMON_DEBUG_PARTY", "25:20,1:20");
                env.put("MOEMON_DEBUG_WARP", warp);
            }
            env.put("MOEMON_COOP_CONFIG", new File(dir, "a.ini").getPath());
            env.put("MOEMON_CONTROLS_CONFIG", new File(dir, "c.ini").getPath());
            env.put("MOEMON_SAVE", new File(dir, "a.sav").getPath());
            env.put("MOEMON_LOBBY", "127.0.0.1:" + lobby);
            env.put("MOEMON_COOP_AUTO", auto);
            env.put("MOEMON_NO_UPDATE", "1");
            env.put("MOEMON_TEST_FPS", "60");
            env.put("MOEMON_PHONE_PORT", String.valueOf(phonePort));
            p = pb.start();
            Thread t = new Thread(() -> {
                try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                    String l;
                    while ((l = r.readLine()) != null) {
                        lines.add(l);
                        synchronized (log) {
                            log.add(l);
                        }
                    }
                } catch (Exception ignored) {
                }
            });
            t.setDaemon(true);
            t.start();
        }

        String waitLine(String needle, int seconds) throws InterruptedException {
            long end = System.currentTimeMillis() + seconds * 1000L;
            while (System.currentTimeMillis() < end) {
                String l = lines.poll(200, TimeUnit.MILLISECONDS);
                if (l != null && l.contains(needle)) {
                    return l;
                }
            }
            fail(name + "'s game never said \"" + needle + "\"");
            return null;
        }

        boolean said(String needle) {
            synchronized (log) {
                for (String l : log) {
                    if (l.contains(needle)) {
                        return true;
                    }
                }
            }
            return false;
        }

        void pair() throws Exception {
            String line = waitLine("[test] phone: save", 180);
            String[] f = line.trim().split("\\s+");
            String save = null, secret = null;
            for (int i = 0; i + 1 < f.length; i++) {
                if (f[i].equals("save")) save = f[i + 1];
                if (f[i].equals("secret")) secret = f[i + 1];
            }
            Pairing qr = Pairing.parse("MOEPHONE:1:127.0.0.1:" + phonePort + ":" + save + ":" + secret + ":AAAAAA");
            saved = Link.pair(crypto, "127.0.0.1", phonePort, qr.saveId, qr.secretRaw, null, name + "PHONE");
        }

        JSONObject snap() throws Exception {
            for (int tries = 0; ; tries++) {
                try (Link l = Link.connect(crypto, saved, "127.0.0.1", phonePort)) {
                    l.send(new JSONObject().put("t", "snap"));
                    JSONObject s;
                    do {
                        s = l.recv();
                    } while (!"snap".equals(s.optString("t")));
                    return s;
                } catch (Exception e) {
                    if (tries > 20) {
                        throw e;
                    }
                    Thread.sleep(1000);
                }
            }
        }

        // an order, sent again (each with a new ID) while the game is busy (orders run when it's free)
        JSONObject order(JSONObject o) throws Exception {
            long end = System.currentTimeMillis() + 60000;
            while (true) {
                try (Link l = Link.connect(crypto, saved, "127.0.0.1", phonePort)) {
                    JSONObject m = new JSONObject(o.toString()).put("t", "order").put("id", ++idSeq);
                    l.send(m);
                    JSONObject d;
                    do {
                        d = l.recv();
                    } while (!"done".equals(d.optString("t")));
                    if (!"busy".equals(d.optString("why")) || System.currentTimeMillis() > end) {
                        return d;
                    }
                }
                Thread.sleep(1000);
            }
        }

        void stop() {
            if (p != null) {
                p.destroy();
                try {
                    p.waitFor(10, TimeUnit.SECONDS);
                } catch (InterruptedException ignored) {
                }
            }
        }
    }

    static Set<String> mons(JSONObject snap) {
        Set<String> out = new HashSet<>();
        JSONArray party = snap.optJSONArray("party");
        for (int i = 0; party != null && i < party.length(); i++) {
            JSONObject m = party.optJSONObject(i);
            if (m != null) {
                out.add(m.optLong("p") + ":" + m.optLong("o"));
            }
        }
        JSONArray boxes = snap.optJSONArray("boxes");
        for (int b = 0; boxes != null && b < boxes.length(); b++) {
            JSONArray ms = boxes.optJSONObject(b).optJSONArray("m");
            for (int i = 0; ms != null && i < ms.length(); i++) {
                JSONObject m = ms.optJSONObject(i);
                if (m != null) {
                    out.add(m.optLong("p") + ":" + m.optLong("o"));
                }
            }
        }
        return out;
    }

    static String key(JSONObject mon) {
        return mon.optLong("p") + ":" + mon.optLong("o");
    }

    static JSONObject friendOf(JSONObject snap, String saveId) {
        JSONObject f = snap.optJSONObject("friends");
        JSONArray list = f == null ? null : f.optJSONArray("list");
        for (int i = 0; list != null && i < list.length(); i++) {
            JSONObject e = list.optJSONObject(i);
            if (saveId.equals(e.optString("id")) && e.has("pk")) {
                return e;
            }
        }
        return null;
    }

    static JSONObject trade(JSONObject snap, String t) {
        JSONArray l = snap.optJSONArray("trades");
        for (int i = 0; l != null && i < l.length(); i++) {
            if (t.equals(l.optJSONObject(i).optString("t"))) {
                return l.optJSONObject(i);
            }
        }
        return null;
    }

    // both trades in WANT (and shown, for "done"), or fail after SECONDS
    static void waitTrade(Game a, Game b, String t, String want, int seconds) throws Exception {
        long end = System.currentTimeMillis() + seconds * 1000L;
        String sa = "?", sb = "?";
        while (System.currentTimeMillis() < end) {
            JSONObject ta = trade(a.snap(), t), tb = b == null ? null : trade(b.snap(), t);
            sa = ta == null ? "none" : ta.optString("st") + (ta.optInt("shown") == 1 ? "+shown" : "");
            sb = b == null ? want : tb == null ? "none" : tb.optString("st") + (tb.optInt("shown") == 1 ? "+shown" : "");
            boolean okA = ta != null && want.equals(ta.optString("st")) && (!want.equals("done") || ta.optInt("shown") == 1);
            boolean okB = b == null || (tb != null && want.equals(tb.optString("st"))
                    && (!want.equals("done") || tb.optInt("shown") == 1));
            if (okA && okB) {
                return;
            }
            Thread.sleep(2000);
        }
        fail("the trade never got to " + want + " (" + a.name + ": " + sa + (b == null ? "" : ", " + b.name + ": " + sb) + ")");
    }

    public static void main(String[] args) throws Exception {
        File root = new File(args[0]), server = new File(args[1]), work = Files.createTempDirectory("tradetest").toFile();
        int srvPort = freePort(), lagPort = freePort();
        Process srv = new ProcessBuilder(server.getPath(), "--port", String.valueOf(srvPort), "--no-router").directory(work)
                .redirectErrorStream(true).redirectOutput(new File(work, "server.log")).start();
        // everything (games and phones) through a line that holds each chunk 120 ms + up to 60 ms each way
        Process lag = new ProcessBuilder("python3", new File(root, "tools/mp_server/lagproxy.py").getPath(),
                String.valueOf(lagPort), String.valueOf(srvPort), "120", "60").redirectErrorStream(true)
                .redirectOutput(new File(work, "lag.log")).start();
        Thread.sleep(800);
        Game alice = new Game(work, "ALICE"), bob = new Game(work, "BOB");
        try {
            alice.start(root, "host:TRADEROOM", lagPort, "0,16,10,10", true, "mp boxput 1 0 25 10", "mp boxput 1 1 4 12",
                    "mp boxput 1 2 152 9", "mp phone pair", "wait 300", "press B 2", "waitfor field 600");
            alice.pair();
            bob.start(root, "join:TRADEROOM", lagPort, "0,16,4,4", true, "mp boxput 1 0 7 11", "mp boxput 1 1 158 13",
                    "mp phone pair", "wait 300", "press B 2", "waitfor field 600");
            bob.pair();
            String aliceId = alice.saved.id(), bobId = bob.saved.id();
            // they meet in the room: each knows the other's keys (and the pair key)
            JSONObject sa = null, sb = null, bobForAlice = null, aliceForBob = null;
            long end = System.currentTimeMillis() + 150000;
            while (System.currentTimeMillis() < end && (bobForAlice == null || aliceForBob == null)) {
                sa = alice.snap();
                sb = bob.snap();
                bobForAlice = friendOf(sa, bobId);
                aliceForBob = friendOf(sb, aliceId);
                Thread.sleep(2000);
            }
            if (bobForAlice == null || aliceForBob == null) {
                fail("they never became friends who can trade");
            }
            if (!bobForAlice.optString("pk").equals(aliceForBob.optString("pk"))) {
                fail("the two saves worked out different pair keys");
            }
            String srvA = sa.getJSONObject("friends").optString("srv"), srvB = sb.getJSONObject("friends").optString("srv");
            System.out.println("friends: the pair key agrees; server " + srvA);
            Set<String> beforeA = mons(sa), beforeB = mons(sb);

            // 1. ALICE offers PIKACHU for anything; BOB gives SQUIRTLE
            JSONObject x = sa.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(0);
            JSONObject y = sb.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(0);
            JSONObject ord = Trades.offer(crypto, srvA, sa.getJSONObject("friends"), bobForAlice,
                    sa.getJSONObject("save").optString("name"), x, Model.boxPlace(0, 0), 0);
            JSONObject done = alice.order(ord);
            if (done.optInt("ok") != 1) {
                fail("ALICE's game didn't take the offer: " + done);
            }
            String t1 = ord.getString("trade");
            if (!"offered".equals(trade(alice.snap(), t1).optString("st"))) {
                fail("ALICE's trade isn't offered");
            }
            List<JSONObject> in = Trades.incoming(crypto, srvB, sb.getJSONObject("friends"));
            JSONObject offer = null;
            for (JSONObject o : in) {
                if (t1.equals(o.optString("t"))) {
                    offer = o;
                }
            }
            if (offer == null || offer.optInt("esc") != 0 || offer.getJSONObject("mon").optInt("s") != x.optInt("s")) {
                fail("BOB's inbox doesn't have ALICE's offer: " + in);
            }
            System.out.println("offer: in BOB's inbox (" + in.size() + "), PIKACHU for anything");
            done = bob.order(Trades.accept(offer, y, Model.boxPlace(0, 0)));
            if (done.optInt("ok") != 1) {
                fail("BOB's game didn't take the answer: " + done);
            }
            waitTrade(alice, bob, t1, "done", 240);
            JSONObject a1 = alice.snap(), b1 = bob.snap();
            Set<String> afterA = mons(a1), afterB = mons(b1);
            if (afterA.contains(key(x)) || !afterA.contains(key(y)) || !afterB.contains(key(x)) || afterB.contains(key(y))
                    || afterA.size() != beforeA.size() || afterB.size() != beforeB.size()) {
                fail("the swap: ALICE " + beforeA.size() + " -> " + afterA.size() + ", BOB " + beforeB.size() + " -> "
                        + afterB.size());
            }
            if (!alice.said("phone trade: the trade scene") || !bob.said("phone trade: the trade scene")) {
                fail("the trade scene didn't play in both games");
            }
            System.out.println("trade: swapped, each POKéMON once, the scene in both");

            // 2. ALICE offers CHARMANDER; BOB turns it down
            JSONObject z = a1.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(1);
            ord = Trades.offer(crypto, srvA, a1.getJSONObject("friends"), friendOf(a1, bobId),
                    a1.getJSONObject("save").optString("name"), z, Model.boxPlace(0, 1), 0);
            if (alice.order(ord).optInt("ok") != 1) {
                fail("the second offer");
            }
            String t2 = ord.getString("trade");
            if (!Trades.callOff(crypto, srvB, b1.getJSONObject("friends"), t2, 1)) {
                fail("BOB couldn't turn it down");
            }
            waitTrade(alice, null, t2, "off", 90);
            if (!mons(alice.snap()).contains(key(z))) {
                fail("CHARMANDER left ALICE after being turned down");
            }
            System.out.println("declined: CHARMANDER stays with ALICE, the offer is off");

            // 3. ALICE offers CHIKORITA for a CYNDAQUIL; her game is closed; BOB accepts (PIKACHU refused: not wanted); she comes back and it completes
            JSONObject a2 = alice.snap(), b2 = bob.snap();
            JSONObject w = a2.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(2);
            JSONObject v = b2.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(1);
            ord = Trades.offer(crypto, srvA, a2.getJSONObject("friends"), friendOf(a2, bobId),
                    a2.getJSONObject("save").optString("name"), w, Model.boxPlace(0, 2), 158);   // (wants CYNDAQUIL)
            if (alice.order(ord).optInt("ok") != 1) {
                fail("the third offer");
            }
            String t3 = ord.getString("trade");
            alice.stop();
            JSONObject offer3 = null;
            for (JSONObject o : Trades.incoming(crypto, srvB, b2.getJSONObject("friends"))) {
                if (t3.equals(o.optString("t"))) {
                    offer3 = o;
                }
            }
            if (offer3 == null) {
                fail("the third offer isn't in BOB's inbox");
            }
            int pika = -1;
            JSONArray bobParty = b2.getJSONArray("party");
            for (int i = 0; i < bobParty.length(); i++) {
                if (bobParty.optJSONObject(i) != null && key(bobParty.getJSONObject(i)).equals(key(x))) {
                    pika = i;
                }
            }
            JSONObject refused = bob.order(Trades.accept(offer3, bobParty.getJSONObject(pika), Model.partyPlace(pika)));
            if (refused.optInt("ok") != 0 || !"notwanted".equals(refused.optString("why"))) {
                fail("BOB gave PIKACHU for an offer that wants CYNDAQUIL: " + refused);
            }
            if (bob.order(Trades.accept(offer3, v, Model.boxPlace(0, 1))).optInt("ok") != 1) {
                fail("BOB's second answer");
            }
            waitTrade(bob, null, t3, "deposited", 90);
            alice.start(root, "host:TRADEROOM", lagPort, "", false);
            waitTrade(alice, bob, t3, "done", 300);
            if (!mons(alice.snap()).contains(key(v)) || !mons(bob.snap()).contains(key(w))) {
                fail("the third trade's POKéMON aren't where they should be");
            }
            System.out.println("restart: ALICE's game came back and the trade completed");
            System.out.println("TRADE TEST PASSED: a trade by mail at lag, a turned-down offer, a game closed in between");
        } finally {
            alice.stop();
            bob.stop();
            lag.destroy();
            srv.destroy();
        }
    }
}
