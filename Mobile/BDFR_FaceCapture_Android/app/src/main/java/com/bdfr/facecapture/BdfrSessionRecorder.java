package com.bdfr.facecapture;

import android.content.Context;

import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public final class BdfrSessionRecorder implements Closeable {
    private final Context context;
    private final String sourceId;
    private final List<byte[]> packets = new ArrayList<>();
    private long sequence = 0;
    private File outputFile;

    public BdfrSessionRecorder(Context context, String sourceId) {
        this.context = context.getApplicationContext();
        this.sourceId = sourceId;
    }

    public synchronized void reset() {
        packets.clear();
        sequence = 0;
        outputFile = null;
    }

    public synchronized void append(BdfrFacialFrame frame) {
        packets.add(
                BdfrPacketCodec.encodePacket(
                        sourceId,
                        sequence++,
                        frame));
    }

    public synchronized int frameCount() {
        return packets.size();
    }

    public synchronized File save(String baseName) throws IOException {
        if (baseName == null || baseName.trim().isEmpty()) {
            throw new IllegalArgumentException("Session base name is empty");
        }

        File directory = new File(context.getFilesDir(), "sessions");
        if (!directory.exists() && !directory.mkdirs()) {
            throw new IOException("Unable to create BDFR session directory");
        }

        outputFile = new File(directory, baseName + ".bdfs");

        try (BufferedOutputStream out =
                     new BufferedOutputStream(new FileOutputStream(outputFile))) {

            writeU32(out, 0x53464442L); // BDFS
            writeU16(out, 1);
            writeU32(out, packets.size());

            for (byte[] packet : packets) {
                writeU32(out, packet.length);
                out.write(packet);
            }
        }

        return outputFile;
    }

    public synchronized File outputFile() {
        return outputFile;
    }

    @Override
    public synchronized void close() {
        packets.clear();
    }

    private static void writeU16(BufferedOutputStream out, int value)
            throws IOException {
        out.write(value & 0xFF);
        out.write((value >>> 8) & 0xFF);
    }

    private static void writeU32(BufferedOutputStream out, long value)
            throws IOException {
        for (int i = 0; i < 4; i++) {
            out.write((int) ((value >>> (i * 8)) & 0xFF));
        }
    }
}
