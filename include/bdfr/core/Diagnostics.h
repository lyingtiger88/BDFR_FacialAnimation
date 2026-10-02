#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace bdfr {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error
};

using LogSink = std::function<void(LogLevel, const std::string&)>;

class Logger {
public:
    static void setSink(LogSink sink);
    static void write(LogLevel level, const std::string& message);
};

class Stopwatch {
public:
    Stopwatch();
    void reset();
    double elapsedMilliseconds() const;

private:
    std::chrono::steady_clock::time_point start_;
};

class ScopedTimer {
public:
    using Callback = std::function<void(const std::string&, double)>;

    ScopedTimer(std::string label, Callback callback);
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    std::string label_;
    Callback callback_;
    Stopwatch stopwatch_;
};

} // namespace bdfr
