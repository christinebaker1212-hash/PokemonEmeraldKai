package com.moemon.companion;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.util.LruCache;

import java.io.InputStream;

/**
 * A card game's printed pictures as the game packs them ("TCGP" / "YGOP": a count, then per card its picture's offset
 * and size and where its illustration sits, then the card back; the app's copies hold WebP pictures and are kept
 * uncompressed in the APK, so one picture is read at a time), decoded at about the size they're shown and kept a while.
 * The POKéMON cards (TcgArt) and the YU-GI-OH! cards (YgoArt).
 */
public final class CardPack {
    public final float W, H;   // (a card's proportions)
    private final AssetManager assets;
    private final String path, magic;
    private int[] table;   // per card (0 the back): offset, size, art x, y, w, h
    private int count;
    private final Paint smooth = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.ANTI_ALIAS_FLAG);
    private final Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Rect src = new Rect();
    private final RectF dst = new RectF();
    private final LruCache<Integer, Bitmap> cache;

    public CardPack(AssetManager assets, String path, String magic, float w, float h, int cacheBytes) {
        this.assets = assets;
        this.path = path;
        this.magic = magic;
        this.W = w;
        this.H = h;
        this.cache = new LruCache<Integer, Bitmap>(cacheBytes) {
            @Override
            protected int sizeOf(Integer key, Bitmap b) {
                return b.getByteCount();
            }
        };
    }

    public int count() {
        table();
        return count;
    }

    // -------------------------------------------------------------------------------------------- the pack

    private synchronized boolean table() {
        if (table != null) {
            return count > 0;
        }
        table = new int[0];
        try (InputStream in = assets.open(path)) {
            byte[] head = new byte[8];
            if (in.read(head) != 8 || head[0] != magic.charAt(0) || head[1] != magic.charAt(1) || head[2] != magic.charAt(2) || head[3] != magic.charAt(3)) {
                return false;
            }
            int n = u32(head, 4);
            byte[] t = new byte[16 * (n + 1)];
            int got = 0;
            while (got < t.length) {
                int r = in.read(t, got, t.length - got);
                if (r <= 0) {
                    return false;
                }
                got += r;
            }
            table = new int[6 * (n + 1)];
            for (int i = 0; i <= n; i++) {
                table[i * 6] = u32(t, i * 16);
                table[i * 6 + 1] = u32(t, i * 16 + 4);
                for (int k = 0; k < 4; k++) {
                    table[i * 6 + 2 + k] = (t[i * 16 + 8 + k * 2] & 255) | (t[i * 16 + 9 + k * 2] & 255) << 8;
                }
            }
            count = n;
        } catch (Exception e) {
            count = 0;
        }
        return count > 0;
    }

    private static int u32(byte[] b, int at) {
        return (b[at] & 255) | (b[at + 1] & 255) << 8 | (b[at + 2] & 255) << 16 | (b[at + 3] & 255) << 24;
    }

    // entry i: card id - 1, the back last
    private int entry(int id) {
        return id <= 0 ? count : id - 1;
    }

    /** The card's picture decoded at 1/sample of its size (1, 2, 4 or 8), null when the pack hasn't got it */
    public Bitmap picture(int id, int sample) {
        if (!table() || (id > count)) {
            return null;
        }
        int key = (id <= 0 ? 0 : id) * 16 + sample;
        Bitmap b = cache.get(key);
        if (b != null) {
            return b;
        }
        int e = entry(id), off = table[e * 6], size = table[e * 6 + 1];
        if (size <= 0) {
            return null;
        }
        try (InputStream in = assets.open(path)) {
            long skip = off;
            while (skip > 0) {
                long k = in.skip(skip);
                if (k <= 0) {
                    return null;
                }
                skip -= k;
            }
            byte[] data = new byte[size];
            int got = 0;
            while (got < size) {
                int r = in.read(data, got, size - got);
                if (r <= 0) {
                    return null;
                }
                got += r;
            }
            BitmapFactory.Options o = new BitmapFactory.Options();
            o.inSampleSize = sample;
            b = BitmapFactory.decodeByteArray(data, 0, size, o);
        } catch (Exception ex) {
            return null;
        }
        if (b != null) {
            cache.put(key, b);
        }
        return b;
    }

    /** Where the card's illustration sits in its picture (at full size): x, y, w, h */
    public int[] artRect(int id) {
        if (!table() || id <= 0 || id > count) {
            return new int[]{0, 0, (int) W, (int) H};
        }
        int e = entry(id);
        int w = table[e * 6 + 4], h = table[e * 6 + 5];
        return w == 0 || h == 0 ? new int[]{0, 0, (int) W, (int) H}
                : new int[]{table[e * 6 + 2], table[e * 6 + 3], w, h};
    }

    private int sampleFor(float screenW) {
        int s = 1;
        while (s < 8 && W / (s * 2) >= screenW * 0.85f) {
            s *= 2;
        }
        return s;
    }

    /** The card (0 its back) at x, y in w x h of the page's pixels, `scale` screen pixels to one of those */
    public void draw(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        Bitmap b = picture(id, sampleFor(w * scale));
        dst.set(x, y, x + w, y + h);
        if (b == null) {
            fill.setColor(id == 0 ? 0xFF2050A0 : 0xFFE8E0C8);
            c.drawRoundRect(dst, w * 0.05f, w * 0.05f, fill);
            return;
        }
        smooth.setAlpha(alpha);
        src.set(0, 0, b.getWidth(), b.getHeight());
        c.drawBitmap(b, src, dst, smooth);
        smooth.setAlpha(255);
    }

    /** The card turned to face the other side of the table (the opponent's cards, as the game's tables show them) */
    public void drawTurned(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        c.save();
        c.rotate(180, x + w / 2, y + h / 2);
        draw(c, id, x, y, w, h, scale, alpha);
        c.restore();
    }

    /** Only the card's illustration, filling x, y, w, h (cut to keep its shape) */
    public void drawArt(Canvas c, int id, float x, float y, float w, float h, float scale, int alpha) {
        int[] r = artRect(id);
        float k = Math.max(w / r[2], h / r[3]);
        Bitmap b = picture(id, sampleFor(W * k * scale));
        if (b == null) {
            draw(c, id, x, y, w, h, scale, alpha);
            return;
        }
        float f = b.getWidth() / W, cw = w / k, ch = h / k;
        float cx = r[0] + (r[2] - cw) / 2, cy = r[1] + (r[3] - ch) / 2;
        src.set(Math.round(cx * f), Math.round(cy * f), Math.round((cx + cw) * f), Math.round((cy + ch) * f));
        dst.set(x, y, x + w, y + h);
        smooth.setAlpha(alpha);
        c.drawBitmap(b, src, dst, smooth);
        smooth.setAlpha(255);
    }

}
