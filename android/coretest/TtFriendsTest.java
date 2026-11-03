import com.moemon.companion.NativeCrypto;
import com.moemon.companion.Tt;
import com.moemon.companion.TtFriendGame;
import com.moemon.companion.core.Hex;
import com.moemon.companion.core.TtFriends;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.util.List;

/**
 * TRIPLE TRIAD with a friend, a turn at a time through a real co-op server (core/TtFriends + the game's engine): ALICE
 * challenges BOB under each set of rules the app offers, BOB accepts, they play to the end with the computer choosing
 * for each, each phone seeing itself as the engine's seat 0; both must see the same board and the same result.
 * A turned-down challenge shows as such.
 */
public class TtFriendsTest {
    static final NativeCrypto c = new NativeCrypto();

    static void fail(String why) {
        System.out.println("TT FRIENDS TEST FAILED: " + why);
        System.exit(1);
    }

    static JSONObject snap(byte[] me, byte[] myKey, byte[] peer, byte[] peerKey, byte[] pk, String peerName) throws Exception {
        return new JSONObject().put("me", Hex.of(me)).put("inbox", Hex.of(c.derive(myKey, "inbox")))
                .put("list", new JSONArray().put(new JSONObject().put("id", Hex.of(peer)).put("key", Hex.of(peerKey))
                        .put("pk", Hex.of(pk)).put("n", peerName)));
    }

    public static void main(String[] args) throws Exception {
        File server = new File(args[0]), work = Files.createTempDirectory("ttfriends").toFile();
        int port;
        try (ServerSocket s = new ServerSocket(0)) {
            port = s.getLocalPort();
        }
        Process srv = new ProcessBuilder(server.getPath(), "--port", String.valueOf(port), "--no-router").directory(work)
                .redirectErrorStream(true).redirectOutput(new File(work, "server.log")).start();
        Thread.sleep(500);
        String addr = "127.0.0.1:" + port;
        try {
            byte[] a = c.random(8), b = c.random(8), ka = c.random(32), kb = c.random(32), pk = c.random(32);
            JSONObject sa = snap(a, ka, b, kb, pk, "424F42"), sb = snap(b, kb, a, ka, pk, "414C494345");
            int[][] rules = {{Tt.bit(Tt.RULE_OPEN)}, {Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME) | Tt.bit(Tt.RULE_PLUS)},
                    {Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME) | Tt.bit(Tt.RULE_PLUS) | Tt.bit(Tt.RULE_ELEMENTAL)},
                    {Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_REVERSE) | Tt.bit(Tt.RULE_FALLEN_ACE)}};
            int games = 0;
            for (int round = 0; round < 3; round++) {
                for (int[] r : rules) {
                    int[] ha = {1 + round, 12, 23, 34, 45}, hb = {6, 17 + round, 28, 39, 50};
                    JSONObject ga = TtFriends.invite(c, addr, sa, sa.getJSONArray("list").getJSONObject(0), "414C494345", ha, r[0]);
                    List<JSONObject> inv = TtFriends.invites(c, addr, sb);
                    JSONObject mine = null;
                    for (JSONObject i : inv) {
                        if (i.optString("g").equals(ga.optString("g"))) {
                            mine = i;
                        }
                    }
                    if (mine == null) {
                        fail("BOB never got the challenge");
                    }
                    JSONObject gb = TtFriends.accept(c, addr, mine, hb);
                    // ALICE learns BOB's hand from the record
                    JSONObject rec = TtFriends.read(c, addr, sa.getJSONArray("list").getJSONObject(0), ga.optString("g"));
                    ga.getJSONArray("hands").put(1, rec.getJSONArray("hand"));
                    int seq = 0;
                    while (true) {
                        String stA = TtFriendGame.status(ga), stB = TtFriendGame.status(gb);
                        if (!stA.equals("yourturn") && !stA.equals("theirturn")) {
                            String want = stA.equals("won") ? "lost" : stA.equals("lost") ? "won" : "draw";
                            if (!stB.equals(want)) {
                                fail("ALICE says " + stA + " but BOB says " + stB + " (" + r[0] + ")");
                            }
                            break;
                        }
                        boolean aliceMoves = stA.equals("yourturn");
                        if (aliceMoves == stB.equals("yourturn")) {
                            fail("both or neither to move: " + stA + " / " + stB);
                        }
                        JSONObject g = aliceMoves ? ga : gb, other = aliceMoves ? gb : ga;
                        long d = TtFriendGame.replay(g);
                        int[] p = Tt.ai(d, 2, seq * 31);
                        Tt.end(d);
                        seq++;
                        JSONObject friend = (aliceMoves ? sa : sb).getJSONArray("list").getJSONObject(0);
                        if (!TtFriends.move(c, addr, friend, g.optString("g"), seq, p[0], p[1])) {
                            fail("a move wasn't taken");
                        }
                        g.getJSONArray("moves").put(new JSONArray().put(p[0]).put(p[1]));
                        JSONObject seen = TtFriends.read(c, addr, (aliceMoves ? sb : sa).getJSONArray("list").getJSONObject(0), g.optString("g"));
                        other.put("moves", seen.getJSONArray("moves"));
                        // the two phones' boards agree (each its own seat 0: owners mirrored)
                        long da = TtFriendGame.replay(ga), db = TtFriendGame.replay(gb);
                        JSONArray ca = new JSONObject(Tt.state(da, 0)).getJSONArray("cells"), cb = new JSONObject(Tt.state(db, 0)).getJSONArray("cells");
                        Tt.end(da);
                        Tt.end(db);
                        for (int k = 0; k < 9; k++) {
                            JSONArray x = ca.getJSONArray(k), y = cb.getJSONArray(k);
                            if (x.getInt(0) != y.getInt(0) || (x.getInt(0) != 0 && x.getInt(1) == y.getInt(1))) {
                                fail("the boards differ at square " + k + " (rules " + r[0] + ")");
                            }
                        }
                    }
                    games++;
                }
            }
            // a turned-down challenge
            JSONObject ga = TtFriends.invite(c, addr, sa, sa.getJSONArray("list").getJSONObject(0), "414C494345", new int[]{1, 2, 3, 4, 5}, 1);
            for (JSONObject i : TtFriends.invites(c, addr, sb)) {
                if (i.optString("g").equals(ga.optString("g"))) {
                    TtFriends.decline(c, addr, i);
                }
            }
            if (TtFriends.read(c, addr, sa.getJSONArray("list").getJSONObject(0), ga.optString("g")).optInt("declined") != 1) {
                fail("the turned-down challenge");
            }
            System.out.println("TT FRIENDS TEST PASSED: " + games + " games by turns through the server, both sides agreeing, a challenge turned down");
        } finally {
            srv.destroy();
        }
    }
}
