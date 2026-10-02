package com.bdfr.facecapture;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import java.io.ByteArrayOutputStream;

public final class BdfrSessionFormatTest {
    @Test
    public void packetMagicMatchesDesktopProtocol() {
        BdfrFacialFrame frame = new BdfrFacialFrame();
        frame.timestampSeconds = 1.0;
        frame.curves.put("jawOpen", 0.4f);

        byte[] packet = BdfrPacketCodec.encodePacket("android-test", 7, frame);

        assertTrue(packet.length > 8);
        assertEquals(0x50464442, BdfrPacketCodec.readMagic(packet));
    }
}
