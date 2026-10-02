package com.bdfr.facecapture;

import android.content.Context;
import android.graphics.Bitmap;

import com.google.mediapipe.framework.image.BitmapImageBuilder;
import com.google.mediapipe.framework.image.MPImage;
import com.google.mediapipe.tasks.components.containers.Category;
import com.google.mediapipe.tasks.core.BaseOptions;
import com.google.mediapipe.tasks.vision.core.RunningMode;
import com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarker;
import com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarkerResult;

import java.io.Closeable;
import java.util.List;
import java.util.Optional;

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

        BdfrFacialFrame frame = new BdfrFacialFrame();
        frame.timestampSeconds = timestampSeconds;

        Optional<List<List<Category>>> blendshapeGroups =
                result.faceBlendshapes();

        if (blendshapeGroups.isEmpty() || blendshapeGroups.get().isEmpty()) {
            frame.confidence = 0.0f;
            return frame;
        }

        List<Category> categories = blendshapeGroups.get().get(0);
        float scoreSum = 0.0f;
        int scoreCount = 0;

        for (Category category : categories) {
            String name = category.categoryName();
            float score = clamp01(category.score());

            if (name == null || name.isEmpty() || name.equals("_neutral")) {
                continue;
            }

            frame.curves.put(name, score);
            scoreSum += score;
            scoreCount++;
        }

        frame.confidence = scoreCount == 0
                ? 0.0f
                : clamp01(scoreSum / scoreCount);

        return frame;
    }

    @Override
    public void close() {
        landmarker.close();
    }

    private static float clamp01(float value) {
        return Math.max(0.0f, Math.min(1.0f, value));
    }
}
