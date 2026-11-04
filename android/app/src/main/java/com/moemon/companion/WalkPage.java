package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;
import com.moemon.companion.core.Walk;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.time.LocalDate;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * WALK (docs/COMPANION_APP.md "Pillar 2: the walk", "Training facilities", "Pillar 3: the Garden in your pocket"):
 * the POKéMON out walking with the phone (its steps today and in all, its level coming, its Friendship moments; FIND
 * and MEET bought with steps; BRING HOME), the training facilities' spots, and the Garden's small kind things (a pet,
 * a BERRY, its JOURNAL) for the party.
 */
public final class WalkPage extends Page {
    private final Paint fill = new Paint();
    private final List<int[]> hits = new ArrayList<>();      // x, y, w, h of each button this frame
    private final List<Runnable> actions = new ArrayList<>();
    private final Random random = new Random();
    private int bottom;

    public WalkPage(GameView v) {
        super(v);
    }

    private Model model() {
        return v.app.model();
    }

    private boolean linked() {
        return v.app.client != null && v.app.client.status() == LinkClient.Status.LINKED;
    }

    private void hit(int x, int y, int w, int h, Runnable r) {
        hits.add(new int[]{x, y, w, h});
        actions.add(r);
    }

    private void act(String label, int x, int y, int w, boolean enabled, Runnable r) {
        button(c0, label, x, y, w, enabled);
        if (enabled) {
            hit(x, y, w, 24, r);
        }
    }

    private Canvas c0;

