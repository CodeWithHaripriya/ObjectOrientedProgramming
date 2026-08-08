#ifndef ASYNCNET_SESSION_MANAGER_HPP
#define ASYNCNET_SESSION_MANAGER_HPP

#include <memory>
#include <unordered_set>
#include <mutex>
#include <atomic>

namespace asyncnet {

class Session;

class SessionManager {
public:
    SessionManager() = default;
    ~SessionManager() = default;

    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

    void addSession(std::shared_ptr<Session> session);
    void removeSession(std::shared_ptr<Session> session);
    void stopAll();

    size_t getActiveConnectionsCount() const;
    uint64_t getTotalProcessedMessages() const { return totalMessagesProcessed_.load(); }
    void incrementProcessedMessages() { ++totalMessagesProcessed_; }

private:
    mutable std::mutex mutex_;
    std::unordered_set<std::shared_ptr<Session>> sessions_;
    std::atomic<uint64_t> totalMessagesProcessed_{0};
};

} // namespace asyncnet

#endif // ASYNCNET_SESSION_MANAGER_HPP
