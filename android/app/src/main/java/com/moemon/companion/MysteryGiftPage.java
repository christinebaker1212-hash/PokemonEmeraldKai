package com.moemon.companion;

import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Paint;

import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

/**
 * MYSTERY GIFT (the user, 2026-11-05): any QR code or a product's bar code, scanned with the camera, gives an item, up
 * to twelve a day. The game picks it from the code, the day and the Trainer ID ("mgift" order: mp_phone_kai.c), so
 * the same code gives the same gift that day and another tomorrow; the code itself never leaves the phone (only its
 * hash). Here: the wrapped box shakes while the game answers, then opens on the item, its color by how good it is.
 */
public final class MysteryGiftPage extends Page {
    private static final String[] TIER_WORDS = {"Hmm...", "Nice!", "Great!", "Wonderful!", "AMAZING!!"};
    private static final int[] TIER_COLORS = {0xFF98A0A8, 0xFF58B8F8, 0xFF58D068, 0xFFF8B030, 0xFFF85898};
    private final Paint fill = new Paint();
    private final long sentAt;
    private JSONObject gift;
    private long openedAt;
    private String refused;

    MysteryGiftPage(GameView v) {
        super(v);
        sentAt = System.currentTimeMillis();
    }

    /** MYSTERY GIFT's scan (ME page): the camera, any code */
    static void openScan(GameView v) {
        if (v.app.client == null) {
            v.message("Link with your game first: the gift goes to its BAG.");
            return;
        }
        if (v.app.locked()) {
            v.message(Companion.LOCKED);
            return;
        }
        Intent i = new Intent(v.getContext(), ScanActivity.class).putExtra("gift", true);
        ((MainActivity) v.getContext()).startActivityForResult(i, 2);
    }

    /** A code was read: its hash to the game, and the box while it answers */
    static void scanned(GameView v, String text) {
        if (text == null || text.isEmpty() || v.app.client == null) {
            return;
        }
        try {
            v.app.client.order(new JSONObject().put("op", "mgift").put("code", hash(text)));
        } catch (org.json.JSONException ignored) {
            return;
        }
        v.showOver(new MysteryGiftPage(v));
    }

