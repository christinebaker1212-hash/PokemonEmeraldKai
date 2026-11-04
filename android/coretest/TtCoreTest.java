import com.moemon.companion.Tt;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * TRIPLE TRIAD on the phone runs the game's own engine (cpp/tt_jni.c over src/tt): the catalog, duels played to the
 * end by the AI on both sides with the events, opponents fitted like the game's, the daily puzzles (each with one or
 * two winning plays, the same every time for a day).
 */
public class TtCoreTest {
    static void fail(String why) {
        System.out.println("TT CORE TEST FAILED: " + why);
        System.exit(1);
    }

    public static void main(String[] args) throws Exception {
        JSONArray cards = new JSONArray(Tt.catalog());
        JSONObject words = new JSONObject(Tt.words());
        if (cards.length() < 400 || words.getJSONArray("rules").length() != 15) {
            fail("the catalog: " + cards.length() + " cards");
        }
        int[] a = {1, 2, 3, 4, 5}, b = {6, 7, 8, 9, 10};
        int games = 0, events = 0;
        for (int g = 0; g < 50; g++) {
            int rules = Tt.bit(Tt.RULE_OPEN) | (g % 2 == 0 ? Tt.bit(Tt.RULE_SAME) : Tt.bit(Tt.RULE_PLUS));
            long d = Tt.start(rules, 0, 1000 + g, a, b, g % 2);
            for (int move = 0; move < 9; move++) {
                int[] p = Tt.ai(d, 1 + g % 3, g * 31 + move);
                String ev = Tt.play(d, p[0], p[1]);
                if (ev == null) {
                    fail("the AI's play was refused");
                }
                events += new JSONArray(ev).length();
            }
            JSONObject st = new JSONObject(Tt.state(d, 0));
            if (st.optInt("over") != 1 || st.getJSONArray("score").getInt(0) + st.getJSONArray("score").getInt(1) != 10) {
                fail("a game didn't end right: " + st);
            }
            Tt.end(d);
            games++;
        }
        JSONObject opp = new JSONObject(Tt.opponent(0, 12345, 1, 20000, 1500, Tt.bit(Tt.RULE_OPEN)));
        if (opp.getJSONArray("hand").length() != 5) {
            fail("an opponent: " + opp);
        }
        long t0 = System.currentTimeMillis();
        for (int day = 20390; day < 20397; day++) {
            String p = Tt.puzzle(day);
            if (p == null || !p.equals(Tt.puzzle(day))) {
                fail("day " + day + "'s puzzle");
            }
            JSONObject pz = new JSONObject(p);
            int wins = pz.getJSONArray("wins").length();
            if (wins < 1 || wins > 2) {
                fail("a puzzle with " + wins + " winning plays");
            }
            // replay it: a winning play wins against the best defense
            long d = Tt.start(pz.getInt("rules"), 0, pz.getInt("seed"), ints(pz.getJSONArray("hand0")), ints(pz.getJSONArray("hand1")), 0);
            JSONArray mv = pz.getJSONArray("moves");
            for (int i = 0; i < mv.length(); i++) {
                Tt.play(d, mv.getJSONArray(i).getInt(0), mv.getJSONArray(i).getInt(1));
            }
            JSONArray w = pz.getJSONArray("wins").getJSONArray(0);
            Tt.play(d, w.getInt(0), w.getInt(1));
            while (new JSONObject(Tt.state(d, 0)).optInt("over") == 0) {
                int[] p2 = new JSONObject(Tt.state(d, 0)).optInt("toMove") == 1 ? Tt.best(d) : Tt.best(d);
                Tt.play(d, p2[0], p2[1]);
            }
            if (new JSONObject(Tt.state(d, 0)).optInt("winner") != 0) {
                fail("day " + day + ": the winning play didn't win");
            }
            Tt.end(d);
        }
        System.out.println("TT CORE TEST PASSED: " + cards.length() + " cards, " + games + " games (" + events
                + " events), an opponent (power " + opp.optInt("power") + "), 7 puzzles in " + (System.currentTimeMillis() - t0) + " ms");
    }

    static int[] ints(JSONArray a) throws Exception {
        int[] o = new int[a.length()];
        for (int i = 0; i < o.length; i++) {
            o[i] = a.getInt(i);
        }
        return o;
    }
}
