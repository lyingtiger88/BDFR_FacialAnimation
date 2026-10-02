package com.bdfr.facecapture;

import java.util.LinkedHashMap;
import java.util.Map;

public final class BdfrFacialFrame {
    public int schemaVersion = 1;
    public double timestampSeconds = 0.0;
    public float confidence = 1.0f;

    public float headPitch = 0.0f;
    public float headYaw = 0.0f;
    public float headRoll = 0.0f;

    public float gazeX = 0.0f;
    public float gazeY = 0.0f;
    public float gazeConfidence = 0.0f;

    public final Map<String, Float> curves = new LinkedHashMap<>();
}
