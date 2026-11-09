import com.moemon.companion.core.Breeding;

import org.json.JSONObject;

import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

/**
 * The BREEDING page's calculator (core/Breeding.java) on the game's own data.json (MOEMON_DATA, or the app's
 * assets): routes across egg groups, the kinds no other kind can pass IVs to, DITTO, a female start, NIDORAN /
 * VOLBEAT eggs, and where egg moves come from. Run: ONLY=breed android/coretest/run.sh (or javac + java by hand).
 */
public final class BreedTest {
    static int fails;
    static JSONObject data;
    static Breeding b;

    static void check(boolean ok, String what) {
        System.out.println((ok ? "ok   " : "FAIL ") + what);
        if (!ok) {
            fails++;
        }
    }

    static int sp(int dex) {
        return b.speciesOfDex(dex);
    }

    static String name(int s) {
        byte[] t = com.moemon.companion.core.Hex.bytes(data.optJSONArray("species").optJSONObject(s).optString("n"));
        StringBuilder sb = new StringBuilder();
        for (byte x : t) {   // (the game's letters: A-Z 0xBB, a-z 0xD5, 0-9 0xA1)
            int c = x & 0xFF;
            sb.append(c >= 0xBB && c < 0xD5 ? (char) ('A' + c - 0xBB) : c >= 0xD5 && c < 0xEF ? (char) ('a' + c - 0xD5)
                    : c >= 0xA1 && c < 0xAB ? (char) ('0' + c - 0xA1) : c == 0xFF ? '\0' : '?');
        }
        return sb.toString().replace("\0", "");
    }

    static void print(List<Breeding.Route> rs) {
        for (Breeding.Route r : rs) {
            StringBuilder sb = new StringBuilder("    route:");
            for (Breeding.Step st : r.steps) {
                sb.append(st.herOwn ? " [her own + any male]" : " [" + name(st.father) + " x " + name(st.mother) + "]")
                  .append(" -> ").append(name(st.hatch)).append(" (").append(st.mothers.size()).append(" mothers)");
            }
            System.out.println(sb);
        }
    }

    public static void main(String[] args) throws Exception {
        String path = System.getenv().getOrDefault("MOEMON_DATA", "android/app/src/main/assets/game/data.json");
        data = new JSONObject(new String(Files.readAllBytes(Paths.get(path)), "UTF-8"));
        b = new Breeding(data);
        List<Breeding.Route> r;

        r = b.routes(sp(4), 0, sp(25), 3);   // CHARMANDER (MONSTER / DRAGON) to PIKACHU (FIELD / FAIRY)
        print(r);
        check(!r.isEmpty() && r.get(0).steps.size() == 2, "CHARMANDER to PIKACHU: two eggs (a MONSTER-FIELD bridge)");
        check(!r.isEmpty() && b.family(r.get(0).steps.get(1).hatch) == b.family(sp(25)), "  the last egg is PIKACHU's family");
        check(r.size() > 1, "  other routes offered");

        r = b.routes(sp(4), 254, sp(25), 3);   // a female CHARMANDER: her own egg first
        print(r);
        check(!r.isEmpty() && r.get(0).steps.get(0).herOwn, "a female CHARMANDER: her own egg (a son) first");

        r = b.routes(sp(4), 0, sp(128), 3);   // TAUROS: males only
        check(r.isEmpty() && b.why == Breeding.TO_ONLY_OWN, "TAUROS (males only): no other kind passes it IVs");
        r = b.routes(sp(4), 0, sp(81), 3);   // MAGNEMITE: no gender
        check(r.isEmpty() && b.why == Breeding.TO_ONLY_OWN, "MAGNEMITE (no gender): no other kind passes it IVs");
        r = b.routes(sp(4), 0, sp(150), 3);   // MEWTWO
        check(r.isEmpty() && b.why == Breeding.TO_NO_EGG, "MEWTWO: hatches from no egg");
        r = b.routes(sp(132), -1, sp(81), 3);   // DITTO to MAGNEMITE: straight in
        check(r.size() == 1 && r.get(0).steps.size() == 1, "DITTO to MAGNEMITE: one egg");
        r = b.routes(sp(81), 255, sp(4), 3);
        check(r.isEmpty() && b.why == Breeding.FROM_GENDERLESS, "MAGNEMITE to CHARMANDER: no gender, only DITTO");
        r = b.routes(sp(4), 0, sp(313), 3);   // VOLBEAT (males only) through an ILLUMISE mother
        print(r);
        check(!r.isEmpty() && b.family(r.get(0).steps.get(r.get(0).steps.size() - 1).hatch) == b.family(sp(313)),
                "VOLBEAT: through an ILLUMISE mother (either kind hatches)");
        r = b.routes(sp(4), 0, sp(6), 3);   // its own family
        check(r.size() == 1 && r.get(0).steps.size() == 1, "CHARMANDER to CHARIZARD: one egg, its own family");
        r = b.routes(sp(25), 0, sp(172), 3);   // PIKACHU to PICHU (a baby: its family's)
        check(r.size() == 1 && r.get(0).steps.size() == 1, "PIKACHU to PICHU: the same family");

        // egg moves: CHARMANDER's own list, and fathers that can pass them
        int[] em = b.eggMoves(sp(5));   // (CHARMELEON: the family's first stage's list)
        check(em.length > 0 && em.length == b.eggMoves(sp(4)).length, "egg moves: CHARMANDER's list for the family (" + em.length + ")");
        List<Breeding.MoveSource> ms = b.moveSources(sp(4), em, sp(25));
        int found = 0;
        for (Breeding.MoveSource m : ms) {
            found += m.fathers.isEmpty() && m.bridge == 0 ? 0 : 1;
            System.out.println("    move " + m.move + ": " + m.fathers.size() + " fathers" + (m.bridge != 0 ? ", via " + name(m.bridge) : ""));
        }
        check(found == em.length, "every one of CHARMANDER's egg moves has a source");
        long t = System.nanoTime();
        for (int i = 0; i < 20; i++) {
            b.routes(sp(4), 0, sp(25), 3);
        }
        check((System.nanoTime() - t) / 20 / 1000000 < 300, "a route takes under 300 ms (" + (System.nanoTime() - t) / 20 / 1000000 + " ms)");
        System.out.println(fails == 0 ? "BREED: all ok" : "BREED: " + fails + " failed");
        System.exit(fails == 0 ? 0 : 1);
    }
}
