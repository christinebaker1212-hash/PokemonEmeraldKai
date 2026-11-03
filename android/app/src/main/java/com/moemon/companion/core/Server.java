package com.moemon.companion.core;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

/**
 * The co-op server's mailbox (docs/PHONE_LINK.md "Phase 4"; mp_server.c MAIL_*), straight from the phone: friends'
 * presence records, mail trades, card turns. Records are found by 16-byte keys only friends can work out and sealed
 * with their friend keys: the server only keeps them. Blocking: run off the screen's thread.
 */
public final class Server implements AutoCloseable {
    static final int MP_C_MAIL = 12, MP_S_MAIL = 32, MAIL_PUT = 1, MAIL_ADD = 2, MAIL_GET = 3, MAIL_DEL = 4, MAIL_TAKE = 5,
            MAIL_DENIED = 6;
    /** The mail trades' escrow (mp_net.h ESC_*): ops and the state's bits */
    public static final int ESC_PUT = 7, ESC_TAKE = 8, ESC_CANCEL = 9, ESC_LOOK = 10;
    public static final int HAVE0 = 1, HAVE1 = 2, TAKEN0 = 4, TAKEN1 = 8, CLOSED = 16;
    private final Socket sock;
    private final DataInputStream in;
    private final OutputStream out;

    public Server(String address) throws IOException {
        String host = address;
        int port = 27015, colon = address.lastIndexOf(':');
        if (colon > 0) {
            host = address.substring(0, colon);
            try {
                port = Integer.parseInt(address.substring(colon + 1));
            } catch (NumberFormatException ignored) {
            }
        }
        if (Thread.currentThread() == Link.screenThread) {
            throw new IllegalStateException("the network on the screen's thread");
        }
        sock = new Socket();
        sock.connect(new InetSocketAddress(host, port), 5000);
        sock.setSoTimeout(8000);
        sock.setTcpNoDelay(true);
        in = new DataInputStream(sock.getInputStream());
        out = sock.getOutputStream();
    }

    private void send(byte[] data) throws IOException {
        byte[] f = new byte[3 + data.length];
        int n = data.length + 1;
        f[0] = (byte) n;
        f[1] = (byte) (n >> 8);
        f[2] = (byte) MP_C_MAIL;
        System.arraycopy(data, 0, f, 3, data.length);
        out.write(f);
        out.flush();
    }

    private byte[] answer() throws IOException {
        for (;;) {
            int n = in.readUnsignedByte() | (in.readUnsignedByte() << 8);
            int type = in.readUnsignedByte();
            byte[] d = new byte[Math.max(0, n - 1)];
            in.readFully(d);
            if (type == MP_S_MAIL) {
                return d;
            }
        }
    }

    /** Every entry kept under KEY (empty: none) */
    public List<byte[]> get(byte[] key) throws IOException {
        byte[] q = new byte[17];
        q[0] = MAIL_GET;
        System.arraycopy(key, 0, q, 1, 16);
        send(q);
        byte[] d = answer();
        List<byte[]> out = new ArrayList<>();
        if (d.length < 18 || d[0] != MAIL_GET) {
            return out;
        }
        int count = d[17] & 255, at = 18;
        for (int i = 0; i < count && at + 2 <= d.length; i++) {
            int n = (d[at] & 255) | ((d[at + 1] & 255) << 8);
            at += 2;
            if (at + n > d.length) {
                break;
            }
            byte[] e = new byte[n];
            System.arraycopy(d, at, e, 0, n);
            out.add(e);
            at += n;
        }
        return out;
    }

    /** One entry more under KEY (anyone who knows the key may), kept TTL seconds */
    public boolean add(byte[] key, int ttl, byte[] data) throws IOException {
        byte[] q = new byte[17 + 6 + data.length];
        q[0] = MAIL_ADD;
        System.arraycopy(key, 0, q, 1, 16);
        int at = 17;
        q[at++] = (byte) ttl;
        q[at++] = (byte) (ttl >> 8);
        q[at++] = (byte) (ttl >> 16);
        q[at++] = (byte) (ttl >> 24);
        q[at++] = (byte) data.length;
        q[at++] = (byte) (data.length >> 8);
        System.arraycopy(data, 0, q, at, data.length);
        send(q);
        return answer()[0] == MAIL_ADD;
    }

    /** Take entry I away (the owner's SECRET, 16 bytes) */
    public boolean take(byte[] key, byte[] secret, int i) throws IOException {
        byte[] q = new byte[34];
        q[0] = MAIL_TAKE;
        System.arraycopy(key, 0, q, 1, 16);
        System.arraycopy(secret, 0, q, 17, 16);
        q[33] = (byte) i;
        send(q);
        return answer()[0] == MAIL_TAKE;
    }

    /** An escrow record's state (ESC_LOOK), or -1 when the server refused */
    public int look(byte[] key) throws IOException {
        byte[] q = new byte[17];
        q[0] = (byte) ESC_LOOK;
        System.arraycopy(key, 0, q, 1, 16);
        send(q);
        byte[] d = answer();
        return d.length >= 18 && d[0] == ESC_LOOK ? d[17] & 255 : -1;
    }

    /** Calls a trade off from SIDE (0 offered, 1 accepted) while both deposits aren't in: the state after, -1 refused */
    public int cancel(byte[] key, int side, byte[] secret) throws IOException {
        byte[] q = new byte[34];
        q[0] = (byte) ESC_CANCEL;
        System.arraycopy(key, 0, q, 1, 16);
        q[17] = (byte) side;
        System.arraycopy(secret, 0, q, 18, 16);
        send(q);
        byte[] d = answer();
        return d.length >= 18 && d[0] == ESC_CANCEL ? d[17] & 255 : -1;
    }

    @Override
    public void close() {
        try {
            sock.close();
        } catch (IOException ignored) {
        }
    }
}
