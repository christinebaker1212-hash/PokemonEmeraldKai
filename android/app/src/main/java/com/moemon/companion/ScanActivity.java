package com.moemon.companion;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Gravity;
import android.view.Surface;
import android.view.TextureView;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.common.HybridBinarizer;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.EnumMap;
import java.util.Map;

/** Reads the PHONE LINK screen's QR code with the camera (camera2 + ZXing); returns its text as "qr" */
public class ScanActivity extends Activity {
    private TextureView preview;
    private CameraDevice camera;
    private CameraCaptureSession session;
    private ImageReader reader;
    private HandlerThread thread;
    private Handler handler;
    private final MultiFormatReader zxing = new MultiFormatReader();
    private volatile boolean done;

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        Map<DecodeHintType, Object> hints = new EnumMap<>(DecodeHintType.class);
        hints.put(DecodeHintType.POSSIBLE_FORMATS, Arrays.asList(BarcodeFormat.QR_CODE));
        hints.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        zxing.setHints(hints);
        FrameLayout root = new FrameLayout(this);
        preview = new TextureView(this);
        root.addView(preview);
        TextView hint = new TextView(this);
        hint.setText("Point the camera at the code on the game's PHONE LINK screen.");
        hint.setTextColor(0xFFFFFFFF);
        hint.setBackgroundColor(0x80000000);
        hint.setPadding(24, 24, 24, 24);
        root.addView(hint, new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM));
        setContentView(root);
    }

    @Override
    protected void onResume() {
        super.onResume();
        thread = new HandlerThread("scan");
        thread.start();
        handler = new Handler(thread.getLooper());
        if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, 1);
            return;
        }
        whenReady();
    }

    @Override
    public void onRequestPermissionsResult(int code, String[] perms, int[] results) {
        if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
            whenReady();
        } else {
            finish();
        }
    }

    private void whenReady() {
        if (preview.isAvailable()) {
            open();
        } else {
            preview.setSurfaceTextureListener(new TextureView.SurfaceTextureListener() {
                public void onSurfaceTextureAvailable(SurfaceTexture s, int w, int h) { open(); }
                public void onSurfaceTextureSizeChanged(SurfaceTexture s, int w, int h) {}
                public boolean onSurfaceTextureDestroyed(SurfaceTexture s) { return true; }
                public void onSurfaceTextureUpdated(SurfaceTexture s) {}
            });
        }
    }

    private void open() {
        CameraManager cm = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            String id = null;
            for (String c : cm.getCameraIdList()) {
                Integer facing = cm.getCameraCharacteristics(c).get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    id = c;
                    break;
                }
            }
            if (id == null && cm.getCameraIdList().length > 0) {
                id = cm.getCameraIdList()[0];
            }
            if (id == null) {
                finish();
                return;
            }
            StreamConfigurationMap map = cm.getCameraCharacteristics(id).get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            Size size = new Size(1280, 720);
            if (map != null) {
                // the size nearest 1280x720 (a QR code that big on screen needs no more)
                long best = Long.MAX_VALUE;
                for (Size s : map.getOutputSizes(ImageFormat.YUV_420_888)) {
                    long d = Math.abs((long) s.getWidth() * s.getHeight() - 1280L * 720L);
                    if (d < best) {
                        best = d;
                        size = s;
                    }
                }
            }
            reader = ImageReader.newInstance(size.getWidth(), size.getHeight(), ImageFormat.YUV_420_888, 2);
            reader.setOnImageAvailableListener(this::onImage, handler);
            preview.getSurfaceTexture().setDefaultBufferSize(size.getWidth(), size.getHeight());
            cm.openCamera(id, new CameraDevice.StateCallback() {
                public void onOpened(CameraDevice d) {
                    camera = d;
                    startPreview();
                }

                public void onDisconnected(CameraDevice d) {
                    d.close();
                }

                public void onError(CameraDevice d, int e) {
                    d.close();
                    finish();
                }
            }, handler);
        } catch (SecurityException | android.hardware.camera2.CameraAccessException e) {
            finish();
        }
    }

    @SuppressWarnings("deprecation")
    private void startPreview() {
        try {
            Surface view = new Surface(preview.getSurfaceTexture()), frames = reader.getSurface();
            final CaptureRequest.Builder req = camera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            req.addTarget(view);
            req.addTarget(frames);
            req.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            camera.createCaptureSession(Arrays.asList(view, frames), new CameraCaptureSession.StateCallback() {
                public void onConfigured(CameraCaptureSession s) {
                    session = s;
                    try {
                        s.setRepeatingRequest(req.build(), null, handler);
                    } catch (Exception ignored) {
                    }
                }

                public void onConfigureFailed(CameraCaptureSession s) {
                    finish();
                }
            }, handler);
        } catch (Exception e) {
            finish();
        }
    }

    private void onImage(ImageReader r) {
        Image img = r.acquireLatestImage();
        if (img == null) {
            return;
        }
        try {
            if (done) {
                return;
            }
            // the Y plane is all ZXing needs
            Image.Plane y = img.getPlanes()[0];
            int w = img.getWidth(), h = img.getHeight(), stride = y.getRowStride();
            ByteBuffer buf = y.getBuffer();
            byte[] data = new byte[w * h];
            for (int row = 0; row < h; row++) {
                buf.position(row * stride);
                buf.get(data, row * w, Math.min(w, buf.remaining()));
            }
            PlanarYUVLuminanceSource src = new PlanarYUVLuminanceSource(data, w, h, 0, 0, w, h, false);
            Result res = zxing.decodeWithState(new BinaryBitmap(new HybridBinarizer(src)));
            String text = res.getText();
            if (text != null && text.startsWith("MOEPHONE:")) {
                done = true;
                runOnUiThread(() -> {
                    setResult(RESULT_OK, new Intent().putExtra("qr", text));
                    finish();
                });
            }
        } catch (Exception ignored) {
            // (no code in this frame)
        } finally {
            zxing.reset();
            img.close();
        }
    }

    @Override
    protected void onPause() {
        if (session != null) {
            session.close();
            session = null;
        }
        if (camera != null) {
            camera.close();
            camera = null;
        }
        if (reader != null) {
            reader.close();
            reader = null;
        }
        if (thread != null) {
            thread.quitSafely();
            thread = null;
        }
        super.onPause();
    }
}
