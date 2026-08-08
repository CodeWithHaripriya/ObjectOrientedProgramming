#ifndef ASYNCNET_LOGGER_HPP
#define ASYNCNET_LOGGER_HPP

#include <string>
#include <mutex>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <thread>

namespace asyncnet {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        currentLevel_ = level;
    }

    void log(LogLevel level, const std::string& message) {
        if (level < currentLevel_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

        std::ostringstream ss;
        ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[" << levelToString(level) << "] "
           << "[T-" << std::this_thread::get_id() << "] "
           << message << "\n";

        if (level == LogLevel::ERROR) {
            std::cerr << ss.str() << std::flush;
        } else {
            std::cout << ss.str() << std::flush;
        }
    }

private:
    Logger() : currentLevel_(LogLevel::INFO) {}
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            default:              return "UNKNW";
        }
    }

    LogLevel currentLevel_;
    std::mutex mutex_;
};

#define LOG_DEBUG(msg) asyncnet::Logger::getInstance().log(asyncnet::LogLevel::DEBUG, msg)
#define LOG_INFO(msg)  asyncnet::Logger::getInstance().log(asyncnet::LogLevel::INFO, msg)
#define LOG_WARN(msg)  asyncnet::Logger::getInstance().log(asyncnet::LogLevel::WARN, msg)
#define LOG_ERROR(msg) asyncnet::Logger::getInstance().log(asyncnet::LogLevel::ERROR, msg)

} // namespace asyncnet

#endif // ASYNCNET_LOGGER_HPP
