import com.moemon.companion.Tcg;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * The POKéMON card game through the app's JNI (cpp/tcg_jni.c) on a desktop JVM: the catalog, deck checks, CARD
 * SCALING's opponents, and whole duels where a side moves only through flows (the duel board's choices) with random
 * legal answers against the game's AI, or both sides do. Every action a flow builds must be one the rules take; a
 * duel replayed from its actions ends the same (two phones agree); hidden cards stay hidden.
 */
public final class TcgCoreTest {
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

    /** One move for SEAT through a flow, picked at random among what options() says can be done: the action */
    static JSONObject randomMove(long duel, int seat, Random rnd, int[] stats) throws Exception {
        JSONObject o = new JSONObject(Tcg.options(duel, seat));
        List<int[]> moves = new ArrayList<>();   // [kind, index]
        JSONArray hand = o.getJSONArray("hand");
        for (int i = 0; i < hand.length(); i++) {
            if (hand.getJSONObject(i).optInt("ok") == 1 && hand.getJSONObject(i).optInt("use") == 1) {
                moves.add(new int[]{Tcg.FLOW_HAND, i});
            }
        }
        JSONArray attacks = o.getJSONArray("attacks");
        for (int i = 0; i < attacks.length(); i++) {
            if (attacks.getJSONObject(i).optInt("ok") == 1) {
                moves.add(new int[]{Tcg.FLOW_ATTACK, i});
                moves.add(new int[]{Tcg.FLOW_ATTACK, i});   // (attacks more often: duels end)
            }
        }
        if (o.getJSONObject("retreat").optInt("ok") == 1 && rnd.nextInt(4) == 0) {
            moves.add(new int[]{Tcg.FLOW_RETREAT, 0});
        }
        JSONArray powers = o.getJSONArray("powers");
        for (int i = 0; i < powers.length(); i++) {
            if (powers.getJSONObject(i).optInt("ok") == 1) {
                moves.add(new int[]{Tcg.FLOW_POWER, powers.getJSONObject(i).optInt("slot")});
            }
        }
        JSONObject st = new JSONObject(Tcg.state(duel, seat));
        JSONArray field = st.getJSONArray("p").getJSONObject(seat).getJSONArray("field");
        if (o.optInt("promote") == 1) {
            moves.clear();
            for (int s = 1; s < 6; s++) {
                if (!field.isNull(s)) {
                    moves.add(new int[]{Tcg.FLOW_PROMOTE, s});
                }
            }
        } else if (o.optInt("setup") == 1) {
            if (o.optInt("ready") == 1 && (moves.isEmpty() || rnd.nextInt(3) == 0)) {
                moves.clear();
                moves.add(new int[]{Tcg.FLOW_READY, 0});
            }
        } else if (o.optInt("end") == 1 && (moves.isEmpty() || rnd.nextInt(8) == 0)) {
            moves.add(new int[]{Tcg.FLOW_PASS, 0});
        }
        if (moves.isEmpty()) {
            return null;
        }
        int[] m = moves.get(rnd.nextInt(moves.size()));
        long flow = Tcg.flowNew();
        try {
            JSONObject q = new JSONObject(Tcg.flowBegin(flow, duel, seat, m[0], m[1]));
            for (int guard = 0; guard < 20; guard++) {
                String kind = q.getString("q");
                if (kind.equals("done")) {
                    stats[m[0]]++;
                    return q.getJSONObject("action");
                }
                if (kind.equals("fail")) {
                    stats[9]++;
                    return null;   // (a Trainer whose choices aren't there: the board says so and nothing happens)
                }
                if (kind.equals("slot")) {
                    int mask = q.getInt("mask");
                    List<Integer> ok = new ArrayList<>();
                    for (int s = 0; s < 6; s++) {
                        if ((mask >> s & 1) != 0) {
                            ok.add(s);
                        }
                    }
                    q = new JSONObject(Tcg.flowSlot(flow, duel, ok.get(rnd.nextInt(ok.size()))));
                } else if (kind.equals("pick")) {
                    JSONArray cards = q.getJSONArray("cards");
                    List<Integer> ok = new ArrayList<>();
                    for (int i = 0; i < cards.length(); i++) {
                        if (cards.getJSONArray(i).getInt(2) == 1) {
                            ok.add(cards.getJSONArray(i).getInt(0));
                        }
                    }
                    java.util.Collections.shuffle(ok, rnd);
                    List<Integer> chosen = new ArrayList<>();
                    if (q.getInt("value") > 0) {
                        int v = 0;
                        for (int p : ok) {
                            if (v >= q.getInt("value")) {
                                break;
                            }
                            chosen.add(p);
                            v += Tcg.cardOf(duel, q.getInt("owner"), p) == 7 ? 2 : 1;
                        }
                    } else {
                        int lo = q.getInt("min"), hi = Math.min(q.getInt("max"), ok.size());
                        int n = hi <= lo ? Math.min(lo, ok.size()) : lo + rnd.nextInt(hi - lo + 1);
                        for (int i = 0; i < n; i++) {
                            chosen.add(ok.get(i));
                        }
                    }
                    int[] c = new int[chosen.size()];
                    for (int i = 0; i < c.length; i++) {
                        c[i] = chosen.get(i);
                    }
                    JSONObject next = new JSONObject(Tcg.flowPick(flow, duel, c));
                    if (next.getString("q").equals("pick") && !next.getString("error").isEmpty()) {
                        stats[10]++;   // (a searched deck without the card it wants: nothing to take)
                        return null;
                    }
                    q = next;
                } else {
                    JSONArray menu = q.getJSONArray("menu");
                    List<Integer> ok = new ArrayList<>();
                    for (int i = 0; i < menu.length(); i++) {
                        if (menu.getJSONArray(i).getInt(2) == 1) {
                            ok.add(menu.getJSONArray(i).getInt(1));
                        }
                    }
                    if (ok.isEmpty()) {
                        return null;
                    }
                    q = new JSONObject(Tcg.flowMenu(flow, duel, ok.get(rnd.nextInt(ok.size()))));
                }
            }
            check(false, "a flow that never ends");
            return null;
        } finally {
            Tcg.flowEnd(flow);
        }
    }

