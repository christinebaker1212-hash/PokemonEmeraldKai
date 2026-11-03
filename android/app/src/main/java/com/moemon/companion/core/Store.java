package com.moemon.companion.core;

/** Where the app keeps its files (Android: the app's files folder; tests: a temporary one) */
public interface Store {
    String read(String name);            // null if there's none
    void write(String name, String text);
}
