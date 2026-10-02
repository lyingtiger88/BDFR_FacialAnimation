#include "bdfr/core/Persistence.h"
#include "bdfr/runtime/SessionStream.h"
#include "bdfr/runtime/UdpTransport.h"
#include "bdfr/speech/TextSpeech.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string joinArgs(int argc, char** argv, int start) {
    std::ostringstream out;
    for (int i = start; i < argc; ++i) {
        if (i > start) out << ' ';
        out << argv[i];
    }
    return out.str();
}

int runText(const std::string& text) {
    const auto events = bdfr::speech::TextSpeechPlanner::plan(text);
    std::cout << "events=" << events.size() << "\n";
    std::cout << "start,duration,viseme,token\n";
    for (const auto& event : events) {
        std::cout << std::fixed << std::setprecision(4)
                  << event.startSeconds << ','
                  << event.durationSeconds << ','
                  << static_cast<int>(event.viseme) << ','
                  << event.token << '\n';
    }
    return 0;
}

int runInspect(const std::string& path) {
    bdfr::Project project;
    std::string error;
    if (!bdfr::Persistence::loadProject(path, project, &error)) {
        std::cerr << "Failed to load project: " << error << "\n";
        return 2;
    }

    std::size_t takeCount = 0;
    for (const auto& session : project.sessions) takeCount += session.takes.size();

    std::cout << "Project: " << project.name << "\n"
              << "Schema: " << project.schemaVersion << "\n"
              << "Actors: " << project.actors.size() << "\n"
              << "Sessions: " << project.sessions.size() << "\n"
              << "Takes: " << takeCount << "\n";
    return 0;
}

int runInspectSession(const std::string& path) {
    std::vector<bdfr::mocap::MocapPacket> packets;
    std::string error;

    if (!bdfr::runtime::SessionStream::load(path, packets, &error)) {
        std::cerr << "Failed to load BDFS session: " << error << "\n";
        return 2;
    }

    std::cout << "Packets: " << packets.size() << "\n";

    if (!packets.empty()) {
        const auto& first = packets.front();
        const auto& last = packets.back();

        std::cout << "Source: " << first.sourceId << "\n"
                  << "First sequence: " << first.sequenceNumber << "\n"
                  << "Last sequence: " << last.sequenceNumber << "\n"
                  << "Start: " << first.frame.timestampSeconds << "\n"
                  << "End: " << last.frame.timestampSeconds << "\n"
                  << "First-frame curves: " << first.frame.curves.size() << "\n";
    }

    return 0;
}

int runListen(const std::string& portText, const std::string& timeoutText) {
    const int port = std::stoi(portText);
    const int timeoutMs = std::stoi(timeoutText);

    if (port < 0 || port > 65535 || timeoutMs < 0) {
        std::cerr << "Invalid port or timeout.\n";
        return 2;
    }

    bdfr::runtime::UdpFrameReceiver receiver;
    std::string error;

    if (!receiver.open(static_cast<std::uint16_t>(port), "0.0.0.0", &error)) {
        std::cerr << "Unable to open receiver: " << error << "\n";
        return 2;
    }

    std::cout << "Listening on UDP port " << receiver.localPort()
              << " for " << timeoutMs << " ms...\n";

    bdfr::mocap::MocapPacket packet;
    if (!receiver.receive(packet, timeoutMs, &error)) {
        std::cerr << "No valid BDFR packet received";
        if (!error.empty()) std::cerr << ": " << error;
        std::cerr << "\n";
        return 3;
    }

    std::cout << "Received BDFR facial packet\n"
              << "Source: " << packet.sourceId << "\n"
              << "Sequence: " << packet.sequenceNumber << "\n"
              << "Timestamp: " << packet.frame.timestampSeconds << "\n"
              << "Confidence: " << packet.frame.confidence << "\n"
              << "Curves: " << packet.frame.curves.size() << "\n";

    for (const auto& [name, value] : packet.frame.curves) {
        std::cout << "  " << name << "=" << value << "\n";
    }

    return 0;
}

void printUsage() {
    std::cout
        << "BDFR Facial Animation CLI\n\n"
        << "Usage:\n"
        << "  bdfr_cli text <dialogue>\n"
        << "  bdfr_cli inspect <project.bdfr.json>\n"
        << "  bdfr_cli inspect-session <recording.bdfs>\n"
        << "  bdfr_cli listen <udp-port> [timeout-ms]\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string command = argv[1];

    try {
        if (command == "text" && argc >= 3) {
            return runText(joinArgs(argc, argv, 2));
        }

        if (command == "inspect" && argc == 3) {
            return runInspect(argv[2]);
        }

        if (command == "inspect-session" && argc == 3) {
            return runInspectSession(argv[2]);
        }

        if (command == "listen" && (argc == 3 || argc == 4)) {
            return runListen(argv[2], argc == 4 ? argv[3] : "5000");
        }
    } catch (const std::exception& exception) {
        std::cerr << "Command failed: " << exception.what() << "\n";
        return 2;
    }

    printUsage();
    return 1;
}
