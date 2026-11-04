package com.moemon.companion;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import java.io.FileInputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * The game's own sounds in the app: assets game/sounds.bin, recorded from the game's sound engine when the APK is
 * built (src/mp/mp_export_sound.c: every sound effect the app uses, every POKéMON's cry, the fanfares and the card
 * games' duel songs as each scene plays them), played the way the game plays them: sound effects on three voices (the
 * same one again starts over), one cry at a time (the music quiet under it, 85/256 as the game's PlayCry_Normal), a
 * fanfare stopping the music and the music going on after it, songs looping where the game loops them (a 50 ms
 * crossfade over the seam). SOUND EFFECTS and MUSIC are switched in the app's MENU > SOUND; the music stops when the app
 * isn't in front, waits for another app's music to end before it starts, and gives way to calls and other players.
 */
public final class Sound {
    public static final int SE = 0, CRY = 1, FANFARE = 2, SONG = 3;
    private static final int SE_VOICES = 3;
    private static final float DUCK = 85 / 256f;
    private static final int[] STEPS = {7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
            50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
            449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499,
            2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635,
            13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};
    private static final int[] MOVE = {-1, -1, -1, -1, 2, 4, 6, 8};

    private static Sound sInstance;

    /** One sound in the pack */
    static final class Entry {
        String name;
        int kind, channels, id, rate, frames, loop, loopEnd, offset, size;
    }

    /** A sound decoded: 16-bit samples, channels interleaved */
    static final class Pcm {
        final Entry e;
        final short[] data;

        Pcm(Entry e, short[] data) {
            this.e = e;
            this.data = data;
        }
    }

    static final class Voice {
        Pcm pcm;
        double pos;            // in the sound's own frames
        double step;           // its rate / the output's
        boolean paused;
        float gain = 1, fadeIn = 1;
        long started;

        float sample(int frame, int c) {
            Entry e = pcm.e;
            int ch = e.channels;
            return pcm.data[frame * ch + (ch == 2 ? c : 0)];
        }

        /** The sound at pos (a song past its loop's end: crossfading into the loop's start), channel c */
        float at(double p, int c) {
            Entry e = pcm.e;
            int i = (int) p;
            float f = (float) (p - i);
            if (e.loop >= 0 && i >= e.loopEnd) {
                int past = i - e.loopEnd, fade = e.frames - e.loopEnd - 1;
                float t = fade > 0 ? Math.min(1, (past + f) / fade) : 1;
                float a = lerp(i, f, c), b = lerp(e.loop + past, f, c);
                return a * (1 - t) + b * t;
            }
            return lerp(i, f, c);
        }

        private float lerp(int i, float f, int c) {
            int last = pcm.e.frames - 1;
            float a = sample(Math.min(i, last), c), b = sample(Math.min(i + 1, last), c);
            return a + (b - a) * f;
        }

        /** On by one output frame; false: it's over */
        boolean advance() {
            Entry e = pcm.e;
            pos += step;
            if (e.loop >= 0 && pos >= e.frames - 1) {
                pos = e.loop + (pos - e.loopEnd);   // (the crossfade done: on from inside the loop)
            }
            return e.loop >= 0 || pos < e.frames - 1;
        }
    }

    private final Context context;
    private final Map<String, Entry> entries = new HashMap<>();
    private final Map<Integer, String> effectNames = new HashMap<>();   // (a sound effect's SE_ number: its name)
    private final Map<String, Pcm> effects = new HashMap<>();
    private final LinkedHashMap<String, Pcm> cries = new LinkedHashMap<String, Pcm>(16, 0.75f, true) {
        @Override
        protected boolean removeEldestEntry(Map.Entry<String, Pcm> eldest) {
            return size() > 24;
        }
    };
    private final Voice[] se = new Voice[SE_VOICES];
    private Voice cry, fanfare, music;
    private String musicName, wantedMusic;
    private final int outRate;
    private AudioTrack track;
    private Thread mixer;
    private boolean front = true, focus;
    private float duck = 1;
    private AudioFocusRequest focusRequest;

    public static synchronized Sound get(Context c) {
        if (sInstance == null) {
            sInstance = new Sound(c.getApplicationContext());
        }
        return sInstance;
    }

    private Sound(Context c) {
        context = c;
        int rate = 48000;
        try {
            rate = AudioTrack.getNativeOutputSampleRate(AudioManager.STREAM_MUSIC);
        } catch (Exception ignored) {
        }
        outRate = rate > 0 ? rate : 48000;
        try (AssetFileDescriptor fd = c.getAssets().openFd("game/sounds.bin");
             FileInputStream in = fd.createInputStream()) {
            FileChannel ch = in.getChannel();
            ByteBuffer head = ByteBuffer.allocate(12).order(ByteOrder.LITTLE_ENDIAN);
            ch.read(head, fd.getStartOffset());
            head.flip();
            if (head.getInt() != 0x444E534D) {   // "MSND"
                return;
            }
            head.getInt();
            int count = head.getInt();
            ByteBuffer index = ByteBuffer.allocate(count * (64 + 28)).order(ByteOrder.LITTLE_ENDIAN);
            ch.read(index, fd.getStartOffset() + 12);
            index.flip();
            for (int i = 0; i < count && index.remaining() > 29; i++) {
                Entry e = new Entry();
                byte[] name = new byte[index.get() & 0xFF];
                index.get(name);
                e.name = new String(name, "US-ASCII");
                e.kind = index.get();
                e.channels = index.get();
                e.id = index.getShort() & 0xFFFF;
                e.rate = index.getInt();
                e.frames = index.getInt();
                e.loop = index.getInt();
                e.loopEnd = index.getInt();
                e.offset = index.getInt();
                e.size = index.getInt();
                entries.put(e.name, e);
                if (e.kind == SE) {
                    effectNames.put(e.id, e.name);
                }
            }
        } catch (Exception ignored) {
            // (no pack: a build without the game's export; the app is silent)
        }
    }

    // ------------------------------------------------------------------------------------------- settings

    private android.content.SharedPreferences prefs() {
        return context.getSharedPreferences("moemon", 0);
    }

    public boolean effectsOn() {
        return prefs().getBoolean("sound_effects", true);
    }

    public boolean musicOn() {
        return prefs().getBoolean("sound_music", true);
    }

    public void setEffects(boolean on) {
        prefs().edit().putBoolean("sound_effects", on).apply();
    }

    public void setMusic(boolean on) {
        prefs().edit().putBoolean("sound_music", on).apply();
        if (on) {
            music(wantedMusic);
        } else {
            synchronized (this) {
                music = null;
                musicName = null;
            }
            dropFocus();
        }
    }

    public boolean has(String name) {
        return entries.containsKey(name);
    }

    // --------------------------------------------------------------------------------------------- playing

    /** A sound effect (se_select, se_rg_card_flip...): the game's PlaySE */
    public void play(String name) {
        if (!effectsOn() || !front) {
            return;
        }
        Pcm p = effect(name);
        if (p == null) {
            return;
        }
        synchronized (this) {
            int slot = -1;
            long oldest = Long.MAX_VALUE;
            for (int i = 0; i < SE_VOICES; i++) {
                if (se[i] != null && se[i].pcm == p) {
                    slot = i;   // (the same sound again: from the start)
                    break;
                }
                if (se[i] == null) {
                    slot = i;
                    oldest = Long.MIN_VALUE;
                } else if (se[i].started < oldest && oldest != Long.MIN_VALUE) {
                    oldest = se[i].started;
                    slot = i;
                }
            }
            se[slot] = voice(p);
        }
        start();
    }

    /** A sound effect by the game's number for it (an SE_ constant: as the game's tables give it) */
    public void play(int seNumber) {
        String n = effectNames.get(seNumber);
        if (n != null) {
            play(n);
        }
    }

    /**
     * A sound as the card tables name them: "se_..." a sound effect, "#N" one by its SE_ number, "!scene:song" that
     * music from here ("!" alone: the music stops), "cry_N" a cry, "mus_..." a fanfare
     */
    public void cue(String what) {
        if (what == null || what.isEmpty()) {
            return;
        }
        if (what.charAt(0) == '#') {
            try {
                play(Integer.parseInt(what.substring(1)));
            } catch (NumberFormatException ignored) {
            }
        } else if (what.charAt(0) == '!') {
            music(what.length() > 1 ? what.substring(1) : null);
        } else if (what.startsWith("cry_")) {
            try {
                cry(Integer.parseInt(what.substring(4)));
            } catch (NumberFormatException ignored) {
            }
        } else if (what.startsWith("mus_")) {
            fanfare(what);
        } else {
            play(what);
        }
    }

    /** A POKéMON's cry, as the game plays it (the music quieter under it) */
    public void cry(int species) {
        if (!effectsOn() || !front || species <= 0) {
            return;
        }
        String key = "cry_" + species;
        Pcm p;
        synchronized (cries) {
            p = cries.get(key);
        }
        if (p == null) {
            p = decode(entries.get(key));
            if (p == null) {
                return;
            }
            synchronized (cries) {
                cries.put(key, p);
            }
        }
        synchronized (this) {
            cry = voice(p);
        }
        start();
    }

    /** A fanfare (mus_obtain_item, mus_level_up...): the music stops for it and goes on after */
    public void fanfare(String name) {
        if (!effectsOn() || !front) {
            return;
        }
        Pcm p = effect(name);
        if (p == null) {
            return;
        }
        synchronized (this) {
            fanfare = voice(p);
        }
        start();
    }

    /** The music to play here (scene:song, as "tcg:mus_vs_trainer"; null: none). The same song again goes on. */
    public void music(String name) {
        wantedMusic = name;
        if (name == null) {
            synchronized (this) {
                music = null;
                musicName = null;
            }
            dropFocus();
            return;
        }
        if (!musicOn() || !front || name.equals(musicName) || !entries.containsKey(name)) {
            return;
        }
        if (!focus && !askFocus()) {
            return;
        }
        final Entry e = entries.get(name);
        synchronized (this) {
            musicName = name;
        }
        // (a song is a few MB decoded: away from the page that asked)
        new Thread(() -> {
            Pcm p = decode(e);
            synchronized (Sound.this) {
                if (p == null || !name.equals(musicName)) {
                    return;
                }
                music = voice(p);
                music.fadeIn = 0;
            }
            start();
        }, "moemon-music").start();
    }

    public String music() {
        return wantedMusic;
    }

    /** The app went behind something: quiet; back in front: the music goes on */
    public void front(boolean inFront) {
        front = inFront;
        if (!inFront) {
            synchronized (this) {
                for (int i = 0; i < SE_VOICES; i++) {
                    se[i] = null;
                }
                cry = null;
                fanfare = null;
                music = null;
                musicName = null;
            }
            dropFocus();
        } else if (wantedMusic != null) {
            music(wantedMusic);
        }
    }

    private Voice voice(Pcm p) {
        return voice(p, outRate);
    }

    static Voice voice(Pcm p, int outRate) {
        Voice v = new Voice();
        v.pcm = p;
        v.step = p.e.rate / (double) outRate;
        v.started = System.nanoTime();
        return v;
    }

    private Pcm effect(String name) {
        Pcm p;
        synchronized (effects) {
            p = effects.get(name);
        }
        if (p == null) {
            p = decode(entries.get(name));
            if (p != null) {
                synchronized (effects) {
                    effects.put(name, p);
                }
            }
        }
        return p;
    }

    /** IMA ADPCM, as mp_export_sound.c wrote it */
    Pcm decode(Entry e) {
        if (e == null) {
            return null;
        }
        byte[] b = new byte[e.size];
        try (AssetFileDescriptor fd = context.getAssets().openFd("game/sounds.bin");
             FileInputStream in = fd.createInputStream()) {
            ByteBuffer buf = ByteBuffer.wrap(b);
            FileChannel ch = in.getChannel();
            long at = fd.getStartOffset() + e.offset;
            while (buf.hasRemaining()) {
                int n = ch.read(buf, at + buf.position());
                if (n <= 0) {
                    return null;
                }
            }
        } catch (Exception ex) {
            return null;
        }
        return new Pcm(e, decode(b, e.channels, e.frames));
    }

    static short[] decode(byte[] b, int channels, int frames) {
        int[] pred = new int[channels], index = new int[channels];
        for (int c = 0; c < channels; c++) {
            pred[c] = (short) ((b[c * 4] & 0xFF) | (b[c * 4 + 1] << 8));
            index[c] = b[c * 4 + 2] & 0xFF;
        }
        int base = 4 * channels, n = frames * channels;
        short[] out = new short[n];
        for (int i = 0; i < n; i++) {
            int c = channels == 2 ? i & 1 : 0;
            int code = (b[base + (i >> 1)] >> ((i & 1) * 4)) & 15;
            int step = STEPS[index[c]], d = step >> 3;
            if ((code & 4) != 0) {
                d += step;
            }
            if ((code & 2) != 0) {
                d += step >> 1;
            }
            if ((code & 1) != 0) {
                d += step >> 2;
            }
            int p = (code & 8) != 0 ? pred[c] - d : pred[c] + d;
            pred[c] = p > 32767 ? 32767 : Math.max(p, -32768);
            index[c] = Math.max(0, Math.min(88, index[c] + MOVE[code & 7]));
            out[i] = (short) pred[c];
        }
        return out;
    }

    // ---------------------------------------------------------------------------------------------- focus

    private final AudioManager.OnAudioFocusChangeListener focusListener = change -> {
        if (change == AudioManager.AUDIOFOCUS_LOSS || change == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT) {
            synchronized (Sound.this) {
                music = null;
                musicName = null;
            }
            if (change == AudioManager.AUDIOFOCUS_LOSS) {
                focus = false;   // (another player took over: the music waits for the next page that asks)
            }
        } else if (change == AudioManager.AUDIOFOCUS_GAIN) {
            focus = true;
            if (wantedMusic != null && front) {
                music(wantedMusic);
            }
        }
    };

    private boolean askFocus() {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            return false;
        }
        if (otherMusic(am)) {
            return false;   // (the player's own music is on: the game's waits)
        }
        focusRequest = new AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
                .setAudioAttributes(new AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME)
                        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build())
                .setOnAudioFocusChangeListener(focusListener).build();
        focus = am.requestAudioFocus(focusRequest) == AudioManager.AUDIOFOCUS_REQUEST_GRANTED;
        return focus;
    }

    /** Another app's music or video is playing (this app's sounds play as a game's) */
    private static boolean otherMusic(AudioManager am) {
        try {
            for (android.media.AudioPlaybackConfiguration c : am.getActivePlaybackConfigurations()) {
                if (c.getAudioAttributes().getUsage() == AudioAttributes.USAGE_MEDIA) {
                    return true;
                }
            }
        } catch (Exception ignored) {
        }
        return false;
    }

    private void dropFocus() {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am != null && focusRequest != null) {
            am.abandonAudioFocusRequest(focusRequest);
        }
        focus = false;
    }

    // ---------------------------------------------------------------------------------------------- mixing

    private synchronized boolean anything() {
        for (Voice v : se) {
            if (v != null) {
                return true;
            }
        }
        return cry != null || fanfare != null || music != null;
    }

    private void start() {
        synchronized (this) {
            if (mixer != null) {
                notifyAll();
                return;
            }
            mixer = new Thread(this::run, "moemon-sound");
            mixer.setPriority(Thread.MAX_PRIORITY);
            mixer.start();
        }
    }

    private void run() {
        int frames = Math.max(128, outRate / 200);   // (5 ms a block)
        try {
            int min = AudioTrack.getMinBufferSize(outRate, AudioFormat.CHANNEL_OUT_STEREO, AudioFormat.ENCODING_PCM_16BIT);
            track = new AudioTrack.Builder()
                    .setAudioAttributes(new AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_GAME)
                            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION).build())
                    .setAudioFormat(new AudioFormat.Builder().setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .setSampleRate(outRate).setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build())
                    .setBufferSizeInBytes(Math.max(min, frames * 4 * 2))
                    .setTransferMode(AudioTrack.MODE_STREAM)
                    .setPerformanceMode(AudioTrack.PERFORMANCE_MODE_LOW_LATENCY)
                    .build();
        } catch (Exception e) {
            synchronized (this) {
                mixer = null;
            }
            return;
        }
        short[] out = new short[frames * 2];
        float[] acc = new float[frames * 2];
        long idleSince = 0;
        boolean playing = false;
        try {
            while (true) {
                if (!anything()) {
                    if (idleSince == 0) {
                        idleSince = System.nanoTime();
                    }
                    if (System.nanoTime() - idleSince > 1_500_000_000L) {
                        // (quiet a while: the device's output let go until the next sound)
                        if (playing) {
                            track.pause();
                            track.flush();
                            playing = false;
                        }
                        synchronized (this) {
                            if (!anything()) {
                                wait(30_000);
                            }
                            if (!anything()) {
                                break;
                            }
                        }
                        idleSince = 0;
                    }
                } else {
                    idleSince = 0;
                }
                if (!playing) {
                    track.play();
                    playing = true;
                }
                mix(acc, frames);
                for (int i = 0; i < frames * 2; i++) {
                    float s = acc[i];
                    out[i] = (short) (s > 32767 ? 32767 : s < -32768 ? -32768 : s);
                }
                track.write(out, 0, frames * 2);
            }
        } catch (InterruptedException ignored) {
        } finally {
            track.release();
            track = null;
            synchronized (this) {
                mixer = null;
            }
            if (anything()) {
                start();
            }
        }
    }

    private synchronized void mix(float[] acc, int frames) {
        java.util.Arrays.fill(acc, 0);
        for (int i = 0; i < SE_VOICES; i++) {
            if (se[i] != null && !add(se[i], acc, frames, 1)) {
                se[i] = null;
            }
        }
        if (cry != null && !add(cry, acc, frames, 1)) {
            cry = null;
        }
        if (fanfare != null && !add(fanfare, acc, frames, 1)) {
            fanfare = null;
        }
        float want = cry != null ? DUCK : 1;
        if (music != null && fanfare == null) {
            float d0 = duck;
            duck += (want - duck) * Math.min(1, frames / (outRate * 0.03f));
            if (!add(music, acc, frames, (d0 + duck) / 2)) {
                music = null;
            }
        } else {
            duck = want;
        }
    }

    /** One voice into the block; false: it ended */
    private boolean add(Voice v, float[] acc, int frames, float gain) {
        for (int i = 0; i < frames; i++) {
            if (v.fadeIn < 1) {
                v.fadeIn = Math.min(1, v.fadeIn + 1f / (outRate * 0.25f));
            }
            float g = gain * v.gain * v.fadeIn;
            acc[i * 2] += v.at(v.pos, 0) * g;
            acc[i * 2 + 1] += v.at(v.pos, 1) * g;
            if (!v.advance()) {
                return false;
            }
        }
        return true;
    }
}
