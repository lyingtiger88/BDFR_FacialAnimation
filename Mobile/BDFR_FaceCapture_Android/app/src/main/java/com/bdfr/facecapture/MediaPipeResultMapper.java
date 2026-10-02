package com.bdfr.facecapture;

import com.google.mediapipe.tasks.components.containers.Category;
import com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarkerResult;

import java.util.List;
import java.util.Optional;

public final class MediaPipeResultMapper {
    private MediaPipeResultMapper() {}

    public static BdfrFacialFrame toBdfrFrame(
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

        float strongest = 0.0f;
        int mappedCount = 0;

        for (Category category : categories) {
            String name = category.categoryName();
            float score = clamp01(category.score());

            if (name == null || name.isEmpty() || name.equals("_neutral")) {
                continue;
            }

            frame.curves.put(name, score);
            strongest = Math.max(strongest, score);
            mappedCount++;
        }

        // Blendshape scores are activations, not detector confidence.
        // Until face-presence confidence is exposed separately here,
        // treat a populated result as usable and retain a conservative score.
        frame.confidence = mappedCount > 0
                ? Math.max(0.5f, strongest)
                : 0.0f;

        return frame;
    }

    private static float clamp01(float value) {
        return Math.max(0.0f, Math.min(1.0f, value));
    }
}
