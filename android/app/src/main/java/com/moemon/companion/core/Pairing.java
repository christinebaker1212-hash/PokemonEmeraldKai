package com.moemon.companion.core;

/** What the PHONE LINK screen's QR code says: MOEPHONE:VERSION:ADDRESS:PORT:SAVE ID:SECRET:LETTERS */
public final class Pairing {
    public final int version;
    public final String host;
    public final int port;
    public final byte[] saveId;
    public final byte[] secretRaw;
    public final String code;

    private Pairing(int version, String host, int port, byte[] saveId, byte[] secretRaw, String code) {
        this.version = version;
        this.host = host;
        this.port = port;
        this.saveId = saveId;
        this.secretRaw = secretRaw;
        this.code = code;
    }

    /** null if it isn't the game's code */
    public static Pairing parse(String text) {
        if (text == null) {
            return null;
        }
        String[] f = text.trim().split(":");
        if (f.length != 7 || !f[0].equals("MOEPHONE")) {
            return null;
        }
        try {
            byte[] save = Hex.bytes(f[4]), secret = Hex.bytes(f[5]);
            if (save == null || save.length != 8 || secret == null || secret.length != 16) {
                return null;
            }
            return new Pairing(Integer.parseInt(f[1]), f[2], Integer.parseInt(f[3]), save, secret, f[6]);
        } catch (NumberFormatException e) {
            return null;
        }
    }
}
