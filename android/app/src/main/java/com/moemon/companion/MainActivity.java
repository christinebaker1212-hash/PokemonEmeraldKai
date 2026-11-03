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
        setContentView(view);
        if (app.saves.isEmpty()) {
            view.showOver(new PairPage(view));
        }
        Updater.check(view, false);
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
