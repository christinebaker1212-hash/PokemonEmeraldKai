package com.moemon.companion.core;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

/**
 * The phone link to the game (docs/PHONE_LINK.md): pairing, finding the game on the local network, a sealed session.
 * Blocking calls: LinkClient runs them on its own thread.
 */
public final class Link implements AutoCloseable {
    public static final int VERSION = 1;          // PHONE_LINK_VERSION (src/mp/phone_proto.h)
    public static final int TCP_PORT = 47610;
    public static final int UDP_PORT = 47611;
    private static final int FRAME_MAX = 1 << 20;

    private final Crypto crypto;
    private final Socket sock;
    private final DataInputStream in;
    private final OutputStream out;
    private byte[] sessionKey;
    private int seqOut, seqIn;
    // through the co-op server (docs/PHONE_LINK.md "Phase 4"): a pipe's channel, frames in chunks inside its messages
    private int channel = -1;
    public boolean viaServer() {
        return channel >= 0;
    }
    static final int MP_C_PIPE = 13, MP_S_PIPE = 33, PIPE_JOIN = 2, PIPE_DATA = 3, PIPE_OPENED = 1, PIPE_CLOSED = 4,
            PIPE_NOHOST = 5, PIPE_CHUNK = 7900;

    /** Refused by the game ({"t":"bye","why":...}): version, othersave, unknown, notpairing, wrongcode... */
    public static final class Refused extends IOException {
        public final String why;

        Refused(String why) {
            super("the game said no: " + why);
            this.why = why;
        }
    }

    private Link(Crypto crypto, String host, int port, int timeoutMs) throws IOException {
        this.crypto = crypto;
        sock = new Socket();
        sock.connect(new InetSocketAddress(host, port), timeoutMs);
        sock.setTcpNoDelay(true);
        in = new DataInputStream(sock.getInputStream());
        out = sock.getOutputStream();
    }

    /**
     * The screen's thread, where Android closes an app that touches the network (NetworkOnMainThreadException).
     * The desktop test sets it to its own thread, so the same mistake fails there too.
     */
    public static volatile Thread screenThread;

    private void frame(byte kind, byte[] body) throws IOException {
        if (Thread.currentThread() == screenThread) {
            throw new IllegalStateException("the network on the screen's thread");
        }
        if (channel >= 0) {
            byte[] whole = new byte[1 + body.length];
            whole[0] = kind;
            System.arraycopy(body, 0, whole, 1, body.length);
            int at = 0;
            do {
                int n = Math.min(PIPE_CHUNK, whole.length - at);
                byte[] m = new byte[4 + n];
                m[0] = PIPE_DATA;
                m[1] = (byte) channel;
                m[2] = (byte) (channel >> 8);
                m[3] = (byte) (at + n < whole.length ? 1 : 0);
                System.arraycopy(whole, at, m, 4, n);
                mpSend(MP_C_PIPE, m);
                at += n;
            } while (at < whole.length);
            return;
        }
        byte[] f = new byte[5 + body.length];
        int n = body.length + 1;
        f[0] = (byte) (n >>> 24);
        f[1] = (byte) (n >>> 16);
        f[2] = (byte) (n >>> 8);
        f[3] = (byte) n;
        f[4] = kind;
        System.arraycopy(body, 0, f, 5, body.length);
        synchronized (out) {
            out.write(f);
            out.flush();
        }
    }

    private void sendPlain(JSONObject o) throws IOException {
        frame((byte) 'P', o.toString().getBytes(StandardCharsets.UTF_8));
    }

    /** A message to the game, sealed */
    public synchronized void send(JSONObject o) throws IOException {
        frame((byte) 'S', crypto.seal(sessionKey, ++seqOut, o.toString().getBytes(StandardCharsets.UTF_8)));
    }

    /** The next message from the game (sealed ones opened); blocks */
    public int badMessages;
    public String lastBad = "";

    // The co-op server's own framing: u16 length (the type and the data), u8 type, data
    private void mpSend(int type, byte[] data) throws IOException {
        byte[] f = new byte[3 + data.length];
        int n = data.length + 1;
        f[0] = (byte) n;
        f[1] = (byte) (n >> 8);
        f[2] = (byte) type;
        System.arraycopy(data, 0, f, 3, data.length);
        synchronized (out) {
            out.write(f);
            out.flush();
        }
    }

    private byte[] mpRecv(int[] type) throws IOException {
        int n = in.readUnsignedByte() | (in.readUnsignedByte() << 8);
        if (n <= 0) {
            throw new IOException("bad server frame");
        }
        type[0] = in.readUnsignedByte();
        byte[] d = new byte[n - 1];
        in.readFully(d);
        return d;
    }

