package com.bdfr.facecapture;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.SystemClock;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.content.ContextCompat;

import com.bdfr.facecapture.databinding.ActivityMainBinding;
import com.google.common.util.concurrent.ListenableFuture;

import java.io.IOException;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class MainActivity extends AppCompatActivity
        implements MediaPipeLiveFaceTracker.Listener {

    private ActivityMainBinding binding;
    private final ExecutorService analysisExecutor = Executors.newSingleThreadExecutor();

    private MediaPipeLiveFaceTracker liveTracker;
    private LiveStreamClient liveStreamClient;
    private boolean liveStreaming = false;

    private long solvedFrameCount = 0;
    private long fpsWindowStartMs = 0;
    private double lastFps = 0.0;

    private final ActivityResultLauncher<String[]> videoPicker =
            registerForActivityResult(new ActivityResultContracts.OpenDocument(), uri -> {
                if (uri != null) {
                    binding.statusText.setText("Video selected: " + uri);
                    // The URI is ready for the VIDEO-mode offline provider.
                }
            });

    private final ActivityResultLauncher<String> cameraPermission =
            registerForActivityResult(new ActivityResultContracts.RequestPermission(), granted -> {
                if (granted) {
                    startCamera();
                } else {
                    binding.statusText.setText(
                            "Camera permission is required for facial capture.");
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.recordButton.setOnClickListener(v ->
                binding.statusText.setText(
                        "Solved-frame recording is the next mobile integration slice."));

        binding.liveButton.setOnClickListener(v -> toggleLiveStreaming());

        binding.calibrateButton.setOnClickListener(v ->
                binding.statusText.setText(
                        "Calibration workflow ready for actor-pose capture."));

        binding.importButton.setOnClickListener(v ->
                videoPicker.launch(new String[] {"video/*"}));

        initializeLiveTracker();

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        } else {
            cameraPermission.launch(Manifest.permission.CAMERA);
        }
    }

    private void initializeLiveTracker() {
        try {
            liveTracker = new MediaPipeLiveFaceTracker(
                    this,
                    "face_landmarker.task",
                    this);
            binding.statusText.setText("MediaPipe Face Landmarker ready.");
        } catch (Exception exception) {
            liveTracker = null;
            binding.statusText.setText(
                    "Face model unavailable: " + exception.getMessage());
        }
    }

    private void startCamera() {
        final ListenableFuture<ProcessCameraProvider> providerFuture =
                ProcessCameraProvider.getInstance(this);

        providerFuture.addListener(() -> {
            try {
                ProcessCameraProvider provider = providerFuture.get();

                Preview preview = new Preview.Builder().build();
                preview.setSurfaceProvider(binding.previewView.getSurfaceProvider());

                ImageAnalysis analysis = new ImageAnalysis.Builder()
                        .setBackpressureStrategy(
                                ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                        .setOutputImageFormat(
                                ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
                        .setOutputImageRotationEnabled(true)
                        .build();

                analysis.setAnalyzer(analysisExecutor, this::analyzeFrame);

                CameraSelector selector = new CameraSelector.Builder()
                        .requireLensFacing(CameraSelector.LENS_FACING_FRONT)
                        .build();

                provider.unbindAll();
                provider.bindToLifecycle(
                        this,
                        selector,
                        preview,
                        analysis);

                updateStatus("Front camera + live analysis ready.");
            } catch (Exception exception) {
                updateStatus(
                        "Camera initialization failed: " + exception.getMessage());
            }
        }, ContextCompat.getMainExecutor(this));
    }

    private void analyzeFrame(@NonNull ImageProxy image) {
        try {
            if (liveTracker == null) {
                return;
            }

            Bitmap bitmap = image.toBitmap();
            long timestampMs = image.getImageInfo().getTimestamp() / 1_000_000L;

            liveTracker.submit(bitmap, timestampMs);
        } catch (Exception exception) {
            updateStatus("Frame analysis failed: " + exception.getMessage());
        } finally {
            image.close();
        }
    }

    private void toggleLiveStreaming() {
        if (liveStreaming) {
            stopLiveStreaming();
            return;
        }

        String host = binding.hostInput.getText().toString().trim();
        String portText = binding.portInput.getText().toString().trim();

        if (host.isEmpty()) {
            updateStatus("Enter the PC IP address or hostname first.");
            return;
        }

        final int port;
        try {
            port = Integer.parseInt(portText);
        } catch (NumberFormatException exception) {
            updateStatus("Invalid UDP port.");
            return;
        }

        try {
            liveStreamClient = new LiveStreamClient(
                    host,
                    port,
                    "bdfr-android");
            liveStreaming = true;
            binding.liveButton.setText("Stop Live");
            updateStatus("Live streaming to " + host + ":" + port);
        } catch (IOException | IllegalArgumentException exception) {
            updateStatus("Unable to start live stream: " + exception.getMessage());
        }
    }

    private void stopLiveStreaming() {
        liveStreaming = false;

        if (liveStreamClient != null) {
            liveStreamClient.close();
            liveStreamClient = null;
        }

        binding.liveButton.setText("Live");
        updateStatus("Live streaming stopped.");
    }

    @Override
    public void onFrame(
            BdfrFacialFrame frame,
            com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarkerResult rawResult) {

        solvedFrameCount++;

        long nowMs = SystemClock.elapsedRealtime();
        if (fpsWindowStartMs == 0) {
            fpsWindowStartMs = nowMs;
        }

        long elapsed = nowMs - fpsWindowStartMs;
        if (elapsed >= 1000) {
            lastFps = solvedFrameCount * 1000.0 / elapsed;
            solvedFrameCount = 0;
            fpsWindowStartMs = nowMs;
        }

        if (liveStreaming && liveStreamClient != null && frame.confidence > 0.0f) {
            liveStreamClient.send(frame);
        }

        updateStatus(String.format(
                Locale.US,
                "Tracking | curves=%d | confidence=%.2f | FPS=%.1f | live=%s",
                frame.curves.size(),
                frame.confidence,
                lastFps,
                liveStreaming ? "ON" : "OFF"));
    }

    @Override
    public void onError(String message) {
        updateStatus("MediaPipe error: " + message);
    }

    private void updateStatus(String message) {
        runOnUiThread(() -> binding.statusText.setText(message));
    }

    @Override
    protected void onDestroy() {
        stopLiveStreaming();

        if (liveTracker != null) {
            liveTracker.close();
            liveTracker = null;
        }

        analysisExecutor.shutdownNow();
        super.onDestroy();
    }
}
