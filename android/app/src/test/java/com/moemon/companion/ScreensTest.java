package com.moemon.companion;

import static org.junit.Assert.assertTrue;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.view.View;

import com.moemon.companion.core.Model;

import org.json.JSONObject;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.RuntimeEnvironment;
import org.robolectric.annotation.Config;
import org.robolectric.annotation.GraphicsMode;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;

/**
 * Every page drawn from a real snapshot (src/test/resources/snap, from the game by android/coretest's SNAP_OUT) at a
 * phone's size, saved as pictures in app/build/screens to look at. Taps and a waiting order too.
 */
@RunWith(RobolectricTestRunner.class)
@Config(sdk = 34)
@GraphicsMode(GraphicsMode.Mode.NATIVE)
public class ScreensTest {
    static final int W = 1080, H = 2340;

    static String res(String name) throws Exception {
        try (InputStream in = ScreensTest.class.getClassLoader().getResourceAsStream("snap/" + name)) {
            return new String(in.readAllBytes(), StandardCharsets.UTF_8);
        }
    }

    static void write(File f, String text) throws Exception {
        try (FileOutputStream o = new FileOutputStream(f)) {
            o.write(text.getBytes(StandardCharsets.UTF_8));
        }
    }

    static void shot(View v, String name) throws Exception {
        v.measure(View.MeasureSpec.makeMeasureSpec(W, View.MeasureSpec.EXACTLY),
                View.MeasureSpec.makeMeasureSpec(H, View.MeasureSpec.EXACTLY));
        v.layout(0, 0, W, H);
        Bitmap b = Bitmap.createBitmap(W, H, Bitmap.Config.ARGB_8888);
        v.draw(new Canvas(b));
        File dir = new File(System.getProperty("screens.out", "build/screens"));
        dir.mkdirs();
        try (FileOutputStream o = new FileOutputStream(new File(dir, name + ".png"))) {
            b.compress(Bitmap.CompressFormat.PNG, 100, o);
        }
    }

    /** A tap at the game's pixels (x, y) of the whole screen */
    static void tap(GameView v, int x, int y) {
        int s = W / v.width();
        long t = SystemClock.uptimeMillis();
        v.onTouchEvent(MotionEvent.obtain(t, t, MotionEvent.ACTION_DOWN, x * s + s / 2f, y * s + s / 2f, 0));
        v.onTouchEvent(MotionEvent.obtain(t, t + 50, MotionEvent.ACTION_UP, x * s + s / 2f, y * s + s / 2f, 0));
    }

    @Test
    public void screens() throws Exception {
        Context ctx = RuntimeEnvironment.getApplication();
        JSONObject file = new JSONObject(res("snap.json"));
        String id = file.getJSONObject("snap").getJSONObject("save").getString("id");
        file.put("asOf", System.currentTimeMillis() - 5 * 60000);
        File dir = ctx.getFilesDir();
        write(new File(dir, "model_" + id + ".json"), file.toString());
        write(new File(dir, "spawnmaps_" + id + ".json"), res("spawnmaps.json"));
        write(new File(dir, "saves.json"), "{\"current\":\"" + id + "\",\"saves\":[{\"save\":\"" + id
                + "\",\"phone\":\"00112233445566778899AABBCCDDEEFF\",\"key\":\"" + "00".repeat(32)
                + "\",\"name\":\"\",\"host\":\"\",\"port\":47610}]}");

        Companion app = Companion.get(ctx);
        JSONObject spawns = new JSONObject(res("spawns.json"));
        app.spawns.put("0,16", spawns);
        GameView v = new GameView(ctx, app);
        shot(v, "boxes");
        assertTrue(v.width() >= 240);

        // a POKéMON chosen, then a move waiting for the game (dashed)
        int x0 = (v.width() - 160) / 2 + 8, y0 = GameView.TOP + 4 + 16;
        tap(v, x0 + 12, y0 + 12);
        shot(v, "boxes_chosen");
        Model m = app.model();
        JSONObject from = Model.boxPlace(0, 0), to = Model.boxPlace(0, 29);
        app.client.orders.add(Model.move(from, m.at(from), to, null));
        shot(v, "boxes_waiting");

        String[] names = {"boxes", "party", "bag", "dex", "spawns"};
        for (int i = 1; i < names.length; i++) {
            v.select(i);
            shot(v, names[i]);
        }
        // the PARTY's first, the BAG's BALLS, the POKéDEX's first entry, a map's POKéMON
        v.select(1);
        tap(v, 60, GameView.TOP + 20);
        shot(v, "party_one");
        v.select(2);
        tap(v, v.width() / 5 + 10, GameView.TOP + 6);
        shot(v, "bag_balls");
        v.select(3);
        tap(v, 20, GameView.TOP + 50);
        shot(v, "dex_entry");
        v.select(4);
        tap(v, 40, GameView.TOP + 10);
        shot(v, "spawns_map");
        // the menu, and pairing
        tap(v, v.width() - 10, 8);
        shot(v, "menu");
        v.back();
        v.showOver(new PairPage(v));
        shot(v, "pair");
    }
}
