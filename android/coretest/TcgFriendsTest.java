import com.moemon.companion.NativeCrypto;
import com.moemon.companion.Tcg;
import com.moemon.companion.TcgFriendGame;
import com.moemon.companion.core.CardDuels;
import com.moemon.companion.core.Hex;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.File;
import java.net.ServerSocket;
import java.nio.file.Files;
import java.util.List;

/**
 * The POKéMON card game with a friend, a turn at a time through a real co-op server (core/CardDuels kind "tcg" + the
 * game's engine): ALICE challenges BOB with her deck, BOB accepts with his, and they play to the end with the game's AI
 * choosing for each (several moves in a row when a turn holds them, the other side's when they must promote); both
 * phones replay the same duel (the same rules state hash after every move) and see the same result. A turned-down
 * challenge shows as such.
 */
public class TcgFriendsTest {
    static final NativeCrypto c = new NativeCrypto();

    static void fail(String why) {
        System.out.println("TCG FRIENDS TEST FAILED: " + why);
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
        File server = new File(args[0]), work = Files.createTempDirectory("tcgfriends").toFile();
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
            String[][] pairs = {{"CharmanderAndFriends", "SquirtleAndFriends"}, {"BulbasaurAndFriends", "LightningAndFire"},
                    {"WaterAndFighting", "GoGoRainDance"}};
            int duels = 0, moves = 0;
            for (String[] pair : pairs) {
                int[] da = Tcg.namedDeck(pair[0]), db = Tcg.namedDeck(pair[1]);
                JSONObject ga = CardDuels.invite(c, addr, sa, toBob, "414C494345", "tcg",
                        new JSONObject().put("deck", ints(da)).put("prizes", 6));
                ga.put("prizes", 6).put("decks", new JSONArray().put(ints(da)).put(JSONObject.NULL));
                JSONObject mine = null;
                for (JSONObject i : CardDuels.invites(c, addr, sb, "tcg")) {
                    if (i.optString("g").equals(ga.optString("g"))) {
                        mine = i;
                    }
                }
                if (mine == null || mine.optJSONArray("deck") == null || mine.optJSONArray("deck").length() != 60) {
                    fail("BOB never got the challenge (with her deck)");
                }
                if (!"invited".equals(TcgFriendGame.status(ga))) {
                    fail("before BOB answers ALICE sees " + TcgFriendGame.status(ga));
                }
                CardDuels.accept(c, addr, mine, "tcg", new JSONObject().put("deck", ints(db)));
                JSONObject gb = new JSONObject().put("g", mine.optString("g")).put("seat", 1).put("seed", mine.optInt("seed"))
                        .put("prizes", mine.optInt("prizes")).put("decks", new JSONArray().put(mine.getJSONArray("deck")).put(ints(db)))
                        .put("moves", new JSONArray());
                // ALICE learns BOB's deck from the record (Companion.syncTcgGames)
                JSONObject rec = CardDuels.read(c, addr, toBob, "tcg", ga.optString("g"));
                ga.getJSONArray("decks").put(1, rec.getJSONObject("zero").getJSONArray("deck"));
                int seq = 0;
                while (true) {
                    String stA = TcgFriendGame.status(ga), stB = TcgFriendGame.status(gb);
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
                    if (seq > 2000) {
                        fail("a duel that never ends");
                    }
                    JSONObject g = aliceMoves ? ga : gb, other = aliceMoves ? gb : ga;
                    long d = TcgFriendGame.replay(g);
                    JSONObject action = new JSONObject(Tcg.ai(d, aliceMoves ? 3 : 1));
                    // (a move the rules refuse ends the turn, as on the game's table)
                    if (new JSONObject(Tcg.apply(Tcg.copy(d), action.toString())).optInt("ok") != 1) {
                        action = new JSONObject().put("t", new JSONObject(Tcg.state(d, 0)).optInt("phase") == Tcg.SETUP
                                ? Tcg.A_SETUP_DONE : Tcg.A_PASS).put("x", new JSONArray());
                    }
                    Tcg.end(d);
                    seq++;
                    if (!CardDuels.move(c, addr, aliceMoves ? toBob : toAlice, "tcg", g.optString("g"), seq, action)) {
                        fail("a move wasn't taken");
                    }
                    g.getJSONArray("moves").put(action);
                    JSONObject seen = CardDuels.read(c, addr, aliceMoves ? toAlice : toBob, "tcg", g.optString("g"));
                    other.put("moves", seen.getJSONArray("moves"));
                    long x = TcgFriendGame.replay(ga), y = TcgFriendGame.replay(gb);
                    if (Tcg.hash(x) != Tcg.hash(y)) {
                        System.out.println("A: " + ga.getJSONArray("moves"));
                        System.out.println("B: " + gb.getJSONArray("moves"));
                        fail("the two phones' duels differ after move " + seq);
                    }
                    Tcg.end(x);
                    Tcg.end(y);
                }
                moves += seq;
                duels++;
            }
            // a turned-down challenge
            JSONObject ga = CardDuels.invite(c, addr, sa, toBob, "414C494345", "tcg",
                    new JSONObject().put("deck", ints(Tcg.namedDeck("CharmanderAndFriends"))).put("prizes", 6));
            List<JSONObject> inv = CardDuels.invites(c, addr, sb, "tcg");
            for (JSONObject i : inv) {
                if (i.optString("g").equals(ga.optString("g"))) {
                    CardDuels.decline(c, addr, i, "tcg");
                }
            }
            if (CardDuels.read(c, addr, toBob, "tcg", ga.optString("g")).optInt("declined") != 1) {
                fail("the turned-down challenge");
            }
            // (TRIPLE TRIAD's invites are another kind: not listed as card duels)
            if (!CardDuels.invites(c, addr, sb, "tt").isEmpty()) {
                fail("a card duel listed as a TRIPLE TRIAD game");
            }
            System.out.println("TCG FRIENDS TEST PASSED: " + duels + " duels by turns (" + moves
                    + " moves) through the server, both phones agreeing after every move, a challenge turned down");
        } finally {
            srv.destroy();
        }
    }
}
