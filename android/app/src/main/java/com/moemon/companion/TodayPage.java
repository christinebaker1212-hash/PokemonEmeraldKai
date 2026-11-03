package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Walk;

import org.json.JSONArray;
import org.json.JSONObject;

import java.time.LocalDate;

/**
 * The daily report (docs/COMPANION_APP.md "Daily report"): the first time the app opens each day, one screen with the
 * walk's steps, training, BERRIES ready, EGGS, the day's goals and MoeGatcha's free pulls. Tap to go on.
 */
public final class TodayPage extends Page {
    private final Paint fill = new Paint();

    public TodayPage(GameView v) {
        super(v);
    }

    /** Shown today yet? (once a day) */
    public static boolean due(GameView v) {
        String today = LocalDate.now().toString();
        return v.app.model() != null && !today.equals(v.app.context.getSharedPreferences("moemon", 0).getString("today", ""));
    }

    @Override
    public void shown() {
        v.app.context.getSharedPreferences("moemon", 0).edit().putString("today", LocalDate.now().toString()).apply();
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    private int row(Canvas c, int w, int y, String what, String value, int color) {
        art.window(c, 0, 16, y + 4, w - 32, 16);
        art.text(c, what, 20, y + 4);
        art.text(c, value, w - 20 - art.width(value), y + 4, color, GameArt.SHADOW);
        return y + 26;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        fill.setColor(0xFF3868A8);
        c.drawRect(0, 0, w, h, fill);
        fill.setColor(0xFF4878B8);
        for (int y = 0; y < h; y += 12) {
            c.drawRect(0, y, w, y + 6, fill);
        }
        LocalDate d = LocalDate.now();
        String title = d.getDayOfWeek().toString() + ", " + d.getMonth().toString().substring(0, 3) + " " + d.getDayOfMonth();
        art.text(c, "GOOD DAY!", 16, 30, GOLD, 0xFF202850);
        art.text(c, title, 16, 46, GameArt.WHITE_TEXT, 0xFF202850);
        int y = 70;
        Model m = v.app.model();
        Walk walk = v.app.walk();
        if (walk != null) {
            JSONObject mon = walk.game.optJSONObject("mon");
            String name = mon == null ? "" : art.plain(GameArt.hexText(mon.optString("n")));
            y = row(c, w, y, name + "'S WALK", walk.stepsOn(d.minusDays(1)) + " YESTERDAY", GameArt.BLUE_TEXT);
            y = row(c, w, y, "STEPS IN ALL", "" + walk.steps(), GameArt.BLUE_TEXT);
        }
        if (m != null) {
            JSONObject train = m.snap.optJSONObject("train");
            JSONArray list = train == null ? null : train.optJSONArray("list");
            if (list != null && list.length() > 0) {
                y = row(c, w, y, "IN TRAINING", list.length() + "/" + train.optInt("slots"), GameArt.BLUE_TEXT);
            }
            JSONObject t = m.snap.optJSONObject("today");
            JSONArray b = t == null ? null : t.optJSONArray("berries");
            if (b != null && (b.optInt(0) > 0 || b.optInt(1) > 0)) {
                y = row(c, w, y, "BERRIES READY", b.optInt(0) + " (" + b.optInt(1) + " GROWING)", 0xFF209020);
            }
            int eggs = 0;
            JSONArray party = m.party();
            for (int i = 0; party != null && i < party.length(); i++) {
                eggs += party.optJSONObject(i) != null && party.optJSONObject(i).optInt("e") == 1 ? 1 : 0;
            }
            if (eggs > 0) {
                y = row(c, w, y, "EGGS IN THE PARTY", "" + eggs, GameArt.BLUE_TEXT);
            }
            JSONArray goals = t == null ? null : t.optJSONArray("goals");
            if (goals != null && goals.length() > 0) {
                art.text(c, "TODAY'S GOALS", 16, y + 4, GOLD, 0xFF202850);
                y += 20;
                for (int i = 0; i < goals.length(); i++) {
                    JSONArray g = goals.optJSONArray(i);
                    String text = g.optString(0).replace('~', 'é');
                    String p = g.optInt(1) >= g.optInt(2) ? "DONE!" : g.optInt(1) + "/" + g.optInt(2);
                    java.util.List<String> lines = wrap(text, w - 80);
                    art.window(c, 0, 16, y + 4, w - 32, lines.size() * 14 + 2);
                    for (int k = 0; k < lines.size(); k++) {
                        art.text(c, lines.get(k), 20, y + 4 + k * 14);
                    }
                    art.text(c, p, w - 20 - art.width(p), y + 4, g.optInt(1) >= g.optInt(2) ? 0xFF209020 : GameArt.TEXT,
                            GameArt.SHADOW);
                    y += lines.size() * 14 + 12;
                }
            }
            JSONArray g = t == null ? null : t.optJSONArray("gatcha");
            if (g != null && (g.optInt(0) > 0 || g.optInt(1) > 0)) {
                y = row(c, w, y + 4, "MOEGATCHA", g.optInt(0) + " FREE, " + g.optInt(1) + " TICKETS", GOLD);
            }
        }
        art.text(c, "Tap to go on.", (w - art.width("Tap to go on.")) / 2, h - 40, GameArt.WHITE_TEXT, 0xFF202850);
    }

    @Override
    public void tap(int x, int y) {
        back();
    }
}
