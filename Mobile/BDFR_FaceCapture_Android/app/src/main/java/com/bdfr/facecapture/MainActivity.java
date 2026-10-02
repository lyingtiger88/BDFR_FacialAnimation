package com.bdfr.facecapture;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.Preview;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.core.content.ContextCompat;

import com.bdfr.facecapture.databinding.ActivityMainBinding;
import com.google.common.util.concurrent.ListenableFuture;

public final class MainActivity extends AppCompatActivity {
    private ActivityMainBinding binding;

    private final ActivityResultLauncher<String> cameraPermission =
            registerForActivityResult(new ActivityResultContracts.RequestPermission(), granted -> {
                if (granted) {
                    startCamera();
                } else {
                    binding.statusText.setText("Camera permission is required for facial capture.");
                }
            });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.recordButton.setOnClickListener(v ->
                binding.statusText.setText("Record pipeline scaffold ready."));
        binding.liveButton.setOnClickListener(v ->
                binding.statusText.setText("Live transport scaffold ready."));
        binding.calibrateButton.setOnClickListener(v ->
                binding.statusText.setText("Calibration workflow scaffold ready."));

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_GRANTED) {
            startCamera();
        } else {
            cameraPermission.launch(Manifest.permission.CAMERA);
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

                CameraSelector selector = new CameraSelector.Builder()
                        .requireLensFacing(CameraSelector.LENS_FACING_FRONT)
                        .build();

                provider.unbindAll();
                provider.bindToLifecycle(this, selector, preview);
                binding.statusText.setText("Front camera ready.");
            } catch (Exception exception) {
                binding.statusText.setText(
                        "Camera initialization failed: " + exception.getMessage());
            }
        }, ContextCompat.getMainExecutor(this));
    }
}
