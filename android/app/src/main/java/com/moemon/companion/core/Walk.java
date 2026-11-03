package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.time.Instant;
import java.time.LocalDate;
import java.time.ZoneId;
import java.util.Random;

/**
 * A POKéMON on a walk with the phone (docs/COMPANION_APP.md "Pillar 2: the walk"; the game's side mp_phone_walk.c).
 * Android-free, so the desktop test runs it. It keeps the game's walk (its POKéMON, where it began, the place's wild
 * POKéMON and finds, the rules) and what the walk has done: steps by the day from the step counter's readings (at most
 * 10,000 a day; a reading after days away is spread over those days), FINDS and MEETINGS bought with steps (3 meetings
 * a day), and, at the end, the homecoming order the game checks against its own rules.
 */
public final class Walk {
    public JSONObject game;                 // the game's "walk": id, sent, mapsec, region, cap, mon, wild, finds, rules
    private JSONObject days = new JSONObject();     // "2026-11-03" -> steps that day
    private JSONArray finds = new JSONArray();      // items found
    private JSONArray caught = new JSONArray();     // [species, level, ball]
    private JSONArray met = new JSONArray();        // [species, level, caught 0/1, date]
    private long lastCounter = -1, lastBoot, lastAt;
    private final ZoneId zone;

    public Walk(JSONObject game, ZoneId zone) {
        this.game = game;
        this.zone = zone;
    }

    public long id() {
        return game.optLong("id");
    }

    private int rule(int i, int dflt) {
        JSONArray r = game.optJSONArray("rules");
        return r == null ? dflt : r.optInt(i, dflt);
    }

    public int stepsADay() {
        return rule(0, 10000);
    }

    public int stepsAFind() {
        return rule(1, 2000);
    }

    public int stepsAMeeting() {
        return rule(2, 3000);
    }

    public int stepsAMoment() {
        return rule(3, 2500);
    }

    public int meetingsADay() {
        return rule(4, 3);
    }

    private LocalDate date(long millis) {
        return Instant.ofEpochMilli(millis).atZone(zone).toLocalDate();
    }

    // ------------------------------------------------------------------------------------------------ steps

    /**
     * A step counter reading: COUNTER is the steps since the phone started (Android's TYPE_STEP_COUNTER), BOOT when it
     * started (millis), NOW the reading's time. The steps since the last reading go to the days between them, evenly,
     * none past a day's 10,000. The first reading of a walk only sets where counting starts.
     */
    public void reading(long counter, long boot, long now) {
        if (counter < 0) {
            return;
        }
        if (lastCounter < 0) {
            lastCounter = counter;
            lastBoot = boot;
            lastAt = Math.max(now, game.optLong("sent") * 1000);
            return;
        }
        long delta = counter - lastCounter, since = lastAt;
        if (Math.abs(boot - lastBoot) > 60000 || delta < 0) {
            delta = counter;   // (the phone restarted: its counter began again from 0, and counted only since then)
            since = Math.max(lastAt, boot);
        }
        LocalDate from = date(Math.min(since, now)), to = date(now);
        long spanned = Math.max(1, to.toEpochDay() - from.toEpochDay() + 1);
        long each = delta / spanned, extra = delta % spanned;
        for (long d = 0; d < spanned; d++) {
            add(from.plusDays(d), each + (d == spanned - 1 ? extra : 0));
        }
        lastCounter = counter;
        lastBoot = boot;
        lastAt = now;
    }

    /** Steps added to a day by hand (the tests' simulated walks) */
    public void add(LocalDate day, long steps) {
        String k = day.toString();
        long had = days.optLong(k);
        try {
            days.put(k, Math.min(stepsADay(), had + Math.max(0, steps)));
        } catch (JSONException ignored) {
        }
    }

    public long steps() {
        long n = 0;
        for (java.util.Iterator<String> it = days.keys(); it.hasNext(); ) {
            n += days.optLong(it.next());
        }
        return n;
    }

    public long stepsOn(LocalDate day) {
        return days.optLong(day.toString());
    }

    public int moments() {
        return (int) Math.min(40, steps() / stepsAMoment());
    }

    // ---------------------------------------------------------------------------------- finds and meetings

    public int findsLeft() {
        return (int) Math.max(0, steps() / stepsAFind() - finds.length());
    }

    public JSONArray finds() {
        return finds;
    }

    /** A find: one of the place's items (the game's pool); 0 when there are none to find yet */
    public int find(Random r) {
        JSONArray pool = game.optJSONArray("finds");
        if (findsLeft() <= 0 || pool == null || pool.length() == 0) {
            return 0;
        }
        int item = pool.optInt(r.nextInt(pool.length()));
        finds.put(item);
        return item;
    }

    public int meetingsToday(LocalDate today) {
        int n = 0;
        for (int i = 0; i < met.length(); i++) {
            n += today.toString().equals(met.optJSONArray(i).optString(3)) ? 1 : 0;
        }
        return n;
    }

    public int meetingsLeft(LocalDate today) {
        long bySteps = steps() / stepsAMeeting() - met.length();
        return (int) Math.max(0, Math.min(bySteps, meetingsADay() - meetingsToday(today)));
    }

    public JSONArray met() {
        return met;
    }

    public JSONArray caught() {
        return caught;
    }

    /** A meeting: a wild POKéMON of the place at one of its levels ([species, level]); null when none can be met */
    public int[] meet(Random r, LocalDate today) {
        JSONArray wild = game.optJSONArray("wild");
        if (meetingsLeft(today) <= 0 || wild == null || wild.length() == 0) {
            return null;
        }
        JSONArray w = wild.optJSONArray(r.nextInt(wild.length()));
        int lo = w.optInt(1), hi = Math.max(lo, w.optInt(2));
        return new int[]{w.optInt(0), lo + r.nextInt(hi - lo + 1)};
    }

    /** How the meeting ended: caught (in BALL) or it got away; either way it counts as one of the day's */
    public void met(int species, int level, boolean wasCaught, int ball, LocalDate today) {
        met.put(new JSONArray().put(species).put(level).put(wasCaught ? 1 : 0).put(today.toString()));
        if (wasCaught) {
            caught.put(new JSONArray().put(species).put(level).put(ball));
        }
    }

    // ----------------------------------------------------------------------------------------- coming home

    /** The homecoming order (the game checks every part of it against its own rules) */
    public JSONObject homeOrder() throws JSONException {
        return new JSONObject().put("op", "home").put("walk", id()).put("steps", steps()).put("finds", finds)
                .put("caught", caught);
    }

    // ------------------------------------------------------------------------------------------------ disk

    public JSONObject toJson() throws JSONException {
        return new JSONObject().put("game", game).put("days", days).put("finds", finds).put("caught", caught)
                .put("met", met).put("lastCounter", lastCounter).put("lastBoot", lastBoot).put("lastAt", lastAt);
    }

    public static Walk fromJson(JSONObject o, ZoneId zone) {
        Walk w = new Walk(o.optJSONObject("game"), zone);
        w.days = o.optJSONObject("days") != null ? o.optJSONObject("days") : new JSONObject();
        w.finds = o.optJSONArray("finds") != null ? o.optJSONArray("finds") : new JSONArray();
        w.caught = o.optJSONArray("caught") != null ? o.optJSONArray("caught") : new JSONArray();
        w.met = o.optJSONArray("met") != null ? o.optJSONArray("met") : new JSONArray();
        w.lastCounter = o.optLong("lastCounter", -1);
        w.lastBoot = o.optLong("lastBoot");
        w.lastAt = o.optLong("lastAt");
        return w;
    }
}