    // One frame from the game: straight, or gathered from the pipe's chunks
    private byte[] readFrame() throws IOException {
        if (channel < 0) {
            int n = in.readInt();
            if (n <= 0 || n > FRAME_MAX) {
                throw new IOException("bad frame");
            }
            byte[] body = new byte[n];
            in.readFully(body);
            return body;
        }
        java.io.ByteArrayOutputStream whole = new java.io.ByteArrayOutputStream();
        int[] type = new int[1];
        for (;;) {
            byte[] d = mpRecv(type);
            if (type[0] != MP_S_PIPE || d.length < 3) {
                continue;   // (the server's pings and the like)
            }
            if (d[0] == PIPE_CLOSED) {
                throw new IOException("the pipe closed");
            }
            if (d[0] != PIPE_DATA || d.length < 4) {
                continue;
            }
            whole.write(d, 4, d.length - 4);
            if (whole.size() > FRAME_MAX) {
                throw new IOException("bad frame");
            }
            if (d[3] == 0) {
                return whole.toByteArray();
            }
        }
    }

    public JSONObject recv() throws IOException {
        byte[] body = readFrame();
        int n = body.length;
        if (n == 0) {
            throw new IOException("bad frame");
        }
        try {
            if (body[0] == 'P') {
                JSONObject o = new JSONObject(new String(body, 1, n - 1, StandardCharsets.UTF_8));
                if ("bye".equals(o.optString("t"))) {
                    throw new Refused(o.optString("why"));
                }
                return o;
            }
            if (body[0] == 'S' && sessionKey != null) {
                byte[] sealed = new byte[n - 1];
                System.arraycopy(body, 1, sealed, 0, n - 1);
                byte[] opened = crypto.open(sessionKey, sealed);
                if (opened == null) {
                    throw new IOException("a damaged message");
                }
                int seq = ((opened[0] & 255) << 24) | ((opened[1] & 255) << 16) | ((opened[2] & 255) << 8) | (opened[3] & 255);
                if (seq <= seqIn) {
                    throw new IOException("a replayed message");
                }
                seqIn = seq;
                String text = new String(opened, 4, opened.length - 4, StandardCharsets.UTF_8);
                try {
                    return new JSONObject(text);
                } catch (JSONException e) {
                    // (a sealed message the app can't read: passed over, the link stays up; a game newer or older
                    // than the app must never make it link again and again)
                    badMessages++;
                    lastBad = text.length() > 200 ? text.substring(0, 200) : text;
                    return new JSONObject().put("t", "unreadable");
                }
            }
        } catch (JSONException e) {
            throw new IOException("bad JSON", e);
        }
        throw new IOException("an unknown frame");
    }

    public void setTimeout(int ms) throws IOException {
        sock.setSoTimeout(ms);
    }

    @Override
    public void close() {
        try {
            sock.close();
        } catch (IOException ignored) {
        }
    }

    /** Pair with the game showing the PHONE LINK screen: by its QR code (secretRaw) or the letters (code) */
    public static SavedLink pair(Crypto crypto, String host, int port, byte[] saveId, byte[] secretRaw, String code,
                                 String phoneName) throws IOException {
        byte[] secret = secretRaw != null ? crypto.qrSecret(secretRaw) : crypto.codeSecret(code.toUpperCase());
        byte[] phoneId = crypto.random(16);
        try (Link l = new Link(crypto, host, port, 4000)) {
            l.setTimeout(8000);
            JSONObject o = new JSONObject();
            o.put("t", "pair");
            o.put("v", VERSION);
            o.put("phone", Hex.of(phoneId));
            o.put("save", Hex.of(saveId));
            o.put("proof", Hex.of(crypto.pairProof(secret, phoneId, saveId)));
            o.put("name", phoneName);
            l.sendPlain(o);
            JSONObject r = l.recv();
            byte[] sealed = Hex.bytes(r.optString("key"));
            byte[] opened = sealed == null ? null : crypto.open(secret, sealed);
            if (!"paired".equals(r.optString("t")) || opened == null || opened.length != 4 + 32) {
                throw new IOException("the game's answer didn't open");
            }
            byte[] key = new byte[32];
            System.arraycopy(opened, 4, key, 0, 32);
            return new SavedLink(saveId, phoneId, key, host, port);
        } catch (JSONException e) {
            throw new IOException(e);
        }
    }

    /** A session with a paired save's game; the welcome message is read */
    public static Link connect(Crypto crypto, SavedLink s, String host, int port) throws IOException {
        return hello(new Link(crypto, host, port, 3000), crypto, s);
    }

