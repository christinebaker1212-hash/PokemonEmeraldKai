import com.moemon.companion.Ygo;

import org.json.JSONArray;
import org.json.JSONObject;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

/**
 * YU-GI-OH! through the app's JNI (cpp/ygo_jni.c) on a desktop JVM: the engine's pack handed over, the catalog, the
 * sets, deck checks, the GBA games' duelists; whole duels where one side (or both) answers every kind of question as
 * the phone's table would (random picks the engine takes: Ygo.check first), the other the game's AI; every duel ends,
 * a duel rebuilt from its answers ends the same (friends exchange only answers), hidden cards stay hidden.
 */
public final class YgoCoreTest {
    static int failures;

    static void check(boolean ok, String what) {
        if (!ok) {
            failures++;
            System.out.println("FAIL " + what);
        }
    }

    static int[] ints(JSONArray a) {
        int[] out = new int[a.length()];
        for (int i = 0; i < out.length; i++) {
            out[i] = a.optInt(i);
        }
        return out;
    }

    /** A random answer the engine takes, as a player tapping around would give: [picks..., zone] or null */
    static int[] randomAnswer(long d, JSONObject q, Random rnd, int[] kinds) throws Exception {
        JSONArray items = q.getJSONArray("items");
        int n = items.length(), type = q.getInt("type");
        for (int tries = 0; tries < 40; tries++) {
            List<Integer> picks = new ArrayList<>();
            switch (type) {
                case Ygo.Q_IDLE:
                case Ygo.Q_BATTLE:
                case Ygo.Q_POSITION:
                    if (n > 0) {
                        picks.add(rnd.nextInt(n));
                    }
                    break;
                case Ygo.Q_CHAIN:
                    if (n > 0 && rnd.nextInt(3) == 0) {
                        picks.add(rnd.nextInt(n));
                    }
                    break;
                case Ygo.Q_YESNO:
                    picks.add(rnd.nextInt(2));
                    break;
                case Ygo.Q_SORT: {
                    for (int i = 0; i < n; i++) {
                        picks.add(i);
                    }
                    Collections.shuffle(picks, rnd);
                    break;
                }
                case Ygo.Q_CARDS:
                case Ygo.Q_OPTION:
                case Ygo.Q_ZONE:
                case Ygo.Q_SUM: {
                    List<Integer> all = new ArrayList<>();
                    for (int i = 0; i < n; i++) {
                        all.add(i);
                    }
                    Collections.shuffle(all, rnd);
                    int lo = q.getInt("min"), hi = Math.min(q.getInt("max"), n);
                    int k = type == Ygo.Q_SUM ? 1 + rnd.nextInt(Math.max(1, n)) : hi <= lo ? lo : lo + rnd.nextInt(hi - lo + 1);
                    for (int i = 0; i < k && i < all.size(); i++) {
                        picks.add(all.get(i));
                    }
                    break;
                }
                default:
                    return null;   // (COUNTER, NAME: the phone's own screens; here the AI's)
            }
            int[] p = new int[picks.size()];
            for (int i = 0; i < p.length; i++) {
                p[i] = picks.get(i);
            }
            int zone = 0;
            if (Ygo.check(d, p, zone)) {
                kinds[type]++;
                int[] out = new int[p.length + 1];
                System.arraycopy(p, 0, out, 0, p.length);
                out[p.length] = zone;
                return out;
            }
        }
        return null;
    }

