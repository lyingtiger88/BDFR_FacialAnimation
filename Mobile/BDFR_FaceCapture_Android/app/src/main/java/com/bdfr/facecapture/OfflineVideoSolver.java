package com.bdfr.facecapture;

import android.content.Context;
import android.graphics.Bitmap;
import android.media.MediaMetadataRetriever;
import android.net.Uri;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class OfflineVideoSolver implements Closeable {
    public interface Listener {
        void onProgress(int solvedFrames, long positionMs, long durationMs);
        void onCompleted(File sessionFile, int solvedFrames);
        void onError(String message);
    }

    private final Context context;
    private final ExecutorService executor =
            Executors.newSingleThreadExecutor();

    public OfflineVideoSolver(Context context) {
        this.context = context.getApplicationContext();
    }

    public void solve(
            Uri uri,
            long frameStepMs,
            Listener listener) {

        final long safeStepMs = Math.max(16L, frameStepMs);

        executor.execute(() -> {
            MediaMetadataRetriever retriever =
                    new MediaMetadataRetriever();
            MediaPipeFaceTracker tracker = null;
            BdfrSessionRecorder recorder = null;

            try {
                retriever.setDataSource(context, uri);

                String durationText =
                        retriever.extractMetadata(
                                MediaMetadataRetriever.METADATA_KEY_DURATION);

                if (durationText == null) {
                    throw new IOException(
                            "Unable to read video duration.");
                }

                long durationMs = Long.parseLong(durationText);

                tracker = new MediaPipeFaceTracker(
                        context,
                        "face_landmarker.task",
                        MediaPipeFaceTracker.Mode.VIDEO);

                recorder = new BdfrSessionRecorder(
                        context,
                        "bdfr-android-offline");

                int solved = 0;

                for (long positionMs = 0;
                     positionMs <= durationMs;
                     positionMs += safeStepMs) {

                    Bitmap bitmap =
                            retriever.getFrameAtTime(
                                    positionMs * 1000L,
                                    MediaMetadataRetriever.OPTION_CLOSEST);

                    if (bitmap == null) {
                        continue;
                    }

                    try {
                        com.google.mediapipe.tasks.vision.facelandmarker.FaceLandmarkerResult result =
                                tracker.detect(bitmap, positionMs);

                        BdfrFacialFrame frame =
                                tracker.toBdfrFrame(
                                        result,
                                        positionMs / 1000.0);

                        if (frame.confidence > 0.0f &&
                                !frame.curves.isEmpty()) {
                            recorder.append(frame);
                            solved++;
                        }
                    } finally {
                        bitmap.recycle();
                    }

                    if (listener != null) {
                        listener.onProgress(
                                solved,
                                positionMs,
                                durationMs);
                    }
                }

                File file = recorder.save(
                        "offline_" + System.currentTimeMillis());

                if (listener != null) {
                    listener.onCompleted(file, solved);
                }

            } catch (Exception exception) {
                if (listener != null) {
                    listener.onError(
                            exception.getMessage() == null
                                    ? exception.toString()
                                    : exception.getMessage());
                }
            } finally {
                if (tracker != null) {
                    tracker.close();
                }
                if (recorder != null) {
                    recorder.close();
                }
                retriever.release();
            }
        });
    }

    @Override
    public void close() {
        executor.shutdownNow();
    }
}