    /**
     * The same session through the co-op server the game is on (ADDRESS "host:port"): the game hosts a pipe whose ID
     * comes from this phone's key, so only this phone can reach it; the server only passes the sealed frames along
     */
    public static Link viaServer(Crypto crypto, SavedLink s, String address) throws IOException {
        String host = address;
        int port = 27015;
        int colon = address.lastIndexOf(':');
        if (colon > 0) {
            host = address.substring(0, colon);
            try {
                port = Integer.parseInt(address.substring(colon + 1));
            } catch (NumberFormatException ignored) {
            }
        }
        Link l = new Link(crypto, host, port, 5000);
        try {
            l.setTimeout(8000);
            byte[] join = new byte[17];
            join[0] = PIPE_JOIN;
            System.arraycopy(crypto.derive(s.key, "pipe"), 0, join, 1, 16);
            l.mpSend(MP_C_PIPE, join);
            int[] type = new int[1];
            for (;;) {
                byte[] d = l.mpRecv(type);
                if (type[0] != MP_S_PIPE || d.length < 1) {
                    continue;
                }
                if (d[0] == PIPE_NOHOST) {
                    throw new IOException("the game isn't on the server");
                }
                if (d[0] == PIPE_OPENED && d.length >= 3) {
                    l.channel = (d[1] & 255) | ((d[2] & 255) << 8);
                    break;
                }
            }
        } catch (IOException e) {
            l.close();
            throw e;
        }
        return hello(l, crypto, s);
    }

    private static Link hello(Link l, Crypto crypto, SavedLink s) throws IOException {
        try {
            l.setTimeout(8000);
            byte[] nonce = crypto.random(24);
            JSONObject o = new JSONObject();
            o.put("t", "hello");
            o.put("v", VERSION);
            o.put("phone", Hex.of(s.phoneId));
            o.put("save", Hex.of(s.saveId));
            o.put("nonce", Hex.of(nonce));
            l.sendPlain(o);
            JSONObject w = l.recv();
            byte[] gameNonce = Hex.bytes(w.optString("nonce"));
            if (!"welcome".equals(w.optString("t")) || gameNonce == null || gameNonce.length != 24) {
                throw new IOException("no welcome");
            }
            l.sessionKey = crypto.sessionKey(s.key, nonce, gameNonce);
            l.setTimeout(0);
            return l;
        } catch (IOException | JSONException e) {
            l.close();
            throw e instanceof IOException ? (IOException) e : new IOException(e);
        }
    }

    /** A game that answered "where are you?" */
    public static final class Found {
        public final String host;
        public final int port, version;
        public final byte[] saveId;

        Found(String host, int port, int version, byte[] saveId) {
            this.host = host;
            this.port = port;
            this.version = version;
            this.saveId = saveId;
        }
    }

    /** Broadcast "where are you?" on the local network; every game that answers within timeoutMs */
    public static List<Found> discover(int timeoutMs) {
        List<Found> found = new ArrayList<>();
        try (DatagramSocket u = new DatagramSocket()) {
            u.setBroadcast(true);
            u.setSoTimeout(250);
            byte[] q = "MOEPHONE?".getBytes(StandardCharsets.US_ASCII);
            long end = System.currentTimeMillis() + timeoutMs;
            int sent = 0;
            while (System.currentTimeMillis() < end) {
                if (sent < 3) {
                    u.send(new DatagramPacket(q, q.length, InetAddress.getByName("255.255.255.255"), UDP_PORT));
                    sent++;
                }
                byte[] buf = new byte[256];
                DatagramPacket p = new DatagramPacket(buf, buf.length);
                try {
                    u.receive(p);
                } catch (SocketTimeoutException e) {
                    continue;
                }
                String a = new String(p.getData(), 0, p.getLength(), StandardCharsets.US_ASCII);
                if (!a.startsWith("MOEPHONE!")) {
                    continue;
                }
                String[] f = a.substring(9).trim().split(":");
                if (f.length < 3) {
                    continue;
                }
                String host = p.getAddress().getHostAddress();
                boolean dup = false;
                for (Found x : found) {
                    dup |= x.host.equals(host) && Hex.of(x.saveId).equals(f[0]);
                }
                if (!dup) {
                    try {
                        found.add(new Found(host, Integer.parseInt(f[1]), Integer.parseInt(f[2]), Hex.bytes(f[0])));
                    } catch (NumberFormatException ignored) {
                    }
                }
            }
        } catch (IOException ignored) {
        }
        return found;
    }
}
