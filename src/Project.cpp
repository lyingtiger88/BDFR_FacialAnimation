#include "bdfr/core/Project.h"

#include <algorithm>
#include <utility>

namespace bdfr {

bool Project::addActor(ActorProfile actor) {
    if (actor.id.empty() || actor.name.empty() || findActor(actor.id) != nullptr) return false;
    actors.push_back(std::move(actor));
    return true;
}

bool Project::addSession(Session session) {
    if (session.id.empty() || findSession(session.id) != nullptr) return false;
    sessions.push_back(std::move(session));
    return true;
}

ActorProfile* Project::findActor(const std::string& actorId) {
    auto it = std::find_if(actors.begin(), actors.end(),
        [&](const ActorProfile& actor) { return actor.id == actorId; });
    return it == actors.end() ? nullptr : &(*it);
}

const ActorProfile* Project::findActor(const std::string& actorId) const {
    auto it = std::find_if(actors.begin(), actors.end(),
        [&](const ActorProfile& actor) { return actor.id == actorId; });
    return it == actors.end() ? nullptr : &(*it);
}

Session* Project::findSession(const std::string& sessionId) {
    auto it = std::find_if(sessions.begin(), sessions.end(),
        [&](const Session& session) { return session.id == sessionId; });
    return it == sessions.end() ? nullptr : &(*it);
}

const Session* Project::findSession(const std::string& sessionId) const {
    auto it = std::find_if(sessions.begin(), sessions.end(),
        [&](const Session& session) { return session.id == sessionId; });
    return it == sessions.end() ? nullptr : &(*it);
}

} // namespace bdfr
