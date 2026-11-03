package com.moemon.companion;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.SystemClock;

/**
 * The walk's steps from Android's step counter (TYPE_STEP_COUNTER: the steps since the phone started, counted by the
 * phone itself even while the app is closed, at next to no battery). Read while the app is open and by StepJob every
 * so often; each reading goes to the walk (Walk.reading), which keeps the days apart.
 */
public final class StepCounter implements SensorEventListener {
    private final Companion app;
    private final SensorManager sm;
    private Runnable done;

    public StepCounter(Context c, Companion app) {
        this.app = app;
        this.sm = (SensorManager) c.getSystemService(Context.SENSOR_SERVICE);
    }

    public static boolean allowed(Context c) {
        return Build.VERSION.SDK_INT < 29
                || c.checkSelfPermission(Manifest.permission.ACTIVITY_RECOGNITION) == PackageManager.PERMISSION_GRANTED;
    }

    /** Ask for the step counter (Android asks the player once) */
    public static void ask(GameView v) {
        if (allowed(v.getContext()) || Build.VERSION.SDK_INT < 29 || !(v.getContext() instanceof Activity)) {
            return;
        }
        ((Activity) v.getContext()).requestPermissions(new String[]{Manifest.permission.ACTIVITY_RECOGNITION}, 7);
    }

    public static boolean present(Context c) {
        SensorManager sm = (SensorManager) c.getSystemService(Context.SENSOR_SERVICE);
        return sm != null && sm.getDefaultSensor(Sensor.TYPE_STEP_COUNTER) != null;
    }

    /** Listen (until stop); DONE runs after the first reading (StepJob) */
    public boolean start(Runnable done) {
        this.done = done;
        Sensor s = sm == null ? null : sm.getDefaultSensor(Sensor.TYPE_STEP_COUNTER);
        if (s == null || !allowed(app.context)) {
            return false;
        }
        return sm.registerListener(this, s, SensorManager.SENSOR_DELAY_NORMAL);
    }

    public void stop() {
        if (sm != null) {
            sm.unregisterListener(this);
        }
    }

    @Override
    public void onSensorChanged(SensorEvent e) {
        long now = System.currentTimeMillis();
        // when the phone started (to the 10 seconds: the counter starts again from 0 then)
        long boot = (now - SystemClock.elapsedRealtime()) / 10000 * 10000;
        app.walkReading((long) e.values[0], boot, now);
        if (done != null) {
            Runnable d = done;
            done = null;
            d.run();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }
}
