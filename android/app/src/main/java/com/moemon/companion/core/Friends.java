package com.moemon.companion.core;

import org.json.JSONArray;
import org.json.JSONObject;

import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Friends' presence from the co-op server (docs/COMPANION_APP.md "Friends list"; the game's mp_phone_friends.c): each
 * friend's record is under PhoneProto_Derive(their friend key, "presence"), sealed with that key; the game's snapshot
 * gives the phone its friends' keys. Android-free (the desktop test reads them too).
 */
public final class Friends {
    /** Every friend's latest record (by save ID in hex), from the server at ADDRESS; friends without one are left out */
    public static Map<String, JSONObject> fetch(Crypto crypto, String address, JSONArray list) throws java.io.IOException {
        Map<String, JSONObject> out = new HashMap<>();
        if (list == null || list.length() == 0 || address == null || address.isEmpty()) {
            return out;
        }
        try (Server s = new Server(address)) {
            for (int i = 0; i < list.length(); i++) {
                JSONObject f = list.optJSONObject(i);
                byte[] key = f == null ? null : Hex.bytes(f.optString("key"));
                if (key == null || key.length != 32) {
                    continue;
                }
                List<byte[]> entries = s.get(crypto.derive(key, "presence"));
                if (entries.isEmpty()) {
                    continue;
                }
                byte[] opened = crypto.open(key, entries.get(entries.size() - 1));
                if (opened == null || opened.length < 5) {
                    continue;
                }
                try {
                    out.put(f.optString("id"), new JSONObject(new String(opened, 4, opened.length - 4, StandardCharsets.UTF_8)));
                } catch (org.json.JSONException ignored) {
                }
            }
        }
        return out;
    }

    /** Playing now: a record from the last few minutes (the game puts one up every minute while it's online) */
    public static boolean online(JSONObject presence, long nowSeconds) {
        return presence != null && nowSeconds - presence.optLong("t") < 300;
    }
}
