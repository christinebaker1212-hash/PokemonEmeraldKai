package com.moemon.companion.core;

/** The protocol core's crypto (the game's own C: Native on Android, a host build in the desktop tests) */
public interface Crypto {
    byte[] random(int n);
    byte[] qrSecret(byte[] raw16);
    byte[] codeSecret(String code);
    byte[] pairProof(byte[] secret, byte[] phoneId, byte[] saveId);
    byte[] sessionKey(byte[] key, byte[] phoneNonce, byte[] gameNonce);
    byte[] seal(byte[] key, int seq, byte[] msg);
    byte[] open(byte[] key, byte[] sealed);
}
