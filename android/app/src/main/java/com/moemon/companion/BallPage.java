package com.moemon.companion;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;

import com.moemon.companion.core.BallFx;
import com.moemon.companion.core.Model;
import com.moemon.companion.core.Planned;

import org.json.JSONArray;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * A POKéMON's POKé BALL (the SUMMARY's BALL tab), the game's own POKé BALL page and ball effect editor on the phone:
 * - THE BALL: the kind of ball (its own, or another from the BAG, which costs one), the color and decoration of a POKé
 *   BALL, its icon drawn as the game draws it (mp_ballstyle.c: the red top's five shades, the 5x5 emblem)
 * - SEND-OUT & RETURN: PRESET (the game's mixes and MY PRESET 1-6), SEND-OUT, PARTICLES, ENERGY, RETURN, SOUND
 * The stage on top plays the piece being looked at, as the game's editor plays it (FxClips: recorded from the game).
 * Pieces the save hasn't earned can be looked at, with where they come from, but not kept. SAVE sends the changes
 * (Model.ballStyle / ballFx); PRESETS loads or saves MY PRESETs; PARTY gives the effect to the whole party.
 */
public class BallPage extends Page {
    private static final int ROW_BALL = 0, ROW_COLOR = 1, ROW_DECO = 2, ROW_PRESET = 3, ROW_FORM = 4, ROW_PART = 5,
            ROW_ENERGY = 6, ROW_RECALL = 7, ROW_SOUND = 8, ROWS = 9;
    private static final String[] LABELS = {"BALL", "COLOR", "DECOR", "PRESET", "SEND-OUT", "PARTICLES", "ENERGY",
            "RETURN", "SOUND"};
    private static final int ROW_H = 22, HEAD_H = 16, BAR_H = 30;
    private static FxClips sClips;

    private final JSONObject place;
    private final Page parent;   // (the SUMMARY it was opened from: BACK goes there)
    private final Paint fill = new Paint(), pix = new Paint();
    private final Rect src = new Rect(), dst = new Rect();
    private final long opened = System.currentTimeMillis();

    // the draft: what the page shows and SAVE sends
    private int ball, color, deco;
    private BallFx fx = new BallFx();
    private int origBall, origStyle;
    private BallFx origFx = new BallFx();
    private String loadedFor;   // (the POKéMON the draft was made from: p/o, and its ball and effect as they were)

    private int focus = ROW_FORM;   // the row last touched: the stage plays its piece, the info box tells of it
    private int picking = -1;       // a row whose list is open
    private int listScroll;
    private long clipStart = System.currentTimeMillis();
    private int clipKind = -1, clipPiece = -1;

    public BallPage(GameView v, JSONObject place, Page parent) {
        super(v);
        this.place = place;
        this.parent = parent;
        pix.setFilterBitmap(false);
        if (sClips == null) {
            sClips = new FxClips(v.getContext().getAssets());
        }
    }

    // --------------------------------------------------------------------------------------------- data

    private JSONObject mon() {
        Planned pl = new Planned(v.app.model(), v.app.client != null ? v.app.client.orders.pending()
                : java.util.Collections.<JSONObject>emptyList());
        return pl.at(place);
    }

    private JSONObject balls() {
        JSONObject b = art.data.optJSONObject("balls");
        return b != null ? b : new JSONObject();
    }

    private JSONObject fxData() {
        JSONObject b = art.data.optJSONObject("ballfx");
        return b != null ? b : new JSONObject();
    }

    private JSONArray pieces(int kind) {
        String key = kind == BallFx.KIND_FORM ? "forms" : kind == BallFx.KIND_PART ? "parts" : "recalls";
        JSONArray a = fxData().optJSONArray(key);
        return a != null ? a : new JSONArray();
    }

    private String pieceName(int kind, int i) {
        if (kind == BallFx.KIND_COLOR) {
            JSONArray c = balls().optJSONArray("colors");
            JSONArray e = c == null ? null : c.optJSONArray(i);
            return e == null ? "?" : e.optString(0);
        }
        JSONArray e = pieces(kind).optJSONArray(i);
        return e == null ? "?" : e.optString(0);
    }

    private int pieceCount(int kind) {
        if (kind == BallFx.KIND_COLOR) {
            JSONArray c = balls().optJSONArray("colors");
            return c == null ? 1 : c.length();
        }
        return Math.max(1, pieces(kind).length());
    }

    private String fxu(int kind) {
        Model m = v.app.model();
        JSONArray a = m == null ? null : m.snap.optJSONArray("fxu");
        return a == null ? null : a.optString(kind, null);
    }

    /** Whether the save may pick piece i of a kind (or this POKéMON already carries it) */
    private boolean have(int kind, int i, int carried) {
        return i == carried || BallFx.unlocked(fxu(kind), i);
    }

    private String howTo(int kind, int i) {
        if (kind == BallFx.KIND_COLOR) {
            JSONArray h = fxData().optJSONArray("colorHow");
            return h == null ? "" : h.optString(i);
        }
        JSONArray e = pieces(kind).optJSONArray(i);
        return e == null ? "" : e.optString(2);
    }

    private String info(int kind, int i) {
        JSONArray e = pieces(kind).optJSONArray(i);
        return e == null ? "" : e.optString(1);
    }

    private static int rowKind(int row) {
        return row == ROW_FORM ? BallFx.KIND_FORM : row == ROW_PART ? BallFx.KIND_PART : row == ROW_RECALL
                ? BallFx.KIND_RECALL : BallFx.KIND_COLOR;
    }

    private static boolean isPieceRow(int row) {
        return row == ROW_FORM || row == ROW_PART || row == ROW_RECALL || row == ROW_ENERGY;
    }

    private int specialOf(int item) {
        JSONArray sp = balls().optJSONArray("special");
        for (int i = 0; sp != null && i < sp.length(); i++) {
            if (sp.optInt(i) == item) {
                return i + 1;
            }
        }
        return 0;
    }

    private static final int POKE_BALL = 4;

    private boolean canColor() {
        return ball == POKE_BALL;
    }

    /** The balls to pick from: its own, then every kind in the BAG (the Hoenn ones, then the special ones) */
    private List<Integer> ballChoices() {
        List<Integer> out = new ArrayList<>();
        out.add(origBall);
        Model m = v.app.model();
        JSONArray bag = m == null ? null : m.snap.optJSONArray("bag");
        List<Integer> kinds = new ArrayList<>();
        for (int i = 1; i <= 12; i++) {
            kinds.add(i);
        }
        JSONArray sp = balls().optJSONArray("special");
        for (int i = 0; sp != null && i < sp.length(); i++) {
            kinds.add(sp.optInt(i));
        }
        JSONArray only = balls().optJSONArray("catchonly");   // (the ASTRAL BALL: only a catch puts one in it)
        for (int item : kinds) {
            boolean catchOnly = false;
            for (int i = 0; only != null && i < only.length(); i++) {
                catchOnly |= only.optInt(i) == item;
            }
            if (item != origBall && !catchOnly && bagCount(bag, item) > 0) {
                out.add(item);
            }
        }
        return out;
    }

    private static int bagCount(JSONArray bag, int item) {
        int n = 0;
        for (int p = 0; bag != null && p < bag.length(); p++) {
            JSONArray pocket = bag.optJSONArray(p);
            for (int i = 0; pocket != null && i < pocket.length(); i++) {
                JSONArray it = pocket.optJSONArray(i);
                if (it != null && it.optInt(0) == item) {
                    n += it.optInt(1);
                }
            }
        }
        return n;
    }

    private void loadFrom(JSONObject mon) {
        String key = mon.optLong("p") + "/" + mon.optLong("o") + "/" + mon.optInt("ball") + "/" + mon.optInt("bst")
                + "/" + mon.optLong("fx") + "/" + mon.optInt("fxs");
        if (key.equals(loadedFor)) {
            return;
        }
        boolean first = loadedFor == null;
        loadedFor = key;
        origBall = mon.optInt("ball", POKE_BALL);
        if (origBall <= 0) {
            origBall = POKE_BALL;
        }
        origStyle = mon.optInt("bst");
        origFx = BallFx.unpack(mon.optLong("fx"), mon.optInt("fxs"));
        if (first || !dirty()) {
            ball = origBall;
            color = BallFx.styleSpecial(origStyle) > 0 ? 0 : BallFx.styleColor(origStyle);
            deco = BallFx.styleSpecial(origStyle) > 0 ? 0 : BallFx.styleDeco(origStyle);
            fx = new BallFx(origFx);
        }
    }

    private boolean styleDirty() {
        int c = canColor() ? color : 0, d = canColor() ? deco : 0;
        int was = BallFx.styleSpecial(origStyle) > 0 ? 0 : origStyle;
        return ball != origBall || (specialOf(ball) == 0 && BallFx.makeStyle(c, d, 0) != (was & 0x0FFF | (was >> 12 == 0xF ? 0xF000 : 0)));
    }

    private boolean fxDirty() {
        return !fx.equals(origFx);
    }

    private boolean dirty() {
        return loadedFor != null && (styleDirty() || fxDirty());
    }

    /** Whatever the draft holds that the save hasn't earned (null: nothing) */
    private String lockedPiece() {
        if (canColor() && !have(BallFx.KIND_COLOR, color, BallFx.styleColor(origStyle))) {
            return "COLOR " + pieceName(BallFx.KIND_COLOR, color);
        }
        int[] kinds = {BallFx.KIND_FORM, BallFx.KIND_PART, BallFx.KIND_COLOR, BallFx.KIND_RECALL};
        String[] what = {"SEND-OUT", "PARTICLES", "ENERGY", "RETURN"};
        for (int k = 0; k < 4; k++) {
            if (!have(kinds[k], fx.get(kinds[k]), origFx.get(kinds[k]))) {
                return what[k] + " " + pieceName(kinds[k], fx.get(kinds[k]));
            }
        }
        return null;
    }

    /** The ready-made mixes, then MY PRESET 1-6 (name, fx; null fx = an empty slot) */
    private List<Object[]> presets() {
        List<Object[]> out = new ArrayList<>();
        JSONArray p = fxData().optJSONArray("presets");
        for (int i = 0; p != null && i < p.length(); i++) {
            JSONArray e = p.optJSONArray(i);
            out.add(new Object[]{e.optString(0), BallFx.unpack(e.optLong(1), e.optInt(2))});
        }
        Model m = v.app.model();
        JSONArray mine = m == null ? null : m.snap.optJSONArray("fxp");
        for (int i = 0; i < 6; i++) {
            JSONArray e = mine == null ? null : mine.optJSONArray(i);
            out.add(new Object[]{"MY PRESET " + (i + 1), e == null ? null : BallFx.unpack(e.optLong(0), e.optInt(1))});
        }
        return out;
    }

    private String presetName() {
        String mine = null;
        for (Object[] p : presets()) {
            if (p[1] != null && p[1].equals(fx)) {
                if (!((String) p[0]).startsWith("MY PRESET")) {
                    return (String) p[0];
                }
                mine = mine == null ? (String) p[0] : mine;
            }
        }
        return mine != null ? mine : "CUSTOM";
    }

    private String soundName(int i) {
        JSONArray s = fxData().optJSONArray("sounds");
        JSONArray e = s == null ? null : s.optJSONArray(i);
        return e == null ? "?" : e.optString(0);
    }

    private String decoName(int d) {
        if (d == 0) {
            return "NONE";
        }
        JSONArray a = balls().optJSONArray("decos");
        JSONArray e = a == null ? null : a.optJSONArray(d - 1);
        return e == null ? "?" : e.optString(0);
    }

    private int decoCount() {
        JSONArray a = balls().optJSONArray("decos");
        return (a == null ? 0 : a.length()) + 1;
    }

    // --------------------------------------------------------------------------------------------- the clip

    /** The piece the stage plays: the focused row's, else the effect's most telling part */
    private void chooseClip() {
        int kind, piece;
        if (focus == ROW_PART) {
            kind = FxClips.KIND_PART;
            piece = fx.part;
        } else if (focus == ROW_RECALL) {
            kind = FxClips.KIND_RECALL;
            piece = fx.recall;
        } else if (focus == ROW_FORM || fx.form != 0 || (fx.part == 0 && fx.recall == 0)) {
            kind = FxClips.KIND_FORM;
            piece = fx.form;
        } else if (fx.part != 0) {
            kind = FxClips.KIND_PART;
            piece = fx.part;
        } else {
            kind = FxClips.KIND_RECALL;
            piece = fx.recall;
        }
        if (kind != clipKind || piece != clipPiece) {
            clipKind = kind;
            clipPiece = piece;
            clipStart = System.currentTimeMillis();
        }
    }

    private int stageScale(int w, int h) {
        // (whole and half steps, so the game's pixels stay even; room left for the rows below)
        float k = Math.min((w - 24) / 112f, (h - 26 - BAR_H - 36 - ROWS * ROW_H - 2 * HEAD_H - 10) / 112f);
        k = Math.max(1f, Math.min(2f, k));
        return (int) (k * 2);   // in halves
    }

    // --------------------------------------------------------------------------------------------- drawing

    private int stageY0() {
        return 24;
    }

    private int stageBottom(int w, int h) {
        return stageY0() + 112 * stageScale(w, h) / 2 + 4;
    }

    private int infoY(int w, int h) {
        return stageBottom(w, h) + 2;
    }

    private int listY(int w, int h) {
        return infoY(w, h) + 36;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        JSONObject mon = mon();
        background(c, w, h, 0);
        String name = mon == null ? "" : art.plain(GameArt.hexText(mon.optString("n")));
        banner(c, w, "POKé BALL", name);
        if (mon == null || mon.optInt("e") == 1) {
            art.window(c, 0, 12, 30, w - 24, 30);
            art.text(c, mon == null ? "It isn't there any more." : "An EGG has no POKé BALL yet.", 18, 36);
            button(c, "BACK", w - 74, h - 28, 70, true);
            return;
        }
        loadFrom(mon);
        chooseClip();
        drawStage(c, w, h);
        drawInfo(c, w, h);
        c.save();
        c.clipRect(0, listY(w, h), w, h - BAR_H);
        if (picking >= 0) {
            drawList(c, w, h);
        } else {
            drawRows(c, w, h);
        }
        c.restore();
        drawBar(c, w, h);
    }

    private void drawStage(Canvas c, int w, int h) {
        int half = stageScale(w, h), sw = 112 * half / 2, x0 = (w - sw) / 2, y0 = stageY0();
        art.window(c, 0, x0 - 4, y0 - 2, sw + 8, sw + 4);
        Bitmap b = null;
        if (sClips.load(clipKind, clipPiece)) {
            int f = Look.animations ? (int) ((System.currentTimeMillis() - clipStart) * FxClips.FPS / 1000)
                    : sClips.frames() * 2 / 5;   // (still: the effect at its height)
            b = sClips.frame(f);
            if (Look.animations) {
                v.postInvalidateDelayed(1000 / FxClips.FPS);
            }
        }
        if (b != null) {
            src.set(0, 0, b.getWidth(), b.getHeight());
            dst.set(x0, y0, x0 + sw, y0 + sw);
            c.drawBitmap(b, src, dst, pix);
        } else {
            fill.setColor(0xFFD8F0D0);
            c.drawRect(x0, y0, x0 + sw, y0 + sw, fill);
        }
        // what's playing, in a strip along the stage's foot
        String what = (clipKind == FxClips.KIND_FORM ? "SEND-OUT: " : clipKind == FxClips.KIND_PART ? "PARTICLES: "
                : "RETURN: ") + pieceName(clipKind == FxClips.KIND_FORM ? BallFx.KIND_FORM : clipKind == FxClips.KIND_PART
                ? BallFx.KIND_PART : BallFx.KIND_RECALL, clipPiece);
        fill.setColor(0xA0182030);
        c.drawRect(x0, y0 + sw - 14, x0 + sw, y0 + sw, fill);
        art.text(c, what, x0 + 4, y0 + sw - 15, GameArt.WHITE_TEXT, 0xFF101828);
        // the ball, as it'll look, in the stage's corner (twice its size: the emblem shows)
        drawBallIcon(c, x0 + 2, y0 + 2, 2);
    }

    /** The ball's icon (24x24 times k) as the game draws it: the red top recolored, the emblem drawn in */
    private void drawBallIcon(Canvas c, int x, int y, int k) {
        int[] px = art.itemPixels(ball);
        JSONObject b = balls();
        if (canColor() && color != 0) {
            JSONArray pal = b.optJSONArray("iconPal"), idx = b.optJSONArray("iconIdx"), shade = b.optJSONArray("iconShade");
            JSONArray ramp = b.optJSONArray("colors").optJSONArray(color).optJSONArray(1);
            for (int i = 0; pal != null && idx != null && i < idx.length(); i++) {
                int from = pal.optInt(idx.optInt(i)) & 0xFFFFFF, to = ramp.optInt(shade.optInt(i)) | 0xFF000000;
                for (int p = 0; p < px.length; p++) {
                    if ((px[p] & 0xFFFFFF) == from && (px[p] >>> 24) != 0) {
                        px[p] = to;
                    }
                }
            }
        }
        if (canColor() && deco > 0) {
            JSONArray d = b.optJSONArray("decos").optJSONArray(deco - 1), xy = b.optJSONArray("decoXY");
            String art5 = d.optString(1);
            JSONArray cols = d.optJSONArray(2);
            for (int i = 0; i < art5.length() && i < 25; i++) {
                int n = art5.charAt(i) - '0';
                if (n > 0) {
                    px[(xy.optInt(1) + i / 5) * 24 + xy.optInt(0) + i % 5] = 0xFF000000 | cols.optInt(n - 1);
                }
            }
        }
        Bitmap bm = Bitmap.createBitmap(px, 24, 24, Bitmap.Config.ARGB_8888);
        src.set(0, 0, 24, 24);
        dst.set(x, y, x + 24 * k, y + 24 * k);
        c.drawBitmap(bm, src, dst, pix);
    }

    private void drawInfo(Canvas c, int w, int h) {
        int y = infoY(w, h);
        art.window(c, 0, 12, y + 4, w - 24, 28);
        String text;
        int kind = rowKind(focus);
        if (isPieceRow(focus)) {
            int i = focus == ROW_ENERGY ? fx.color : fx.get(kind);
            int carried = focus == ROW_ENERGY ? origFx.color : origFx.get(kind);
            if (!have(kind, i, carried)) {
                text = "Not earned yet. " + howTo(kind, i) + ".";
            } else if (focus == ROW_ENERGY) {
                text = "The color of its energy: beams, glows, motes, pixels and ink.";
            } else {
                text = info(kind, i);
            }
        } else if (focus == ROW_COLOR) {
            text = !canColor() ? "Only a POKé BALL takes a color." : !have(BallFx.KIND_COLOR, color, BallFx.styleColor(origStyle))
                    ? "Not earned yet. " + howTo(BallFx.KIND_COLOR, color) + "." : "The color of the ball's top.";
        } else if (focus == ROW_DECO) {
            text = !canColor() ? "Only a POKé BALL takes a decoration." : "An emblem on the ball's top.";
        } else if (focus == ROW_BALL) {
            text = ball == origBall ? "The ball it lives in. Another kind from your BAG costs one."
                    : "Moving it into this ball uses one from your BAG.";
        } else if (focus == ROW_SOUND) {
            JSONArray s = fxData().optJSONArray("sounds");
            JSONArray e = s == null ? null : s.optJSONArray(fx.sound);
            text = e == null ? "" : e.optString(1);
        } else {
            text = "A ready-made mix, or one of your own. Change any part to make it yours.";
        }
        List<String> lines = wrap(text, w - 40);
        for (int i = 0; i < Math.min(2, lines.size()); i++) {
            art.text(c, lines.get(i), 18, y + 3 + i * 13, i == 0 && text.startsWith("Not earned") ? GameArt.RED_TEXT
                    : GameArt.TEXT, GameArt.SHADOW);
        }
    }

    private String value(int row) {
        switch (row) {
            case ROW_BALL: return art.plain(art.name("items", ball));
            case ROW_COLOR: return canColor() ? pieceName(BallFx.KIND_COLOR, color) : "-";
            case ROW_DECO: return canColor() ? decoName(deco) : "-";
            case ROW_PRESET: return presetName();
            case ROW_FORM: return pieceName(BallFx.KIND_FORM, fx.form);
            case ROW_PART: return pieceName(BallFx.KIND_PART, fx.part);
            case ROW_ENERGY: return pieceName(BallFx.KIND_COLOR, fx.color);
            case ROW_RECALL: return pieceName(BallFx.KIND_RECALL, fx.recall);
            default: return soundName(fx.sound);
        }
    }

    private boolean rowLocked(int row) {
        switch (row) {
            case ROW_COLOR: return canColor() && !have(BallFx.KIND_COLOR, color, BallFx.styleColor(origStyle));
            case ROW_FORM: return !have(BallFx.KIND_FORM, fx.form, origFx.form);
            case ROW_PART: return !have(BallFx.KIND_PART, fx.part, origFx.part);
            case ROW_ENERGY: return !have(BallFx.KIND_COLOR, fx.color, origFx.color);
            case ROW_RECALL: return !have(BallFx.KIND_RECALL, fx.recall, origFx.recall);
            default: return false;
        }
    }

    private boolean rowUsable(int row) {
        return (row != ROW_COLOR && row != ROW_DECO) || canColor();
    }

    private int rowY(int row, int w, int h) {
        int y = listY(w, h) - scroll;
        if (row >= ROW_PRESET) {
            y += HEAD_H;
        }
        return y + HEAD_H + row * ROW_H;
    }

    private void heading(Canvas c, int w, int y, String text) {
        sectionBar(c, 8, w - 8, y + 1, HEAD_H - 2, text, null);
    }

    private void drawRows(Canvas c, int w, int h) {
        for (int r = 0; r < ROWS; r++) {
            int y = rowY(r, w, h);
            boolean on = r == focus, usable = rowUsable(r);
            art.window(c, 0, 12, y + 3, w - 24, ROW_H - 6);
            if (on) {
                fill.setColor(0x30F85838);
                c.drawRect(12, y + 3, w - 12, y + ROW_H - 3, fill);
            }
            art.text(c, LABELS[r], 16, y + 1, usable ? GameArt.BLUE_TEXT : GameArt.GRAY_TEXT, GameArt.SHADOW);
            // < value >
            int vx = 92, vw = w - 12 - vx - 4;
            String val = value(r);
            int tw = art.width(val);
            int swatch = 0;
            if ((r == ROW_COLOR && canColor() && color != 0) || r == ROW_ENERGY) {
                swatch = 12;
            }
            int cx = vx + (vw - tw - swatch) / 2;
            if (swatch > 0) {
                drawSwatch(c, cx, y + 6, r == ROW_ENERGY ? fx.color : color);
                cx += swatch;
            }
            art.text(c, val, cx, y + 1, !usable ? GameArt.GRAY_TEXT : rowLocked(r) ? GameArt.RED_TEXT : GameArt.TEXT,
                    GameArt.SHADOW);
            if (usable && r != ROW_PRESET || r == ROW_PRESET) {
                art.text(c, "<", vx, y + 1, GameArt.GRAY_TEXT, GameArt.SHADOW);
                art.text(c, ">", w - 16 - art.width(">") - 4, y + 1, GameArt.GRAY_TEXT, GameArt.SHADOW);
            }
        }
        // (the headings over the rows' window frames)
        heading(c, w, rowY(ROW_BALL, w, h) - HEAD_H, "THE BALL");
        heading(c, w, rowY(ROW_PRESET, w, h) - HEAD_H, "SEND-OUT & RETURN");
    }

    private void drawSwatch(Canvas c, int x, int y, int colorIndex) {
        JSONArray cols = balls().optJSONArray("colors");
        JSONArray ramp = cols == null ? null : cols.optJSONArray(colorIndex).optJSONArray(1);
        int mid = ramp == null || ramp.optInt(2) == 0 ? 0xFFE03030 : 0xFF000000 | ramp.optInt(2);
        fill.setColor(0xFF202020);
        c.drawRect(x, y, x + 9, y + 9, fill);
        fill.setColor(mid);
        c.drawRect(x + 1, y + 1, x + 8, y + 8, fill);
    }

    // the open list: every choice of the row, the chosen one marked, earned or not
    private int listCount() {
        switch (picking) {
            case ROW_BALL: return ballChoices().size();
            case ROW_COLOR: case ROW_ENERGY: return pieceCount(BallFx.KIND_COLOR);
            case ROW_DECO: return decoCount();
            case ROW_PRESET: return presets().size();
            case ROW_SOUND: return fxData().optJSONArray("sounds") == null ? 1 : fxData().optJSONArray("sounds").length();
            default: return pieceCount(rowKind(picking));
        }
    }

    private String listName(int i) {
        switch (picking) {
            case ROW_BALL: return art.plain(art.name("items", ballChoices().get(i)));
            case ROW_COLOR: case ROW_ENERGY: return pieceName(BallFx.KIND_COLOR, i);
            case ROW_DECO: return decoName(i);
            case ROW_PRESET: {
                Object[] p = presets().get(i);
                return (String) p[0] + (p[1] == null ? " (EMPTY)" : "");
            }
            case ROW_SOUND: return soundName(i);
            default: return pieceName(rowKind(picking), i);
        }
    }

    private int listCurrent() {
        switch (picking) {
            case ROW_BALL: return Math.max(0, ballChoices().indexOf(ball));
            case ROW_COLOR: return color;
            case ROW_ENERGY: return fx.color;
            case ROW_DECO: return deco;
            case ROW_SOUND: return fx.sound;
            case ROW_PRESET: {
                List<Object[]> ps = presets();
                for (int i = 0; i < ps.size(); i++) {
                    if (ps.get(i)[1] != null && ps.get(i)[1].equals(fx)) {
                        return i;
                    }
                }
                return -1;
            }
            default: return fx.get(rowKind(picking));
        }
    }

    private boolean listLocked(int i) {
        switch (picking) {
            case ROW_COLOR: return !have(BallFx.KIND_COLOR, i, BallFx.styleColor(origStyle));
            case ROW_ENERGY: return !have(BallFx.KIND_COLOR, i, origFx.color);
            case ROW_FORM: case ROW_PART: case ROW_RECALL:
                return !have(rowKind(picking), i, origFx.get(rowKind(picking)));
            case ROW_PRESET: {
                BallFx p = (BallFx) presets().get(i)[1];
                return p != null && !(have(BallFx.KIND_FORM, p.form, origFx.form) && have(BallFx.KIND_PART, p.part, origFx.part)
                        && have(BallFx.KIND_COLOR, p.color, origFx.color) && have(BallFx.KIND_RECALL, p.recall, origFx.recall));
            }
            default: return false;
        }
    }

    private static final int LIST_ROW = 18;

    private void drawList(Canvas c, int w, int h) {
        int y0 = listY(w, h);
        heading(c, w, y0, LABELS[picking] + ": TAP ONE");
        int n = listCount(), cur = listCurrent(), cols = w >= 300 ? 2 : 1, cw = (w - 24) / cols;
        for (int i = 0; i < n; i++) {
            int x = 12 + (i % cols) * cw, y = y0 + HEAD_H + 2 + (i / cols) * LIST_ROW - listScroll;
            if (y < y0 + HEAD_H - LIST_ROW || y > h) {
                continue;
            }
            boolean on = i == cur, locked = listLocked(i);
            fill.setColor(on ? 0xFFF8F8F8 : 0xD8F8F8F8);
            c.drawRect(x + 1, y + 1, x + cw - 1, y + LIST_ROW - 1, fill);
            if (on) {
                fill.setColor(0xFFF85838);
                c.drawRect(x + 1, y + 1, x + 4, y + LIST_ROW - 1, fill);
            }
            int tx = x + 8;
            if (picking == ROW_COLOR || picking == ROW_ENERGY) {
                drawSwatch(c, tx, y + 4, i);
                tx += 13;
            }
            art.text(c, listName(i), tx, y, locked ? GameArt.GRAY_TEXT : GameArt.TEXT, GameArt.SHADOW);
            if (locked) {
                chip(c, "LOCKED", x + cw - 48, y + 4, 0xFFB04040);
            }
        }
    }

    private int listMax(int w, int h) {
        int cols = w >= 300 ? 2 : 1;
        int rows = (listCount() + cols - 1) / cols;
        return Math.max(0, HEAD_H + 4 + rows * LIST_ROW - (h - BAR_H - listY(w, h)));
    }

    private void drawBar(Canvas c, int w, int h) {
        int y = h - BAR_H, bw = (w - 16) / 4;
        fill.setColor(0xC0182030);
        c.drawRect(0, y, w, h, fill);
        if (picking >= 0) {
            button(c, "DONE", 8 + 3 * bw, y + 1, bw, true);
            return;
        }
        boolean edit = canEdit();
        button(c, dirty() ? "SAVE" : "SAVED", 8, y + 1, bw, edit && dirty() && lockedPiece() == null);
        button(c, "PRESETS", 8 + bw, y + 1, bw, edit);
        button(c, "PARTY", 8 + 2 * bw, y + 1, bw, edit);
        button(c, dirty() ? "UNDO" : "BACK", 8 + 3 * bw, y + 1, bw, true);
    }

    private boolean canEdit() {
        return v.app.client != null && !v.app.locked();
    }

    // --------------------------------------------------------------------------------------------- input

    @Override
    public int scrollMax() {
        int w = v.width(), h = v.contentHeight();
        return picking >= 0 ? 0 : Math.max(0, rowY(ROWS, w, h) + scroll - (h - BAR_H) + 4);
    }

    @Override
    public void scrollBy(int dy) {
        if (picking >= 0) {
            listScroll = Math.max(0, Math.min(listMax(v.width(), v.contentHeight()), listScroll + dy));
        } else {
            super.scrollBy(dy);
        }
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width(), h = v.contentHeight();
        JSONObject mon = mon();
        if (mon == null || mon.optInt("e") == 1) {
            if (y >= h - 28) {
                back();
            }
            return;
        }
        if (y >= h - BAR_H) {
            tapBar(x, w);
            return;
        }
        if (y < listY(w, h)) {
            if (y < stageBottom(w, h)) {
                clipStart = System.currentTimeMillis();   // (the stage: play it again from the start)
                v.sound.play("se_select");
            }
            return;
        }
        if (picking >= 0) {
            int cols = w >= 300 ? 2 : 1, cw = (w - 24) / cols;
            int row = (y - listY(w, h) - HEAD_H - 2 + listScroll) / LIST_ROW, col = Math.max(0, Math.min(cols - 1, (x - 12) / cw));
            int i = row * cols + col;
            if (row >= 0 && i < listCount()) {
                choose(picking, i);
            }
            return;
        }
        for (int r = 0; r < ROWS; r++) {
            int ry = rowY(r, w, h);
            if (y >= ry && y < ry + ROW_H) {
                if (!rowUsable(r)) {
                    focus = r;
                    v.sound.play("se_failure");
                    return;
                }
                focus = r;
                if (x < 100) {
                    step(r, -1);
                } else if (x >= w - 40) {
                    step(r, 1);
                } else {
                    picking = r;
                    listScroll = Math.max(0, (listCurrent() / (w >= 300 ? 2 : 1)) * LIST_ROW - 3 * LIST_ROW);
                    v.sound.play("se_win_open");
                }
                return;
            }
        }
    }

    private void step(int row, int d) {
        int n = row == ROW_BALL ? ballChoices().size() : row == ROW_DECO ? decoCount() : row == ROW_PRESET ? presets().size()
                : row == ROW_SOUND ? (fxData().optJSONArray("sounds") == null ? 1 : fxData().optJSONArray("sounds").length())
                : pieceCount(rowKind(row));
        int saved = picking;
        picking = row;
        int cur = listCurrent();
        picking = saved;
        int next = ((cur < 0 ? 0 : cur) + d + n) % n;
        if (row == ROW_PRESET) {
            // (presets: skip empty MY PRESET slots)
            List<Object[]> ps = presets();
            for (int k = 0; k < n && ps.get(next)[1] == null; k++) {
                next = (next + d + n) % n;
            }
        }
        choose(row, next);
    }

    private void choose(int row, int i) {
        switch (row) {
            case ROW_BALL: ball = ballChoices().get(i); break;
            case ROW_COLOR: color = i; break;
            case ROW_DECO: deco = i; break;
            case ROW_ENERGY: fx.color = i; break;
            case ROW_SOUND: fx.sound = i; break;
            case ROW_PRESET: {
                BallFx p = (BallFx) presets().get(i)[1];
                if (p == null) {
                    v.sound.play("se_failure");
                    return;
                }
                fx = new BallFx(p);
                break;
            }
            default: fx.set(rowKind(row), i); break;
        }
        v.sound.play("se_select");
        v.buzz(false);
    }

    private void tapBar(int x, int w) {
        int k = Math.min(3, Math.max(0, (x - 8) / ((w - 16) / 4)));
        if (picking >= 0) {
            if (k == 3) {
                picking = -1;
                v.sound.play("se_select");
            }
            return;
        }
        if (k == 3) {
            if (dirty()) {
                loadedFor = null;   // (UNDO: back to how it is in the game)
                JSONObject mon = mon();
                if (mon != null) {
                    loadFrom(mon);
                    ball = origBall;
                    color = BallFx.styleSpecial(origStyle) > 0 ? 0 : BallFx.styleColor(origStyle);
                    deco = BallFx.styleSpecial(origStyle) > 0 ? 0 : BallFx.styleDeco(origStyle);
                    fx = new BallFx(origFx);
                }
                v.sound.play("se_select");
            } else {
                back();
            }
            return;
        }
        if (!canEdit()) {
            v.message(v.app.client == null ? "Link with the game to change its POKé BALL." : Companion.LOCKED);
            return;
        }
        JSONObject mon = mon();
        if (mon == null) {
            return;
        }
        if (k == 0) {
            save(mon);
        } else if (k == 1) {
            presetsMenu(mon);
        } else {
            String locked = lockedPiece();
            if (locked != null) {
                v.message("Your game hasn't earned " + locked + " yet.");
                return;
            }
            Menu m = Menu.message(v, "Give this send-out and return to every POKéMON in your party?").setRows();
            m.add("YES", () -> {
                v.app.client.order(Model.ballFx(null, null, fx, -1));
                v.buzz(false);
            });
            m.add("NO", null);
            v.showMenu(m);
        }
    }

    private void save(JSONObject mon) {
        if (!dirty()) {
            return;
        }
        String locked = lockedPiece();
        if (locked != null) {
            v.message("Your game hasn't earned " + locked + " yet. " + "Pick another to SAVE.");
            return;
        }
        Runnable send = () -> {
            if (styleDirty()) {
                int special = specialOf(ball);
                v.app.client.order(Model.ballStyle(place, mon, ball, special > 0 || !canColor() ? 0 : color,
                        special > 0 || !canColor() ? 0 : deco, special));
            }
            if (fxDirty()) {
                v.app.client.order(Model.ballFx(place, mon, fx, -1));
            }
            v.buzz(true);
            v.sound.play("se_success");
        };
        if (ball != origBall) {
            Menu m = Menu.message(v, "Use one " + art.plain(art.name("items", ball)) + " from your BAG and move it in?")
                    .setRows();
            m.add("YES", send::run);
            m.add("NO", null);
            v.showMenu(m);
        } else {
            send.run();
        }
    }

    private void presetsMenu(JSONObject mon) {
        Menu m = Menu.make(v, "MY PRESETS");
        List<Object[]> ps = presets();
        int first = ps.size() - 6;
        for (int i = 0; i < 6; i++) {
            final int slot = i;
            BallFx p = (BallFx) ps.get(first + i)[1];
            if (p != null) {
                m.add("USE MY PRESET " + (i + 1), () -> {
                    fx = new BallFx(p);
                    focus = ROW_PRESET;
                });
            }
        }
        for (int i = 0; i < 6; i++) {
            final int slot = i;
            BallFx p = (BallFx) ps.get(first + i)[1];
            m.add("SAVE AS MY PRESET " + (i + 1) + (p == null ? " (EMPTY)" : ""), () -> {
                String locked = lockedPiece();
                if (locked != null) {
                    v.message("Your game hasn't earned " + locked + " yet.");
                    return;
                }
                v.app.client.order(Model.ballFx(null, null, fx, slot));
                v.buzz(false);
            });
        }
        m.add("CANCEL", null);
        v.showMenu(m);
    }

    @Override
    public void back() {
        if (picking >= 0) {
            picking = -1;
            return;
        }
        v.showOver(parent);
    }

    // (the screens test)
    void openList(int row) {
        picking = row;
        listScroll = 0;
        if (row >= 0) {
            focus = row;
        }
    }

    void pickForTest(int row, int i) {
        focus = row;
        choose(row, i);
    }
}
