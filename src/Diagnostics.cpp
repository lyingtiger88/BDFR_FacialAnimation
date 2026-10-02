#include "bdfr/core/Diagnostics.h"

#include <iostream>
#include <mutex>
#include <utility>

namespace bdfr {

namespace {
std::mutex gLoggerMutex;
LogSink gSink;

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}
}

void Logger::setSink(LogSink sink) {
    std::lock_guard<std::mutex> lock(gLoggerMutex);
    gSink = std::move(sink);
}

void Logger::write(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(gLoggerMutex);
    if (gSink) {
        gSink(level, message);
        return;
    }
    std::cerr << "[BDFR][" << levelName(level) << "] " << message << '\n';
}

Stopwatch::Stopwatch() : start_(std::chrono::steady_clock::now()) {}

void Stopwatch::reset() {
    start_ = std::chrono::steady_clock::now();
}

double Stopwatch::elapsedMilliseconds() const {
    const auto elapsed = std::chrono::steady_clock::now() - start_;
    return std::chrono::duration<double, std::milli>(elapsed).count();
}

ScopedTimer::ScopedTimer(std::string label, Callback callback)
    : label_(std::move(label)), callback_(std::move(callback)) {}

ScopedTimer::~ScopedTimer() {
    if (callback_) callback_(label_, stopwatch_.elapsedMilliseconds());
}

} // namespace bdfr
