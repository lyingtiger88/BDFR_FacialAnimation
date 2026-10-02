#pragma once

#include "bdfr/core/Session.h"

#include <cstdint>
#include <string>
#include <vector>

namespace bdfr {

struct ActorProfile {
    std::string id;
    std::string name;
    std::string notes;
};

struct Project {
    std::uint32_t schemaVersion = 1;
    std::string name;
    std::vector<ActorProfile> actors;
    std::vector<Session> sessions;

    bool addActor(ActorProfile actor);
    bool addSession(Session session);
    ActorProfile* findActor(const std::string& actorId);
    const ActorProfile* findActor(const std::string& actorId) const;
    Session* findSession(const std::string& sessionId);
    const Session* findSession(const std::string& sessionId) const;
};

} // namespace bdfr
