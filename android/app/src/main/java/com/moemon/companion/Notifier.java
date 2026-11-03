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

    /** Friends who've started playing since the last look (FRIEND: off unless turned on; once a friend a day) */
    public static void friends(Context c, Companion app) {
        if (!on(c, 4) || quiet(c, LocalTime.now())) {
            return;
        }
        Model m = app.model();
        JSONObject f = m == null ? null : m.snap.optJSONObject("friends");
        JSONArray list = f == null ? null : f.optJSONArray("list");
        if (list == null || list.length() == 0) {
            return;
        }
        java.util.Map<String, JSONObject> p;
        try {
            p = com.moemon.companion.core.Friends.fetch(app.crypto, f.optString("srv"), list);
        } catch (Exception e) {
            return;
        }
        long now = System.currentTimeMillis() / 1000;
        LocalDate today = LocalDate.now();
        for (int i = 0; i < list.length(); i++) {
            JSONObject friend = list.optJSONObject(i);
            String id = friend.optString("id");
            boolean was = prefs(c).getBoolean("online_" + id, false), is = com.moemon.companion.core.Friends.online(p.get(id), now);
            prefs(c).edit().putBoolean("online_" + id, is).apply();
            if (is && !was && !today.toString().equals(prefs(c).getString("friendDay_" + id, ""))) {
                String name = app.art.plain(GameArt.hexText(friend.optString("n")));
                prefs(c).edit().putString("friendDay_" + id, today.toString()).remove("notif_last_FRIEND").apply();
                post(c, 4, today, name + " is playing!", "Say hi, or join their room from FRIENDS.");
            }
        }
    }

    /** A friend's new offer of a trade by mail (TURN: each offer once; not held to once a day) */
    public static void trades(Context c, Companion app) {
        if (!on(c, 5) || quiet(c, LocalTime.now())) {
            return;
        }
        Model m = app.model();
        JSONObject f = m == null ? null : m.snap.optJSONObject("friends");
        if (f == null) {
            return;
        }
        java.util.List<JSONObject> offers;
        try {
            offers = com.moemon.companion.core.Trades.incoming(app.crypto, f.optString("srv"), f);
        } catch (Exception e) {
            return;
        }
        app.offers = offers;
        String told = prefs(c).getString("offersTold", "");
        for (JSONObject o : offers) {
            if ((o.optInt("esc") & (com.moemon.companion.core.Server.CLOSED | com.moemon.companion.core.Server.HAVE1)) != 0
                    || told.contains(o.optString("t"))) {
                continue;
            }
            told = (o.optString("t") + "," + told);
            told = told.substring(0, Math.min(told.length(), 33 * 32));
            prefs(c).edit().putString("offersTold", told).remove("notif_last_TURN").apply();
            String peer = app.art.plain(GameArt.hexText(o.optString("n")));
            JSONObject mon = o.optJSONObject("mon");
            String name = mon == null ? "a POKéMON" : app.art.plain(GameArt.hexText(mon.optString("n")));
            post(c, 5, LocalDate.now(), peer + " wants to trade!", peer + " offers " + name + ". See it in FRIENDS.");
            break;
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
