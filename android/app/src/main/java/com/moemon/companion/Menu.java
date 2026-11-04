package com.moemon.companion;

import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.SavedLink;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/** A menu or message in the game's window over the screen: rows to tap, or words and a tap to close */
public final class Menu {
    public interface Action {
        void run();
    }

    private final GameView v;
    private final GameArt art;
    public Menu parent;
    private final String title;
    private final List<String> rows = new ArrayList<>();
    private final List<Action> actions = new ArrayList<>();
    private final List<String> lines = new ArrayList<>();
    private int x, y, mw, mh, rowY;
    private final Paint dim = new Paint();

    private Menu(GameView v, String title) {
        this.v = v;
        this.art = v.art;
        this.title = title;
        dim.setColor(0x80000000);
    }

    /** No rows yet (a message's OK taken away, to give it its own) */
    public Menu setRows() {
        rows.clear();
        actions.clear();
        return this;
    }

    public Menu add(String row, Action a) {
        rows.add(row);
        actions.add(a);
        return this;
    }

    public static Menu make(GameView v, String title) {
        return new Menu(v, title);
    }

    /** Words in a window; long ones wrap to the window's width */
    public static Menu message(GameView v, String text) {
        Menu m = new Menu(v, null);
        int max = Math.min(v.width() - 40, 220);
        for (String para : text.split("\n")) {
            StringBuilder line = new StringBuilder();
            for (String word : para.split(" ")) {
                String next = line.length() == 0 ? word : line + " " + word;
                if (v.art.width(next) > max && line.length() > 0) {
                    m.lines.add(line.toString());
                    line = new StringBuilder(word);
                } else {
                    line = new StringBuilder(next);
                }
            }
            m.lines.add(line.toString());
        }
        m.add("OK", null);
        return m;
    }

    private final long born = System.currentTimeMillis();
    /** (replacing a menu on screen, as a setting's row reopens its menu: no opening again) */
    boolean still;

    public void draw(Canvas c, int w, int h) {
        // (a menu opening: the dim fades in, the window grows from a little smaller)
        float e = still ? 1f : GameView.eased(born, 140);
        int dimAlpha = dim.getAlpha();
        dim.setAlpha((int) (dimAlpha * e));
        c.drawRect(0, 0, w, h, dim);
        dim.setAlpha(dimAlpha);
        if (e < 1f) {
            v.postInvalidateOnAnimation();
        }
        c.save();
        float k = 0.92f + 0.08f * e;
        c.scale(k, k, w / 2f, h / 2f);
        drawWindow(c, w, h);
        c.restore();
    }

    private void drawWindow(Canvas c, int w, int h) {
        int widest = title == null ? 0 : art.width(title);
        for (String r : rows) {
            widest = Math.max(widest, art.width(r) + 12);
        }
        for (String l : lines) {
            widest = Math.max(widest, art.width(l));
        }
        mw = Math.min(w - 24, Math.max(120, widest + 16));
        mh = (title != null ? 18 : 0) + lines.size() * 16 + (lines.isEmpty() ? 0 : 6) + rows.size() * 20;
        x = (w - mw) / 2;
        y = Math.max(GameView.TOP + 12, (h - mh) / 2);
        art.window(c, 0, x, y, mw, mh);
        int ty = y;
        if (title != null) {
            art.text(c, title, x + 6, ty, GameArt.BLUE_TEXT, GameArt.SHADOW);
            ty += 18;
        }
        for (String l : lines) {
            art.text(c, l, x + 6, ty);
            ty += 16;
        }
        if (!lines.isEmpty()) {
            ty += 6;
        }
        rowY = ty;
        for (String r : rows) {
            art.text(c, r, x + 14, ty + 2);
            ty += 20;
        }
    }

    public void tap(int tx, int ty) {
        if (tx < x - 8 || tx >= x + mw + 8 || ty < y - 8 || ty >= y + mh + 8) {
            v.showMenu(parent);   // (outside: close)
            return;
        }
        int i = (ty - rowY) / 20;
        if (ty >= rowY && i >= 0 && i < rows.size()) {
            v.sound.play("se_select");
            v.showMenu(parent);
            if (actions.get(i) != null) {
                actions.get(i).run();
            }
        }
    }

    /** The app's menu (the dots on top): saves, pairing, what couldn't be done, updates, text size */
    /** Each kind of notification on its own switch, how many a day, the quiet hours */
    static Menu notifications(GameView v) {
        android.content.Context c = v.getContext();
        Menu m = make(v, "NOTIFICATIONS");
        for (int i = 0; i < Notifier.KINDS.length; i++) {
            final int k = i;
            m.add((Notifier.on(c, k) ? "ON  " : "OFF ") + Notifier.NAMES[k], () -> {
                Notifier.set(c, k, !Notifier.on(c, k));
                v.showMenu(notifications(v));
            });
        }
        m.add("A DAY: " + Notifier.perDay(c), () -> {
            int n = Notifier.perDay(c) % 5 + 1;
            Notifier.prefs(c).edit().putInt("notif_perDay", n).apply();
            v.showMenu(notifications(v));
        });
        int from = Notifier.prefs(c).getInt("quietFrom", 21), to = Notifier.prefs(c).getInt("quietTo", 8);
        m.add("QUIET " + from + ":00 TO " + to + ":00", () -> {
            int[][] choices = {{21, 8}, {22, 7}, {20, 9}, {23, 6}, {0, 0}};
            int next = 0;
            for (int i = 0; i < choices.length; i++) {
                if (choices[i][0] == from && choices[i][1] == to) {
                    next = (i + 1) % choices.length;
                }
            }
            Notifier.prefs(c).edit().putInt("quietFrom", choices[next][0]).putInt("quietTo", choices[next][1]).apply();
            v.showMenu(notifications(v));
        });
        m.add("CANCEL", null);
        return m;
    }

