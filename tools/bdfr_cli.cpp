#include "bdfr/core/Persistence.h"
#include "bdfr/speech/TextSpeech.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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

void printUsage() {
    std::cout
        << "BDFR Facial Animation CLI\n\n"
        << "Usage:\n"
        << "  bdfr_cli text <dialogue>\n"
        << "  bdfr_cli inspect <project.bdfr.json>\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string command = argv[1];
    if (command == "text" && argc >= 3) {
        return runText(joinArgs(argc, argv, 2));
    }
    if (command == "inspect" && argc == 3) {
        return runInspect(argv[2]);
    }

    printUsage();
    return 1;
}
