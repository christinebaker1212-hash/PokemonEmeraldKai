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
                "mp additem 139 20", "mp shinymon 4 25 20", "mp phone pair",
                "wait 4200", "mp wild 263:3", "wait 2000000", "quit").getBytes());
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

            // the shiny one in the party comes with its own colors (not its species')
            JSONObject shiny = app.model().party().getJSONObject(3);
            assertEquals(25, shiny.optInt("s"));
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

            // SPAWNS: the list, then a map's POKéMON from the game
            tap(v, v.width() - 20, v.height() - 10);
            idleUntil(() -> app.spawnMaps != null, 20, "the spawn maps");
            tap(v, 40, GameView.TOP + 22 + 2 + 22 + 10);
            idleUntil(() -> !app.spawns.isEmpty(), 20, "a map's spawns");
            draw(v);

            // a battle in the game: the app offers nothing until it's over
            v.select(0);
            idleUntil(app::locked, 150, "the battle seen by the app");
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
