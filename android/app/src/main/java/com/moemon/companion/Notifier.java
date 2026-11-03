package com.moemon.companion;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

import com.moemon.companion.core.Model;
import com.moemon.companion.core.Walk;

import org.json.JSONArray;
import org.json.JSONObject;

import java.time.LocalDate;
import java.time.LocalTime;

/**
 * The app's notifications (docs/COMPANION_APP.md "Notifications"): rare, in the POKéMON's own voice, a few a day at
 * most (3 by default), none in the quiet hours (21:00 to 08:00 by default), each kind on its own switch, on the phone's
 * own clock. Each kind once a day.
 */
public final class Notifier {
    public static final String[] KINDS = {"BERRY", "TRAINING", "EGG", "GARDEN", "FRIEND", "TURN", "REMATCH", "GATCHA"};
    public static final String[] NAMES = {"BERRIES ready", "Training done", "EGG ready to hatch", "A Garden moment",
            "A friend starts playing", "A trade or duel turn", "MOENAV rematch call", "New MoeGatcha lineup"};
    private static final boolean[] ON = {true, true, true, true, false, true, false, false};
    private static final String CHANNEL = "moemon";

    private Notifier() {}

    static SharedPreferences prefs(Context c) {
        return c.getSharedPreferences("moemon", 0);
    }

    public static boolean on(Context c, int kind) {
        return prefs(c).getBoolean("notif_" + KINDS[kind], ON[kind]);
    }

    public static void set(Context c, int kind, boolean on) {
        prefs(c).edit().putBoolean("notif_" + KINDS[kind], on).apply();
    }

    public static int perDay(Context c) {
        return prefs(c).getInt("notif_perDay", 3);
    }

    public static boolean quiet(Context c, LocalTime t) {
        int from = prefs(c).getInt("quietFrom", 21), to = prefs(c).getInt("quietTo", 8), h = t.getHour();
        return from > to ? h >= from || h < to : h >= from && h < to;
    }

    /** Whatever is due now: posts at most what today's budget has left */
    public static void check(Context c, Companion app) {
        LocalDate today = LocalDate.now();
        if (quiet(c, LocalTime.now())) {
            return;
        }
        Model m = app.model();
        Walk w = app.walk();
        // an EGG on the walk ready to hatch (its cycles all walked off: 256 steps each)
        if (w != null) {
            JSONObject mon = w.game.optJSONObject("mon");
            if (mon != null && mon.optInt("e") == 1 && w.steps() / 256 >= mon.optInt("f")) {
                post(c, 2, today, "Your EGG is moving!", "It's ready to hatch. Bring it home to the game.");
            }
        }
        if (m != null) {
            // BERRIES ready to pick (as of the last link)
            JSONObject t = m.snap.optJSONObject("today");
            JSONArray b = t == null ? null : t.optJSONArray("berries");
            if (b != null && b.optInt(0) > 0) {
                post(c, 0, today, "BERRIES are ready!", b.optInt(0) + " BERRY trees have BERRIES to pick.");
            }
            // a training that has done a whole day's worth
            JSONObject train = m.snap.optJSONObject("train");
            JSONArray list = train == null ? null : train.optJSONArray("list");
            long now = m.snap.optLong("now") + (System.currentTimeMillis() - m.asOf) / 1000;
            for (int i = 0; list != null && i < list.length(); i++) {
                if (now - list.optJSONObject(i).optLong("last") >= 86400) {
                    post(c, 1, today, "Training is done for the day", "Collect it on the WALK page.");
                    break;
                }
            }
        }
        // a Garden moment: a word from the walker, or the party's first
        JSONObject who = w != null ? w.game.optJSONObject("mon") : null;
        if (who == null && m != null && m.party() != null && m.party().length() > 0) {
            who = m.party().optJSONObject(0);
        }
        if (who != null && who.optInt("e") != 1) {
            String name = app.art.plain(GameArt.hexText(who.optString("n")));
            String[] lines = w != null ? new String[]{name + " is enjoying the walk!", name + " sniffs the breeze happily.",
                    name + " spotted something shiny!"} : new String[]{name + " is thinking of you.",
                    name + " is waiting for you in the GARDEN."};
            post(c, 3, today, name, lines[today.getDayOfYear() % lines.length]);
        }
    }

    // a notification of KIND, once a day, while today's budget lasts
    private static void post(Context c, int kind, LocalDate today, String title, String text) {
        SharedPreferences p = prefs(c);
        if (!on(c, kind) || today.toString().equals(p.getString("notif_last_" + KINDS[kind], ""))) {
            return;
        }
        int used = today.toString().equals(p.getString("notif_day", "")) ? p.getInt("notif_used", 0) : 0;
        if (used >= perDay(c)) {
            return;
        }
        NotificationManager nm = (NotificationManager) c.getSystemService(Context.NOTIFICATION_SERVICE);
        if (nm == null) {
            return;
        }
        nm.createNotificationChannel(new NotificationChannel(CHANNEL, "Moemon", NotificationManager.IMPORTANCE_DEFAULT));
        PendingIntent open = PendingIntent.getActivity(c, kind, new Intent(c, MainActivity.class),
                PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_UPDATE_CURRENT);
        Notification n = new Notification.Builder(c, CHANNEL)
                .setSmallIcon(android.R.drawable.star_on)
                .setContentTitle(title)
                .setContentText(text)
                .setContentIntent(open)
                .setAutoCancel(true)
                .build();
        try {
            nm.notify(100 + kind, n);
        } catch (SecurityException e) {
            return;   // (notifications not allowed)
        }
        p.edit().putString("notif_last_" + KINDS[kind], today.toString()).putString("notif_day", today.toString())
                .putInt("notif_used", used + 1).apply();
    }
}
