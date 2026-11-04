package com.moemon.companion;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.util.HashMap;
import java.util.Map;

/**
 * The app's player of the game's sounds (Sound.java): the pack's ADPCM decodes as the game's export wrote it (the
 * decoded sounds aren't silence and their ends die away), and a song plays through its loop's seam without a jump:
 * the crossfade carries it from the loop's end into its start, and it goes round again and again.
 */
public class SoundTest {
    private static Map<String, Sound.Entry> index(byte[] b) {
        ByteBuffer in = ByteBuffer.wrap(b).order(ByteOrder.LITTLE_ENDIAN);
        assertEquals(0x444E534D, in.getInt());
        in.getInt();
        int count = in.getInt();
        Map<String, Sound.Entry> out = new HashMap<>();
        for (int i = 0; i < count; i++) {
            Sound.Entry e = new Sound.Entry();
            byte[] name = new byte[in.get() & 0xFF];
            in.get(name);
            e.name = new String(name);
            e.kind = in.get();
            e.channels = in.get();
            e.id = in.getShort() & 0xFFFF;
            e.rate = in.getInt();
            e.frames = in.getInt();
            e.loop = in.getInt();
            e.loopEnd = in.getInt();
            e.offset = in.getInt();
            e.size = in.getInt();
            out.put(e.name, e);
        }
        return out;
    }

    private static Sound.Pcm load(byte[] b, Sound.Entry e) {
        byte[] data = new byte[e.size];
        System.arraycopy(b, e.offset, data, 0, e.size);
        return new Sound.Pcm(e, Sound.decode(data, e.channels, e.frames));
    }

    @Test
    public void packDecodesAndSongsLoopSmoothly() throws Exception {
        File f = new File("src/main/assets/game/sounds.bin");
        org.junit.Assume.assumeTrue("no sounds pack (android/build_assets.py)", f.exists());
        byte[] b = Files.readAllBytes(f.toPath());
        Map<String, Sound.Entry> all = index(b);
        for (String n : new String[]{"se_select", "se_failure", "cry_25", "mus_obtain_item"}) {
            Sound.Pcm p = load(b, all.get(n));
            int peak = 0;
            for (short x : p.data) {
                peak = Math.max(peak, Math.abs(x));
            }
            assertTrue(n + " is sound", peak > 1000);
            int last = p.data[p.data.length - 1];
            assertTrue(n + " dies away (" + last + ")", Math.abs(last) < 2000);
        }
        for (String n : new String[]{"tt:mus_rg_vs_trainer", "tcg:mus_vs_trainer", "ygo:mus_vs_trainer"}) {
            Sound.Entry e = all.get(n);
            Sound.Pcm p = load(b, e);
            assertTrue(n + " has a loop", e.loop >= 0 && e.loop < e.loopEnd && e.loopEnd < e.frames);
            // played at the song's own rate: through the seam twice, each step no bigger than the song's own steps
            Sound.Voice v = Sound.voice(p, e.rate);
            v.pos = e.loopEnd - e.rate / 10.0;
            int biggest = 0;
            for (int i = 0; i < e.frames; i++) {
                biggest = Math.max(biggest, Math.abs(p.data[i * e.channels] - p.data[Math.max(0, i - 1) * e.channels]));
            }
            float before = v.at(v.pos, 0);
            int rounds = 0;
            double lastPos = v.pos;
            for (int i = 0; i < (e.loopEnd - e.loop) * 2 + e.rate; i++) {
                assertTrue(n + " plays on", v.advance());
                float now = v.at(v.pos, 0);
                assertTrue(n + ": a jump of " + (now - before) + " at " + v.pos, Math.abs(now - before) <= biggest * 1.5f + 64);
                before = now;
                if (v.pos < lastPos) {
                    rounds++;
                }
                lastPos = v.pos;
                assertTrue(n + " stays in the song", v.pos >= 0 && v.pos < e.frames);
            }
            assertTrue(n + " went round its loop", rounds >= 2);
        }
    }
}
