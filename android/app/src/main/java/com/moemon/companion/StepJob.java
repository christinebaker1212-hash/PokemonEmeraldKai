package com.moemon.companion;

import android.app.job.JobInfo;
import android.app.job.JobParameters;
import android.app.job.JobScheduler;
import android.app.job.JobService;
import android.content.ComponentName;
import android.content.Context;

/**
 * Every so often while the app is closed (Android decides: about every 15 minutes at best): one step counter reading
 * for the walk, the notifications that are due (Notifier: a few a day at most), the home-screen widget.
 */
public final class StepJob extends JobService {
    private static final int ID = 71;

    public static void schedule(Context c) {
        JobScheduler js = (JobScheduler) c.getSystemService(Context.JOB_SCHEDULER_SERVICE);
        if (js == null || js.getPendingJob(ID) != null) {
            return;
        }
        JobInfo.Builder b = new JobInfo.Builder(ID, new ComponentName(c, StepJob.class))
                .setPeriodic(15 * 60 * 1000L)
                .setPersisted(true);
        try {
            js.schedule(b.build());
        } catch (RuntimeException ignored) {
            // (no RECEIVE_BOOT_COMPLETED: not persisted)
        }
    }

    @Override
    public boolean onStartJob(final JobParameters params) {
        new Thread(() -> {
            try {
                final Companion app = Companion.get(this);
                final StepCounter sc = new StepCounter(this, app);
                final Object lock = new Object();
                final boolean[] read = {false};
                if (app.walk() != null && sc.start(() -> {
                    synchronized (lock) {
                        read[0] = true;
                        lock.notifyAll();
                    }
                })) {
                    synchronized (lock) {
                        if (!read[0]) {
                            lock.wait(5000);
                        }
                    }
                    sc.stop();
                }
                Notifier.check(this, app);
                Notifier.friends(this, app);
                Notifier.trades(this, app);
                Notifier.cardTurns(this, app);
                WalkWidget.updateAll(this, app);
            } catch (Exception ignored) {
            }
            jobFinished(params, false);
        }, "moemon-steps").start();
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }
}
