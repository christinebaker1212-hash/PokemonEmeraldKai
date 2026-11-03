package com.moemon.companion;

import android.app.ActivityManager;
import android.app.ApplicationExitInfo;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.List;

/**
 * Why the app closed last time, from what the app kept (a Java crash: Companion's handler) and from Android's own
 * record (ApplicationExitInfo, Android 11+: native crashes, freezes, the system ending it), shown once with a COPY
 * button so the player can send the whole report.
 */
public final class CrashReport {
    public final String summary;   // one line for the message
    public final String full;      // everything, for the clipboard

    private CrashReport(String summary, String full) {
        this.summary = summary;
        this.full = full;
    }

    static String reasonName(int r) {
        switch (r) {
            case ApplicationExitInfo.REASON_CRASH: return "CRASH";
            case ApplicationExitInfo.REASON_CRASH_NATIVE: return "CRASH NATIVE";
            case ApplicationExitInfo.REASON_ANR: return "NOT RESPONDING";
            case ApplicationExitInfo.REASON_LOW_MEMORY: return "LOW MEMORY";
            case ApplicationExitInfo.REASON_EXCESSIVE_RESOURCE_USAGE: return "TOO MUCH CPU";
            case ApplicationExitInfo.REASON_INITIALIZATION_FAILURE: return "START FAILED";
            case ApplicationExitInfo.REASON_SIGNALED: return "SIGNALED";
            case ApplicationExitInfo.REASON_DEPENDENCY_DIED: return "DEPENDENCY DIED";
            case ApplicationExitInfo.REASON_OTHER: return "OTHER";
            default: return "REASON " + r;
        }
    }

    /** The trace Android kept: text for a freeze; for a native crash, the readable parts of its tombstone */
    private static String trace(ApplicationExitInfo e) {
        try (InputStream in = e.getTraceInputStream()) {
            if (in == null) {
                return "";
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] b = new byte[16384];
            int n;
            while ((n = in.read(b)) > 0 && out.size() < 512 * 1024) {
                out.write(b, 0, n);
            }
            byte[] all = out.toByteArray();
            if (e.getReason() == ApplicationExitInfo.REASON_ANR) {
                String t = new String(all, "UTF-8");
                return t.length() > 6000 ? t.substring(0, 6000) : t;
            }
            // (a protobuf: its strings in order are the signal, the abort message and each frame's library / function)
            StringBuilder s = new StringBuilder();
            StringBuilder run = new StringBuilder();
            for (byte x : all) {
                if (x >= 32 && x < 127) {
                    run.append((char) x);
                } else {
                    if (run.length() >= 6) {
                        s.append(run).append('\n');
                    }
                    run.setLength(0);
                }
                if (s.length() > 6000) {
                    break;
                }
            }
            return s.toString();
        } catch (Exception ex) {
            return "";
        }
    }

    /** The report for the last close the player hasn't been told of, or null */
    public static CrashReport take(Context c, Companion app) {
        StringBuilder full = new StringBuilder();
        String summary = null;
        String java = app.lastCrash();
        if (java != null) {
            String[] lines = java.split("\n");
            summary = Companion.firstLine(lines.length > 1 ? lines[1] : java);
            full.append(java).append('\n');
        }
        if (Build.VERSION.SDK_INT >= 30) {
            SharedPreferences prefs = c.getSharedPreferences("moemon", 0);
            long seen = prefs.getLong("exitSeen", 0);
            ActivityManager am = (ActivityManager) c.getSystemService(Context.ACTIVITY_SERVICE);
            List<ApplicationExitInfo> list = am.getHistoricalProcessExitReasons(c.getPackageName(), 0, 6);
            if (!list.isEmpty()) {
                prefs.edit().putLong("exitSeen", list.get(0).getTimestamp()).apply();
            }
            // (the newest bad one not told of yet; the first time, ones from the last 6 hours: an update's own
            // exit comes after the crash before it)
            long now = System.currentTimeMillis();
            for (ApplicationExitInfo e : list) {
                int r = e.getReason();
                boolean bad = r == ApplicationExitInfo.REASON_CRASH || r == ApplicationExitInfo.REASON_CRASH_NATIVE
                        || r == ApplicationExitInfo.REASON_ANR || r == ApplicationExitInfo.REASON_LOW_MEMORY
                        || r == ApplicationExitInfo.REASON_EXCESSIVE_RESOURCE_USAGE
                        || r == ApplicationExitInfo.REASON_INITIALIZATION_FAILURE;
                boolean fresh = seen != 0 ? e.getTimestamp() > seen : now - e.getTimestamp() < 6 * 3600 * 1000L;
                if (!bad || !fresh) {
                    continue;
                }
                String d = e.getDescription() == null ? "" : e.getDescription();
                full.append("Android: ").append(reasonName(r)).append(" (").append(d).append("), status ")
                        .append(e.getStatus()).append(", ").append(Build.MANUFACTURER).append(' ').append(Build.MODEL)
                        .append(", Android ").append(Build.VERSION.RELEASE).append(", app ")
                        .append(Updater.version(c)).append('\n').append(trace(e));
                if (summary == null) {
                    summary = Companion.firstLine(reasonName(r) + " " + d);
                }
                break;
            }
        }
        return summary == null ? null : new CrashReport(summary, full.toString());
    }

    public void copy(Context c) {
        ClipboardManager cm = (ClipboardManager) c.getSystemService(Context.CLIPBOARD_SERVICE);
        cm.setPrimaryClip(ClipData.newPlainText("Moemon Companion report", full));
    }
}
