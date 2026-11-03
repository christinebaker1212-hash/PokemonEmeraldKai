package com.moemon.companion;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.widget.RemoteViews;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Walk;

import org.json.JSONObject;

import java.time.LocalDate;

/**
 * The home-screen widget (docs/COMPANION_APP.md "Home-screen widget"): the walker (or the party's first POKéMON, your
 * follower) in the game's pixels, with today's steps or the day's line. Drawn by GameArt into a picture.
 */
public final class WalkWidget extends AppWidgetProvider {
    @Override
    public void onUpdate(Context c, AppWidgetManager m, int[] ids) {
        try {
            update(c, m, ids, Companion.get(c));
        } catch (Exception ignored) {
        }
    }

    public static void updateAll(Context c, Companion app) {
        AppWidgetManager m = AppWidgetManager.getInstance(c);
        int[] ids = m.getAppWidgetIds(new ComponentName(c, WalkWidget.class));
        if (ids.length > 0) {
            update(c, m, ids, app);
        }
    }

    private static void update(Context c, AppWidgetManager m, int[] ids, Companion app) {
        RemoteViews rv = new RemoteViews(c.getPackageName(), R.layout.widget_walk);
        rv.setImageViewBitmap(R.id.widget_picture, picture(app));
        rv.setOnClickPendingIntent(R.id.widget_root, PendingIntent.getActivity(c, 0, new Intent(c, MainActivity.class),
                PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT));
        m.updateAppWidget(ids, rv);
    }

    /** The widget's picture: 120x72 game pixels, at 3x */
    static Bitmap picture(Companion app) {
        int s = 3, w = 120, h = 72;
        Bitmap b = Bitmap.createBitmap(w * s, h * s, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(s, s);
        GameArt art = app.art;
        Paint fill = new Paint();
        fill.setColor(0xFF88C8F8);
        c.drawRect(0, 0, w, h, fill);
        fill.setColor(0xFF68B848);
        c.drawRect(0, 48, w, h, fill);
        Walk walk = app.walk();
        Model model = app.model();
        JSONObject mon = walk != null ? walk.game.optJSONObject("mon") : null;
        if (mon == null && model != null && model.party() != null && model.party().length() > 0) {
            mon = model.party().optJSONObject(0);
        }
        if (mon != null) {
            if (mon.optInt("e") == 1) {
                art.icon(c, art.data.optInt("egg"), 8, 24, GameArt.ICON_PLAIN);
            } else {
                art.front(c, mon.optInt("s"), mon.optString("pal", null), 0, 8, 64, false);
            }
        }
        art.window(c, 0, 64, 10, 50, 50);
        if (walk != null) {
            art.text(c, "TODAY", 66, 10, GameArt.GRAY_TEXT, GameArt.SHADOW);
            art.text(c, "" + walk.stepsOn(LocalDate.now()), 66, 24, GameArt.BLUE_TEXT, GameArt.SHADOW);
            art.text(c, "STEPS", 66, 38);
        } else if (mon != null) {
            art.text(c, art.plain(GameArt.hexText(mon.optString("n"))), 66, 10);
            art.text(c, "is in", 66, 24);
            art.text(c, "the game.", 66, 38);
        } else {
            art.text(c, "MOEMON", 66, 10);
            art.text(c, "COMPANION", 66, 24);
        }
        return b;
    }
}