    /** The game's sounds and its music (the card duels' songs) on their own switches */
    static Menu sound(GameView v) {
        Sound s = v.sound;
        Menu m = make(v, "SOUND");
        m.add((s.effectsOn() ? "ON  " : "OFF ") + "SOUND EFFECTS", () -> {
            s.setEffects(!s.effectsOn());
            v.showMenu(sound(v));
        });
        m.add((s.musicOn() ? "ON  " : "OFF ") + "MUSIC", () -> {
            s.setMusic(!s.musicOn());
            v.showMenu(sound(v));
        });
        m.add("CANCEL", null);
        return m;
    }

    public static Menu main(GameView v) {
        Companion app = v.app;
        Menu m = make(v, "MENU");
        m.add("SAVE FILES", () -> {
            Menu s = make(v, "SAVE FILES");
            for (SavedLink l : app.saves) {
                String name = l.name.isEmpty() ? l.id().substring(0, 6) : nameText(v, l.name);
                boolean on = app.client != null && app.client.saved() == l;
                s.add((on ? "> " : "") + name, () -> {
                    Menu one = make(v, name);
                    one.add("USE", () -> app.use(l));
                    one.add("FORGET", () -> {
                        // (a POKéMON out walking with this phone comes home first: a lost link must never mean a
                        // lost POKéMON; FORGET PHONES in the game brings it home as it left)
                        if (app.client != null && app.client.saved() == l && app.walk() != null) {
                            v.message("A POKéMON is out on a walk with this phone. Bring it home first (WALK > HOME).");
                            return;
                        }
                        v.showMenu(confirm(v, "Forget this save? Pair again to link it.", () -> app.forget(l)));
                    });
                    v.showMenu(one);
                });
            }
            s.add("PAIR A GAME", () -> v.showOver(new PairPage(v)));
            v.showMenu(s);
        });
        if (app.client != null) {
            int n = app.client.orders.failed().size();
            m.add("COULDN'T DO (" + n + ")", () -> v.showMenu(failed(v)));
            m.add("LINK AGAIN", () -> app.client.kick());
        }
        m.add("TEXT SIZE", () -> {
            Menu t = make(v, "TEXT SIZE");
            String[] names = {"FIT", "2x", "3x", "4x"};
            for (int i = 0; i < names.length; i++) {
                final int k = i == 0 ? 0 : i + 1;
                t.add(names[i], () -> {
                    v.getContext().getSharedPreferences("moemon", 0).edit().putInt("scale", k).apply();
                    v.rescale();
                });
            }
            v.showMenu(t);
        });
        m.add("LOOK", () -> v.showMenu(Look.menu(v)));
        m.add("NOTIFICATIONS", () -> v.showMenu(notifications(v)));
        m.add("SOUND", () -> v.showMenu(sound(v)));
        m.add("UPDATE", () -> Updater.check(v, true));
        m.add("ABOUT", () -> v.message("MOEMON COMPANION " + Updater.version(v.getContext())
                + "\nThe game's own POKéMON, words and art. Pair it from OPTIONS > CO-OP > PHONE LINK in the game."));
        return m;
    }

    static String nameText(GameView v, String hex) {
        // (the player's name in plain letters for a menu row: the game's letters back through the charmap)
        return v.art.plain(GameArt.hexText(hex));
    }

    public static Menu confirm(GameView v, String text, Action yes) {
        Menu m = message(v, text);
        m.rows.clear();
        m.actions.clear();
        m.add("YES", yes);
        m.add("NO", null);
        return m;
    }

    static Menu failed(GameView v) {
        Companion app = v.app;
        List<JSONObject> f = app.client.orders.failed();
        StringBuilder s = new StringBuilder();
        if (f.isEmpty()) {
            s.append("Everything asked of the game was done.");
        }
        for (int i = Math.max(0, f.size() - 6); i < f.size(); i++) {
            JSONObject o = f.get(i);
            String op = o.optString("op");
            s.append(op.equals("move") ? "MOVE: " : op.equals("give") ? "GIVE: " : "TAKE: ")
                    .append(Companion.why(o.optString("why"))).append("\n");
        }
        Menu m = message(v, s.toString().trim());
        if (!f.isEmpty()) {
            m.rows.clear();
            m.actions.clear();
            m.add("CLEAR", () -> {
                app.client.orders.clearFailed();
                app.changed();
            });
            m.add("OK", null);
        }
        return m;
    }

    static void openScan(GameView v) {
        ((MainActivity) v.getContext()).startActivityForResult(new Intent(v.getContext(), ScanActivity.class), 1);
    }
}
