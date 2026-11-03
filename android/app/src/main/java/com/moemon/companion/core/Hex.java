package com.moemon.companion.core;

/** Hex, as the link writes bytes (upper case) */
public final class Hex {
    private static final char[] DIGITS = "0123456789ABCDEF".toCharArray();

    public static String of(byte[] b) {
        StringBuilder s = new StringBuilder(b.length * 2);
        for (byte x : b) {
            s.append(DIGITS[(x >> 4) & 15]).append(DIGITS[x & 15]);
        }
        return s.toString();
    }

    public static byte[] bytes(String s) {
        if (s == null || (s.length() & 1) != 0) {
            return null;
        }
        byte[] b = new byte[s.length() / 2];
        for (int i = 0; i < b.length; i++) {
            int hi = Character.digit(s.charAt(2 * i), 16), lo = Character.digit(s.charAt(2 * i + 1), 16);
            if (hi < 0 || lo < 0) {
                return null;
            }
            b[i] = (byte) (hi * 16 + lo);
        }
        return b;
    }

    private Hex() {}
}