    public static void main(String[] args) throws Exception {
        byte[] pak = Files.readAllBytes(Paths.get(args[0], "graphics/ygo/edo.pak"));
        check(Ygo.setPak(pak), "the engine's pack");
        long t0 = System.nanoTime();
        JSONArray cat = new JSONArray(Ygo.catalog());
        int ready = 0;
        for (int i = 0; i < cat.length(); i++) {
            ready += cat.getJSONObject(i).optInt("ok");
        }
        System.out.println("catalog: " + cat.length() + " cards (" + ready + " playable) in " + (System.nanoTime() - t0) / 1000000 + " ms");
        check(cat.length() > 2000, "the catalog");
        JSONArray sets = new JSONArray(Ygo.sets());
        List<Integer> starters = new ArrayList<>();
        for (int i = 0; i < sets.length(); i++) {
            if (sets.getJSONObject(i).getInt("starter") == 1) {
                starters.add(i);
            }
        }
        check(starters.size() >= 2, "Starter Decks");
        JSONObject yugi = new JSONObject(Ygo.setDeck(starters.get(0))), kaiba = new JSONObject(Ygo.setDeck(starters.get(1)));
        check(Ygo.validate(ints(yugi.getJSONArray("main")), ints(yugi.getJSONArray("extra")), null) == null,
                "a Starter Deck is legal: " + Ygo.validate(ints(yugi.getJSONArray("main")), ints(yugi.getJSONArray("extra")), null));
        check(Ygo.validate(new int[]{1, 2, 3}, new int[0], null) != null, "a deck of 3: refused");
        int rating = Ygo.deckRating(ints(yugi.getJSONArray("main")));
        JSONObject opp = new JSONObject(Ygo.opponent(rating, 1, 77));
        System.out.println("Yugi's deck rated " + rating + "; a duelist: " + opp.getString("name") + " (" + opp.getString("game")
                + ") " + opp.getInt("rating"));
        check(Ygo.validate(ints(opp.getJSONArray("main")), ints(opp.getJSONArray("extra")), null) == null, "the duelist's deck is legal");

        Random rnd = new Random(11);
        int[] kinds = new int[16];
        int duels = 24, finished = 0, aiFallbacks = 0;
        for (int n = 0; n < duels; n++) {
            boolean both = n % 4 == 0;
            JSONObject d0 = n % 2 == 0 ? yugi : kaiba, d1 = n % 3 == 0 ? new JSONObject(Ygo.opponent(1500, 3, n)) : n % 2 == 0 ? kaiba : yugi;
            int seed = 4000 + n * 131;
            long d = Ygo.start(ints(d0.getJSONArray("main")), ints(d0.getJSONArray("extra")), ints(d1.getJSONArray("main")),
                    ints(d1.getJSONArray("extra")), seed, n & 1);
            check(d != 0, "a duel starts");
            List<int[]> log = new ArrayList<>();
            int steps = 0;
            while (steps++ < 6000) {
                JSONObject q = new JSONObject(Ygo.query(d));
                if (q.getInt("type") == Ygo.Q_NONE) {
                    break;
                }
                int who = q.getInt("player");
                int[] a = null;
                if (who == 0 || both) {
                    a = randomAnswer(d, q, rnd, kinds);
                    // (the random player keeps the duel moving: in the Main / Battle Phase it ends turns often)
                    if (a != null && (q.getInt("type") == Ygo.Q_IDLE || q.getInt("type") == Ygo.Q_BATTLE) && rnd.nextInt(3) == 0) {
                        a = null;
                    }
                }
                if (a == null) {
                    JSONObject ai = new JSONObject(Ygo.ai(d, 2));
                    JSONArray p = ai.getJSONArray("p");
                    a = new int[p.length() + 1];
                    for (int i = 0; i < p.length(); i++) {
                        a[i] = p.getInt(i);
                    }
                    a[p.length()] = ai.getInt("z");
                    if (who == 0 || both) {
                        aiFallbacks++;
                    }
                }
                int[] picks = java.util.Arrays.copyOf(a, a.length - 1);
                JSONObject r = new JSONObject(Ygo.answer(d, picks, a[a.length - 1]));
                if (r.getInt("ok") != 1) {
                    check(false, "an answer check() passed was refused: " + r.getString("error"));
                    break;
                }
                log.add(a);
            }
            JSONObject st = new JSONObject(Ygo.state(d, 0));
            if (st.getInt("over") == 1) {
                finished++;
            }
            // hidden: the other side's hand and its face-down cards
            JSONObject them = st.getJSONArray("players").getJSONObject(1);
            JSONArray objs = st.getJSONArray("objs");
            JSONArray hand = them.getJSONArray("hand");
            for (int i = 0; i < hand.length(); i++) {
                JSONObject o = objs.getJSONObject(hand.getInt(i));
                check(o.getInt("c") == 0 || o.getInt("up") == 1, "their hand is hidden");
            }
            // rebuilt from its answers: the same duel
            long again = Ygo.start(ints(d0.getJSONArray("main")), ints(d0.getJSONArray("extra")), ints(d1.getJSONArray("main")),
                    ints(d1.getJSONArray("extra")), seed, n & 1);
            for (int[] a : log) {
                JSONObject r = new JSONObject(Ygo.answer(again, java.util.Arrays.copyOf(a, a.length - 1), a[a.length - 1]));
                if (r.getInt("ok") != 1) {
                    check(false, "a replayed answer refused");
                    break;
                }
            }
            check(Ygo.hash(again) == Ygo.hash(d), "a replay agrees (duel " + n + ")");
            check(new JSONArray(Ygo.events(d, 0, 0)).length() > 0, "events");
            Ygo.end(d);
            Ygo.end(again);
        }
        System.out.println("duels " + duels + ", finished " + finished + "; answered: idle " + kinds[1] + ", battle " + kinds[2]
                + ", chain " + kinds[3] + ", yes/no " + kinds[4] + ", cards " + kinds[5] + ", option " + kinds[6] + ", position "
                + kinds[7] + ", zone " + kinds[8] + ", sum " + kinds[9] + ", sort " + kinds[10] + "; the AI's for the rest: " + aiFallbacks);
        check(finished == duels, "every duel ends (" + finished + ")");
        System.out.println(failures == 0 ? "YgoCoreTest: PASS" : "YgoCoreTest: " + failures + " failures");
        System.exit(failures == 0 ? 0 : 1);
    }
}