    @Override
    public int scrollMax() {
        return Math.max(0, bottom - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        c0 = c;
        hits.clear();
        actions.clear();
        Walk walk = v.app.walk();
        int y = 24 - scroll;
        y = walk != null ? drawWalker(c, w, y, walk) : drawNoWalker(c, w, y);
        y = drawTraining(c, w, y + 8);
        y = drawGarden(c, w, y + 8);
        bottom = y + scroll + 8;
        String right = walk == null ? "" : walk.stepsOn(LocalDate.now()) + " TODAY";
        banner(c, w, "WALK", right);
        if (walk != null) {
            v.postInvalidateDelayed(60);   // (it walks)
        }
    }

    // ------------------------------------------------------------------------------------------- the walker

    private int drawNoWalker(Canvas c, int w, int y) {
        art.window(c, 0, 12, y + 4, w - 24, 44);
        int ty = y + 4;
        for (String l : wrap("Send a POKéMON on a walk with your phone. Your real steps give it EXP, Friendship and "
                + "finds, and it brings them home to the game.", w - 32)) {
            art.text(c, l, 16, ty);
            ty += 14;
        }
        y = Math.max(ty, y + 48) + 10;
        if (v.app.lastHome != null) {
            JSONObject hm = v.app.lastHome;
            String s = "Last walk: " + hm.optInt("steps") + " steps, +" + hm.optInt("exp") + " EXP, "
                    + hm.optJSONArray("finds").length() + " found, " + hm.optJSONArray("caught").length() + " caught.";
            for (String l : wrap(s, w - 24)) {
                art.text(c, l, 12, y, GameArt.WHITE_TEXT, 0xFF305060);
                y += 14;
            }
            y += 4;
        }
        Model m = model();
        boolean other = m != null && m.snap.optInt("walkOther") == 1;
        act(other ? "OUT WITH ANOTHER PHONE" : "SEND ONE ON A WALK", 12, y, w - 24, m != null && !other, this::pickWalker);
        return y + 30;
    }

    private void pickWalker() {
        Model m = model();
        if (m == null) {
            return;
        }
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        Menu menu = Menu.make(v, "WHO GOES?");
        JSONArray party = m.party();
        for (int k = 0; party != null && k < party.length(); k++) {
            JSONObject mon = party.optJSONObject(k);
            if (mon != null) {
                final JSONObject place = Model.partyPlace(k);
                menu.add(art.plain(GameArt.hexText(mon.optString("n"))), () -> sendOff(place, mon));
            }
        }
        JSONArray box = m.box(m.snap.optInt("box")).optJSONArray("m");
        for (int k = 0; box != null && k < box.length(); k++) {
            JSONObject mon = box.optJSONObject(k);
            if (mon != null) {
                final JSONObject place = Model.boxPlace(m.snap.optInt("box"), k);
                menu.add(art.plain(GameArt.hexText(mon.optString("n"))) + " (BOX)", () -> sendOff(place, mon));
            }
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void sendOff(JSONObject place, JSONObject mon) {
        if (v.app.client == null) {
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "walk").put("at", place).put("p", mon.optLong("p"))
                    .put("o", mon.optLong("o")));
        } catch (JSONException ignored) {
        }
        StepCounter.ask(v);
        v.buzz(true);
        v.app.changed();
    }

    private int drawWalker(Canvas c, int w, int y, Walk walk) {
        JSONObject mon = walk.game.optJSONObject("mon");
        int species = mon == null ? 0 : mon.optInt("s");
        boolean egg = mon != null && mon.optInt("e") == 1;
        long t = System.currentTimeMillis();
        // the stage: the sky by the hour, a path, and it walking along
        int hour = java.time.LocalTime.now().getHour();
        int sky = hour < 6 || hour >= 20 ? 0xFF283868 : hour < 9 ? 0xFFF8B878 : hour >= 17 ? 0xFFF89868 : 0xFF88C8F8;
        fill.setColor(sky);
        c.drawRect(8, y + 4, w - 8, y + 104, fill);
        fill.setColor(0xFF68B848);
        c.drawRect(8, y + 78, w - 8, y + 104, fill);
        fill.setColor(0xFFD8C088);
        c.drawRect(8, y + 86, w - 8, y + 96, fill);
        // passing scenery: tufts sliding by
        fill.setColor(0xFF489830);
        int off = (int) ((t / 40) % 48);
        for (int x = 8 - off; x < w; x += 48) {
            c.drawRect(x + 10, y + 80, x + 14, y + 85, fill);
            c.drawRect(x + 30, y + 98, x + 34, y + 103, fill);
        }
        int bob = (int) (Math.abs(Math.sin(t / 220.0)) * 4);
        int px = w / 2 - 32;
        if (egg) {
            art.icon(c, art.data.optInt("egg"), w / 2 - 16, y + 52 - bob, GameArt.ICON_PLAIN);
        } else {
            art.front(c, species, art.monPal(mon), px, y + 22 - bob, 64, false);
        }
        // now and then a bubble: the Garden's round pink ones
        if ((t / 3000) % 3 == 0) {
            fill.setColor(0xFFF8C8D8);
            c.drawCircle(px + 70, y + 20, 9, fill);
            c.drawCircle(px + 62, y + 30, 3, fill);
            art.text(c, (t / 9000) % 2 == 0 ? "!" : "+", px + 67, y + 12, 0xFFD83860, 0);
        }
        // name and level, steps
        int sy = y + 112;
        art.window(c, 0, 12, sy, w - 24, 56);
        art.textHex(c, mon == null ? "" : mon.optString("n"), 16, sy, GameArt.TEXT, GameArt.SHADOW);
        long steps = walk.steps(), today = walk.stepsOn(LocalDate.now());
        String total = steps + " STEPS";
        art.text(c, total, w - 16 - art.width(total), sy, GameArt.BLUE_TEXT, GameArt.SHADOW);
        art.text(c, "TODAY", 16, sy + 14, GameArt.GRAY_TEXT, GameArt.SHADOW);
        bar(c, 56, sy + 20, w - 120, today, walk.stepsADay(), 0xFF58B8F8);
        String td = today + "/" + walk.stepsADay();
        art.text(c, td, w - 16 - art.width(td), sy + 14);
        if (!egg && mon != null) {
            // its level when it comes home (EXP a step, to the BADGE cap)
            int[] lv = levelAfter(mon, steps, walk.game.optInt("cap", 100));
            art.text(c, "Lv" + mon.optInt("l") + " > " + lv[0] + (lv[0] >= walk.game.optInt("cap", 100) ? " (CAP)" : ""), 16, sy + 28);
            bar(c, 16, sy + 44, w - 40, lv[1], 100, 0xFF58D068);
            String mo = walk.moments() + " MOMENTS";
            art.text(c, mo, w - 16 - art.width(mo), sy + 28, 0xFFD83860, GameArt.SHADOW);
        } else {
            art.text(c, "Its steps help it hatch.", 16, sy + 28);
        }
        y = sy + 66;
        if (!StepCounter.allowed(v.getContext())) {
            act("ALLOW STEP COUNTING", 12, y, w - 24, true, () -> StepCounter.ask(v));
            y += 28;
        }
        // FIND, MEET, HOME
        int bw = (w - 24) / 3;
        LocalDate now = LocalDate.now();
        act("FIND " + walk.findsLeft(), 12, y, bw, walk.findsLeft() > 0, () -> {
            int item = walk.find(random);
            if (item > 0) {
                v.app.saveWalk();
                v.message("It found " + art.plain(art.name("items", item)) + "!");
                v.buzz(true);
            }
        });
        act("MEET " + walk.meetingsLeft(now), 12 + bw, y, bw, walk.meetingsLeft(now) > 0, () -> {
            int[] m = walk.meet(random, now);
            if (m != null) {
                v.showOver(new CatchPage(v, walk, m[0], m[1]));
            }
        });
        act("HOME", 12 + 2 * bw, y, bw, true, this::bringHome);
        y += 30;
        // its finds and who it met
        JSONArray finds = walk.finds(), met = walk.met();
        if (finds.length() > 0 || met.length() > 0) {
            int x = 12;
            for (int i = 0; i < finds.length(); i++, x += 26) {
                if (x > w - 34) {
                    x = 12;
                    y += 26;
                }
                art.itemIcon(c, finds.optInt(i), x, y);
            }
            for (int i = 0; i < met.length(); i++, x += 34) {
                if (x > w - 40) {
                    x = 12;
                    y += 30;
                }
                JSONArray e = met.optJSONArray(i);
                art.icon(c, e.optInt(0), x, y - 4, e.optInt(2) == 1 ? GameArt.ICON_PLAIN : GameArt.ICON_GRAY);
            }
            y += 32;
        }
        return y;
    }

    /** Its level after STEPS more EXP (capped), and the share of the way to the next, in hundredths */
    private int[] levelAfter(JSONObject mon, long steps, int cap) {
        JSONArray xp = mon.optJSONArray("xp");
        JSONObject sp = art.species(mon.optInt("s"));
        JSONArray tables = art.data.optJSONArray("expTables");
        JSONArray table = tables == null || sp == null ? null : tables.optJSONArray(sp.optInt("gr"));
        int lv = mon.optInt("l");
        if (xp == null || table == null) {
            return new int[]{lv, 0};
        }
        long exp = xp.optLong(0) + steps;
        long capExp = table.optLong(Math.min(100, cap));
        if (lv < cap) {
            exp = Math.min(exp, capExp);
        } else {
            exp = xp.optLong(0);
        }
        while (lv < 100 && exp >= table.optLong(lv + 1)) {
            lv++;
        }
        long lo = table.optLong(lv), hi = table.optLong(Math.min(100, lv + 1));
        return new int[]{lv, hi > lo ? (int) ((exp - lo) * 100 / (hi - lo)) : 100};
    }

    private void bar(Canvas c, int x, int y, int w, long v0, long max, int color) {
        fill.setColor(0xFF404040);
        c.drawRect(x, y, x + w, y + 5, fill);
        fill.setColor(0xFFD0D0D0);
        c.drawRect(x + 1, y + 1, x + w - 1, y + 4, fill);
        fill.setColor(color);
        c.drawRect(x + 1, y + 1, x + 1 + (int) ((w - 2) * Math.min(1.0, v0 / (double) Math.max(1, max))), y + 4, fill);
    }

    private void bringHome() {
        Walk walk = v.app.walk();
        if (walk == null || v.app.client == null) {
            return;
        }
        if (!linked()) {
            v.message("Link with the game to bring it home.");
            return;
        }
        Menu m = Menu.make(v, "BRING IT HOME?");
        m.add("YES", () -> {
            try {
                v.app.client.order(walk.homeOrder());
                v.buzz(true);
            } catch (JSONException ignored) {
            }
        });
        m.add("NOT YET", null);
        v.showMenu(m);
    }

    // ------------------------------------------------------------------------------------------- training

    private int drawTraining(Canvas c, int w, int y) {
        Model m = model();
        JSONObject train = m == null ? null : m.snap.optJSONObject("train");
        JSONArray list = train == null ? new JSONArray() : train.optJSONArray("list");
        int slots = train == null ? 2 : train.optInt("slots", 2);
        sectionBar(c, 8, w - 8, y + 1, 16, "TRAINING", list.length() + "/" + slots);
        y += 22;
        long now = m == null ? 0 : m.snap.optLong("now") + (System.currentTimeMillis() - m.asOf) / 1000;
        for (int i = 0; i < list.length(); i++) {
            final JSONObject r = list.optJSONObject(i);
            JSONObject mon = find(r.optLong("p"), r.optLong("o"));
            art.window(c, 0, 12, y + 4, w - 24, 28);
            if (mon != null) {
                art.icon(c, mon.optInt("s"), 10, y, GameArt.ICON_PLAIN);
                art.textHex(c, mon.optString("n"), 46, y + 2, GameArt.TEXT, GameArt.SHADOW);
            }
            art.text(c, TrainText.what(r.optInt("k"), r.optInt("s")), 46, y + 16, GameArt.GRAY_TEXT, GameArt.SHADOW);
            String est = TrainText.estimate(r.optInt("k"), Math.max(0, now - r.optLong("last")));
            art.text(c, est, w - 16 - art.width(est), y + 2, 0xFF209020, GameArt.SHADOW);
            hit(12, y, w - 24, 34, () -> {
                Menu menu = Menu.make(v, "TRAINING");
                menu.add("STOP AND COLLECT", () -> order(new JSONObject(), "untrain", r));
                menu.add("CANCEL", null);
                v.showMenu(menu);
            });
            y += 38;
        }
        if (list.length() == 0) {
            for (String l : wrap("POKéMON in the boxes can train while you're away: EVs, levels to the BADGE cap, or rest.",
                    w - 24)) {
                art.text(c, l, 12, y, GameArt.WHITE_TEXT, 0xFF305060);
                y += 14;
            }
            y += 4;
        }
        int bw = (w - 24) / 2;
        act("COLLECT ALL", 12, y, bw, list.length() > 0, () -> order(new JSONObject(), "collect", null));
        act("+ TRAIN", 12 + bw, y, bw, m != null && list.length() < slots, this::pickTrainee);
        return y + 28;
    }

    private JSONObject find(long p, long o) {
        Model m = model();
        if (m == null) {
            return null;
        }
        JSONArray boxes = m.boxes();
        for (int b = 0; boxes != null && b < boxes.length(); b++) {
            JSONArray s = boxes.optJSONObject(b).optJSONArray("m");
            for (int k = 0; s != null && k < s.length(); k++) {
                JSONObject mon = s.optJSONObject(k);
                if (mon != null && mon.optLong("p") == p && mon.optLong("o") == o) {
                    return mon;
                }
            }
        }
        return null;
    }

    private void pickTrainee() {
        Model m = model();
        int b = m.snap.optInt("box");
        JSONArray box = m.box(b).optJSONArray("m");
        Menu menu = Menu.make(v, "WHO TRAINS? (BOX)");
        for (int k = 0; box != null && k < box.length(); k++) {
            final JSONObject mon = box.optJSONObject(k);
            if (mon == null || mon.optInt("e") == 1) {
                continue;
            }
            final JSONObject place = Model.boxPlace(b, k);
            menu.add(art.plain(GameArt.hexText(mon.optString("n"))), () -> pickKind(place, mon));
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void pickKind(JSONObject place, JSONObject mon) {
        Menu menu = Menu.make(v, "WHAT KIND?");
        menu.add("STAT (EVs)", () -> {
            Menu stats = Menu.make(v, "WHICH STAT?");
            for (int s = 0; s < 6; s++) {
                final int stat = s;
                stats.add(TrainText.STATS[s], () -> train(place, mon, 0, stat));
            }
            stats.add("CANCEL", null);
            v.showMenu(stats);
        });
        menu.add("LEVEL", () -> train(place, mon, 1, 0));
        menu.add("REST", () -> train(place, mon, 2, 0));
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void train(JSONObject place, JSONObject mon, int kind, int stat) {
        try {
            order(new JSONObject().put("at", place).put("p", mon.optLong("p")).put("o", mon.optLong("o")).put("k", kind)
                    .put("s", stat), "train", null);
        } catch (JSONException ignored) {
        }
    }

    private void order(JSONObject o, String op, JSONObject rec) {
        if (v.app.client == null) {
            return;
        }
        try {
            o.put("op", op);
            if (rec != null) {
                o.put("p", rec.optLong("p")).put("o", rec.optLong("o"));
            }
            v.app.client.order(o);
        } catch (JSONException ignored) {
        }
        v.buzz(false);
        v.app.changed();
    }

    // --------------------------------------------------------------------------------------------- the Garden

    private int drawGarden(Canvas c, int w, int y) {
        Model m = model();
        sectionBar(c, 8, w - 8, y + 1, 16, "GARDEN", null);
        y += 22;
        JSONArray party = m == null ? null : m.party();
        for (int k = 0; party != null && k < party.length(); k++) {
            final JSONObject mon = party.optJSONObject(k);
            if (mon == null || mon.optInt("e") == 1) {
                continue;
            }
            final JSONObject place = Model.partyPlace(k);
            art.window(c, 0, 12, y + 4, w - 24, 28);
            art.icon(c, mon.optInt("s"), 10, y, GameArt.ICON_PLAIN);
            art.textHex(c, mon.optString("n"), 46, y + 2, GameArt.TEXT, GameArt.SHADOW);
            int hearts = mon.optInt("f") / 51;
            for (int i = 0; i < 5; i++) {
                fill.setColor(i < hearts ? 0xFFE84870 : 0xFFD8D8D8);
                c.drawRect(46 + i * 9, y + 20, 53 + i * 9, y + 26, fill);
            }
            int bx = w - 12 - 3 * 44;
            act("PET", bx, y + 4, 44, true, () -> kind(place, mon, 0, 0));
            act("BERRY", bx + 44, y + 4, 44, true, () -> pickBerry(place, mon));
            act("DIARY", bx + 88, y + 4, 44, true, () -> v.showOver(new JournalPage(v, place, mon)));
            y += 38;
        }
        if (party == null || party.length() == 0) {
            art.text(c, "Nothing from the game yet.", 12, y, GameArt.WHITE_TEXT, 0xFF305060);
            y += 18;
        }
        return y;
    }

    private void pickBerry(JSONObject place, JSONObject mon) {
        Model m = model();
        JSONArray bag = m == null ? null : m.snap.optJSONArray("bag");
        JSONArray berries = bag == null ? null : bag.optJSONArray(3);
        if (berries == null || berries.length() == 0) {
            v.message("You don't have any BERRIES to share.");
            return;
        }
        Menu menu = Menu.make(v, "WHICH BERRY?");
        for (int i = 0; i < berries.length(); i++) {
            final int item = berries.optJSONArray(i).optInt(0);
            menu.add(art.plain(art.name("items", item)), () -> kind(place, mon, 1, item));
        }
        menu.add("CANCEL", null);
        v.showMenu(menu);
    }

    private void kind(JSONObject place, JSONObject mon, int verb, int item) {
        try {
            order(new JSONObject().put("at", place).put("p", mon.optLong("p")).put("o", mon.optLong("o")).put("verb", verb)
                    .put("item", item), "kind", null);
        } catch (JSONException ignored) {
        }
        if (!linked()) {
            v.message("It waits for the next link: then " + art.plain(GameArt.hexText(mon.optString("n")))
                    + " gets it in the game.");
        }
    }

    // ------------------------------------------------------------------------------------------------ touch

    @Override
    public void tap(int x, int y) {
        for (int i = hits.size() - 1; i >= 0; i--) {
            int[] r = hits.get(i);
            if (in(x, y, r[0], r[1], r[2], r[3])) {
                actions.get(i).run();
                return;
            }
        }
    }
}
