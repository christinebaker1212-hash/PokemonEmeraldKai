package com.moemon.companion;

import com.moemon.companion.core.Crypto;

/** The core's crypto through the game's own C (Native): the same on the phone and in the desktop test */
public final class NativeCrypto implements Crypto {
    @Override public byte[] random(int n) { return Native.random(n); }
    @Override public byte[] qrSecret(byte[] raw16) { return Native.qrSecret(raw16); }
    @Override public byte[] codeSecret(String code) { return Native.codeSecret(code); }
    @Override public byte[] pairProof(byte[] secret, byte[] phoneId, byte[] saveId) { return Native.pairProof(secret, phoneId, saveId); }
    @Override public byte[] sessionKey(byte[] key, byte[] pn, byte[] gn) { return Native.sessionKey(key, pn, gn); }
    @Override public byte[] seal(byte[] key, int seq, byte[] msg) { return Native.seal(key, seq, msg); }
    @Override public byte[] open(byte[] key, byte[] sealed) { return Native.open(key, sealed); }
    @Override public byte[] derive(byte[] key32, String word) { return Native.derive(key32, word); }

    public static int linkVersion() { return Native.linkVersion(); }
}
