package com.moemon.companion;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeTrue;
import static org.robolectric.Shadows.shadowOf;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.os.Looper;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.view.View;

import com.moemon.companion.core.LinkClient;
import com.moemon.companion.core.Model;

import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.annotation.GraphicsMode;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.function.BooleanSupplier;

/**
 * The whole app (Companion, GameView, its pages and taps, the link) against a headless game, the way a player uses
 * it: pair by the QR code's text, GIVE an item to a box POKéMON, MOVE one to another place, open SPAWNS, with a frame
 * drawn after each. Needs `make linux` and android/coretest/run.sh (the link library for this machine); skipped
 * without them.
 */
@RunWith(RobolectricTestRunner.class)
@Config(sdk = 34)
@GraphicsMode(GraphicsMode.Mode.NATIVE)
public class LiveTest {
    static final int W = 1080, H = 2340;
    static final LinkedBlockingQueue<String> lines = new LinkedBlockingQueue<>();

    static void idleUntil(BooleanSupplier ok, int seconds, String what) throws Exception {
        long end = System.currentTimeMillis() + seconds * 1000L;
        while (System.currentTimeMillis() < end) {
            shadowOf(Looper.getMainLooper()).idle();
            if (ok.getAsBoolean()) {
                return;
            }
            Thread.sleep(50);
        }
        throw new AssertionError("never: " + what);
    }

    static void draw(View v) {
        v.measure(View.MeasureSpec.makeMeasureSpec(W, View.MeasureSpec.EXACTLY),
                View.MeasureSpec.makeMeasureSpec(H, View.MeasureSpec.EXACTLY));
        v.layout(0, 0, W, H);
        v.draw(new Canvas(Bitmap.createBitmap(W, H, Bitmap.Config.ARGB_8888)));
    }

    static void tap(GameView v, int x, int y) {
        int s = W / v.width();
        long t = SystemClock.uptimeMillis();
        v.onTouchEvent(MotionEvent.obtain(t, t, MotionEvent.ACTION_DOWN, x * s + s / 2f, y * s + s / 2f, 0));
        v.onTouchEvent(MotionEvent.obtain(t, t + 50, MotionEvent.ACTION_UP, x * s + s / 2f, y * s + s / 2f, 0));
        shadowOf(Looper.getMainLooper()).idle();
        draw(v);
    }

    static String field(String line, String key) {
        String[] f = line.trim().split("\\s+");
        for (int i = 0; i + 1 < f.length; i++) {
            if (f[i].equals(key)) {
                return f[i + 1];
            }
        }
        return null;
    }

