package com.moemon.companion;

// The game's phone link protocol core (src/mp/phone_proto.c), built into the app: the same bytes as the game's
final class Native {
    static { System.loadLibrary("moemonlink"); }

    static native byte[] random(int n);
    static native byte[] qrSecret(byte[] raw16);
    static native byte[] codeSecret(String code);
    static native byte[] pairProof(byte[] secret, byte[] phoneId, byte[] saveId);
    static native byte[] sessionKey(byte[] key, byte[] phoneNonce, byte[] gameNonce);
    static native byte[] seal(byte[] key, int seq, byte[] msg);
    static native byte[] open(byte[] key, byte[] sealed);   // null if forged; else seq (4 bytes) + message
    static native byte[] derive(byte[] key32, String word);
    static native int linkVersion();

    private Native() {}
}
