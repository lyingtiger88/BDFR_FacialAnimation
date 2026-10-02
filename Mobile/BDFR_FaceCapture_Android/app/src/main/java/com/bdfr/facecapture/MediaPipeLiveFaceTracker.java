package com.bdfr.facecapture;

import android.content.Context;
import android.graphics.Bitmap;

import com.google.mediapipe.framework.image.BitmapImageBuilder;
import com.google.mediapipe.framework.image.MPImage;
import com.google.mediapipe.tasks.core.BaseOptions;
import com.google.mediapipe.tasks.vision.core.RunningMode;
import com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarker;
import com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarkerResult;

import java.io.Closeable;
import java.util.concurrent.atomic.AtomicLong;

public final class MediaPipeLiveFaceTracker implements Closeable {
    public interface Listener {
        void onFrame(BdfrFacialFrame frame, FaceLandmarkerResult rawResult);
        void onError(String message);
    }

    private final FaceLandmarker landmarker;
    private final Listener listener;
    private final AtomicLong lastTimestampMs = new AtomicLong(-1);

    public MediaPipeLiveFaceTracker(
            Context context,
            String modelAssetPath,
            Listener listener) {

        this.listener = listener;

        BaseOptions baseOptions = BaseOptions.builder()
                .setModelAssetPath(modelAssetPath)
                .build();

        FaceLandmarker.FaceLandmarkerOptions options =
                FaceLandmarker.FaceLandmarkerOptions.builder()
                        .setBaseOptions(baseOptions)
                        .setRunningMode(RunningMode.LIVE_STREAM)
                        .setNumFaces(1)
                        .setMinFaceDetectionConfidence(0.5f)
                        .setMinFacePresenceConfidence(0.5f)
                        .setMinTrackingConfidence(0.5f)
                        .setOutputFaceBlendshapes(true)
                        .setOutputFacialTransformationMatrixes(true)
                        .setResultListener((result, inputImage) -> {
                            BdfrFacialFrame frame =
                                    MediaPipeResultMapper.toBdfrFrame(
                                            result,
                                            result.timestampMs() / 1000.0);
                            if (this.listener != null) {
                                this.listener.onFrame(frame, result);
                            }
                        })
                        .setErrorListener(error -> {
                            if (this.listener != null) {
                                this.listener.onError(
                                        error == null
                                                ? "Unknown MediaPipe error"
                                                : error.getMessage());
                            }
                        })
                        .build();

        landmarker = FaceLandmarker.createFromOptions(context, options);
    }

    public boolean submit(Bitmap bitmap, long timestampMs) {
        long previous = lastTimestampMs.get();
        if (timestampMs <= previous) {
            return false;
        }

        if (!lastTimestampMs.compareAndSet(previous, timestampMs)) {
            long current = lastTimestampMs.get();
            if (timestampMs <= current) {
                return false;
            }
            lastTimestampMs.set(timestampMs);
        }

        MPImage image = new BitmapImageBuilder(bitmap).build();
        landmarker.detectAsync(image, timestampMs);
        return true;
    }

    @Override
    public void close() {
        landmarker.close();
    }
}
