package com.moemon.companion;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

/** The app: one GameView. Pairing when there's no save yet; the link runs while the app is open. */
public class MainActivity extends Activity {
    private GameView view;

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        Companion app;
        try {
            app = Companion.get(this);
        } catch (Exception e) {
            TextView t = new TextView(this);
            t.setText("The app's art is missing (android/build_assets.py): " + e);
            setContentView(t);
            return;
        }
        view = new GameView(this, app);
        android.widget.FrameLayout root = new android.widget.FrameLayout(this);
        root.setBackgroundColor(0xFF283848);
        root.addView(view);
        root.setOnApplyWindowInsetsListener((v, insets) -> {
            int il, it, ir, ib;
            if (android.os.Build.VERSION.SDK_INT >= 30) {
                android.graphics.Insets i = insets.getInsets(android.view.WindowInsets.Type.systemBars()
                        | android.view.WindowInsets.Type.displayCutout());
                il = i.left; it = i.top; ir = i.right; ib = i.bottom;
            } else {
                il = insets.getSystemWindowInsetLeft(); it = insets.getSystemWindowInsetTop();
                ir = insets.getSystemWindowInsetRight(); ib = insets.getSystemWindowInsetBottom();
            }
            v.setPadding(il, it, ir, ib);
            return insets;
        });
        setContentView(root);
        if (app.saves.isEmpty()) {
            view.showOver(new PairPage(view));
        }
        CrashReport crash = CrashReport.take(this, app);
        if (crash != null) {
            android.util.Log.e("Moemon", crash.full);
            Menu m = Menu.message(view, "The app closed last time: " + crash.summary
                    + "\nCOPY puts the whole report in the clipboard, to send to the game's team.");
            m.setRows();
            m.add("COPY", () -> {
                crash.copy(this);
                view.message("Copied. Paste it in your message.");
            });
            m.add("OK", null);
            view.showMenu(m);
        } else {
            Updater.check(view, false);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (view != null) {
            view.app.resume();
        }
    }

    @Override
    protected void onPause() {
        if (view != null) {
            view.app.pause();
        }
        super.onPause();
    }

    @Override
    public void onBackPressed() {
        if (view == null || !view.back()) {
            super.onBackPressed();
        }
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request == 1 && result == RESULT_OK && data != null && view != null) {
            String text = data.getStringExtra("qr");
            Page over = view.overPage();
            PairPage p = over instanceof PairPage ? (PairPage) over : new PairPage(view);
            view.showOver(p);
            p.start(text, null);
        }
    }
}
