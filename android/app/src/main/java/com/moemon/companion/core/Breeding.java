package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * The breeding calculator (the BREEDING page): how a POKéMON's IVs reach another kind, egg by egg, by the game's own
 * rules (daycare.c): an egg is the mother's kind at its first stage (the other parent's with DITTO); two parents need
 * an egg group in common and opposite genders; the UNDISCOVERED group never breeds, genderless kinds only with DITTO;
 * NIDORAN's and VOLBEAT / ILLUMISE's eggs are either kind; MANAPHY's egg is PHIONE. So a POKéMON carries its IVs into
 * another kind as the father: his mate's kind hatches, and a son of that kind carries them on.
 *
 * <p>Kinds are taken by families (every stage of an evolution line: a hatchling can be evolved before it breeds); a
 * route is the fewest eggs from the POKéMON the player starts with to an egg of the target's family, and among those
 * the one whose mothers the player has caught most of. Each step lists every mother that would do, caught ones first
 * (CAUGHT: the POKéDEX's bits by national number, as the snapshot sends them).
 */
public final class Breeding {
    public static final int MALE_ONLY = 0, FEMALE_ONLY = 254, GENDERLESS = 255;
    public static final int EG_DITTO = 13, EG_UNDISCOVERED = 15;
    private static final int DEX_NIDORAN_F = 29, DEX_NIDORAN_M = 32, DEX_VOLBEAT = 313, DEX_ILLUMISE = 314,
            DEX_PHIONE = 489, DEX_MANAPHY = 490, DEX_DITTO = 132;

    /** Why there's no route: none, a choice missing, FROM can't breed, TO hatches from no egg, TO takes nothing from
     * another kind (genderless / males only), FROM has no gender (only DITTO, its own kind), no chain at all */
    public static final int OK = 0, NOT_CHOSEN = 1, FROM_NO_EGGS = 2, TO_NO_EGG = 3, TO_ONLY_OWN = 4,
            FROM_GENDERLESS = 5, NO_CHAIN = 6;

    private final int count;
    private final int[] gender, eg0, eg1, dex, root, prev;
    private final boolean[] real, hasFather, isMother;
    private final int[] motherGroups;   // per family: the egg groups (bits) of the kinds whose females hatch it
    private final int[] dexToSpecies;
    private final int[][] eggMoves, levelMoves;
    private final String[] tmMask;
    private final int[] tmMoves;
    private String caught;

    /** DATA: data.json (its species, "egg" and "tms") */
    public Breeding(JSONObject data) {
        this(data.optJSONArray("species"), data.optInt("egg"), data.optJSONArray("tms"));
    }

    /** SPECIES: data.json's species list (index = species number); EGG: its "egg" (the forms after it aren't kinds) */
    public Breeding(JSONArray species, int egg, JSONArray tms) {
        count = Math.min(species.length(), egg > 0 ? egg : species.length());
        eggMoves = new int[count][];
        levelMoves = new int[count][];
        tmMask = new String[count];
        tmMoves = new int[tms == null ? 0 : tms.length()];
        for (int i = 0; i < tmMoves.length; i++) {
            tmMoves[i] = tms.optInt(i);
        }
        gender = new int[count];
        eg0 = new int[count];
        eg1 = new int[count];
        dex = new int[count];
        root = new int[count];
        prev = new int[count];
        real = new boolean[count];
        hasFather = new boolean[count];
        isMother = new boolean[count];
        motherGroups = new int[count];
        Arrays.fill(prev, -1);
        int maxDex = 0;
        for (int s = 1; s < count; s++) {
            JSONObject o = species.optJSONObject(s);
            JSONArray eg = o == null ? null : o.optJSONArray("eg");
            if (eg == null) {
                continue;
            }
            dex[s] = o.optInt("dex");
            gender[s] = o.optInt("g");
            eg0[s] = eg.optInt(0);
            eg1[s] = eg.optInt(1);
            real[s] = dex[s] > 0;
            maxDex = Math.max(maxDex, dex[s]);
            eggMoves[s] = ints(o.optJSONArray("egm"), -1);
            levelMoves[s] = ints(o.optJSONArray("lv"), 1);
            tmMask[s] = o.optString("tm");
        }
        dexToSpecies = new int[maxDex + 1];
        for (int s = count - 1; s > 0; s--) {
            if (real[s]) {
                dexToSpecies[dex[s]] = s;
            }
        }
        for (int s = 1; s < count; s++) {
            JSONObject o = species.optJSONObject(s);
            JSONArray evo = o == null || !real[s] ? null : o.optJSONArray("evo");
            for (int e = 0; evo != null && e < evo.length(); e++) {
                JSONArray one = evo.optJSONArray(e);
                int into = one == null ? 0 : one.optInt(2);
                if (into > 0 && into < count && real[into] && prev[into] < 0 && into != s) {
                    prev[into] = s;
                }
            }
        }
        for (int s = 0; s < count; s++) {
            int r = s, guard = 0;
            while (prev[r] >= 0 && guard++ < 8) {
                r = prev[r];
            }
            root[s] = r;
        }
        for (int s = 1; s < count; s++) {
            if (breeds(s) && canBeMale(s)) {
                hasFather[root[s]] = true;
            }
            isMother[s] = breeds(s) && canBeFemale(s);
        }
        for (int s = 1; s < count; s++) {
            if (isMother[s]) {
                for (int h : hatches(s)) {
                    motherGroups[root[h]] |= (1 << eg0[s]) | (1 << eg1[s]);
                }
            }
        }
    }

    /** A JSON array's numbers (AT >= 0: each one's entry AT, as "lv"'s [level, move]) */
    private static int[] ints(JSONArray a, int at) {
        int[] out = new int[a == null ? 0 : a.length()];
        for (int i = 0; i < out.length; i++) {
            out[i] = at < 0 ? a.optInt(i) : a.optJSONArray(i) == null ? 0 : a.optJSONArray(i).optInt(at);
        }
        return out;
    }

    public void setCaught(String hex) {
        caught = hex;
    }

    public boolean caught(int species) {
        int d = species > 0 && species < count ? dex[species] : 0;
        int i = (d - 1) / 4;
        if (caught == null || d <= 0 || i >= caught.length()) {
            return false;
        }
        int nib = Character.digit(caught.charAt(i), 16);
        return nib > 0 && (nib & (8 >> ((d - 1) % 4))) != 0;
    }

    public boolean isReal(int s) {
        return s > 0 && s < count && real[s];
    }

    public int family(int s) {
        return root[s];
    }

    public int gender(int s) {
        return gender[s];
    }

    public int[] groups(int s) {
        return eg1[s] != eg0[s] ? new int[]{eg0[s], eg1[s]} : new int[]{eg0[s]};
    }

    public boolean breeds(int s) {
        return isReal(s) && eg0[s] != EG_UNDISCOVERED && eg0[s] != EG_DITTO && eg0[s] != 0;
    }

    public boolean isDitto(int s) {
        return isReal(s) && dex[s] == DEX_DITTO;
    }

    public boolean canBeMale(int s) {
        return gender[s] != FEMALE_ONLY && gender[s] != GENDERLESS;
    }

    public boolean canBeFemale(int s) {
        return gender[s] != MALE_ONLY && gender[s] != GENDERLESS;
    }

    public int speciesOfDex(int d) {
        return d > 0 && d < dexToSpecies.length ? dexToSpecies[d] : 0;
    }

    public int dexOf(int s) {
        return s > 0 && s < count ? dex[s] : 0;
    }

    public int count() {
        return count;
    }

    /** What a mother of kind S hatches (its family's first stage; NIDORAN / VOLBEAT and ILLUMISE: either one) */
    public int[] hatches(int s) {
        int r = root[s], d = dex[r];
        if (d == DEX_NIDORAN_F || d == DEX_NIDORAN_M) {
            return new int[]{speciesOfDex(DEX_NIDORAN_F), speciesOfDex(DEX_NIDORAN_M)};
        }
        if (d == DEX_VOLBEAT || d == DEX_ILLUMISE) {
            return new int[]{speciesOfDex(DEX_ILLUMISE), speciesOfDex(DEX_VOLBEAT)};
        }
        if (d == DEX_MANAPHY) {
            return new int[]{speciesOfDex(DEX_PHIONE)};
        }
        return new int[]{r};
    }

    private static boolean overlap(int a0, int a1, int b0, int b1) {
        return a0 == b0 || a0 == b1 || a1 == b0 || a1 == b1;
    }

    private boolean later(int from, int s) {
        for (int p = prev[s], guard = 0; p >= 0 && guard < 8; p = prev[p], guard++) {
            if (p == from) {
                return true;
            }
        }
        return false;
    }

    /** Family F's kinds that breed and can be a father (a son can be evolved first); FROM: only it and what it becomes */
    private List<Integer> fathers(int f, int from) {
        List<Integer> out = new ArrayList<>();
        for (int s = 1; s < count; s++) {
            if (root[s] == f && breeds(s) && canBeMale(s) && (from < 0 || s == from || later(from, s))) {
                out.add(s);
            }
        }
        return out;
    }

    private int fatherFor(List<Integer> dads, int mother) {
        for (int d : dads) {
            if (overlap(eg0[d], eg1[d], eg0[mother], eg1[mother])) {
                return d;
            }
        }
        return -1;
    }

    // ------------------------------------------------------------------------------------------------- routes

    /** One egg: a father of kind FATHER (carrying the IVs) and a mother of kind MOTHER hatch HATCH */
    public static final class Step {
        public int father;          // (0 when the player's own POKéMON is the mother: any father sharing a group)
        public int mother;
        public int hatch;
        public int group;           // the egg group they share (EG_DITTO: with DITTO)
        public boolean herOwn;      // the player's POKéMON (female) is the mother
        public final List<Integer> mothers = new ArrayList<>();   // every kind that would do, caught first
    }

    public static final class Route {
        public final List<Step> steps = new ArrayList<>();
        public int caughtMothers;
    }

    /** Why the last routes() found none (OK when it did) */
    public int why;

    /**
     * Up to MAX routes (the fewest eggs; then the most caught mothers; the others without the first's bridges) from a
     * POKéMON of kind FROM and gender G (0 male, 254 female, 255 genderless; -1: male if it can be) to an egg of TO's
     * family.
     */
    public List<Route> routes(int from, int g, int to, int max) {
        List<Route> out = new ArrayList<>();
        why = OK;
        if (!isReal(from) || !isReal(to)) {
            why = NOT_CHOSEN;
            return out;
        }
        int target = root[to];
        if (g < 0) {
            g = canBeMale(from) ? 0 : gender[from];
        }
        if (isDitto(from)) {   // (DITTO breeds with anything: its IVs go straight into the egg)
            if (!breeds(to)) {
                why = TO_NO_EGG;
                return out;
            }
            out.add(single(from, to, hatches(to)[0], EG_DITTO));
            return out;
        }
        if (!breeds(from)) {
            why = FROM_NO_EGGS;
            return out;
        }
        boolean hatchable = false, fromOthers = false;
        for (int s = 1; s < count; s++) {
            if (isMother[s]) {
                for (int h : hatches(s)) {
                    hatchable |= root[h] == target;
                }
            }
            if (breeds(s) && root[s] == target) {
                hatchable = true;   // (its own kind with DITTO)
            }
        }
        for (int s = 1; s < count && !fromOthers; s++) {
            if (isMother[s]) {
                for (int h : hatches(s)) {
                    fromOthers |= root[h] == target;
                }
            }
        }
        if (!hatchable) {
            why = TO_NO_EGG;
            return out;
        }
        if (target == root[from] && g != FEMALE_ONLY) {   // (its own family: a mate of its kind, or DITTO)
            out.add(single(from, speciesOfDex(DEX_DITTO), hatches(from)[0], EG_DITTO));
            return out;
        }
        if (!fromOthers) {
            why = TO_ONLY_OWN;
            return out;
        }
        if (g == GENDERLESS) {
            why = FROM_GENDERLESS;
            return out;
        }
        Route best = search(from, g, target, new ArrayList<>());
        if (best == null) {
            why = NO_CHAIN;
            return out;
        }
        out.add(best);
        for (int i = 0; i < best.steps.size() - 1 && out.size() < max; i++) {
            List<Integer> ban = new ArrayList<>();
            ban.add(root[best.steps.get(i).hatch]);
            Route alt = search(from, g, target, ban);
            if (alt != null && alt.steps.size() <= best.steps.size() + 1 && !already(alt, out)) {
                out.add(alt);
            }
        }
        return out;
    }

    private Route single(int father, int mother, int hatch, int group) {
        Route r = new Route();
        Step st = new Step();
        st.father = father;
        st.mother = mother;
        st.hatch = hatch;
        st.group = group;
        r.steps.add(st);
        return r;
    }

    private boolean already(Route r, List<Route> list) {
        for (Route o : list) {
            if (o.steps.size() != r.steps.size()) {
                continue;
            }
            boolean same = true;
            for (int i = 0; i < r.steps.size(); i++) {
                same &= root[o.steps.get(i).hatch] == root[r.steps.get(i).hatch];
            }
            if (same) {
                return true;
            }
        }
        return false;
    }

    /**
     * Breadth first over families (the IVs' carrier's): from the start, every mother sharing a group with one of the
     * family's fathers leads to the families she hatches, if they have sons to carry on (or are the target).
     */
    private Route search(int from, int g, int target, List<Integer> banned) {
        int[] dist = new int[count], score = new int[count], viaDad = new int[count], viaMom = new int[count],
                viaFrom = new int[count];
        Arrays.fill(dist, -1);
        Arrays.fill(viaFrom, -1);
        List<Integer> layer = new ArrayList<>();
        int start = root[from];
        boolean female = g == FEMALE_ONLY;
        if (female) {
            // her eggs are her own family (or NIDORAN's / VOLBEAT's other one), by any father sharing a group
            for (int h : hatches(from)) {
                int f = root[h];
                if (dist[f] < 0 && (f == target || hasFather[f])) {
                    dist[f] = 1;
                    score[f] = 1;
                    viaDad[f] = 0;
                    viaMom[f] = from;
                    layer.add(f);
                }
            }
        } else {
            dist[start] = 0;
            layer.add(start);
        }
        while (!layer.isEmpty() && dist[target] < 0) {
            List<Integer> next = new ArrayList<>();
            for (int f : layer) {
                List<Integer> dads = fathers(f, f == start && !female ? from : -1);
                if (dads.isEmpty()) {
                    continue;
                }
                for (int m = 1; m < count; m++) {
                    if (!isMother[m]) {
                        continue;
                    }
                    int dad = fatherFor(dads, m);
                    if (dad < 0) {
                        continue;
                    }
                    for (int h : hatches(m)) {
                        int to = root[h];
                        if (to != target && (banned.contains(to) || !hasFather[to])) {
                            continue;
                        }
                        int s = score[f] + (caught(m) ? 1 : 0);
                        if (dist[to] < 0) {
                            dist[to] = dist[f] + 1;
                            next.add(to);
                        } else if (dist[to] != dist[f] + 1 || s <= score[to]) {
                            continue;
                        }
                        score[to] = s;
                        viaDad[to] = dad;
                        viaMom[to] = m;
                        viaFrom[to] = f;
                    }
                }
            }
            layer = next;
        }
        if (dist[target] < 0) {
            return null;
        }
        List<Integer> chain = new ArrayList<>();
        for (int f = target, guard = 0; f >= 0 && guard < 64; f = viaFrom[f], guard++) {
            chain.add(0, f);
            if (dist[f] == 0 || (female && dist[f] == 1)) {
                break;
            }
        }
        Route r = new Route();
        for (int f : chain) {
            if (dist[f] == 0) {
                continue;   // (the start: the player's own male)
            }
            Step st = new Step();
            st.father = viaDad[f];
            st.mother = viaMom[f];
            st.herOwn = st.father == 0;
            if (!st.herOwn) {
                choices(st, f);
            }
            st.hatch = hatchOf(st.mother, f);
            st.group = st.herOwn ? eg0[from] : shared(st.father, st.mother);
            r.caughtMothers += caught(st.mother) ? 1 : 0;
            r.steps.add(st);
        }
        return r;
    }

    private int hatchOf(int mother, int family) {
        for (int h : hatches(mother)) {
            if (root[h] == family) {
                return h;
            }
        }
        return root[mother];
    }

    private int shared(int a, int b) {
        return eg0[a] == eg0[b] || eg0[a] == eg1[b] ? eg0[a] : eg1[a];
    }

    /** Every mother that gives this egg with this father, caught ones first; the step's mother a caught one if any */
    private void choices(Step st, int family) {
        List<Integer> yes = new ArrayList<>(), no = new ArrayList<>();
        for (int m = 1; m < count; m++) {
            if (!isMother[m] || !overlap(eg0[st.father], eg1[st.father], eg0[m], eg1[m])) {
                continue;
            }
            boolean gives = false;
            for (int h : hatches(m)) {
                gives |= root[h] == family;
            }
            if (gives) {
                (caught(m) ? yes : no).add(m);
            }
        }
        st.mothers.addAll(yes);
        st.mothers.addAll(no);
        if (!caught(st.mother) && !yes.isEmpty()) {
            st.mother = yes.get(0);
        }
    }

    // ------------------------------------------------------------------------------------------------- egg moves

    /** The egg moves of kind S's eggs (its family's first stage's list: what an egg takes from either parent) */
    public int[] eggMoves(int s) {
        int[] m = isReal(s) ? eggMoves[root[s]] : null;
        return m != null ? m : new int[0];
    }

    /** Kind S learns MOVE by leveling up or from a TM / HM (moves from TMs don't pass to eggs, but the parent can
     * be taught one and pass it if it's the egg's egg move) */
    public boolean learns(int s, int move) {
        if (!isReal(s)) {
            return false;
        }
        for (int m : levelMoves[s]) {
            if (m == move) {
                return true;
            }
        }
        String tm = tmMask[s];
        for (int i = 0; tm != null && i < tmMoves.length; i++) {
            if (tmMoves[i] == move) {
                int nib = i / 4 < tm.length() ? Character.digit(tm.charAt(i / 4), 16) : 0;
                return (nib & (8 >> (i % 4))) != 0;
            }
        }
        return false;
    }

    /** By leveling up only (the page says how: "by level" or "by TM") */
    public boolean learnsByLevel(int s, int move) {
        if (!isReal(s)) {
            return false;
        }
        for (int m : levelMoves[s]) {
            if (m == move) {
                return true;
            }
        }
        return false;
    }

    private boolean isEggMove(int s, int move) {
        for (int m : eggMoves(s)) {
            if (m == move) {
                return true;
            }
        }
        return false;
    }

    /** One wanted egg move: the fathers that know it themselves and can sire the target (caught first), or, when
     * none can, a kind that takes it as its own egg move from one of those (BRIDGE, its fathers in FATHERS) */
    public static final class MoveSource {
        public int move;
        public final List<Integer> fathers = new ArrayList<>();   // they learn it (level / TM) and share a group
        public int bridge;                                        // 0, or a kind that gets it as an egg move first
        public final List<Integer> bridgeFathers = new ArrayList<>();
        public boolean lastFatherKnows;                           // the route's last father can learn it himself
    }

    /** Where each of MOVES comes from for an egg of TO's family; LASTFATHER: the IV route's last father (0: none) */
    public List<MoveSource> moveSources(int to, int[] moves, int lastFather) {
        List<MoveSource> out = new ArrayList<>();
        int family = root[to];
        for (int move : moves) {
            MoveSource ms = new MoveSource();
            ms.move = move;
            ms.lastFatherKnows = lastFather > 0 && learns(lastFather, move);
            List<Integer> yes = new ArrayList<>(), no = new ArrayList<>();
            for (int d = 1; d < count; d++) {
                if (!breeds(d) || !canBeMale(d) || !learns(d, move) || !siresFamily(d, family)) {
                    continue;
                }
                (caught(d) ? yes : no).add(d);
            }
            ms.fathers.addAll(yes);
            ms.fathers.addAll(no);
            if (ms.fathers.isEmpty()) {
                // one egg further back: a father of the target's groups whose own egg moves have it, and who can get it
                for (int b = 1; b < count && ms.bridge == 0; b++) {
                    if (!breeds(b) || !canBeMale(b) || !isEggMove(b, move) || !siresFamily(b, family)) {
                        continue;
                    }
                    for (int d = 1; d < count; d++) {
                        if (breeds(d) && canBeMale(d) && learns(d, move) && siresFamily(d, root[b])) {
                            ms.bridgeFathers.add(d);
                        }
                    }
                    if (!ms.bridgeFathers.isEmpty()) {
                        ms.bridge = b;
                    }
                }
            }
            out.add(ms);
        }
        return out;
    }

    /** A male of kind D and a female of some kind hatching FAMILY share an egg group */
    private boolean siresFamily(int d, int family) {
        return (motherGroups[family] & ((1 << eg0[d]) | (1 << eg1[d]))) != 0;
    }
}
