package com.moemon.companion;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * The app's look, the player's own (MENU > LOOK): a color theme for the pages' backgrounds and banners, the window
 * frame (any of the game's, as its OPTIONS > FRAME), the animations on or off, the page the app opens on. Kept in the
 * app's settings; the pages read the colors from Page's fields, the windows the frame from GameArt.
 */
public final class Look {
    /** name, stripes, stripes' second color, banner, banner's dark edge */
    static final Object[][] THEMES = {
            {"SKY", 0xFF70B8C8, 0xFF68B0C0, 0xFF3868A8, 0xFF284878},
            {"EMERALD", 0xFF78C898, 0xFF70C090, 0xFF2E8A58, 0xFF1E6040},
            {"RUBY", 0xFFE89898, 0xFFE09090, 0xFFB83848, 0xFF802838},
            {"SAPPHIRE", 0xFF80A0E0, 0xFF7898D8, 0xFF3050A8, 0xFF203878},
            {"LAVENDER", 0xFFB8A8D8, 0xFFB0A0D0, 0xFF6848A8, 0xFF483078},
            {"SAND", 0xFFE0C890, 0xFFD8C088, 0xFFA87830, 0xFF785420},
            {"NIGHT", 0xFF485068, 0xFF404860, 0xFF283048, 0xFF181E30},
    };
    static final int FRAMES = 29;
    static final String[] START = {"BOXES", "PARTY", "BAG", "DEX", "DEVICE", "WALK", "CARDS", "FRIENDS", "LAST"};
    static final int[] START_PAGES = {0, 1, 2, 3, 5, 6, 8, 7, -1};

    static int theme, frame, start;
    static boolean animations = true;
    private static boolean loaded;

    static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences("moemon", 0);
    }

    static void load(Context c) {
        SharedPreferences p = prefs(c);
        theme = clamp(p.getInt("look_theme", 0), THEMES.length);
        frame = clamp(p.getInt("look_frame", 0), FRAMES);
        start = clamp(p.getInt("look_start", 0), START.length);
        animations = p.getBoolean("look_anim", true);
        loaded = true;
        apply();
    }

    static void ensure(Context c) {
        if (!loaded) {
            load(c);
        }
    }

    private static int clamp(int v, int n) {
        return v < 0 || v >= n ? 0 : v;
    }

    static void save(Context c) {
        prefs(c).edit().putInt("look_theme", theme).putInt("look_frame", frame).putInt("look_start", start)
                .putBoolean("look_anim", animations).apply();
        apply();
    }

    static void apply() {
        Object[] t = THEMES[theme];
        Page.SKY = (Integer) t[1];
        Page.SKY2 = (Integer) t[2];
        Page.BANNER = (Integer) t[3];
        Page.BANNER_DARK = (Integer) t[4];
        GameArt.userFrame = frame;
    }

    /** The page the app opens on (LAST: the one it was on) */
    static int startPage(Context c) {
        int p = START_PAGES[start];
        return p >= 0 ? p : prefs(c).getInt("look_last", 0);
    }

    static void rememberPage(Context c, int page) {
        if (START_PAGES[start] < 0) {
            prefs(c).edit().putInt("look_last", page).apply();
        }
    }

    /** MENU > LOOK: each row steps its choice on, the page behind shows it at once */
    static Menu menu(GameView v) {
        Context c = v.getContext();
        Menu m = Menu.make(v, "LOOK");
        m.add("THEME: " + THEMES[theme][0], () -> {
            theme = (theme + 1) % THEMES.length;
            save(c);
            v.showMenu(menu(v));
        });
        m.add("WINDOW: FRAME " + (frame + 1), () -> {
            frame = (frame + 1) % FRAMES;
            save(c);
            v.showMenu(menu(v));
        });
        m.add("ANIMATIONS: " + (animations ? "ON" : "OFF"), () -> {
            animations = !animations;
            save(c);
            v.showMenu(menu(v));
        });
        m.add("OPEN ON: " + START[start], () -> {
            start = (start + 1) % START.length;
            save(c);
            v.showMenu(menu(v));
        });
        m.add("DEFAULTS", () -> {
            theme = frame = start = 0;
            animations = true;
            save(c);
            v.showMenu(menu(v));
        });
        m.add("DONE", null);
        return m;
    }
}
