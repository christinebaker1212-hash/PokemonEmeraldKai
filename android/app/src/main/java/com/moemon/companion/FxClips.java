package com.moemon.companion;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.util.zip.Inflater;

/**
 * The ball effects as the game's own editor plays them (assets game/fxclips.bin, recorded at build time by
 * src/mp/mp_fxclip.c and packed by android/build_assets.py): a clip per SEND-OUT, PARTICLES and RETURN piece, the ball
 * dropping in, the effect, a pause, the return. One clip is unpacked at a time; frames come 30 a second.
 */
public final class FxClips {
    public static final int KIND_FORM = 0, KIND_PART = 1, KIND_RECALL = 2;
    public static final int FPS = 30;

    private byte[] file;
    private int count, w, h;
    private int curKind = -1, curPiece = -1, curFrames;
    private int[][] frames;
    private Bitmap bitmap;
    private int shownFrame = -1;

    public FxClips(AssetManager assets) {
        try (InputStream in = assets.open("game/fxclips.bin")) {
            ByteArrayOutputStream o = new ByteArrayOutputStream();
            byte[] buf = new byte[65536];
            int n;
            while ((n = in.read(buf)) > 0) {
                o.write(buf, 0, n);
            }
            file = o.toByteArray();
            if (file.length < 10 || file[0] != 'M' || file[1] != 'F' || file[2] != 'X' || file[3] != 'A') {
                file = null;
                return;
            }
            count = u16(4);
            w = u16(6);
            h = u16(8);
        } catch (Exception e) {
            file = null;
        }
    }

    public boolean ok() {
        return file != null;
    }

    public int width() {
        return w;
    }

    public int height() {
        return h;
    }

    private int u16(int at) {
        return (file[at] & 255) | (file[at + 1] & 255) << 8;
    }

    private int u32(int at) {
        return u16(at) | u16(at + 2) << 16;
    }

    /** Unpacks a clip (the one shown stays until another is asked for); FALSE if there's none */
    public boolean load(int kind, int piece) {
        if (file == null) {
            return false;
        }
        if (kind == curKind && piece == curPiece) {
            return frames != null;
        }
        curKind = kind;
        curPiece = piece;
        frames = null;
        shownFrame = -1;
        for (int i = 0; i < count; i++) {
            int e = 10 + i * 12;
            if ((file[e] & 255) != kind || (file[e + 1] & 255) != piece) {
                continue;
            }
            int n = u16(e + 2), off = u32(e + 4), size = u32(e + 8);
            try {
                Inflater inf = new Inflater();
                inf.setInput(file, off, size);
                byte[] raw = new byte[n * w * h * 2];
                int got = 0;
                while (got < raw.length && !inf.finished()) {
                    int k = inf.inflate(raw, got, raw.length - got);
                    if (k == 0 && (inf.needsInput() || inf.needsDictionary())) {
                        break;
                    }
                    got += k;
                }
                inf.end();
                int[][] out = new int[n][w * h];
                int[] prev = new int[w * h];
                for (int f = 0; f < n; f++) {
                    int base = f * w * h * 2;
                    for (int p = 0; p < w * h; p++) {
                        int v = ((raw[base + p * 2] & 255) | (raw[base + p * 2 + 1] & 255) << 8) ^ prev[p];
                        prev[p] = v;
                        int r = v & 31, g = (v >> 5) & 31, b = (v >> 10) & 31;
                        out[f][p] = 0xFF000000 | (r << 3 | r >> 2) << 16 | (g << 3 | g >> 2) << 8 | (b << 3 | b >> 2);
                    }
                }
                frames = out;
                curFrames = n;
                return true;
            } catch (Exception ex) {
                return false;
            }
        }
        return false;
    }

    public int frames() {
        return frames == null ? 0 : curFrames;
    }

    /** The loaded clip's frame (wrapping), as a bitmap to draw */
    public Bitmap frame(int f) {
        if (frames == null || curFrames == 0) {
            return null;
        }
        f = ((f % curFrames) + curFrames) % curFrames;
        if (bitmap == null) {
            bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        }
        if (f != shownFrame) {
            bitmap.setPixels(frames[f], 0, w, 0, 0, w, h);
            shownFrame = f;
        }
        return bitmap;
    }
}
