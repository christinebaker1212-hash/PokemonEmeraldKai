package com.moemon.companion.core;

import org.json.JSONException;
import org.json.JSONObject;

/** A paired save: its ID, this phone's ID and key for it, where the game was last found */
public final class SavedLink {
    public final byte[] saveId;
    public final byte[] phoneId;
    public final byte[] key;
    public String name;          // the player's name (game text, hex) from the last snapshot
    public String host;
    public int port;

    public SavedLink(byte[] saveId, byte[] phoneId, byte[] key, String host, int port) {
        this.saveId = saveId;
        this.phoneId = phoneId;
        this.key = key;
        this.host = host;
        this.port = port;
        this.name = "";
    }

    public String id() {
        return Hex.of(saveId);
    }

    public JSONObject toJson() throws JSONException {
        JSONObject o = new JSONObject();
        o.put("save", Hex.of(saveId));
        o.put("phone", Hex.of(phoneId));
        o.put("key", Hex.of(key));
        o.put("name", name);
        o.put("host", host);
        o.put("port", port);
        return o;
    }

    public static SavedLink fromJson(JSONObject o) {
        SavedLink s = new SavedLink(Hex.bytes(o.optString("save")), Hex.bytes(o.optString("phone")),
                Hex.bytes(o.optString("key")), o.optString("host", ""), o.optInt("port", 47610));
        s.name = o.optString("name", "");
        return s.saveId != null && s.phoneId != null && s.key != null ? s : null;
    }
}
