package com.bdfr.facecapture;

import android.content.Context;

import com.google.mediapipe.tasks.core.BaseOptions;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;

public final class FaceModelManager {
    public interface Listener {
        void onProgress(long downloadedBytes, long totalBytes);
        void onReady(File modelFile);
        void onError(String message);
    }

    public static final String FILE_NAME = "face_landmarker.task";

    public static final String OFFICIAL_MODEL_URL =
            "https://storage.googleapis.com/mediapipe-models/" +
            "face_landmarker/face_landmarker/float16/latest/" +
            "face_landmarker.task";

    private final Context context;

    public FaceModelManager(Context context) {
        this.context = context.getApplicationContext();
    }

    public File modelFile() {
        File directory = new File(context.getFilesDir(), "models");
        return new File(directory, FILE_NAME);
    }

    public boolean hasDownloadedModel() {
        File file = modelFile();
        return file.isFile() && file.length() > 1024;
    }

    public boolean hasBundledAsset() {
        try (InputStream ignored =
                     context.getAssets().open(FILE_NAME)) {
            return true;
        } catch (IOException exception) {
            return false;
        }
    }

    public boolean isAvailable() {
        return hasDownloadedModel() || hasBundledAsset();
    }

    public BaseOptions createBaseOptions() throws IOException {
        if (hasDownloadedModel()) {
            File file = modelFile();

            try (FileChannel channel =
                         FileChannel.open(
                                 file.toPath(),
                                 java.nio.file.StandardOpenOption.READ)) {

                MappedByteBuffer mapped =
                        channel.map(
                                FileChannel.MapMode.READ_ONLY,
                                0,
                                channel.size());

                return BaseOptions.builder()
                        .setModelAssetBuffer(mapped)
                        .build();
            }
        }

        if (hasBundledAsset()) {
            return BaseOptions.builder()
                    .setModelAssetPath(FILE_NAME)
                    .build();
        }

        throw new IOException(
                "Face Landmarker model is not installed.");
    }

    public void download(Listener listener) {
        File target = modelFile();
        File directory = target.getParentFile();

        if (directory == null) {
            if (listener != null) {
                listener.onError("Invalid model directory.");
            }
            return;
        }

        if (!directory.exists() && !directory.mkdirs()) {
            if (listener != null) {
                listener.onError("Unable to create model directory.");
            }
            return;
        }

        File temporary =
                new File(directory, FILE_NAME + ".download");

        HttpURLConnection connection = null;

        try {
            URL url = new URL(OFFICIAL_MODEL_URL);

            connection =
                    (HttpURLConnection) url.openConnection();

            connection.setConnectTimeout(15000);
            connection.setReadTimeout(30000);
            connection.setInstanceFollowRedirects(true);
            connection.setRequestProperty(
                    "User-Agent",
                    "BDFR-FaceCapture/0.1");

            int response = connection.getResponseCode();

            if (response < 200 || response >= 300) {
                throw new IOException(
                        "Model download HTTP " + response);
            }

            long total = connection.getContentLengthLong();
            long downloaded = 0;

            try (BufferedInputStream input =
                         new BufferedInputStream(
                                 connection.getInputStream());
                 FileOutputStream output =
                         new FileOutputStream(temporary)) {

                byte[] buffer = new byte[64 * 1024];
                int read;

                while ((read = input.read(buffer)) >= 0) {
                    if (read == 0) {
                        continue;
                    }

                    output.write(buffer, 0, read);
                    downloaded += read;

                    if (listener != null) {
                        listener.onProgress(
                                downloaded,
                                total);
                    }
                }

                output.getFD().sync();
            }

            if (temporary.length() <= 1024) {
                throw new IOException(
                        "Downloaded model is unexpectedly small.");
            }

            if (target.exists() && !target.delete()) {
                throw new IOException(
                        "Unable to replace existing model.");
            }

            if (!temporary.renameTo(target)) {
                throw new IOException(
                        "Unable to finalize model download.");
            }

            if (listener != null) {
                listener.onReady(target);
            }

        } catch (Exception exception) {
            if (temporary.exists()) {
                //noinspection ResultOfMethodCallIgnored
                temporary.delete();
            }

            if (listener != null) {
                listener.onError(
                        exception.getMessage() == null
                                ? exception.toString()
                                : exception.getMessage());
            }
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
}