    public static void main(String[] args) throws Exception {
        JSONArray cat = new JSONArray(Tcg.catalog());
        check(cat.length() == 442, "442 cards (" + cat.length() + ")");
        check(cat.getJSONObject(7).getString("n").equals("Bulbasaur"), "card 8 is Bulbasaur");
        check(new JSONArray(Tcg.packs()).length() == Tcg.PACKS, "11 boosters");
        int[] charmander = Tcg.namedDeck("CharmanderAndFriends"), squirtle = Tcg.namedDeck("SquirtleAndFriends");
        check(charmander != null && squirtle != null, "the starter decks");
        check(Tcg.validate(charmander, null) == null, "a starter deck is legal");
        int[] owned = new int[443];
        check(Tcg.validate(charmander, owned) != null, "not owned: refused (" + Tcg.validate(charmander, owned) + ")");
        int[] bad = charmander.clone();
        for (int i = 0; i < 60; i++) {
            bad[i] = 6;   // (sixty Psychic Energy: no Basic Pokemon)
        }
        check(Tcg.validate(bad, null) != null, "no Basic Pokemon: refused");

        long t0 = System.nanoTime();
        int rating = Tcg.rateDeck(charmander);
        System.out.println("CharmanderAndFriends rated " + rating + " in " + (System.nanoTime() - t0) / 1000000 + " ms");
        check(rating > 1000 && rating < 2000, "a sensible rating");
        for (int badges = 0; badges <= 8; badges += 4) {
            JSONObject opp = new JSONObject(Tcg.opponent(rating, badges, 12345 + badges));
            check(Tcg.validate(ints(opp.getJSONArray("deck")), null) == null, "the opponent's deck is legal");
            System.out.println("  " + badges + " badges: " + opp.getString("name") + " " + opp.getInt("rating") + " level "
                    + opp.getInt("level"));
        }

        // whole duels: seat 0 through flows with random answers, seat 1 the AI (or flows too)
        Random rnd = new Random(7);
        int[] stats = new int[12];
        int finished = 0, rejected = 0, duels = 150;
        for (int n = 0; n < duels; n++) {
            int seed = 1000 + n * 7919;
            boolean both = n % 3 == 0;
            int[] d0 = n % 2 == 0 ? charmander : squirtle, d1 = n % 2 == 0 ? squirtle : Tcg.namedDeck("BulbasaurAndFriends");
            if (n % 5 == 4) {
                d1 = ints(new JSONObject(Tcg.opponent(1500, 8, n)).getJSONArray("deck"));
            }
            long duel = Tcg.start(d0, d1, seed, n % 4 == 0 ? 4 : 6);
            check(duel != 0, "a duel starts");
            List<String> log = new ArrayList<>();
            int steps = 0;
            while (Tcg.actor(duel) >= 0 && steps < 3000) {
                steps++;
                int seat = Tcg.actor(duel);
                JSONObject action;
                if (seat == 0 || both) {
                    action = randomMove(duel, seat, rnd, stats);
                    if (action == null) {
                        // nothing worth doing (or a fizzled choice): end the turn as the board would
                        JSONObject o = new JSONObject(Tcg.options(duel, seat));
                        int kind = o.optInt("ready") == 1 ? Tcg.FLOW_READY : o.optInt("end") == 1 ? Tcg.FLOW_PASS : -1;
                        if (kind < 0) {
                            action = new JSONObject(Tcg.ai(duel, 3));   // (promotion with nothing chosen: rare)
                        } else {
                            long f = Tcg.flowNew();
                            action = new JSONObject(Tcg.flowBegin(f, duel, seat, kind, 0)).getJSONObject("action");
                            Tcg.flowEnd(f);
                        }
                    }
                } else {
                    action = new JSONObject(Tcg.ai(duel, n % 4));
                }
                JSONObject r = new JSONObject(Tcg.apply(duel, action.toString()));
                if (r.getInt("ok") != 1) {
                    if (seat == 0 || both) {
                        rejected++;
                        System.out.println("  rejected (duel " + n + "): " + action + ": " + r.getString("error"));
                    }
                    // the AI's own refusals end its turn the way the game's table does
                    JSONObject pass = new JSONObject().put("t", Tcg.A_PASS).put("x", new JSONArray());
                    JSONObject st = new JSONObject(Tcg.state(duel, seat));
                    if (st.getInt("phase") == Tcg.SETUP) {
                        pass.put("t", Tcg.A_SETUP_DONE);
                    }
                    if (new JSONObject(Tcg.apply(duel, pass.toString())).getInt("ok") != 1) {
                        break;
                    }
                    log.add(pass.toString());
                    continue;
                }
                log.add(action.toString());
            }
            JSONObject end = new JSONObject(Tcg.state(duel, 0));
            if (end.getInt("phase") == Tcg.FINISHED) {
                finished++;
            }
            // the same duel replayed from its actions ends the same (what a friend's phone does)
            long again = Tcg.start(d0, d1, seed, n % 4 == 0 ? 4 : 6);
            for (String a : log) {
                check(new JSONObject(Tcg.apply(again, a)).getInt("ok") == 1, "a replayed action is taken");
            }
            check(Tcg.hash(again) == Tcg.hash(duel), "a replay agrees (duel " + n + ")");
            // hidden cards: the other side's hand and both decks never shown
            JSONObject seen = new JSONObject(Tcg.state(duel, 0));
            JSONArray theirs = seen.getJSONArray("p").getJSONObject(1).getJSONArray("hand");
            boolean hidden = true;
            for (int i = 0; i < theirs.length(); i++) {
                hidden &= theirs.getJSONArray(i).getInt(1) == 0;
            }
            JSONArray f1 = seen.getJSONArray("p").getJSONObject(1).getJSONArray("field");
            boolean clairvoyance = false;
            for (int s = 0; s < 6; s++) {
                clairvoyance |= !f1.isNull(s) && f1.getJSONObject(s).optInt("power") != 0;
            }
            check(hidden || clairvoyance || theirs.length() == 0, "their hand is hidden");
            JSONArray events = new JSONArray(Tcg.events(duel, 0, 0));
            check(events.length() > 0, "events");
            Tcg.end(duel);
            Tcg.end(again);
        }
        System.out.println("duels " + duels + ", finished " + finished + ", flow moves: hand " + stats[0] + ", attack "
                + stats[1] + ", retreat " + stats[2] + ", power " + stats[3] + ", promote " + stats[4] + ", pass " + stats[6]
                + ", ready " + stats[7] + "; fizzled " + stats[9] + ", empty searches " + stats[10]);
        check(rejected == 0, rejected + " flow-built actions the rules refused");
        check(finished >= duels * 9 / 10, "duels end (" + finished + ")");
        check(stats[1] > 0 && stats[0] > 0 && stats[4] > 0, "every kind of move came up");
        System.out.println(failures == 0 ? "TcgCoreTest: PASS" : "TcgCoreTest: " + failures + " failures");
        System.exit(failures == 0 ? 0 : 1);
    }
}
