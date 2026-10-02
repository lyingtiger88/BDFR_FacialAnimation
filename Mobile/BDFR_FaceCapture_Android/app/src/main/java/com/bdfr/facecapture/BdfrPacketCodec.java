package com.bdfr.facecapture;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class BdfrPacketCodec {
    private static final int FRAME_MAGIC = 0x52464442; // BDFR
    private static final int PACKET_MAGIC = 0x50464442; // BDFP
    private static final short VERSION = 1;

    private BdfrPacketCodec() {}

    public static byte[] encodeFrame(BdfrFacialFrame frame) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        writeU32(out, FRAME_MAGIC);
        writeU16(out, VERSION & 0xFFFF);
        writeU32(out, frame.schemaVersion);
        writeDouble(out, frame.timestampSeconds);
        writeFloat(out, frame.confidence);
        writeFloat(out, frame.headPitch);
        writeFloat(out, frame.headYaw);
        writeFloat(out, frame.headRoll);
        writeFloat(out, frame.gazeX);
        writeFloat(out, frame.gazeY);
        writeFloat(out, frame.gazeConfidence);

        List<String> names = new ArrayList<>(frame.curves.keySet());
        Collections.sort(names);
        writeU32(out, names.size());

        for (String name : names) {
            byte[] id = name.getBytes(StandardCharsets.UTF_8);
            if (id.length > 0xFFFF) {
                throw new IllegalArgumentException("Curve name too long");
            }
            writeU16(out, id.length);
            out.writeBytes(id);
            writeFloat(out, frame.curves.get(name));
        }

        return out.toByteArray();
    }

    public static byte[] encodePacket(String sourceId, long sequenceNumber, BdfrFacialFrame frame) {
        byte[] source = sourceId.getBytes(StandardCharsets.UTF_8);
        if (source.length > 0xFFFF) {
            throw new IllegalArgumentException("Source id too long");
        }

        byte[] frameBytes = encodeFrame(frame);
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        writeU32(out, PACKET_MAGIC);
        writeU16(out, VERSION & 0xFFFF);
        writeU64(out, sequenceNumber);
        writeU16(out, source.length);
        out.writeBytes(source);
        writeU32(out, frameBytes.length);
        out.writeBytes(frameBytes);
        return out.toByteArray();
    }

    private static void writeU16(ByteArrayOutputStream out, int value) {
        out.write(value & 0xFF);
        out.write((value >>> 8) & 0xFF);
    }

    private static void writeU32(ByteArrayOutputStream out, long value) {
        for (int i = 0; i < 4; i++) {
            out.write((int) ((value >>> (i * 8)) & 0xFF));
        }
    }

    private static void writeU64(ByteArrayOutputStream out, long value) {
        for (int i = 0; i < 8; i++) {
            out.write((int) ((value >>> (i * 8)) & 0xFF));
        }
    }

    private static void writeFloat(ByteArrayOutputStream out, float value) {
        writeU32(out, Float.floatToIntBits(value) & 0xFFFFFFFFL);
    }

    private static void writeDouble(ByteArrayOutputStream out, double value) {
        writeU64(out, Double.doubleToLongBits(value));
    }

    public static int readMagic(byte[] bytes) {
        if (bytes.length < 4) return 0;
        return ByteBuffer.wrap(bytes, 0, 4)
                .order(ByteOrder.LITTLE_ENDIAN)
                .getInt();
    }
}