    @Test
    public void live() throws Exception {
        File root = new File(System.getProperty("moemon.root", "../.."));
        File game = new File(root, "pokeemerald64"), lib = new File(root, "build/coretest/libmoemonlink.so");
        assumeTrue("needs make linux and android/coretest/run.sh", game.exists() && lib.exists());
        int port = 48100 + (int) (System.nanoTime() % 90);
        File work = Files.createTempDirectory("livetest").toFile();
        File script = new File(work, "a.txt");
        Files.write(script.toPath(), String.join("\n",
                "pressuntil A field 5000", "wait 30",
                "mp boxput 1 0 25 5", "mp boxput 1 1 1 5", "mp boxput 1 2 4 5",
                "mp additem 139 20", "mp shinymon 4 25 20", "mp additem 660 1", "mp regionclear 3", "mp dexcatch 263",
                "mp additem 13 3", "mp additem 68 2", "mp additem 96 1", "mp additem 693 1", "mp hp 1 5", "mp shinymon 5 16 5", "mp phone pair",
                "wait 300", "mash B 200 15", "wait 2700", "mp wild 263:3", "wait 2000000", "quit").getBytes());
        ProcessBuilder pb = new ProcessBuilder(game.getPath()).directory(root).redirectErrorStream(true);
        java.util.Map<String, String> env = pb.environment();
        env.put("SDL_VIDEODRIVER", "dummy");
        env.put("SDL_AUDIODRIVER", "dummy");
        env.put("MOEMON_TEST", script.getPath());
        env.put("MOEMON_SHOTDIR", work.getPath());
        env.put("MOEMON_DEBUG_QUICKSTART", "1");
        env.put("MOEMON_SKIP_COOP_SETUP", "1");
        env.put("MOEMON_COOP_CONFIG", new File(work, "a.ini").getPath());
        env.put("MOEMON_CONTROLS_CONFIG", new File(work, "a_controls.ini").getPath());
        env.put("MOEMON_SAVE", new File(work, "a.sav").getPath());
        env.put("MOEMON_PHONE_PORT", String.valueOf(port));
        env.put("MOEMON_TEST_FPS", "60");
        env.put("MOEMON_DEBUG_PARTY", "25:20,1:20,4:20");
        Process p = pb.start();
        Thread reader = new Thread(() -> {
            try (BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                String l;
                while ((l = r.readLine()) != null) {
                    lines.add(l);
                }
            } catch (Exception ignored) {
            }
        });
        reader.setDaemon(true);
        reader.start();
        try {
            String line = null;
            long end = System.currentTimeMillis() + 180000;
            while (line == null && System.currentTimeMillis() < end) {
                String l = lines.poll(200, TimeUnit.MILLISECONDS);
                if (l != null && l.contains("[test] phone: save")) {
                    line = l;
                }
            }
            assertNotNull("the game never offered pairing", line);
            final String saveId = field(line, "save");

            Context ctx = RuntimeEnvironment.getApplication();
            Companion.resetForTests();
        Companion.resetForTests();
            Companion app = Companion.get(ctx);
            GameView v = new GameView(ctx, app);
            draw(v);
            String qr = "MOEPHONE:1:127.0.0.1:" + port + ":" + field(line, "save") + ":" + field(line, "secret") + ":"
                    + field(line, "code");
            final String[] err = {"pending"};
            app.pair(qr, null, e -> err[0] = e);
            idleUntil(() -> !"pending".equals(err[0]), 30, "paired");
            assertEquals(null, err[0]);
            app.resume();
            idleUntil(() -> app.client.status() == LinkClient.Status.LINKED && app.model() != null
                    && app.model().saveId().equals(saveId), 30, "linked with a snapshot");
            draw(v);

            // USE, as the BAG's USE in the game: a POTION on the hurt BULBASAUR, a RARE CANDY on the PIDGEY, a THUNDER
            // STONE on PIKACHU (its evolution plays in the game: B through its messages)
            JSONObject bulba = app.model().party().getJSONObject(1);
            UseFlow.use(v, Model.partyPlace(1), bulba, 13, null);
            idleUntil(() -> app.client.orders.pending().isEmpty()
                    && app.model().party().optJSONObject(1).optInt("hp") > 5, 20, "the POTION healed it");
            JSONObject pidgey = app.model().party().getJSONObject(4);
            int lv = pidgey.optInt("l");
            UseFlow.use(v, Model.partyPlace(4), pidgey, 68, null);
            idleUntil(() -> app.client.orders.pending().isEmpty()
                    && app.model().party().optJSONObject(4).optInt("l") == lv + 1, 20, "the RARE CANDY raised it a level");
            assertTrue("nothing refused", app.client.orders.failed().isEmpty());
            JSONObject pika = app.model().party().getJSONObject(0);
            UseFlow.use(v, Model.partyPlace(0), pika, 96, null);
            idleUntil(() -> app.model().party().optJSONObject(0).optInt("s") == 26, 90, "PIKACHU evolved in the game");
            draw(v);
            ScreensTest.shot(v, "live_evolved");

            // the shiny one in the party comes with its own colors (not its species')
            JSONObject shiny = app.model().party().getJSONObject(3);
            assertEquals(25, shiny.optInt("s"));   // (the shiny one in slot 4; slot 1's PIKACHU evolved)
            assertEquals(1, shiny.optInt("sh"));
            String pal = shiny.optString("pal");
            assertEquals(64, pal.length());
            assertTrue("a shiny's palette isn't its species'", !pal.equals(app.art.species(25).optString("pal")));
            assertTrue("a plain one has no palette of its own", !app.model().party().getJSONObject(0).has("pal"));
            v.select(1);
            tap(v, 60, GameView.TOP + 24 + 3 * 50 + 20);
            ScreensTest.shot(v, "live_shiny");
            v.showOver(null);
            tap(v, 60, GameView.TOP + 24 + 0 * 50 + 20);
            ScreensTest.shot(v, "live_plain");
            v.showOver(null);

            // GIVE: the box's first POKéMON, its card's GIVE, the BAG's first holdable item
            v.select(0);
            int x0 = (v.width() - 160) / 2 + 8, y0 = GameView.TOP + 4 + 16;
            tap(v, x0 + 12, y0 + 12);
            int cardY = GameView.TOP + 4 + 144 + 6 + 36 + 22, by = cardY + 52 + 10 + 12;
            int bw = (v.width() - 20) / 4;
            tap(v, 10 + 2 * bw + bw / 2, by);
            assertTrue("GIVE opens the BAG's items", v.overPage() instanceof ItemPickPage);
            tap(v, 40, GameView.TOP + 26 + 13);
            idleUntil(() -> app.client.orders.pending().isEmpty() && !app.client.orders.failed().isEmpty()
                    || app.client.orders.pending().isEmpty() && app.model().at(Model.boxPlace(0, 0)).optInt("i") != 0,
                    30, "the item given: pending " + app.client.orders.pending() + " over " + v.overPage());
            assertTrue("refused: " + app.client.orders.failed() + " " + app.model().at(Model.boxPlace(0, 0)),
                    app.client.orders.failed().isEmpty());
            draw(v);

            // MOVE: the second POKéMON to the box's last place
            tap(v, x0 + 24 + 12, y0 + 12);
            long p2 = app.model().at(Model.boxPlace(0, 1)).optLong("p");
            tap(v, 10 + bw + bw / 2, by);
            tap(v, x0 + 5 * 24 + 12, y0 + 4 * 24 + 12);
            idleUntil(() -> {
                JSONObject m = app.model().at(Model.boxPlace(0, 29));
                return app.client.orders.pending().isEmpty() && m != null && m.optLong("p") == p2;
            }, 30, "the POKéMON moved");
            draw(v);
            assertTrue("nothing refused", app.client.orders.failed().isEmpty());

            // MOVE to another box: the third POKéMON, MOVE, the next box (its arrow), its first place
            tap(v, x0 + 2 * 24 + 12, y0 + 12);
            long p3 = app.model().at(Model.boxPlace(0, 2)).optLong("p");
            tap(v, 10 + bw + bw / 2, by);
            tap(v, x0 - 8 + 160 + 8, GameView.TOP + 4 + 6);
            tap(v, x0 + 12, y0 + 12);
            idleUntil(() -> {
                JSONObject m = app.model().at(Model.boxPlace(1, 0));
                return app.client.orders.pending().isEmpty() && m != null && m.optLong("p") == p3;
            }, 30, "the POKéMON moved to the next box");
            assertTrue("nothing refused", app.client.orders.failed().isEmpty());

            // the POKéDEX: where ZIGZAGOON lives, from the game; its first place opens SPAWNS
            int zig = app.art.speciesOfDex(263);
            java.util.List<Integer> nums = new java.util.ArrayList<>();
            nums.add(263);
            v.select(3);
            v.showOver(new DexEntryPage(v, nums, 0));
            draw(v);   // (the entry asks the game when it's drawn)
            idleUntil(() -> app.where.containsKey(zig), 20, "where it lives");
            assertTrue("ZIGZAGOON lives somewhere", app.where.get(zig).optJSONArray("slots").length() > 0);
            draw(v);
            ScreensTest.shot(v, "live_where");

            // SPAWNS: the list, then a map's POKéMON from the game
            v.select(4);
            idleUntil(() -> app.spawnMaps != null, 20, "the spawn maps");
            tap(v, 40, GameView.TOP + 44 + 2 + 10);
            idleUntil(() -> !app.spawns.isEmpty(), 20, "a map's spawns");
            draw(v);

            // the walk: a box POKéMON goes off with the phone, walks, finds something, comes home with it all
            {
                Model wm = app.model();
                org.json.JSONArray b0 = wm.box(0).getJSONArray("m");
                int ws = 0;
                while (b0.optJSONObject(ws) == null) {
                    ws++;
                }
                JSONObject walker = b0.getJSONObject(ws);
                app.client.order(new JSONObject().put("op", "walk").put("at", Model.boxPlace(0, ws))
                        .put("p", walker.getLong("p")).put("o", walker.getLong("o")));
                idleUntil(() -> app.walk() != null, 20, "the walk to begin");
                long now = System.currentTimeMillis(), boot = now - 3600000;
                app.walkReading(1000, boot, now);
                app.walkReading(5600, boot, now + 1000);
                assertEquals("steps counted", 4600, app.walk().steps());
                int item = app.walk().find(new java.util.Random(3));
                assertTrue("a find from the place", item > 0);
                v.select(6);
                draw(v);
                ScreensTest.shot(v, "live_walk");
                app.client.order(app.walk().homeOrder());
                idleUntil(() -> app.lastHome != null, 20, "the homecoming");
                assertEquals("home with its steps", 4600, app.lastHome.getInt("steps"));
                assertEquals("home with its find", item, app.lastHome.getJSONArray("finds").getInt(0));
                idleUntil(() -> {
                    Model m2 = app.model();
                    for (int i = 0; i < m2.party().length(); i++) {
                        if (m2.party().optJSONObject(i).optLong("p") == walker.optLong("p")) {
                            return true;
                        }
                    }
                    return false;
                }, 20, "the walker back in the party");
                assertTrue("no walk left", app.walk() == null);
                v.select(0);
            }

            // the live map: where the player is, and what they've found at the place tapped (the game's own answer)
            org.json.JSONArray tm = app.model().snap.getJSONObject("place").optJSONArray("tm");
            assertTrue("the town map knows where the player is", tm != null);
            v.showOver(new MapPage(v, "HOENN MAP"));
            draw(v);
            int ms = Math.max(1, (v.width() - 8) / 240), mx = (v.width() - 240 * ms) / 2;
            tap(v, mx + (8 + tm.getInt(1) * 8) * ms + 4, GameView.TOP + 46 + (16 + tm.getInt(2) * 8) * ms + 4);
            int sec = app.model().snap.getJSONObject("place").getInt("sec");
            idleUntil(() -> app.places.containsKey(sec), 20, "the place's finds");
            assertTrue("the place lists its wild POKéMON", app.places.get(sec).has("wild"));
            draw(v);
            ScreensTest.shot(v, "live_map");
            v.showOver(null);
            // MATCH CALL's list and the POKéGEAR's radio and a call, from the game
            app.client.ask("calls");
            idleUntil(() -> app.calls != null, 20, "MATCH CALL's list");
            app.client.send(new JSONObject().put("t", "gear").put("radio", 33));
            idleUntil(() -> app.gear != null && app.gear.has("radio"), 20, "the radio");
            assertTrue("16.5 plays PLACES AND PEOPLE out of JOHTO", !app.gear.optString("name").equals("FF"));
            app.gear = null;
            app.client.send(new JSONObject().put("t", "gear").put("call", 0));
            idleUntil(() -> app.gear != null && app.gear.has("call"), 20, "MOM's call");
            assertTrue("MOM says something", app.gear.optJSONArray("lines").length() > 0);

            // the POKéTCH on the DEVICE page: the game's own LCD, a touch, the side button
            v.select(5);
            ((DevicePage) v.page(5)).show(DevicePage.POKETCH);
            draw(v);
            idleUntil(() -> app.watchFrame != null, 20, "the POKéTCH's LCD");
            draw(v);
            ScreensTest.shot(v, "live_poketch");
            int app0 = app.watchFrame.optInt("app");
            int s3 = Math.max(1, (v.width() - 40) / 192), lx = (v.width() - 192 * s3 - 16) / 2;
            long pressed = System.currentTimeMillis();
            tap(v, lx + 192 * s3 + 16, GameView.TOP + 56 + 80 * s3);   // (the side button)
            idleUntil(() -> app.watchFrame.optInt("app") != app0, 20, "the next app");
            // Phase 2's gate: a touch feels instant (the game's new screen back within a few frames)
            long took = System.currentTimeMillis() - pressed;
            System.out.println("LIVE: the POKéTCH's next app came back in " + took + " ms");
            assertTrue("a touch answered in " + took + " ms", took < 250);
            draw(v);
            ScreensTest.shot(v, "live_poketch2");
            v.select(0);

            // a battle in the game: the app offers nothing until it's over
            v.select(0);
            idleUntil(app::locked, 150, "the battle seen by the app");
            // battle mode: the battle as the player sees it, live
            idleUntil(() -> app.battle != null && app.battle.optJSONArray("mons") != null
                    && app.battle.optJSONArray("mons").length() >= 2, 20, "the battle's feed with the POKéMON out");
            org.json.JSONArray mons = app.battle.getJSONArray("mons");
            boolean foe = false, mine = false;
            for (int i = 0; i < mons.length(); i++) {
                JSONObject b = mons.getJSONObject(i);
                foe |= b.getInt("side") == 1 && b.getInt("s") > 0 && b.has("pct") && !b.has("hp") && !b.has("m");
                mine |= b.getInt("me") == 1 && b.has("m");
            }
            assertTrue("the wild POKéMON, its HP as a share only", foe);
            assertTrue("your POKéMON with its moves", mine);
            v.select(5);
            draw(v);
            ScreensTest.shot(v, "live_battle");
            v.select(0);
            draw(v);
            int pending = app.client.orders.pending().size();
            tap(v, x0 + 12, y0 + 12);
            tap(v, 10 + 2 * bw + bw / 2, by);
            assertTrue("GIVE is closed in a battle", !(v.overPage() instanceof ItemPickPage));
            assertEquals("no order in a battle", pending, app.client.orders.pending().size());
            app.pause();
        } finally {
            p.destroy();
        }
    }
}
