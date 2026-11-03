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
    static final int[] LAND = {20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1};
    static final int[] WATER = {60, 30, 5, 4, 1};
    static final int[] FISH = {70, 30, 60, 20, 20, 40, 40, 15, 4, 1};

    /** A slot's chance (method 0 grass, 1 surf, 2 rock smash, 3 rods) and its method's name (the rod for fishing) */
    static int rate(int method, int slot) {
        int[] r = method == 0 ? LAND : method == 3 ? FISH : WATER;
        return slot < r.length ? r[slot] : 0;
    }

    static String method(int method, int slot) {
        switch (method) {
            case 0: return "GRASS";
            case 1: return "SURF";
            case 2: return "ROCK SMASH";
            default: return slot < 2 ? "OLD ROD" : slot < 5 ? "GOOD ROD" : "SUPER ROD";
        }
    }

    /** The map's index in the spawn maps, or -1 */
    int indexOf(int group, int num) {
        JSONArray l = maps();
        for (int i = 0; i < l.length(); i++) {
            JSONArray m = l.optJSONObject(i).optJSONArray("map");
            if (m != null && m.optInt(0) == group && m.optInt(1) == num) {
                return i;
            }
        }
        return -1;
    }

    /** Open a map's POKéMON (from the POKéDEX's WHERE TO FIND) */
    void openMap(int index) {
        if (index < 0) {
            return;
        }
        open = index;
        openScroll = 0;
        scroll = 0;
        JSONObject m = maps().optJSONObject(index);
        if (v.app.client != null) {
            try {
                v.app.client.send(new JSONObject().put("t", "spawns").put("map", m.optJSONArray("map")));
            } catch (org.json.JSONException ignored) {
            }
        }
    }
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
            return Math.max(0, n * 30 + 70 - v.contentHeight());
        }
        return Math.max(0, rowsOfList().size() * ROW + HEAD + 8 - v.contentHeight());
    }

    private JSONObject spawnsOf(int i) {
        JSONObject m = maps().optJSONObject(i);
        return m == null ? null : v.app.spawns.get(key(m.optJSONArray("map")));
    }

    private static final String[] REGIONS = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    private static final int HEAD = 22;

    /** The list's rows: a region's name (-1 - region) before its maps, then each map's index in spawnmaps */
    private java.util.List<Integer> rowsOfList() {
        java.util.List<Integer> out = new java.util.ArrayList<>();
        JSONArray l = maps();
        for (int r = 0; r < 4; r++) {
            boolean any = false;
            for (int i = 0; i < l.length(); i++) {
                if (l.optJSONObject(i).optInt("region") == r) {
                    if (!any) {
                        out.add(-1 - r);
                        any = true;
                    }
                    out.add(i);
                }
            }
        }
        return out;
    }

    /** A place's name, numbered when the game has several areas by that name (a cave's floors) */
    String label(int i) {
        JSONArray l = maps();
        String name = l.optJSONObject(i).optString("name");
        int n = 0, k = 0;
        for (int j = 0; j < l.length(); j++) {
            if (l.optJSONObject(j).optString("name").equals(name)) {
                n++;
                if (j <= i) {
                    k = n;
                }
            }
        }
        String plain = art.plain(GameArt.hexText(name));
        return n > 1 && k > 1 ? plain + " " + k : plain;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        if (open >= 0) {
            drawMap(c, w, h);
            return;
        }
        JSONArray l = maps();
        if (l.length() == 0) {
            art.window(c, 0, 12, HEAD + 8, w - 24, 32);
            art.text(c, "Link with the game to see", 16, HEAD + 8);
            art.text(c, "where POKéMON live.", 16, HEAD + 24);
            banner(c, w, "SPAWNS", "");
            return;
        }
        java.util.List<Integer> rows = rowsOfList();
        for (int k = 0; k < rows.size(); k++) {
            int y = HEAD + 2 + k * ROW - scroll, i = rows.get(k);
            if (y < HEAD - ROW || y > h) {
                continue;
            }
            if (i < 0) {
                art.text(c, REGIONS[-1 - i], 8, y + 2, GOLD, 0xFF305060);
                continue;
            }
            JSONObject m = l.optJSONObject(i);
            art.window(c, 0, 12, y + 4, w - 24, ROW - 8);
            art.text(c, label(i), 14, y + 3);
            JSONArray has = m.optJSONArray("has");
            int cx = w - 16;
            String[] names = {"GRASS", "SURF", "ROCK", "ROD"};
            int[] colors = {0xFF48A848, 0xFF4878D8, 0xFFA08048, 0xFF3898B8};
            for (int t = 3; t >= 0; t--) {
                if (has != null && has.optInt(t) == 1) {
                    cx -= art.width(names[t]) + 8;
                    chip(c, names[t], cx, y + 7, colors[t]);
                }
            }
        }
        banner(c, w, "SPAWNS", l.length() + " PLACES");
    }

    private void drawMap(Canvas c, int w, int h) {
        JSONObject m = maps().optJSONObject(open);
        int y = HEAD + 2 - scroll;
        JSONObject sp = spawnsOf(open);
        if (sp == null) {
            art.window(c, 0, 12, y + 8, w - 24, 16);
            art.text(c, v.app.client != null && v.app.client.status() == com.moemon.companion.core.LinkClient.Status.LINKED
                    ? "Asking the game..." : "Link with the game to see this.", 14, y + 8);
        } else {
            String method = "";
            int rare = 0, found = 0;
            for (Row r : rows(sp)) {
                if (r.rare) {
                    rare++;
                    found += r.metCount > 0 ? 1 : 0;
                }
            }
            if (rare > 0) {
                art.text(c, "RARE ONES FOUND: " + found + "/" + rare, 8, y + 2, GameArt.WHITE_TEXT, 0xFF305060);
                y += 18;
            }
            for (Row r : rows(sp)) {
                if (!r.method.equals(method)) {
                    method = r.method;
                    y += 4;
                    art.text(c, method, 8, y, GOLD, 0xFF305060);
                    y += 18;
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
                    art.text(c, pct, w - 18 - art.width(pct), y + 2);
                    android.graphics.Paint f = new android.graphics.Paint();
                    f.setColor(0xFFD0D8E0);
                    c.drawRect(w - 70, y + 20, w - 18, y + 23, f);
                    f.setColor(r.rare ? 0xFFD89010 : 0xFF48A848);
                    c.drawRect(w - 70, y + 20, w - 70 + Math.max(1, r.rate * 52 / 100), y + 23, f);
                }
                y += 30;
            }
        }
        banner(c, w, label(open), "BACK");
    }

    @Override
    public void tap(int x, int y) {
        if (open >= 0) {
            if (y < 20 && x > v.width() - 60) {
                open = -1;
                scroll = openScroll;
            }
            return;
        }
        if (y < HEAD) {
            return;
        }
        java.util.List<Integer> rows = rowsOfList();
        int k = (y + scroll - HEAD - 2) / ROW;
        if (k < 0 || k >= rows.size() || rows.get(k) < 0) {
            return;
        }
        int i = rows.get(k);
        open = i;
        openScroll = scroll;
        scroll = 0;
        JSONObject m = maps().optJSONObject(i);
        if (v.app.client != null) {
            try {
                v.app.client.send(new JSONObject().put("t", "spawns").put("map", m.optJSONArray("map")));
            } catch (org.json.JSONException ignored) {
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
