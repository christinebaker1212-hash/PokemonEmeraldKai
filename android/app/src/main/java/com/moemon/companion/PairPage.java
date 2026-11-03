package com.moemon.companion;

import android.graphics.Canvas;

/**
 * Pairing (docs/COMPANION_APP.md "Linking"): scan the PHONE LINK screen's QR code, or type its 6 letters (the app then
 * finds the game on this network).
 */
public final class PairPage extends Page {
    private static final String ALPHABET = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    private final StringBuilder code = new StringBuilder();
    private boolean typing, busy;
    private String error = "";

    public PairPage(GameView v) {
        super(v);
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        art.text(c, "PAIR A GAME", 8, 4, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
        if (!v.app.saves.isEmpty()) {
            button(c, "BACK", w - 70, 0, 66, true);
        }
        art.window(c, 0, 12, 30, w - 24, 64);
        art.text(c, "In the game, open OPTIONS,", 14, 30);
        art.text(c, "then CO-OP > PHONE LINK.", 14, 46);
        art.text(c, "Then scan its code, or type", 14, 62);
        art.text(c, "the 6 letters under it.", 14, 78);
        int y = 108;
        if (busy) {
            art.window(c, 0, 12, y + 4, w - 24, 16);
            art.text(c, "Linking...", 14, y + 4);
            return;
        }
        if (!error.isEmpty()) {
            art.window(c, 0, 12, y + 4, w - 24, 32);
            java.util.List<String> lines = wrap(error, w - 32);
            for (int i = 0; i < Math.min(2, lines.size()); i++) {
                art.text(c, lines.get(i), 14, y + 4 + i * 16, GameArt.RED_TEXT, GameArt.SHADOW);
            }
            y += 44;
        }
        if (!typing) {
            button(c, "SCAN THE CODE", 12, y, w - 24, true);
            button(c, "TYPE THE LETTERS", 12, y + 30, w - 24, true);
            return;
        }
        // the letters typed, then the game's 32 letters to tap
        art.window(c, 0, (w - 90) / 2, y + 4, 90, 16);
        for (int i = 0; i < 6; i++) {
            String ch = i < code.length() ? String.valueOf(code.charAt(i)) : "-";
            art.text(c, ch, (w - 90) / 2 + 6 + i * 14, y + 4);
        }
        int ky = y + 30, kw = (w - 16) / 8;
        for (int i = 0; i < ALPHABET.length(); i++) {
            int kx = 8 + (i % 8) * kw, kyy = ky + (i / 8) * 26;
            art.window(c, 0, kx + 4, kyy + 4, kw - 8, 16);
            String ch = String.valueOf(ALPHABET.charAt(i));
            art.text(c, ch, kx + (kw - art.width(ch)) / 2, kyy + 4);
        }
        int by = ky + 4 * 26 + 4, bw = (w - 16) / 3;
        button(c, "DEL", 8, by, bw, code.length() > 0);
        button(c, "CANCEL", 8 + bw, by, bw, true);
        button(c, "OK", 8 + 2 * bw, by, bw, code.length() == 6);
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width();
        if (busy) {
            return;
        }
        if (y < 24 && x > w - 70 && !v.app.saves.isEmpty()) {
            back();
            return;
        }
        int top = 108 + (error.isEmpty() ? 0 : 44);
        if (!typing) {
            if (y >= top && y < top + 24) {
                error = "";
                Menu.openScan(v);
            } else if (y >= top + 30 && y < top + 54) {
                error = "";
                typing = true;
            }
            return;
        }
        int ky = top + 30, kw = (w - 16) / 8;
        if (y >= ky && y < ky + 4 * 26) {
            int i = (y - ky) / 26 * 8 + Math.min(7, Math.max(0, (x - 8) / kw));
            if (i < ALPHABET.length() && code.length() < 6) {
                code.append(ALPHABET.charAt(i));
                v.buzz(false);
            }
            return;
        }
        int by = ky + 4 * 26 + 4, bw = (w - 16) / 3;
        if (y >= by && y < by + 24) {
            int k = (x - 8) / bw;
            if (k == 0 && code.length() > 0) {
                code.setLength(code.length() - 1);
            } else if (k == 1) {
                typing = false;
                code.setLength(0);
            } else if (k == 2 && code.length() == 6) {
                start(null, code.toString());
            }
        }
    }

    /** From the scanner (MainActivity), or the letters */
    public void start(String qr, String letters) {
        busy = true;
        error = "";
        v.invalidate();
        v.app.pair(qr, letters, err -> {
            busy = false;
            if (err == null) {
                v.buzz(true);
                v.showOver(null);
                v.select(0);
            } else {
                error = Companion.why(err);
                code.setLength(0);
            }
            v.invalidate();
        });
    }
}
