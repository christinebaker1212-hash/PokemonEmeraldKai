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

        // the POKé BALL page: the second box POKéMON in the HAUNTED mix, a save with only the first pieces earned
        {
            JSONObject snap = file.getJSONObject("snap");
            snap.getJSONArray("boxes").getJSONObject(0).getJSONArray("m").getJSONObject(1).put("fx", 17813).put("fxs", 7)
                    .put("bst", 0x15);
            snap.put("fxu", new org.json.JSONArray("[\"FF7\",\"FFF\",\"FFFF\",\"7F\"]"));
            snap.put("fxp", new org.json.JSONArray("[[27354,5],0,0,0,0,0]"));
            write(new File(dir, "model_" + id + ".json"), file.toString());
        }
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
        Look.animations = false;   // (the pictures at once, not mid-slide)
        shot(v, "boxes");
        assertTrue(v.width() >= 240);

        // a POKéMON chosen, then a move waiting for the game (dashed)
        // (the box at its scale: as large as the screen allows in whole screen pixels, BoxesPage.bk)
        float bk = Math.max(1, Math.min(2, (float) Math.floor((v.width() - 28) * v.pixelScale() / 160f) / v.pixelScale()));
        int wpW = Math.round(160 * bk), wpH = Math.round(144 * bk), cl = (int) (24 * bk);
        int x0 = (v.width() - wpW) / 2 + (int) (8 * bk), y0 = GameView.TOP + 4 + (int) (16 * bk);
        tap(v, x0 + cl / 2, y0 + cl / 2);
        shot(v, "boxes_chosen");
        Model m = app.model();
        JSONObject from = Model.boxPlace(0, 0), to = Model.boxPlace(0, 29);
        app.client.orders.add(Model.move(from, m.at(from), to, null));
        shot(v, "boxes_waiting");
        // ALL BOXES: every box at once, the party pinned on top; SELECT two and they're outlined
        tap(v, v.width() - 20, GameView.TOP + 4 + wpH + 6 + 2);
        shot(v, "boxes_all");
        tap(v, v.width() - 15, GameView.TOP + 8);
        tap(v, 6 + 7 + 19 + 9, GameView.TOP + 62 + 14 + 9);
        tap(v, 6 + 7 + 38 + 9, GameView.TOP + 62 + 14 + 9);
        shot(v, "boxes_select");
        tap(v, 12, GameView.TOP + 8);

        String[] names = {"boxes", "party", "bag", "dex", "spawns"};
        for (int i = 1; i < names.length; i++) {
            v.select(i);
            shot(v, names[i]);
        }
        // the summary's three pages, from the box's card
        v.select(0);
        tap(v, x0 + cl + cl / 2, y0 + cl / 2);
        tap(v, 60, GameView.TOP + 4 + wpH + 6 + 36 + 22 + 20);
        shot(v, "summary_info");
        tap(v, v.width() / 2, GameView.TOP + 112 + 8);
        shot(v, "summary_skills");
        int tw = (v.width() - 16) / 4;
        tap(v, 8 + 2 * tw + tw / 2, GameView.TOP + 112 + 8);
        tap(v, 60, GameView.TOP + 136 + 4 + 12);
        shot(v, "summary_moves");
        // its POKé BALL: the page, then the SEND-OUT list open, then a locked piece picked
        tap(v, 8 + 3 * tw + tw / 2, GameView.TOP + 112 + 8);
        shot(v, "ball_page");
        BallPage bp = (BallPage) v.overPage();
        bp.openList(4);
        shot(v, "ball_list");
        bp.pickForTest(4, 20);
        bp.openList(-1);
        shot(v, "ball_locked");
        v.showOver(null);
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
        for (int k = 1; k <= 5; k++) {   // (its new parts: abilities, matchups, evolution, moves, breeding, training)
            v.overPage().scrollTo(320 + k * 480);
            shot(v, "dex_entry_more" + k);
        }
        v.overPage().scrollTo(0);
        v.showOver(new TypeChartPage(v));
        shot(v, "types_chart");
        v.overPage().tap(100, 26 + 14 + 12 + 2 + 16 * 4 + 6);
        shot(v, "types_chart_pick");
        v.showOver(new TypeChartPage(v).defending(12, 3));
        shot(v, "types_defense");
        v.showOver(new TypeChartPage(v).attacking(10));
        shot(v, "types_attack");
        v.showOver(null);
        v.showMenu(Look.menu(v));
        shot(v, "look_menu");
        for (int k = 0; k < 6; k++) {   // (NIGHT, and frame 2)
            Look.theme = (Look.theme + 1) % Look.THEMES.length;
        }
        Look.frame = 1;
        Look.apply();
        v.showMenu(Menu.message(v, "A window in frame 2, on the NIGHT theme."));
        shot(v, "look_night");
        Look.theme = 0;
        Look.frame = 0;
        Look.apply();
        v.showMenu(null);
        tap(v, 200, GameView.TOP + 72 + 8);   // (the type filter's picker)
        shot(v, "dex_types");
        tap(v, 14 + 38 + 4, GameView.TOP + 96 + 22 + 4);   // (FIRE)
        shot(v, "dex_fire");
        tap(v, 200, GameView.TOP + 72 + 8);
        tap(v, 20, GameView.TOP + 96 + 6);   // (ALL TYPES)
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
        // FRIENDS: two friends, one online in a room with their party
        String may = app.art.toHex("MAY"), wally = app.art.toHex("WALLY");
        app.model().snap.put("friends", new JSONObject("{\"share\":0,\"srv\":\"127.0.0.1:1\",\"list\":[{\"id\":\"0102030405060708\","
                + "\"key\":\"" + "11".repeat(32) + "\",\"pk\":\"" + "33".repeat(32) + "\",\"n\":\"" + may + "\",\"seen\":1},{\"id\":\"0807060504030201\",\"key\":\""
                + "22".repeat(32) + "\",\"n\":\"" + wally + "\",\"seen\":1}]}"));
        app.friends.put("0102030405060708", new JSONObject().put("n", may).put("t", System.currentTimeMillis() / 1000)
                .put("sec", 3).put("do", "field").put("room", "MAYS ROOM").put("rkey", "00").put("srv", "")
                .put("party", new org.json.JSONArray("[[25,20,0],[280,15,0],[0,0,1]]")));
        app.friends.put("0807060504030201", new JSONObject().put("n", wally).put("t", System.currentTimeMillis() / 1000 - 7200));
        app.friendsAt = System.currentTimeMillis();
        // trades by mail: WALLY's offer, ours on offer to MAY, one done
        long now = System.currentTimeMillis() / 1000;
        java.util.List<JSONObject> offers = new java.util.ArrayList<>();
        offers.add(new JSONObject().put("t", "AA".repeat(16)).put("from", "0807060504030201").put("n", wally)
                .put("mon", new JSONObject().put("s", 283).put("l", 12).put("n", app.art.toHex("MUDKIP"))).put("want", 0)
                .put("exp", now + 5 * 86400).put("esc", 0));
        app.offers = offers;
        app.model().snap.put("trades", new org.json.JSONArray()
                .put(new JSONObject().put("t", "BB".repeat(16)).put("role", 0).put("st", "offered").put("want", 280).put("pn", may)
                        .put("changed", now).put("mine", new JSONObject().put("s", 133).put("l", 18).put("n", app.art.toHex("EEVEE"))))
                .put(new JSONObject().put("t", "CC".repeat(16)).put("role", 1).put("st", "done").put("shown", 0).put("pn", wally)
                        .put("changed", now).put("mine", new JSONObject().put("s", 1).put("l", 20).put("n", app.art.toHex("BULBASAUR")))
                        .put("theirs", new JSONObject().put("s", 4).put("l", 16).put("n", app.art.toHex("CHARMANDER")))));
        v.select(7);
        shot(v, "friends");
        v.showOver(null);
        // CARDS: TRIPLE TRIAD with the game's engine: the hub, a duel under way, a deck, a pack opened
        StringBuilder owned = new StringBuilder();
        for (int card = 1; card <= 60; card++) {
            owned.append(card > 1 ? "," : "").append("[").append(card).append(",").append(card % 7 == 0 ? 2 : 1).append("]");
        }
        app.model().snap.put("tt", new JSONObject("{\"on\":1,\"day\":20395,\"region\":1,\"tier\":1,\"rules\":"
                + (Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME)) + ",\"trade\":0,\"skill\":0,\"active\":0,\"rec\":[12,5,1],"
                + "\"today\":{\"packs\":1,\"packsMax\":3,\"puzzles\":0,\"puzzlesMax\":1},\"packs\":[2,1,0,0,0],"
                + "\"decks\":[{\"n\":\"CHOCOBO RUN\",\"c\":[1,2,3,4,5]},{\"n\":\"DECK 2\",\"c\":[0,0,0,0,0]},"
                + "{\"n\":\"DECK 3\",\"c\":[0,0,0,0,0]},{\"n\":\"DECK 4\",\"c\":[0,0,0,0,0]},{\"n\":\"DECK 5\",\"c\":[0,0,0,0,0]},"
                + "{\"n\":\"DECK 6\",\"c\":[0,0,0,0,0]}],\"owned\":[" + owned + "],\"seen\":[70,71,72]}"));
        v.select(8);
        shot(v, "cards");
        TtDuelPage.Setup ds = new TtDuelPage.Setup();
        ds.rules = Tt.bit(Tt.RULE_OPEN) | Tt.bit(Tt.RULE_SAME);
        ds.seed = 12345;
        ds.first = 0;
        ds.level = 2;
        ds.mine = new int[]{1, 2, 3, 4, 5};
        ds.theirs = new int[]{6, 7, 8, 9, 10};
        ds.who = "LASS  ***";
        ds.moves = new org.json.JSONArray("[[0,4],[0,0],[1,8]]");
        TtDuelPage duelPage = new TtDuelPage(v, ds);
        v.showOver(duelPage);
        duelPage.tap(0, 0);
        shot(v, "cards_duel");
        v.showOver(new TtDeckPage(v, 0));
        shot(v, "cards_deck");
        TtPackPage pack = new TtPackPage(v, 1, new int[]{11, 61, 25});
        v.showOver(pack);
        Thread.sleep(3200);
        shot(v, "cards_pack");
        v.showOver(null);
        tcgScreens(v, app);
        ygoScreens(v, app);
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

    /** CARDS, the POKéMON card game: the hub, a duel under way (a card up close, a target to choose), a deck, a booster */
    static void tcgScreens(GameView v, Companion app) throws Exception {
        int[] charmander = Tcg.namedDeck("CharmanderAndFriends"), squirtle = Tcg.namedDeck("SquirtleAndFriends");
        java.util.Map<Integer, Integer> have = new java.util.TreeMap<>();
        for (int id : charmander) {
            have.merge(id, 1, Integer::sum);
        }
        for (int id = 8; id < 120; id += 3) {
            have.merge(id, 1, Integer::sum);
        }
        StringBuilder owned = new StringBuilder(), deck = new StringBuilder();
        for (java.util.Map.Entry<Integer, Integer> e : have.entrySet()) {
            owned.append(owned.length() == 0 ? "" : ",").append("[").append(e.getKey()).append(",").append(e.getValue()).append("]");
        }
        for (int id : charmander) {
            deck.append(deck.length() == 0 ? "" : ",").append(id);
        }
        StringBuilder empty = new StringBuilder();
        for (int i = 0; i < 60; i++) {
            empty.append(i == 0 ? "0" : ",0");
        }
        StringBuilder decks = new StringBuilder("{\"n\":\"EMBER FRIENDS\",\"ace\":" + charmander[59] + ",\"c\":[" + deck + "]}");
        for (int i = 1; i < 8; i++) {
            decks.append(",{\"n\":\"\",\"ace\":0,\"c\":[").append(empty).append("]}");
        }
        app.model().snap.put("tcg", new JSONObject("{\"on\":1,\"day\":20395,\"active\":0,\"skill\":12,\"rated\":4,\"badges\":3,"
                + "\"wins\":7,\"today\":{\"packs\":1,\"packsMax\":3},\"packs\":[1,0,2,0,0,0,0,1,0,0,0],\"decks\":[" + decks
                + "],\"owned\":[" + owned + "]}"));
        CardsPage.showGame(1);
        v.select(8);
        shot(v, "cards_tcg");
        v.page(8).scrollTo(420);
        shot(v, "cards_tcg_more");
        v.page(8).scrollTo(0);
        TcgDuelPage.Setup s = new TcgDuelPage.Setup();
        s.mine = charmander;
        s.theirs = squirtle;
        s.seed = 4242;
        s.level = 2;
        s.who = "BUG CATCHER  SKILLED";
        TcgDuelPage duel = new TcgDuelPage(v, s);
        v.showOver(duel);
        duel.autoPlay(60);
        shot(v, "tcg_duel");
        duel.inspect(0, 0);
        shot(v, "tcg_duel_card");
        duel.inspect(0, -1);
        duel.back();
        if (duel.beginHand("ATTACH") || duel.beginHand("PLAY")) {
            shot(v, "tcg_duel_target");
        }
        v.showOver(new TcgDeckPage(v, 0));
        shot(v, "tcg_deck");
        TcgPackPage pack = new TcgPackPage(v, 2, new int[]{8, 20, 33, 41, 50, 60, 77, 90, 3, 37}, 1 << 9 | 1 << 2);
        v.showOver(pack);
        Thread.sleep(4200);
        shot(v, "tcg_pack");
        v.showOver(new TcgBinderPage(v, 0, CardsPage.setName(0)));
        shot(v, "tcg_binder");
        v.showOver(new TcgCardPage(v, 37, 1, null));
        shot(v, "tcg_card");
        v.showOver(null);
        CardsPage.showGame(0);
    }

    /** CARDS, YU-GI-OH!: the hub, a duel under way (a card up close with what it can do), a deck, a booster, the binder */
    static void ygoScreens(GameView v, Companion app) throws Exception {
        app.ygoArt.engine();
        org.json.JSONArray sets = app.ygoArt.sets();
        int yugi = -1, kaiba = -1;
        for (int i = 0; i < sets.length(); i++) {
            if (sets.getJSONObject(i).getInt("starter") == 1) {
                if (yugi < 0) {
                    yugi = i;
                } else if (kaiba < 0) {
                    kaiba = i;
                }
            }
        }
        JSONObject dy = new JSONObject(Ygo.setDeck(yugi)), dk = new JSONObject(Ygo.setDeck(kaiba));
        java.util.Map<Integer, Integer> have = new java.util.TreeMap<>();
        org.json.JSONArray m = dy.getJSONArray("main");
        for (int i = 0; i < m.length(); i++) {
            have.merge(m.getInt(i), 1, Integer::sum);
        }
        for (int id = 1; id < 300; id += 4) {
            have.merge(id, 1, Integer::sum);
        }
        StringBuilder owned = new StringBuilder();
        for (java.util.Map.Entry<Integer, Integer> e : have.entrySet()) {
            owned.append(owned.length() == 0 ? "" : ",").append("[").append(e.getKey()).append(",").append(e.getValue()).append("]");
        }
        StringBuilder decks = new StringBuilder("{\"n\":\"YUGI\",\"ace\":" + m.getInt(0) + ",\"m\":" + m + ",\"x\":[]}");
        for (int i = 1; i < 8; i++) {
            decks.append(",{\"n\":\"\",\"ace\":0,\"m\":[],\"x\":[]}");
        }
        app.model().snap.put("ygo", new JSONObject("{\"on\":1,\"active\":0,\"skill\":0,\"rated\":2,\"badges\":3,\"wins\":4,"
                + "\"losses\":2,\"list\":\"\",\"today\":{\"packs\":0,\"packsMax\":3},\"packs\":[[0,2],[1,1]],\"decks\":[" + decks
                + "],\"owned\":[" + owned + "]}"));
        CardsPage.showGame(2);
        v.select(8);
        shot(v, "cards_ygo");
        v.page(8).scrollTo(420);
        shot(v, "cards_ygo_more");
        v.page(8).scrollTo(0);
        YgoDuelPage.Setup s = new YgoDuelPage.Setup();
        s.main0 = YgoDuelPage.ints(dy.getJSONArray("main"));
        s.extra0 = YgoDuelPage.ints(dy.getJSONArray("extra"));
        s.main1 = YgoDuelPage.ints(dk.getJSONArray("main"));
        s.extra1 = YgoDuelPage.ints(dk.getJSONArray("extra"));
        s.seed = 777;
        s.first = 1;
        s.level = 2;
        s.who = "DUELIST  SKILLED";
        YgoDuelPage duel = new YgoDuelPage(v, s);
        v.showOver(duel);
        duel.autoPlay(80);
        shot(v, "ygo_duel");
        if (duel.inspectHand()) {
            shot(v, "ygo_duel_card");
        }
        duel.inspect(-1);
        v.showOver(new YgoDeckPage(v, 0));
        shot(v, "ygo_deck");
        int[] pack = new int[9];
        for (int i = 0; i < 9; i++) {
            pack[i] = m.getInt(i * 3);
        }
        v.showOver(new YgoPackPage(v, 0, pack, 1 << 8 | 1 << 3));
        Thread.sleep(3600);
        shot(v, "ygo_pack");
        v.showOver(new YgoBinderPage(v, 0));
        shot(v, "ygo_binder");
        v.showOver(new YgoCardPage(v, m.getInt(0), 1, null));
        shot(v, "ygo_card");
        v.showOver(null);
        CardsPage.showGame(0);
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
