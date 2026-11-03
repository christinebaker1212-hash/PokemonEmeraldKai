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
        Companion.resetForTests();
        JSONObject file = new JSONObject(res("snap.json"));
        String id = file.getJSONObject("snap").getJSONObject("save").getString("id");
        file.put("asOf", System.currentTimeMillis() - 5 * 60000);
        File dir = ctx.getFilesDir();
        write(new File(dir, "model_" + id + ".json"), file.toString());
        write(new File(dir, "spawnmaps_" + id + ".json"), res("spawnmaps.json"));
        write(new File(dir, "saves.json"), "{\"current\":\"" + id + "\",\"saves\":[{\"save\":\"" + id
                + "\",\"phone\":\"00112233445566778899AABBCCDDEEFF\",\"key\":\"" + "00".repeat(32)
                + "\",\"name\":\"\",\"host\":\"\",\"port\":47610}]}");

        // a POKéMON out walking (for WALK's pictures): the snapshot's first box POKéMON
        JSONObject walkMon = file.getJSONObject("snap").getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(0);
        java.time.LocalDate today = java.time.LocalDate.now();
        write(new File(dir, "walk_" + id + ".json"), new JSONObject().put("game", new JSONObject().put("id", 5)
                .put("sent", System.currentTimeMillis() / 1000 - 86400).put("cap", 20).put("mon", walkMon)
                .put("wild", new org.json.JSONArray("[[286,2,4]]")).put("finds", new org.json.JSONArray("[13]"))
                .put("rules", new org.json.JSONArray("[10000,2000,3000,2500,3]")))
                .put("days", new JSONObject().put(today.minusDays(1).toString(), 8200).put(today.toString(), 4321))
                .put("finds", new org.json.JSONArray("[13,13]")).put("met", new org.json.JSONArray().put(
                        new org.json.JSONArray().put(286).put(3).put(1).put(today.toString())))
                .put("caught", new org.json.JSONArray("[[286,3,4]]")).put("lastCounter", 100).toString());
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
        // the summary's three pages, from the box's card
        v.select(0);
        tap(v, x0 + 24 + 12, y0 + 12);
        tap(v, 60, GameView.TOP + 4 + 144 + 6 + 36 + 22 + 20);
        shot(v, "summary_info");
        tap(v, v.width() / 2, GameView.TOP + 112 + 8);
        shot(v, "summary_skills");
        tap(v, v.width() - 30, GameView.TOP + 112 + 8);
        tap(v, 60, GameView.TOP + 136 + 4 + 12);
        shot(v, "summary_moves");
        v.showOver(null);
        // the shiny in the party, its summary
        v.select(1);
        tap(v, 60, GameView.TOP + 24 + 3 * 50 + 20);
        shot(v, "party_summary");
        v.showOver(null);
        // the BAG with an item chosen, the BALLS
        v.select(2);
        tap(v, 60, GameView.TOP + 40 + 2 + 12);
        shot(v, "bag_item");
        tap(v, v.width() / 5 + 10, GameView.TOP + 28);
        tap(v, 60, GameView.TOP + 40 + 2 + 12);
        shot(v, "bag_balls");
        // the POKéDEX: the cards, BULBASAUR's entry (and its lower part), a search for PIKA
        v.select(3);
        shot(v, "dex");
        tap(v, 6 + 41, GameView.TOP + 96 + 49);
        shot(v, "dex_entry");
        v.overPage().scrollTo(320);
        shot(v, "dex_entry_lower");
        v.showOver(null);
        tap(v, 60, GameView.TOP + 30);
        v.overPage();
        v.page(3).typed("PIKA");
        v.page(3).typingDone();
        shot(v, "dex_search");
        v.page(3).backspace();
        v.page(3).backspace();
        v.page(3).backspace();
        v.page(3).backspace();
        // a map's POKéMON, and the box list
        v.select(4);
        tap(v, 40, GameView.TOP + 44 + 2 + 10);
        shot(v, "spawns_map");
        v.select(0);
        tap(v, v.width() / 2, GameView.TOP + 6);
        shot(v, "box_list");
        v.showOver(null);
        devices(v, app);
        // WALK: the walker, training and the Garden; a meeting; the day's report; the widget
        v.select(6);
        shot(v, "walk");
        v.page(6).scrollTo(400);
        shot(v, "walk_lower");
        v.page(6).scrollTo(0);
        v.showOver(new CatchPage(v, app.walk(), 286, 3));
        shot(v, "walk_meet");
        v.showOver(new TodayPage(v));
        shot(v, "today");
        v.showOver(null);
        try (FileOutputStream o = new FileOutputStream(new File(new File(System.getProperty("screens.out", "build/screens")),
                "widget.png"))) {
            WalkWidget.picture(app).compress(Bitmap.CompressFormat.PNG, 100, o);
        }
        // the menu, and pairing
        tap(v, v.width() - 10, 8);
        shot(v, "menu");
        v.back();
        v.showOver(new PairPage(v));
        shot(v, "pair");
    }

    /** The DEVICE page's devices and apps, with the snapshot given all three devices, conditions and ribbons */
    static void devices(GameView v, Companion app) throws Exception {
        Model m = app.model();
        JSONObject save = m.save();
        save.put("devices", new org.json.JSONArray("[1,1,1]"));
        save.put("rbadges", new org.json.JSONArray("[3,1,0,0]"));
        JSONObject first = m.party().getJSONObject(0);
        first.put("cond", new org.json.JSONArray("[120,40,200,80,30,140]"));
        first.put("rib", new org.json.JSONArray("[0,1,2,5,21,24,25]"));
        JSONObject place = m.snap.getJSONObject("place");
        place.put("friends", new org.json.JSONArray("[[\"" + app.art.toHex("MAY") + "\",0,14,6,0]]"));
        v.select(5);
        shot(v, "device_pokenav");
        // the live map with a place's finds
        app.places.put(1, new JSONObject("{\"t\":\"place\",\"sec\":1,\"trainers\":[[1,1,\"" + app.art.toHex("YOUNGSTER")
                + "\",\"" + app.art.toHex("CALVIN") + "\"],[2,0,\"" + app.art.toHex("BUG CATCHER") + "\",\""
                + app.art.toHex("RICK") + "\"]],\"items\":[[13,1,0],[17,0,1],[22,0,0]],\"berries\":[[1,133,5],[2,0,0]],"
                + "\"wild\":[[261,1,1],[263,1,0],[265,0,0],[270,1,0]]}"));
        v.showOver(new MapPage(v, "HOENN MAP"));
        shot(v, "device_map");
        v.showOver(new ConditionPage(v));
        shot(v, "device_condition");
        app.calls = new JSONObject("{\"t\":\"calls\",\"has\":1,\"list\":[{\"k\":0,\"i\":0,\"c\":\"" + app.art.toHex("LEADER")
                + "\",\"n\":\"" + app.art.toHex("ROXANNE") + "\",\"sec\":11,\"r\":0,\"pic\":65},{\"k\":1,\"i\":3,\"c\":\""
                + app.art.toHex("YOUNGSTER") + "\",\"n\":\"" + app.art.toHex("CALVIN") + "\",\"sec\":17,\"r\":1,\"pic\":38}]}");
        v.showOver(new CallsPage(v));
        shot(v, "device_calls");
        RibbonsPage rp = new RibbonsPage(v);
        v.showOver(rp);
        shot(v, "device_ribbons");
        tap(v, 60, GameView.TOP + 24 + 10);
        shot(v, "device_ribbons_mon");
        v.showOver(null);
        // the POKéGEAR's cards
        tap(v, 130, GameView.TOP + 28);
        shot(v, "device_pokegear");
        v.page(5).swipe(1);
        v.page(5).swipe(1);
        shot(v, "device_pokegear_phone");
        v.page(5).swipe(1);
        app.gear = new JSONObject("{\"t\":\"gear\",\"radio\":9,\"song\":1,\"name\":\"" + app.art.toHex("OAK'S POKéMON TALK")
                + "\",\"line\":\"" + app.art.toHex("With me, MARY!") + "\"}");
        v.page(5).press(60, GameView.TOP + 70);
        v.page(5).dragEnd(60, GameView.TOP + 70);
        shot(v, "device_pokegear_radio");
        // battle mode: the MOVE TESTER and the party's HP
        int sp = m.party().getJSONObject(0).getInt("s");
        app.battle = new JSONObject("{\"t\":\"battle\",\"on\":1,\"wild\":1,\"mons\":[{\"b\":0,\"side\":0,\"me\":1,\"s\":" + sp
                + ",\"l\":12,\"st\":0,\"ty\":[11,11],\"hp\":30,\"mhp\":38,\"m\":[33,55,39,145],\"pp\":[35,25,30,30]},"
                + "{\"b\":1,\"side\":1,\"me\":0,\"s\":261,\"l\":4,\"st\":0,\"ty\":[17,17],\"pct\":64}],"
                + "\"party\":[[" + sp + ",30,38,0,0],[252,0,20,0,0],[280,15,15,0,0]]}");
        shot(v, "device_battle");
        app.battle = null;
    }
}
