#pragma once

#include "bdfr/capture/FaceObservation.h"

#include <cstdint>
#include <vector>

namespace bdfr::capture {

struct VideoFrame {
    std::vector<std::uint8_t> pixels;
    int width = 0;
    int height = 0;
    int strideBytes = 0;
    PixelFormat format = PixelFormat::RGB24;
    double timestampSeconds = 0.0;

    ImageView view() const noexcept {
        ImageView out;
        out.data = pixels.empty() ? nullptr : pixels.data();
        out.sizeBytes = pixels.size();
        out.width = width;
        out.height = height;
        out.strideBytes = strideBytes;
        out.format = format;
        out.timestampSeconds = timestampSeconds;
        return out;
    }
};

class IVideoSource {
public:
    virtual ~IVideoSource() = default;
    virtual bool next(VideoFrame& frame) = 0;
    virtual void reset() = 0;
};

} // namespace bdfr::capture
