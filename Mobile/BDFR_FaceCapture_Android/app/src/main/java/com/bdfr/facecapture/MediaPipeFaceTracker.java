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
public final class MediaPipeFaceTracker implements Closeable {
    public enum Mode {
        IMAGE,
        VIDEO
    }

    private final FaceLandmarker landmarker;
    private final Mode mode;

    public MediaPipeFaceTracker(
            Context context,
            String modelAssetPath,
            Mode mode) {

        this.mode = mode;

        BaseOptions baseOptions = BaseOptions.builder()
                .setModelAssetPath(modelAssetPath)
                .build();

        FaceLandmarker.FaceLandmarkerOptions options =
                FaceLandmarker.FaceLandmarkerOptions.builder()
                        .setBaseOptions(baseOptions)
                        .setRunningMode(
                                mode == Mode.VIDEO
                                        ? RunningMode.VIDEO
                                        : RunningMode.IMAGE)
                        .setNumFaces(1)
                        .setMinFaceDetectionConfidence(0.5f)
                        .setMinFacePresenceConfidence(0.5f)
                        .setMinTrackingConfidence(0.5f)
                        .setOutputFaceBlendshapes(true)
                        .setOutputFacialTransformationMatrixes(true)
                        .build();

        landmarker = FaceLandmarker.createFromOptions(context, options);
    }

    public FaceLandmarkerResult detect(Bitmap bitmap, long timestampMs) {
        MPImage image = new BitmapImageBuilder(bitmap).build();

        if (mode == Mode.VIDEO) {
            return landmarker.detectForVideo(image, timestampMs);
        }

        return landmarker.detect(image);
    }

    public BdfrFacialFrame toBdfrFrame(
            FaceLandmarkerResult result,
            double timestampSeconds) {
        return MediaPipeResultMapper.toBdfrFrame(result, timestampSeconds);
    }

    @Override
    public void close() {
        landmarker.close();
    }

    private static float clamp01(float value) {
        return Math.max(0.0f, Math.min(1.0f, value));
    }
}
