#pragma once

#include "bdfr/core/FacialTypes.h"
#include "bdfr/core/Sequence.h"
#include "bdfr/core/Project.h"
#include "bdfr/core/Session.h"

#include <string>

namespace bdfr {

class JsonCodec {
public:
    static std::string encodeFrame(const FacialFrame& frame);
    static bool decodeFrame(const std::string& json, FacialFrame& frame, std::string* error = nullptr);

    static std::string encodeSequence(const FacialSequence& sequence);
    static bool decodeSequence(const std::string& json, FacialSequence& sequence, std::string* error = nullptr);

    static std::string encodeSession(const Session& session);
    static bool decodeSession(const std::string& json, Session& session, std::string* error = nullptr);

    static std::string encodeProject(const Project& project);
    static bool decodeProject(const std::string& json, Project& project, std::string* error = nullptr);
};

} // namespace bdfr
