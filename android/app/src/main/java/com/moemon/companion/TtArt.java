package com.moemon.companion;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * TRIPLE TRIAD's cards as the game draws them (src/tt/tt_gfx.c Tt_DrawCard): the plate in the seat's color (blue the
 * player, red the other side, grey neither) behind Final Fantasy XIV's picture, the type at the top right, the stars
 * at the top left, and the four numbers in their diamond near the bottom (green raised, red lowered). Composed at the
 * pictures' own 104x128 from the game's art pack (assets game/tt_cards.bin, "TTCA") and kept a while.
 */
public final class TtArt {
    public static final int W = 104, H = 128;
    public static final int SEAT_PLAYER = 0, SEAT_OTHER = 1, SEAT_NONE = 2;
    private static final int EX_BACK = 0, EX_PLATE = 2, EX_STAR = 4, EX_TYPES = 5, EX_DIGITS = 9;
    private static final int[][] COLORS = {{0x3a78e8, 0x8cc0ff, 0x183c88}, {0xe0484c, 0xff9ca0, 0x801820},
            {0x808890, 0xc8ccd4, 0x404448}};

    private final AssetManager assets;
    private byte[] pack;
    private int top;
    private final Bitmap[] plates = new Bitmap[3];
    private Bitmap back, star, digits;
    private final Bitmap[] types = new Bitmap[4];
    private final Paint smooth = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.ANTI_ALIAS_FLAG);
    private final Paint tint = new Paint(Paint.FILTER_BITMAP_FLAG);
    private final Rect src = new Rect();
    private final RectF dst = new RectF();
    private final Map<String, Bitmap> cards = new LinkedHashMap<String, Bitmap>(64, 0.75f, true) {
        @Override
        protected boolean removeEldestEntry(Map.Entry<String, Bitmap> e) {
            return size() > 72;
        }
    };
    private JSONObject[] catalog;
    public JSONObject words;

    public TtArt(AssetManager assets) {
        this.assets = assets;
    }

    // ------------------------------------------------------------------------------------------------- data

    /** The card's catalog entry (Tt.catalog: n, s[4], st, ty, o, r, t), null for none */
    public synchronized JSONObject card(int id) {
        if (catalog == null) {
            try {
                JSONArray a = new JSONArray(Tt.catalog());
                int max = 0;
                for (int i = 0; i < a.length(); i++) {
                    max = Math.max(max, a.getJSONObject(i).getInt("id"));
                }
                catalog = new JSONObject[max + 1];
                for (int i = 0; i < a.length(); i++) {
                    catalog[a.getJSONObject(i).getInt("id")] = a.getJSONObject(i);
                }
                words = new JSONObject(Tt.words());
            } catch (Exception e) {
                catalog = new JSONObject[1];
            }
        }
        return id > 0 && id < catalog.length ? catalog[id] : null;
    }

    public int maxId() {
        card(0);
        return catalog.length - 1;
    }

    public String ruleName(int rule) {
        card(0);
        JSONArray r = words == null ? null : words.optJSONArray("rules");
        return r == null ? "" : r.optJSONArray(rule).optString(0);
    }

    public String ruleHelp(int rule) {
        card(0);
        JSONArray r = words == null ? null : words.optJSONArray("rules");
        return r == null ? "" : r.optJSONArray(rule).optString(1);
    }

    /** The rules' names, joined ("OPEN, SAME, PLUS"); "NO RULES" for none */
    public String rulesText(int rules) {
        StringBuilder s = new StringBuilder();
        for (int r = 0; r < 15; r++) {
            if ((rules >> r & 1) != 0) {
                s.append(s.length() == 0 ? "" : ", ").append(ruleName(r));
            }
        }
        return s.length() == 0 ? "NO RULES" : s.toString();
    }

    // -------------------------------------------------------------------------------------------- the pack

    private synchronized boolean load() {
        if (pack != null) {
            return pack.length > 12;
        }
        try (InputStream in = assets.open("game/tt_cards.bin")) {
            ByteArrayOutputStream o = new ByteArrayOutputStream(11 << 20);
            byte[] b = new byte[1 << 16];
            int n;
            while ((n = in.read(b)) > 0) {
                o.write(b, 0, n);
            }
            pack = o.toByteArray();
        } catch (Exception e) {
            pack = new byte[0];
            return false;
        }
        top = rd(4);
        return pack.length > 12;
    }

    private int rd(int at) {
        return (pack[at] & 255) | (pack[at + 1] & 255) << 8 | (pack[at + 2] & 255) << 16 | (pack[at + 3] & 255) << 24;
    }

    private Bitmap blob(int at) {
        int off = rd(at), size = rd(at + 4);
        if (size <= 0 || off + size > pack.length) {
            return null;
        }
        BitmapFactory.Options o = new BitmapFactory.Options();
        o.inScaled = false;
        o.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeByteArray(pack, off, size, o);
    }

    private Bitmap picture(int id) {
        return load() && id >= 1 && id <= top ? blob(12 + (id - 1) * 24) : null;
    }

    private Bitmap extra(int k) {
        return load() ? blob(12 + top * 24 + k * 8) : null;
    }

    private Bitmap plate(int seat) {
        seat = Math.max(0, Math.min(2, seat));
        if (plates[seat] == null) {
            Bitmap g = extra(EX_PLATE);
            if (g == null) {
                return null;
            }
            // the grey plate takes the seat's color, keeping its light and shade (tt_gfx.c Decode)
            int color = mix(COLORS[seat][0], 0xffffff, 0.08f);
            int[] px = new int[g.getWidth() * g.getHeight()];
            g.getPixels(px, 0, g.getWidth(), 0, 0, g.getWidth(), g.getHeight());
            for (int i = 0; i < px.length; i++) {
                int p = px[i];
                float l = (((p >> 16) & 255) * 0.3f + ((p >> 8) & 255) * 0.59f + (p & 255) * 0.11f) / 80f;
                int r = Math.min(255, (int) (((color >> 16) & 255) * l)), gg = Math.min(255, (int) (((color >> 8) & 255) * l)),
                        b = Math.min(255, (int) ((color & 255) * l));
                px[i] = (p & 0xFF000000) | r << 16 | gg << 8 | b;
            }
            plates[seat] = Bitmap.createBitmap(px, g.getWidth(), g.getHeight(), Bitmap.Config.ARGB_8888);
        }
        return plates[seat];
    }

    private static int mix(int a, int b, float t) {
        int out = 0;
        for (int s = 0; s <= 16; s += 8) {
            int x = (a >> s) & 255, y = (b >> s) & 255;
            out |= ((int) (x + (y - x) * t)) << s;
        }
        return out;
    }

    // -------------------------------------------------------------------------------------------- a card

    /** Card ID's face for SEAT (mods: + / - on each side, null none), 104x128; the back for id 0 */
    public synchronized Bitmap face(int id, int seat, int[] mods) {
        String key = id + ":" + seat + (mods == null ? "" : ":" + mods[0] + "," + mods[1] + "," + mods[2] + "," + mods[3]);
        Bitmap b = cards.get(key);
        if (b != null) {
            return b;
        }
        if (id == 0) {
            if (back == null) {
                back = extra(EX_BACK);
            }
            return back;
        }
        JSONObject c = card(id);
        b = Bitmap.createBitmap(W, H, Bitmap.Config.ARGB_8888);
        Canvas k = new Canvas(b);
        Bitmap p = plate(seat), pic = picture(id);
        if (p != null) {
            k.drawBitmap(p, null, new Rect(0, 0, W, H), smooth);
        }
        if (pic != null) {
            k.drawBitmap(pic, null, new Rect(0, 0, W, H), smooth);
        }
        if (c != null) {
            int type = c.optInt("ty");
            if (type > 0 && type <= 4) {
                if (types[type - 1] == null) {
                    types[type - 1] = extra(EX_TYPES + type - 1);
                }
                if (types[type - 1] != null) {
                    float s = W * 0.28f;
                    k.drawBitmap(types[type - 1], null, new RectF(W - W * 0.30f - W * 0.02f, H * 0.03f, W - W * 0.02f - W * 0.02f,
                            H * 0.03f + s), smooth);
                }
            }
            if (star == null) {
                star = extra(EX_STAR);
            }
            int stars = c.optInt("st"), ss = (int) (W * 0.2f);
            for (int i = 0; star != null && i < stars; i++) {
                k.drawBitmap(star, null, new RectF(W * 0.04f + i * (ss - 2), H * 0.03f, W * 0.04f + i * (ss - 2) + ss,
                        H * 0.03f + ss), smooth);
            }
            if (digits == null) {
                digits = extra(EX_DIGITS);
            }
            JSONArray sides = c.optJSONArray("s");
            int dw = 10, dh = 14;
            float cx = W / 2f, nby = H - 5;
            for (int side = 0; digits != null && sides != null && side < 4; side++) {
                int m = mods == null ? 0 : mods[side];
                int v = Math.max(1, Math.min(10, sides.optInt(side) + m));
                float x, y;
                switch (side) {
                    case 0: x = cx - dw / 2f; y = nby - dh * 2 - 2; break;
                    case 2: x = cx - dw / 2f; y = nby - dh; break;
                    case 3: x = cx - dw / 2f - dw - 1; y = nby - dh * 1.5f - 1; break;
                    default: x = cx + dw / 2f + 1; y = nby - dh * 1.5f - 1; break;
                }
                src.set((v == 10 ? 10 : v) * dw, 0, (v == 10 ? 10 : v) * dw + dw, dh);
                tint.setColorFilter(m == 0 ? null : new android.graphics.PorterDuffColorFilter(m > 0 ? 0xFF90FF70 : 0xFFFF8070,
                        android.graphics.PorterDuff.Mode.MULTIPLY));
                k.drawBitmap(digits, src, new RectF(Math.round(x), Math.round(y), Math.round(x) + dw, Math.round(y) + dh), tint);
            }
        }
        cards.put(key, b);
        return b;
    }

    /**
     * Draws card ID at x, y (w x h in the page's pixels; flip: the width's scale, -1..1, for a card turning over: the back
     * shows while it's negative unless showBack is false). alpha 0-255.
     */
    public void draw(Canvas c, int id, int seat, int[] mods, float x, float y, float w, float h, float flip, int alpha) {
        Bitmap b = face(flip < 0 ? 0 : id, seat, mods);
        if (b == null) {
            return;
        }
        float dw = w * Math.abs(flip);
        dst.set(x + (w - dw) / 2, y, x + (w + dw) / 2, y + h);
        smooth.setAlpha(alpha);
        c.drawBitmap(b, null, dst, smooth);
        smooth.setAlpha(255);
    }

    public int color(int seat, int shade) {
        return 0xFF000000 | COLORS[Math.max(0, Math.min(2, seat))][shade];
    }
}
