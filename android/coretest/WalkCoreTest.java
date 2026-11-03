import com.moemon.companion.core.Walk;

import org.json.JSONArray;
import org.json.JSONObject;

import java.time.LocalDate;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.util.Random;

/**
 * The walk's own rules on the phone (core/Walk.java), a simulated week: readings spread over days away, the day's
 * 10,000, a phone restart, finds and meetings bought with steps (3 meetings a day), the homecoming order.
 */
public class WalkCoreTest {
    static void check(boolean ok, String what) {
        if (!ok) {
            System.out.println("WALK CORE TEST FAILED: " + what);
            System.exit(1);
        }
    }

    public static void main(String[] args) throws Exception {
        ZoneId utc = ZoneOffset.UTC;
        long day0 = LocalDate.of(2026, 11, 1).atStartOfDay(utc).toInstant().toEpochMilli();
        JSONObject game = new JSONObject().put("id", 77).put("sent", day0 / 1000 + 3600).put("cap", 20)
                .put("wild", new JSONArray("[[286,2,4],[288,3,3]]")).put("finds", new JSONArray("[13,14]"))
                .put("rules", new JSONArray("[10000,2000,3000,2500,3]"));
        Walk w = new Walk(game, utc);
        long boot = day0 - 86400000L;
        // the first reading only sets where counting starts
        w.reading(5000, boot, day0 + 7200000);
        check(w.steps() == 0, "the first reading counts nothing");
        w.reading(8000, boot, day0 + 8 * 3600000);
        check(w.steps() == 3000, "3,000 steps the first day (" + w.steps() + ")");
        // 40,000 more read three days later: spread over the four days, none past 10,000
        w.reading(48000, boot, day0 + 3 * 86400000L + 3600000);
        LocalDate d0 = LocalDate.of(2026, 11, 1);
        check(w.stepsOn(d0) == 10000 && w.stepsOn(d0.plusDays(1)) == 10000 && w.stepsOn(d0.plusDays(3)) == 10000,
                "spread and capped (" + w.stepsOn(d0) + ", " + w.stepsOn(d0.plusDays(1)) + ", " + w.stepsOn(d0.plusDays(3)) + ")");
        // the phone restarted: its counter began again
        long boot2 = day0 + 4 * 86400000L;
        w.reading(2500, boot2, day0 + 4 * 86400000L + 3600000);
        check(w.stepsOn(d0.plusDays(4)) == 2500, "after a restart the counter's own steps (" + w.stepsOn(d0.plusDays(4)) + ")");
        // the rest of the week, a long walk a day
        for (int d = 5; d < 7; d++) {
            w.add(d0.plusDays(d), 14000);
        }
        long steps = w.steps();
        check(steps == 40000 + 2500 + 20000, "a week's steps (" + steps + ")");
        // finds: one per 2,000 steps, from the pool only
        Random r = new Random(1);
        int found = 0;
        while (w.find(r) != 0) {
            found++;
        }
        check(found == steps / 2000, "finds by steps (" + found + ")");
        for (int i = 0; i < w.finds().length(); i++) {
            check(w.finds().optInt(i) == 13 || w.finds().optInt(i) == 14, "finds from the pool");
        }
        // meetings: 3 a day at most
        LocalDate today = d0.plusDays(6);
        int met = 0;
        int[] m;
        while ((m = w.meet(r, today)) != null) {
            check(m[0] == 286 ? m[1] >= 2 && m[1] <= 4 : m[1] == 3, "a meeting at the place's levels");
            w.met(m[0], m[1], met % 2 == 0, 4, today);
            met++;
        }
        check(met == 3, "3 meetings a day (" + met + ")");
        check(w.caught().length() == 2, "the caught ones listed (" + w.caught().length() + ")");
        // the homecoming order, and the walk kept on disk the same
        JSONObject home = w.homeOrder();
        check(home.getLong("walk") == 77 && home.getLong("steps") == steps && home.getJSONArray("finds").length() == found
                && home.getJSONArray("caught").length() == 2, "the homecoming order");
        Walk again = Walk.fromJson(new JSONObject(w.toJson().toString()), utc);
        check(again.steps() == steps && again.meetingsLeft(today) == 0 && again.findsLeft() == 0, "kept on disk");
        System.out.println("WALK CORE TEST PASSED: a simulated week (" + steps + " steps, " + found + " finds, 3 meetings)");
    }
}
