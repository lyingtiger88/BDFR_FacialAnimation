package com.bdfr.facecapture;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

public final class BdfrPacketCodecTest {
    @Test
    public void frameAndPacketUseExpectedMagic() {
        BdfrFacialFrame frame = new BdfrFacialFrame();
        frame.timestampSeconds = 1.25;
        frame.curves.put("jawOpen", 0.5f);

        byte[] frameBytes = BdfrPacketCodec.encodeFrame(frame);
        byte[] packetBytes = BdfrPacketCodec.encodePacket("phone-01", 42, frame);

        assertTrue(frameBytes.length > 8);
        assertTrue(packetBytes.length > frameBytes.length);
        assertEquals(0x52464442, BdfrPacketCodec.readMagic(frameBytes));
        assertEquals(0x50464442, BdfrPacketCodec.readMagic(packetBytes));
    }
}
