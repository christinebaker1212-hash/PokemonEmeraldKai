package com.moemon.companion;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInstaller;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

/**
 * Updates from the game's latest release on GitHub (docs/COMPANION_APP.md "Platform, install and updates"): the APK
 * attached to it, installed through Android's own "Install?" (the PackageInstaller session). Asked once a day, and
 * from MENU > UPDATE.
 */
public final class Updater {
    static final String LATEST = "https://api.github.com/repos/christinebaker1212-hash/PokemonEmeraldKai/releases/latest";

    private Updater() {}

    public static String version(Context c) {
        try {
            return c.getPackageManager().getPackageInfo(c.getPackageName(), 0).versionName;
        } catch (Exception e) {
            return "?";
        }
    }

    /** a < b as versions "1.70.0" */
    static boolean older(String a, String b) {
        String[] x = a.split("\\."), y = b.split("\\.");
        for (int i = 0; i < Math.max(x.length, y.length); i++) {
            int p = i < x.length ? num(x[i]) : 0, q = i < y.length ? num(y[i]) : 0;
            if (p != q) {
                return p < q;
            }
        }
        return false;
    }

    private static int num(String s) {
        try {
            return Integer.parseInt(s.replaceAll("[^0-9]", ""));
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private static String get(String url) throws Exception {
        HttpURLConnection c = (HttpURLConnection) new URL(url).openConnection();
        c.setConnectTimeout(8000);
        c.setReadTimeout(15000);
        c.setRequestProperty("Accept", "application/vnd.github+json");
        try (InputStream in = c.getInputStream()) {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] b = new byte[16384];
            int n;
            while ((n = in.read(b)) > 0) {
                out.write(b, 0, n);
            }
            return out.toString("UTF-8");
        }
    }

    public static void check(GameView v, boolean asked) {
        Context ctx = v.getContext();
        android.content.SharedPreferences prefs = ctx.getSharedPreferences("moemon", 0);
        long now = System.currentTimeMillis();
        if (!asked && now - prefs.getLong("updateChecked", 0) < 24 * 3600 * 1000L) {
            return;
        }
        prefs.edit().putLong("updateChecked", now).apply();
        Handler main = new Handler(Looper.getMainLooper());
        new Thread(() -> {
            try {
                JSONObject rel = new JSONObject(get(LATEST));
                String tag = rel.optString("tag_name").replaceFirst("^v", "");
                String apk = null;
                JSONArray assets = rel.optJSONArray("assets");
                for (int i = 0; assets != null && i < assets.length(); i++) {
                    JSONObject a = assets.getJSONObject(i);
                    if (a.optString("name").endsWith(".apk")) {
                        apk = a.optString("browser_download_url");
                    }
                }
                final String url = apk;
                if (url != null && older(version(ctx), tag)) {
                    main.post(() -> v.showMenu(Menu.confirm(v, "MOEMON COMPANION " + tag + " is out. Update now?",
                            () -> download(v, url))));
                } else if (asked) {
                    main.post(() -> v.message("This is the newest app."));
                }
            } catch (Exception e) {
                if (asked) {
                    main.post(() -> v.message("Couldn't reach the updates. Check the internet."));
                }
            }
        }, "moemon-update").start();
    }

    static void download(GameView v, String url) {
        Context ctx = v.getContext();
        if (!ctx.getPackageManager().canRequestPackageInstalls()) {
            // (once: Android asks to allow installs from this app)
            v.message("Allow \"Install unknown apps\" for MOEMON, then UPDATE again from the MENU.");
            ctx.startActivity(new Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES,
                    Uri.parse("package:" + ctx.getPackageName())));
            return;
        }
        v.message("Downloading the update...");
        Handler main = new Handler(Looper.getMainLooper());
        new Thread(() -> {
            try {
                HttpURLConnection c = (HttpURLConnection) new URL(url).openConnection();
                c.setInstanceFollowRedirects(true);
                c.setConnectTimeout(10000);
                c.setReadTimeout(30000);
                PackageInstaller pi = ctx.getPackageManager().getPackageInstaller();
                PackageInstaller.SessionParams params = new PackageInstaller.SessionParams(
                        PackageInstaller.SessionParams.MODE_FULL_INSTALL);
                int id = pi.createSession(params);
                try (PackageInstaller.Session s = pi.openSession(id);
                     InputStream in = c.getInputStream();
                     OutputStream out = s.openWrite("moemon.apk", 0, -1)) {
                    byte[] b = new byte[65536];
                    int n;
                    while ((n = in.read(b)) > 0) {
                        out.write(b, 0, n);
                    }
                    s.fsync(out);
                    out.close();
                    Intent done = new Intent(ctx, InstallReceiver.class);
                    PendingIntent pend = PendingIntent.getBroadcast(ctx, id, done,
                            PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_MUTABLE);
                    s.commit(pend.getIntentSender());
                }
            } catch (Exception e) {
                main.post(() -> v.message("The download didn't finish. Try again later."));
            }
        }, "moemon-download").start();
    }
}
