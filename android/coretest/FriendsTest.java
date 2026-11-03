import com.moemon.companion.NativeCrypto;
import com.moemon.companion.core.Friends;
import com.moemon.companion.core.Hex;
import com.moemon.companion.core.Link;
import com.moemon.companion.core.Pairing;
import com.moemon.companion.core.SavedLink;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.util.Map;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

/**
 * Phase 4's friends on a real co-op server with two games (docs/PHONE_LINK.md "Phase 4"): ALICE hosts a room, BOB joins,
 * they give each other their friend keys; the phone (paired with ALICE's game on the local network) reads BOB's
 * presence from the server's mailbox (sealed with BOB's key: who, where, the room to join, the party), then links to
 * ALICE's game through the server instead of the local network (the pipe) and gets its snapshot.
 */
public class FriendsTest {
    static final LinkedBlockingQueue<String> linesA = new LinkedBlockingQueue<>();

    static void fail(String why) {
        System.out.println("FRIENDS TEST FAILED: " + why);
        System.exit(1);
    }

    static int freePort() throws Exception {
        try (ServerSocket s = new ServerSocket(0)) {
            return s.getLocalPort();
        }
    }

    static Process game(File root, File work, String name, String auto, int lobby, int phonePort, LinkedBlockingQueue<String> lines,
                        String... script) throws Exception {
        File dir = new File(work, name);
        dir.mkdirs();
        File s = new File(dir, "s.txt");
        StringBuilder b = new StringBuilder("pressuntil A field 5000\nwait 30\n");
        for (String l : script) {
            b.append(l).append('\n');
        }
        b.append("wait 2000000\nquit\n");
        Files.write(s.toPath(), b.toString().getBytes());
        ProcessBuilder pb = new ProcessBuilder(new File(root, "pokeemerald64").getPath()).directory(root).redirectErrorStream(true);
        java.util.Map<String, String> env = pb.environment();
        env.put("SDL_VIDEODRIVER", "dummy");
        env.put("SDL_AUDIODRIVER", "dummy");
        env.put("MOEMON_TEST", s.getPath());
        env.put("MOEMON_SHOTDIR", dir.getPath());
        env.put("MOEMON_DEBUG_QUICKSTART", "1");
        env.put("MOEMON_DEBUG_NAME", name);
        env.put("MOEMON_COOP_CONFIG", new File(dir, "a.ini").getPath());
        env.put("MOEMON_CONTROLS_CONFIG", new File(dir, "c.ini").getPath());
        env.put("MOEMON_SAVE", new File(dir, "a.sav").getPath());
        env.put("MOEMON_LOBBY", "127.0.0.1:" + lobby);
        env.put("MOEMON_COOP_AUTO", auto);
        env.put("MOEMON_NO_UPDATE", "1");
        env.put("MOEMON_TEST_FPS", "60");
        env.put("MOEMON_DEBUG_PARTY", "25:20,1:20");
        if (phonePort > 0) {
            env.put("MOEMON_PHONE_PORT", String.valueOf(phonePort));
        }
        Process p = pb.start();
        Thread t = new Thread(() -> {
            try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String l;
                while ((l = r.readLine()) != null) {
                    if (lines != null) {
                        lines.add(l);
                    }
                }
            } catch (Exception ignored) {
            }
        });
        t.setDaemon(true);
        t.start();
        return p;
    }

    static String waitLine(String needle, int seconds) throws InterruptedException {
        long end = System.currentTimeMillis() + seconds * 1000L;
        while (System.currentTimeMillis() < end) {
            String l = linesA.poll(200, TimeUnit.MILLISECONDS);
            if (l != null && l.contains(needle)) {
                return l;
            }
        }
        fail("ALICE's game never said \"" + needle + "\"");
        return null;
    }

    public static void main(String[] args) throws Exception {
        File root = new File(args[0]), server = new File(args[1]), work = Files.createTempDirectory("friendstest").toFile();
        NativeCrypto crypto = new NativeCrypto();
        int lobby = freePort(), phonePort = freePort();
        Process srv = new ProcessBuilder(server.getPath(), "--port", String.valueOf(lobby), "--no-router").directory(work)
                .redirectErrorStream(true).redirectOutput(new File(work, "server.log")).start();
        Thread.sleep(500);
        String[] script = new String[31];
        for (int i = 0; i < 30; i++) {
            script[i] = "mp boxput 1 " + i + " " + (1 + i * 7) + " " + (5 + i);   // (a full box: a snapshot in several chunks)
        }
        script[30] = "mp phone pair";
        Process a = game(root, work, "ALICE", "host:TESTROOM", lobby, phonePort, linesA, script);
        Process b = null;
        try {
            String line = waitLine("[test] phone: save", 180);
            String[] f = line.trim().split("\\s+");
            String save = null, secret = null;
            for (int i = 0; i + 1 < f.length; i++) {
                if (f[i].equals("save")) save = f[i + 1];
                if (f[i].equals("secret")) secret = f[i + 1];
            }
            Pairing qr = Pairing.parse("MOEPHONE:1:127.0.0.1:" + phonePort + ":" + save + ":" + secret + ":AAAAAA");
            SavedLink saved = Link.pair(crypto, "127.0.0.1", phonePort, qr.saveId, qr.secretRaw, null, "FRIENDS");
            b = game(root, work, "BOB", "join:TESTROOM", lobby, 0, null);
            // ALICE's snapshot lists BOB once they've met in the room
            JSONObject friends = null;
            long end = System.currentTimeMillis() + 150000;
            while (System.currentTimeMillis() < end) {
                try (Link l = Link.connect(crypto, saved, "127.0.0.1", phonePort)) {
                    l.send(new JSONObject().put("t", "snap"));
                    JSONObject snap;
                    do {
                        snap = l.recv();
                    } while (!"snap".equals(snap.optString("t")));
                    friends = snap.optJSONObject("friends");
                }
                if (friends != null && friends.optJSONArray("list").length() > 0) {
                    break;
                }
                Thread.sleep(2000);
            }
            if (friends == null || friends.optJSONArray("list").length() == 0) {
                fail("ALICE never met BOB in the room");
            }
            JSONArray list = friends.getJSONArray("list");
            System.out.println("friends: ALICE knows " + list.length() + " (server " + friends.optString("srv") + ")");
            // BOB's presence from the server, sealed with BOB's key
            JSONObject bob = null;
            end = System.currentTimeMillis() + 120000;
            while (System.currentTimeMillis() < end && bob == null) {
                Map<String, JSONObject> p = Friends.fetch(crypto, "127.0.0.1:" + lobby, list);
                bob = p.get(list.getJSONObject(0).getString("id"));
                if (bob == null) {
                    Thread.sleep(3000);
                }
            }
            if (bob == null) {
                fail("BOB's presence never came");
            }
            System.out.println("presence: " + bob);
            if (!"TESTROOM".equals(bob.optString("room")) || !Friends.online(bob, System.currentTimeMillis() / 1000)
                    || bob.optJSONArray("party").length() != 2) {
                fail("BOB's presence should say he's online in TESTROOM with his party");
            }
            // away from home: the phone reaches ALICE's game through the server
            try (Link l = Link.viaServer(crypto, saved, "127.0.0.1:" + lobby)) {
                if (!l.viaServer()) {
                    fail("not through the server");
                }
                l.send(new JSONObject().put("t", "snap"));
                JSONObject snap;
                do {
                    snap = l.recv();
                } while (!"snap".equals(snap.optString("t")));
                if (!Hex.of(qr.saveId).equals(snap.getJSONObject("save").optString("id"))) {
                    fail("the snapshot through the server isn't ALICE's");
                }
                System.out.println("through the server: a snapshot of " + snap.toString().length() + " bytes");
                if (snap.toString().length() < 7900 * 1.5) {
                    fail("the snapshot should take several chunks through the server");
                }
            }
            System.out.println("FRIENDS TEST PASSED: friend keys in a room, presence from the server, the phone through the server");
        } finally {
            a.destroy();
            if (b != null) {
                b.destroy();
            }
            srv.destroy();
        }
    }
}
