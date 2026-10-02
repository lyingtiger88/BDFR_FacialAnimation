package com.bdfr.facecapture;

import java.io.Closeable;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicLong;

public final class LiveStreamClient implements Closeable {
    private final DatagramSocket socket;
    private final InetAddress address;
    private final int port;
    private final String sourceId;
    private final AtomicLong sequence = new AtomicLong(0);
    private final ExecutorService executor = Executors.newSingleThreadExecutor();

    public LiveStreamClient(String host, int port, String sourceId) throws IOException {
        if (port <= 0 || port > 65535) {
            throw new IllegalArgumentException("Invalid UDP port");
        }
        this.socket = new DatagramSocket();
        this.address = InetAddress.getByName(host);
        this.port = port;
        this.sourceId = sourceId;
    }

    public void send(BdfrFacialFrame frame) {
        final long next = sequence.getAndIncrement();
        final byte[] bytes = BdfrPacketCodec.encodePacket(sourceId, next, frame);

        executor.execute(() -> {
            try {
                DatagramPacket packet = new DatagramPacket(bytes, bytes.length, address, port);
                socket.send(packet);
            } catch (IOException ignored) {
                // UI/runtime diagnostics layer will own reporting in the next integration slice.
            }
        });
    }

    public long nextSequenceNumber() {
        return sequence.get();
    }

    @Override
    public void close() {
        executor.shutdownNow();
        socket.close();
    }
}
