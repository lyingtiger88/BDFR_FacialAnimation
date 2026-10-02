#include "bdfr/runtime/LiveSessionReceiver.h"
#include "bdfr/runtime/SessionStream.h"

#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

volatile std::sig_atomic_t gRunning = 1;

void onSignal(int) {
    gRunning = 0;
}

struct Options {
    int port = 5000;
    int durationSeconds = 0;
    int pollMs = 100;
    std::string recordPath;
    bool printCurves = true;
};

void usage() {
    std::cout
        << "BDFR Live Receiver\n\n"
        << "Usage:\n"
        << "  bdfr_live_receiver [--port 5000] [--duration 30]\n"
        << "                     [--record take.bdfs] [--no-curves]\n\n"
        << "Ctrl+C stops an unlimited session.\n";
}

bool parseArgs(int argc, char** argv, Options& out) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto needValue = [&](const char* name) -> const char* {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << name << "\n";
                return nullptr;
            }
            return argv[++i];
        };

        try {
            if (arg == "--port") {
                const char* value = needValue("--port");
                if (!value) return false;
                out.port = std::stoi(value);
            } else if (arg == "--duration") {
                const char* value = needValue("--duration");
                if (!value) return false;
                out.durationSeconds = std::stoi(value);
            } else if (arg == "--poll-ms") {
                const char* value = needValue("--poll-ms");
                if (!value) return false;
                out.pollMs = std::stoi(value);
            } else if (arg == "--record") {
                const char* value = needValue("--record");
                if (!value) return false;
                out.recordPath = value;
            } else if (arg == "--no-curves") {
                out.printCurves = false;
            } else if (arg == "--help" || arg == "-h") {
                usage();
                std::exit(EXIT_SUCCESS);
            } else {
                std::cerr << "Unknown argument: " << arg << "\n";
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid argument: " << e.what() << "\n";
            return false;
        }
    }

    return out.port > 0 && out.port <= 65535 &&
           out.durationSeconds >= 0 &&
           out.pollMs >= 1 && out.pollMs <= 5000;
}

double steadySeconds() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(
        clock::now().time_since_epoch()).count();
}

} // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parseArgs(argc, argv, options)) {
        usage();
        return EXIT_FAILURE;
    }

    std::signal(SIGINT, onSignal);
#ifdef SIGTERM
    std::signal(SIGTERM, onSignal);
#endif

    bdfr::runtime::LiveSessionReceiver receiver(0.035);
    std::string error;

    if (!receiver.open(
            static_cast<std::uint16_t>(options.port),
            "0.0.0.0",
            &error)) {
        std::cerr << "Unable to open UDP " << options.port
                  << ": " << error << "\n";
        return 2;
    }

    std::cout
        << "BDFR LIVE RECEIVER READY\n"
        << "UDP port: " << receiver.localPort() << "\n"
        << "Waiting for Android / mocap BDFP packets...\n\n";

    const double start = steadySeconds();
    double lastStats = start;
    std::uint64_t localSequence = 0;
    std::vector<bdfr::mocap::MocapPacket> recorded;

    while (gRunning) {
        const double now = steadySeconds();

        if (options.durationSeconds > 0 &&
            now - start >= options.durationSeconds) {
            break;
        }

        error.clear();
        receiver.poll(options.pollMs, now, &error);

        bdfr::FacialFrame frame;
        while (receiver.popReady(now, frame)) {
            bdfr::mocap::MocapPacket packet;
            packet.sourceId = "desktop-live-record";
            packet.sequenceNumber = localSequence++;
            packet.frame = frame;

            if (!options.recordPath.empty()) {
                recorded.push_back(packet);
            }

            std::cout << std::fixed << std::setprecision(3)
                      << "FRAME t=" << frame.timestampSeconds
                      << " conf=" << frame.confidence
                      << " curves=" << frame.curves.size();

            const auto jaw = frame.curves.find("jawOpen");
            const auto blinkL = frame.curves.find("eyeBlinkLeft");
            const auto smileL = frame.curves.find("mouthSmileLeft");

            if (jaw != frame.curves.end())
                std::cout << " jaw=" << jaw->second;
            if (blinkL != frame.curves.end())
                std::cout << " blinkL=" << blinkL->second;
            if (smileL != frame.curves.end())
                std::cout << " smileL=" << smileL->second;

            std::cout << "\n";

            if (options.printCurves) {
                for (const auto& [name, value] : frame.curves) {
                    std::cout << "  " << name << "=" << value << "\n";
                }
            }
        }

        if (now - lastStats >= 1.0) {
            const auto& s = receiver.stats();
            std::cout
                << "STATS received=" << s.packetsReceived
                << " lost=" << s.packetsLost
                << " outOfOrder=" << s.outOfOrderPackets
                << " buffered=" << s.bufferedFrames
                << " clockOffset=" << std::fixed << std::setprecision(4)
                << s.clockOffsetSeconds << "s\n";
            lastStats = now;
        }
    }

    if (!options.recordPath.empty()) {
        if (!bdfr::runtime::SessionStream::save(
                options.recordPath,
                recorded,
                &error)) {
            std::cerr << "Failed to save recording: "
                      << error << "\n";
            return 3;
        }

        std::cout << "Saved " << recorded.size()
                  << " frames to " << options.recordPath << "\n";
    }

    const auto& stats = receiver.stats();

    std::cout
        << "\nBDFR LIVE SESSION COMPLETE\n"
        << "received=" << stats.packetsReceived << "\n"
        << "lost=" << stats.packetsLost << "\n"
        << "outOfOrder=" << stats.outOfOrderPackets << "\n";

    return stats.packetsReceived > 0
        ? EXIT_SUCCESS
        : 4;
}
