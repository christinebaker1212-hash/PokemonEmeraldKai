package com.moemon.companion;

import android.graphics.Canvas;
import android.graphics.Paint;

import com.moemon.companion.core.Model;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * The POKéDEX by national number, as the save has it: seen ones by their icon, caught ones with a POKé BALL, unseen
 * ones a silhouette and "?????" as in the game. Filters (ALL, SEEN, CAUGHT, MISSING) and a sort by name.
 */
public final class DexPage extends Page {
    private static final String[] FILTERS = {"ALL", "SEEN", "CAUGHT", "MISSING"};
    private static final int HEAD = 44, CELL_H = 44;
    private int filter, entry = -1;
    private boolean byName;
    private final Paint ball = new Paint();
    private List<Integer> shown = new ArrayList<>();

    public DexPage(GameView v) {
        super(v);
    }

    static boolean bit(String hex, int dex) {
        int i = (dex - 1) / 4;
        if (hex == null || i >= hex.length()) {
            return false;
        }
        int nib = Character.digit(hex.charAt(i), 16);
        return (nib & (8 >> ((dex - 1) % 4))) != 0;
    }

    private JSONObject dex() {
        Model m = v.app.model();
        JSONObject d = m == null ? null : m.snap.optJSONObject("dex");
        return d != null ? d : new JSONObject();
    }

    private List<Integer> list() {
        JSONObject d = dex();
        String seen = d.optString("seen"), caught = d.optString("caught");
        List<Integer> out = new ArrayList<>();
        for (int n = 1; n <= d.optInt("count"); n++) {
            boolean s = bit(seen, n), c = bit(caught, n);
            if (filter == 1 && !s || filter == 2 && !c || filter == 3 && c) {
                continue;
            }
            out.add(n);
        }
        if (byName) {
            // (unseen ones have no name yet: they stay by number after the rest)
            Collections.sort(out, (a, b) -> {
                boolean sa = bit(seen, a), sb = bit(seen, b);
                if (sa != sb) {
                    return sa ? -1 : 1;
                }
                if (!sa) {
                    return a - b;
                }
                return art.plain(art.speciesName(art.speciesOfDex(a))).compareTo(art.plain(art.speciesName(art.speciesOfDex(b))));
            });
        }
        return out;
    }

    private int cols() {
        return Math.max(1, (v.width() - 8) / 40);
    }

    @Override
    public int scrollMax() {
        return Math.max(0, (shown.size() + cols() - 1) / cols() * CELL_H + HEAD - v.contentHeight());
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONObject d = dex();
        String seen = d.optString("seen"), caught = d.optString("caught");

        shown = list();
        int cols = cols(), cw = (w - 8) / cols;
        for (int i = 0; i < shown.size(); i++) {
            int n = shown.get(i), x = 4 + (i % cols) * cw, y = HEAD + (i / cols) * CELL_H - scroll;
            if (y < HEAD - CELL_H || y > h) {
                continue;
            }
            boolean s = bit(seen, n), k = bit(caught, n);
            art.icon(c, art.speciesOfDex(n), x + (cw - 32) / 2, y, s ? GameArt.ICON_PLAIN : GameArt.ICON_SILHOUETTE);
            String num = String.format(java.util.Locale.US, "%03d", n);
            art.text(c, num, x + (cw - art.width(num)) / 2, y + 26, k ? GameArt.TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
            if (k) {
                ball.setColor(0xFFF85838);
                c.drawRect(x + cw - 9, y + 2, x + cw - 3, y + 5, ball);
                ball.setColor(0xFFF8F8F8);
                c.drawRect(x + cw - 9, y + 5, x + cw - 3, y + 8, ball);
            }
        }
        // the header: counts, filters, sort
        int ns = 0, nc = 0;
        for (int n = 1; n <= d.optInt("count"); n++) {
            ns += bit(seen, n) ? 1 : 0;
            nc += bit(caught, n) ? 1 : 0;
        }
        banner(c, w, "POKéDEX", "SEEN " + ns + "  OWN " + nc);
        Paint bar = new Paint();
        bar.setColor(BANNER_DARK);
        c.drawRect(0, 20, w, HEAD - 2, bar);
        String sort = byName ? "ABC" : "NO.";
        int fw = (w - 40) / FILTERS.length;
        for (int i = 0; i < FILTERS.length; i++) {
            boolean on = i == filter;
            if (on) {
                art.window(c, 0, i * fw + 4, 24, fw - 8, 12);
            }
            int nw = art.width(FILTERS[i]);
            art.text(c, FILTERS[i], i * fw + (fw - nw) / 2, 22, on ? GameArt.TEXT : GameArt.WHITE_TEXT,
                    on ? GameArt.SHADOW : 0xFF202850);
        }
        art.window(c, 0, w - 34, 24, 28, 12);
        art.text(c, sort, w - 32, 22, GameArt.TEXT, GameArt.SHADOW);
    }

    @Override
    public void tap(int x, int y) {
        if (y < 20) {
            return;
        }
        if (y < HEAD) {
            if (x > v.width() - 40) {
                byName = !byName;
            } else {
                filter = Math.min(FILTERS.length - 1, x / ((v.width() - 40) / FILTERS.length));
            }
            scroll = 0;
            return;
        }
        int cols = cols(), cw = (v.width() - 8) / cols;
        int i = (y + scroll - HEAD) / CELL_H * cols + Math.min(cols - 1, Math.max(0, (x - 4) / cw));
        if (i >= 0 && i < shown.size()) {
            v.showOver(new DexEntryPage(v, new ArrayList<>(shown), i));
        }
    }

    @Override
    public void shown() {
        entry = -1;
    }
}
