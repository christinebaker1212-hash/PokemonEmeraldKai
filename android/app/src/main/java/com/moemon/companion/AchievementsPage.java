package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * Every achievement, as the game's ACHIEVEMENTS menu has them ({"t":"ach"}: mp_phone_kai.c): a category at a time
 * (chips along the top, ALL first), each with its words, its progress and its reward (MOEGATCHA pulls, a POKé BALL
 * piece); TO DO / DONE narrow the list. Done ones carry a gold mark.
 */
public class AchievementsPage extends Page {
    private static final int HEAD = 46, ROW_GAP = 4;
    private final Page parent;
    private final Paint fill = new Paint();
    private int cat = -1, show;   // show: 0 all, 1 to do, 2 done
    private long asked;
    private int contentH;

    public AchievementsPage(GameView v, Page parent) {
        super(v);
        this.parent = parent;
    }

    @Override
    public void shown() {
        ask();
    }

    private void ask() {
        if (v.app.client != null && System.currentTimeMillis() - asked > 3000) {
            asked = System.currentTimeMillis();
            try {
                v.app.client.send(new JSONObject().put("t", "ach"));
            } catch (org.json.JSONException ignored) {
            }
        }
    }

    private List<JSONArray> rows() {
        List<JSONArray> out = new ArrayList<>();
        JSONArray l = v.app.ach == null ? null : v.app.ach.optJSONArray("list");
        for (int i = 0; l != null && i < l.length(); i++) {
            JSONArray a = l.optJSONArray(i);
            if ((cat < 0 || a.optInt(0) == cat) && (show == 0 || (show == 1) == (a.optInt(5) == 0))) {
                out.add(a);
            }
        }
        return out;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        background(c, w, h, scroll);
        JSONObject ach = v.app.ach;
        JSONArray l = ach == null ? null : ach.optJSONArray("list");
        int done = 0;
        for (int i = 0; l != null && i < l.length(); i++) {
            done += l.optJSONArray(i).optInt(5);
        }
        if (ach == null) {
            banner(c, w, "ACHIEVEMENTS", "");
            art.window(c, 0, 12, 30, w - 24, 16);
            art.text(c, v.app.client == null ? "Link with your game to see them." : "Asking your game...", 18, 30);
            ask();
            return;
        }
        // the list (under the chips)
        int y = HEAD + 4 - scroll;
        c.save();
        c.clipRect(0, HEAD, w, h);
        for (JSONArray a : rows()) {
            List<String> desc = wrap(a.optString(2), w - 60);
            int rh = 20 + desc.size() * 12 + 14;
            if (y + rh > HEAD && y < h) {
                boolean got = a.optInt(5) == 1;
                art.window(c, 0, 12, y + 4, w - 24, rh - 6);
                // the mark: a gold medal when done, an empty ring when not
                fill.setColor(got ? 0xFFB08010 : 0xFFA0A8B0);
                c.drawCircle(22, y + 12, 6, fill);
                fill.setColor(got ? 0xFFF8D040 : 0xFFF0F4F8);
                c.drawCircle(22, y + 12, 4, fill);
                art.text(c, a.optString(1), 32, y + 2, got ? GameArt.TEXT : GameArt.BLUE_TEXT, GameArt.SHADOW);
                String st = a.optString(3);
                art.text(c, st, w - 18 - art.width(st), y + 2, got ? 0xFF208830 : GameArt.GRAY_TEXT, GameArt.SHADOW);
                int ty = y + 16;
                for (String d : desc) {
                    art.text(c, d, 32, ty, GameArt.GRAY_TEXT, GameArt.SHADOW);
                    ty += 12;
                }
                art.small(c, a.optString(4), 32, ty, 0xFFC07818, GameArt.SHADOW);
            }
            y += rh + ROW_GAP;
        }
        c.restore();
        contentH = y + scroll + 8;
        // the banner and the chips on top
        fill.setColor(SKY);
        c.drawRect(0, 0, w, HEAD, fill);
        banner(c, w, "ACHIEVEMENTS", done + " / " + (l == null ? 0 : l.length()));
        int x = 6;
        String[] shows = {"ALL", "TO DO", "DONE"};
        for (int i = 0; i < 3; i++) {
            x += DexPage.chip(c, art, fill, shows[i], x, 22, show == i) + 3;
        }
        // the category: < name >
        JSONArray cats = ach.optJSONArray("cats");
        String cn = cat < 0 ? "EVERY KIND" : cats.optString(cat);
        int cx = x + 6, cw = w - 6 - cx;
        art.text(c, "<", cx, 21, GameArt.WHITE_TEXT, 0xFF202850);
        art.text(c, ">", w - 12, 21, GameArt.WHITE_TEXT, 0xFF202850);
        art.text(c, cn, cx + (cw - art.width(cn)) / 2, 21, GameArt.WHITE_TEXT, 0xFF202850);
    }

    @Override
    public int scrollMax() {
        return Math.max(0, contentH - v.contentHeight());
    }

    @Override
    public void tap(int x, int y) {
        if (y >= 20 && y < HEAD) {
            int cx = 6;
            String[] shows = {"ALL", "TO DO", "DONE"};
            for (int i = 0; i < 3; i++) {
                int bw = art.width(shows[i]) + 12;
                if (x >= cx && x < cx + bw) {
                    show = i;
                    scroll = 0;
                    v.sound.play("se_select");
                    return;
                }
                cx += bw + 3;
            }
            JSONArray cats = v.app.ach == null ? null : v.app.ach.optJSONArray("cats");
            int n = cats == null ? 0 : cats.length();
            int mid = (cx + v.width()) / 2;
            cat = x < mid ? (cat < 0 ? n - 1 : cat - 1) : (cat + 1 >= n ? -1 : cat + 1);
            scroll = 0;
            v.sound.play("se_select");
        }
    }

    @Override
    public void swipe(int dir) {
        JSONArray cats = v.app.ach == null ? null : v.app.ach.optJSONArray("cats");
        int n = cats == null ? 0 : cats.length();
        cat = dir > 0 ? (cat + 1 >= n ? -1 : cat + 1) : (cat < 0 ? n - 1 : cat - 1);
        scroll = 0;
        v.sound.play("se_select");
    }

    @Override
    public void back() {
        v.showOver(parent == null || parent instanceof MePage ? null : parent);
    }
}
