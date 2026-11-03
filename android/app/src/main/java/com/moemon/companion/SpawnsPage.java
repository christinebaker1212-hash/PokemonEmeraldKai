package com.moemon.companion;

import android.graphics.Canvas;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * The spawn viewer (docs/COMPANION_APP.md "POKéDEX and spawn viewer"): every map with wild POKéMON, then a map's
 * POKéMON by method as the game makes them now (its GENERATION's pools). Common and uncommon ones always show; a rare
 * one (only in slots of 1% or less) is a silhouette and a hint until you've met it there with that method.
 */
public final class SpawnsPage extends Page {
    private static final int ROW = 22;
    // the game's slot rates (wild_encounter.c): grass 12, water 5, ROCK SMASH 5, fishing 10 (OLD 0-1, GOOD 2-4, SUPER 5-9)
    private static final int[] LAND = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};
    private static final int[] WATER = {60, 30, 5, 4, 1};
    private static final int[] FISH = {70, 30, 60, 20, 20, 40, 40, 15, 4, 1};
    private int open = -1;            // the map open (its index in spawnmaps)
    private int openScroll;

    public SpawnsPage(GameView v) {
        super(v);
    }

    private JSONArray maps() {
        JSONObject m = v.app.spawnMaps;
        JSONArray a = m == null ? null : m.optJSONArray("maps");
        return a != null ? a : new JSONArray();
    }

    private static final class Row {
        String method;
        int species, rate, min = 999, max, metCount;
        boolean rare = true;
        String hint;
    }

    private static String key(JSONArray map) {
        return map.optInt(0) + "," + map.optInt(1);
    }

    /** A map's POKéMON, grouped by method and species, with their chance */
    private List<Row> rows(JSONObject sp) {
        List<Row> out = new ArrayList<>();
        String[][] parts = {{"land", "GRASS"}, {"water", "SURF"}, {"rock", "ROCK SMASH"}, {"fish", "FISHING"}};
        for (String[] part : parts) {
            JSONObject m = sp.optJSONObject(part[0]);
            if (m == null) {
                continue;
            }
            JSONArray slots = m.optJSONArray("slots");
            int[] rates = part[0].equals("land") ? LAND : part[0].equals("fish") ? FISH : WATER;
            Map<String, Row> by = new LinkedHashMap<>();
            for (int i = 0; slots != null && i < slots.length(); i++) {
                JSONArray s = slots.optJSONArray(i);
                String method = part[1];
                if (part[0].equals("fish")) {
                    method = i < 2 ? "OLD ROD" : i < 5 ? "GOOD ROD" : "SUPER ROD";
                }
                String k = method + ":" + s.optInt(0);
                Row r = by.get(k);
                if (r == null) {
                    r = new Row();
                    r.method = method;
                    r.species = s.optInt(0);
                    by.put(k, r);
                }
                int rate = i < rates.length ? rates[i] : 0;
                r.rate += rate;
                r.min = Math.min(r.min, s.optInt(1));
                r.max = Math.max(r.max, s.optInt(2));
                r.metCount += s.optInt(3);
                r.rare &= rate <= 1;
            }
            for (Row r : by.values()) {
                if (r.rare) {
                    r.hint = r.method.equals("GRASS") ? "Something rare rustles here."
                            : r.method.endsWith("ROD") ? "Only now and then on a " + r.method + "."
                            : r.method.equals("SURF") ? "Something rare swims here."
                            : "Something rare hides in the rocks.";
                }
                out.add(r);
            }
        }
        return out;
    }

    @Override
    public int scrollMax() {
        if (open >= 0) {
            JSONObject sp = spawnsOf(open);
            int n = sp == null ? 0 : rows(sp).size() + 4;
            return Math.max(0, n * 30 + 40 - v.contentHeight());
        }
        return Math.max(0, maps().length() * ROW + 30 - v.contentHeight());
    }

    private JSONObject spawnsOf(int i) {
        JSONObject m = maps().optJSONObject(i);
        return m == null ? null : v.app.spawns.get(key(m.optJSONArray("map")));
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        if (open >= 0) {
            drawMap(c, w, h);
            return;
        }
        JSONArray l = maps();
        if (l.length() == 0) {
            art.window(c, 0, 12, 20, w - 24, 32);
            art.text(c, "Link with the game to see", 16, 20);
            art.text(c, "where POKéMON live.", 16, 36);
            return;
        }
        for (int i = 0; i < l.length(); i++) {
            int y = 4 + i * ROW - scroll;
            if (y < -ROW || y > h) {
                continue;
            }
            JSONObject m = l.optJSONObject(i);
            art.window(c, 0, 12, y + 4, w - 24, ROW - 8);
            art.textHex(c, m.optString("name"), 14, y + 3, GameArt.TEXT, GameArt.SHADOW);
            JSONArray has = m.optJSONArray("has");
            String ways = (has != null && has.optInt(0) == 1 ? "G" : "") + (has != null && has.optInt(1) == 1 ? "S" : "")
                    + (has != null && has.optInt(2) == 1 ? "R" : "") + (has != null && has.optInt(3) == 1 ? "F" : "");
            art.text(c, ways, w - 18 - art.width(ways), y + 3, GameArt.GRAY_TEXT, GameArt.SHADOW);
        }
    }

    private void drawMap(Canvas c, int w, int h) {
        JSONObject m = maps().optJSONObject(open);
        int y = 4 - scroll;
        art.textHex(c, m.optString("name"), 8, y, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        button(c, "BACK", w - 70, y - 4, 66, true);
        y += 24;
        JSONObject sp = spawnsOf(open);
        if (sp == null) {
            art.window(c, 0, 12, y + 4, w - 24, 16);
            art.text(c, v.app.client != null && v.app.client.status() == com.moemon.companion.core.LinkClient.Status.LINKED
                    ? "Asking the game..." : "Link with the game to see this.", 14, y + 4);
            return;
        }
        String method = "";
        for (Row r : rows(sp)) {
            if (!r.method.equals(method)) {
                method = r.method;
                art.text(c, method, 8, y + 4, 0xFFF8E070, GameArt.WHITE_SHADOW);
                y += 22;
            }
            art.window(c, 0, 12, y + 4, w - 24, 22);
            boolean hidden = r.rare && r.metCount == 0;
            art.icon(c, r.species, 8, y, hidden ? GameArt.ICON_SILHOUETTE : GameArt.ICON_PLAIN);
            if (hidden) {
                art.text(c, r.hint, 40, y + 6, GameArt.GRAY_TEXT, GameArt.SHADOW);
            } else {
                art.text(c, art.speciesName(r.species), 40, y, GameArt.TEXT, GameArt.SHADOW);
                String lv = r.min == r.max ? "Lv" + r.min : "Lv" + r.min + "-" + r.max;
                art.text(c, lv, 40, y + 13, GameArt.GRAY_TEXT, GameArt.SHADOW);
                String pct = r.rate + "%";
                art.text(c, pct, w - 18 - art.width(pct), y + 6);
            }
            y += 30;
        }
        // how many of its rare ones were found
        int rare = 0, found = 0;
        for (Row r : rows(sp)) {
            if (r.rare) {
                rare++;
                found += r.metCount > 0 ? 1 : 0;
            }
        }
        if (rare > 0) {
            art.text(c, "RARE ONES FOUND: " + found + "/" + rare, 8, y + 4, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        }
    }

    @Override
    public void tap(int x, int y) {
        if (open >= 0) {
            if (y + scroll < 24 && x > v.width() - 70) {
                open = -1;
                scroll = openScroll;
            }
            return;
        }
        int i = (y + scroll - 4) / ROW;
        JSONArray l = maps();
        if (i >= 0 && i < l.length()) {
            open = i;
            openScroll = scroll;
            scroll = 0;
            JSONObject m = l.optJSONObject(i);
            if (v.app.client != null) {
                try {
                    v.app.client.send(new JSONObject().put("t", "spawns").put("map", m.optJSONArray("map")));
                } catch (org.json.JSONException ignored) {
                }
            }
        }
    }

    @Override
    public void shown() {
        if (v.app.spawnMaps == null && v.app.client != null) {
            v.app.client.ask("spawnmaps");
        }
    }
}