    static String hash(String text) {
        try {
            byte[] d = MessageDigest.getInstance("SHA-256").digest(text.trim().getBytes(StandardCharsets.UTF_8));
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 16; i++) {
                sb.append(String.format("%02x", d[i]));
            }
            return sb.toString();
        } catch (Exception e) {
            return Integer.toHexString(text.hashCode());
        }
    }

    @Override
    public boolean fullScreen() {
        return true;
    }

    @Override
    public void draw(Canvas c, int w, int h) {
        long now = System.currentTimeMillis();
        if (gift == null && refused == null) {
            if (v.app.lastGiftAt > sentAt && v.app.lastGift != null) {
                gift = v.app.lastGift;
                openedAt = now + 700;   // (the box shakes a little longer, then opens)
            } else if (v.app.toastAt > sentAt && !v.app.toastGood) {
                refused = v.app.toast;
                v.sound.play("se_failure");
            } else if (now - sentAt > 15000) {
                refused = "The game didn't answer. Is it running, with the PHONE LINK on?";
            }
        }
        int tier = gift == null ? 0 : Math.max(0, Math.min(4, gift.optInt("tier")));
        boolean open = gift != null && now >= openedAt;
        // the background: dark stripes, the tier's color once open
        fill.setColor(0xFF181828);
        c.drawRect(0, 0, w, h, fill);
        fill.setColor(open ? (TIER_COLORS[tier] & 0x00FFFFFF) | 0x30000000 : 0x18FFFFFF);
        int off = (int) ((now / 50) % 24);
        for (int x = -h; x < w + 24; x += 24) {
            for (int k = 0; k < 6; k++) {
                c.drawLine(x + k + off, 0, x + k + off + h, h, fill);
            }
        }
        banner(c, w, "MYSTERY GIFT", open || refused != null ? "OK" : "");
        int cx = w / 2, cy = h / 2 - 30;
        if (!open) {
            // the wrapped box, shaking harder until it opens
            float shake = refused != null ? 0 : (float) Math.sin(now / 45.0) * (gift != null ? 4 : 2);
            drawBox(c, cx + shake, cy, false, 0);
            String line = refused != null ? "" : "Wrapping your gift...";
            art.text(c, line, (w - art.width(line)) / 2, cy + 50, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
            if (refused != null) {
                int ty = cy + 48;
                for (String l : wrap(refused, w - 32)) {
                    art.text(c, l, (w - art.width(l)) / 2, ty, 0xFFF8A050, GameArt.WHITE_SHADOW);
                    ty += 16;
                }
            }
        } else {
            long t = now - openedAt;
            if (t < 40) {
                // (the moment it opens: its sound)
                v.sound.play(tier >= 4 ? "se_shiny" : "se_rg_ball_click");
                if (tier >= 3) {
                    v.sound.fanfare("mus_obtain_item");
                }
            }
            drawBox(c, cx, cy, true, t);
            // the item rising out of the box, with sparkles by tier
            float rise = Math.min(1f, t / 500f);
            int iy = (int) (cy - 10 - 34 * rise);
            for (int k = 0; k < 6 + tier * 4; k++) {
                double a = k * 2 * Math.PI / (6 + tier * 4) + t / 600.0;
                float r = 18 + 10 * rise + (float) Math.sin(t / 150.0 + k) * 3;
                fill.setColor(TIER_COLORS[tier]);
                float sx = (float) (cx + Math.cos(a) * r), sy = (float) (iy + 12 + Math.sin(a) * r * 0.7);
                c.drawRect(sx - 1, sy - 1, sx + 1, sy + 1, fill);
            }
            art.itemIcon(c, gift.optInt("item"), cx - 12, iy);
            String word = TIER_WORDS[tier];
            art.text(c, word, (w - art.width(word)) / 2, cy + 44, TIER_COLORS[tier], 0xFF101018);
            String name = art.plain(GameArt.hexText(gift.optString("n")));
            int qty = gift.optInt("qty", 1);
            String got = (qty > 1 ? name + " x" + qty : name);
            art.text(c, got, (w - art.width(got)) / 2, cy + 62, GameArt.WHITE_TEXT, GameArt.WHITE_SHADOW);
            String sent = "It's in your BAG in the game.";
            art.text(c, sent, (w - art.width(sent)) / 2, cy + 80, GameArt.GRAY_TEXT, 0xFF101018);
            int left = gift.optInt("left");
            String l = left > 0 ? left + " LEFT TODAY" : "That's all for today!";
            art.text(c, l, (w - art.width(l)) / 2, cy + 100, GOLD, 0xFF101018);
            if (left > 0) {
                button(c, "SCAN ANOTHER", w / 2 - 60, h - 40, 120, true);
            }
        }
        v.invalidate();   // (it moves)
    }

    // a present: a box and its lid, a ribbon across; open, the lid lifts away and tilts
    private void drawBox(Canvas c, float cx, float cy, boolean open, long t) {
        int bw = 40, bh = 30;
        fill.setColor(0xFF101018);
        c.drawRect(cx - bw / 2f - 1, cy - 1, cx + bw / 2f + 1, cy + bh + 1, fill);
        fill.setColor(0xFFE84848);
        c.drawRect(cx - bw / 2f, cy, cx + bw / 2f, cy + bh, fill);
        fill.setColor(0xFFF8D030);
        c.drawRect(cx - 3, cy, cx + 3, cy + bh, fill);
        float lift = open ? Math.min(16f, t / 16f) : 0;
        float tilt = open ? Math.min(35f, t / 12f) : 0;
        float aside = open ? Math.min(34f, t / 10f) : 0;   // (it flies off to the left, clear of the gift)
        c.save();
        c.translate(-aside, 0);
        c.rotate(-tilt, cx - bw / 2f - 2, cy - lift);
        fill.setColor(0xFF101018);
        c.drawRect(cx - bw / 2f - 3, cy - 9 - lift, cx + bw / 2f + 3, cy + 1 - lift, fill);
        fill.setColor(0xFFF85858);
        c.drawRect(cx - bw / 2f - 2, cy - 8 - lift, cx + bw / 2f + 2, cy - lift, fill);
        fill.setColor(0xFFF8D030);
        c.drawRect(cx - 3, cy - 8 - lift, cx + 3, cy - lift, fill);
        // the bow
        c.drawRect(cx - 9, cy - 13 - lift, cx - 2, cy - 8 - lift, fill);
        c.drawRect(cx + 2, cy - 13 - lift, cx + 9, cy - 8 - lift, fill);
        c.restore();
    }

    @Override
    public void tap(int x, int y) {
        int w = v.width(), h = v.contentHeight();
        if (y < 20 && x > w - 60 && (gift != null || refused != null)) {
            back();
            return;
        }
        if (gift != null && System.currentTimeMillis() >= openedAt && gift.optInt("left") > 0 && y >= h - 40 && y < h - 16) {
            v.showOver(null);
            openScan(v);
        }
    }
}
