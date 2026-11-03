import com.moemon.companion.NativeCrypto;
import com.moemon.companion.Ygo;
import com.moemon.companion.YgoFriendGame;
import com.moemon.companion.core.CardDuels;
import com.moemon.companion.core.Hex;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.util.List;

/**
 * YU-GI-OH! with a friend, a move at a time through a real co-op server (core/CardDuels kind "ygo" + the game's
 * engine): ALICE challenges BOB with her deck, BOB accepts with his, and they play to the end with the game's AI
 * answering for each (every question goes to the phone it's for: chains and responses in the other's turn too); both
 * phones rebuild the same duel from its answers (the same state hash after every answer) and see the same result; a
 * concession travels as its own move. A turned-down challenge shows as such.
 */
public class YgoFriendsTest {
    static final NativeCrypto c = new NativeCrypto();

    static void fail(String why) {
        System.out.println("YGO FRIENDS TEST FAILED: " + why);
        System.exit(1);
    }

    static JSONObject snap(byte[] me, byte[] myKey, byte[] peer, byte[] peerKey, byte[] pk, String peerName) throws Exception {
        return new JSONObject().put("me", Hex.of(me)).put("inbox", Hex.of(c.derive(myKey, "inbox")))
                .put("list", new JSONArray().put(new JSONObject().put("id", Hex.of(peer)).put("key", Hex.of(peerKey))
                        .put("pk", Hex.of(pk)).put("n", peerName)));
    }

    static JSONArray ints(int[] a) {
        JSONArray o = new JSONArray();
        for (int x : a) {
            o.put(x);
        }
        return o;
    }

    public static void main(String[] args) throws Exception {
        File server = new File(args[0]), work = Files.createTempDirectory("ygofriends").toFile();
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
            JSONObject toBob = sa.getJSONArray("list").getJSONObject(0), toAlice = sb.getJSONArray("list").getJSONObject(0);
            Ygo.setPak(Files.readAllBytes(java.nio.file.Paths.get(args[1], "graphics/ygo/edo.pak")));
            JSONArray sets = new JSONArray(Ygo.sets());
            java.util.List<Integer> starters = new java.util.ArrayList<>();
            for (int i = 0; i < sets.length(); i++) {
                if (sets.getJSONObject(i).getInt("starter") == 1) {
                    starters.add(i);
                }
            }
            int duels = 0, moves = 0;
            for (int n = 0; n < 3; n++) {
                JSONObject a0 = new JSONObject(Ygo.setDeck(starters.get(n % starters.size()))),
                        b0 = new JSONObject(Ygo.setDeck(starters.get((n + 1) % starters.size())));
                JSONArray da = new JSONArray().put(a0.getJSONArray("main")).put(a0.getJSONArray("extra")),
                        db = new JSONArray().put(b0.getJSONArray("main")).put(b0.getJSONArray("extra"));
                JSONObject ga = CardDuels.invite(c, addr, sa, toBob, "414C494345", "ygo", new JSONObject().put("deck", da));
                ga.put("decks", new JSONArray().put(da).put(JSONObject.NULL));
                JSONObject mine = null;
                for (JSONObject i : CardDuels.invites(c, addr, sb, "ygo")) {
                    if (i.optString("g").equals(ga.optString("g"))) {
                        mine = i;
                    }
                }
                if (mine == null || mine.optJSONArray("deck") == null) {
                    fail("BOB never got the challenge (with her deck)");
                }
                CardDuels.accept(c, addr, mine, "ygo", new JSONObject().put("deck", db));
                JSONObject gb = new JSONObject().put("g", mine.optString("g")).put("seat", 1).put("seed", mine.optInt("seed"))
                        .put("first", mine.optInt("first")).put("decks", new JSONArray().put(mine.getJSONArray("deck")).put(db))
                        .put("moves", new JSONArray());
                JSONObject rec = CardDuels.read(c, addr, toBob, "ygo", ga.optString("g"));
                ga.getJSONArray("decks").put(1, rec.getJSONObject("zero").getJSONArray("deck"));
                int seq = 0;
                boolean conceded = false;
                while (true) {
                    String stA = YgoFriendGame.status(ga), stB = YgoFriendGame.status(gb);
                    if (!stA.equals("yourturn") && !stA.equals("theirturn")) {
                        String want = stA.equals("won") ? "lost" : "won";
                        if (!stB.equals(want)) {
                            fail("ALICE says " + stA + " but BOB says " + stB);
                        }
                        break;
                    }
                    boolean aliceMoves = stA.equals("yourturn");
                    if (aliceMoves == stB.equals("yourturn")) {
                        fail("both or neither to move: " + stA + " / " + stB);
                    }
                    if (seq > 4000) {
                        fail("a duel that never ends");
                    }
                    JSONObject g = aliceMoves ? ga : gb, other = aliceMoves ? gb : ga;
                    JSONObject move;
                    if (n == 2 && seq == 60 && !aliceMoves) {
                        // (the last duel: BOB concedes on his move)
                        move = new JSONObject().put("concede", 1);
                        conceded = true;
                    } else {
                        long d = YgoFriendGame.replay(g);
                        JSONObject ai = new JSONObject(Ygo.ai(d, aliceMoves ? 3 : 1));
                        Ygo.end(d);
                        move = new JSONObject().put("p", ai.getJSONArray("p")).put("z", ai.getInt("z"));
                    }
                    seq++;
                    if (!CardDuels.move(c, addr, aliceMoves ? toBob : toAlice, "ygo", g.optString("g"), seq, move)) {
                        fail("a move wasn't taken");
                    }
                    g.getJSONArray("moves").put(move);
                    JSONObject seen = CardDuels.read(c, addr, aliceMoves ? toAlice : toBob, "ygo", g.optString("g"));
                    other.put("moves", seen.getJSONArray("moves"));
                    long x = YgoFriendGame.replay(ga), y = YgoFriendGame.replay(gb);
                    if (Ygo.hash(x) != Ygo.hash(y)) {
                        fail("the two phones' duels differ after move " + seq);
                    }
                    Ygo.end(x);
                    Ygo.end(y);
                    if (conceded && n == 2) {
                        if (!YgoFriendGame.status(ga).equals("won")) {
                            fail("BOB's concession: ALICE sees " + YgoFriendGame.status(ga));
                        }
                    }
                }
                moves += seq;
                duels++;
            }
            // a turned-down challenge
            JSONObject ga = CardDuels.invite(c, addr, sa, toBob, "414C494345", "ygo", new JSONObject().put("deck", new JSONArray()));
            List<JSONObject> inv = CardDuels.invites(c, addr, sb, "ygo");
            for (JSONObject i : inv) {
                if (i.optString("g").equals(ga.optString("g"))) {
                    CardDuels.decline(c, addr, i, "ygo");
                }
            }
            if (CardDuels.read(c, addr, toBob, "ygo", ga.optString("g")).optInt("declined") != 1) {
                fail("the turned-down challenge");
            }
            if (!CardDuels.invites(c, addr, sb, "tcg").isEmpty()) {
                fail("a YU-GI-OH! duel listed as a POKéMON card duel");
            }
            System.out.println("YGO FRIENDS TEST PASSED: " + duels + " duels by turns (" + moves
                    + " answers) through the server, both phones agreeing after every one, a concession, a challenge turned down");
        } finally {
            srv.destroy();
        }
    }
}
