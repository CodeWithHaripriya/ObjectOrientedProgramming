#include "SessionManager.hpp"
#include "Session.hpp"
#include "Logger.hpp"

namespace asyncnet {

void SessionManager::addSession(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
    LOG_DEBUG("Active sessions count: " + std::to_string(sessions_.size()));
}

void SessionManager::removeSession(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
    LOG_DEBUG("Active sessions count: " + std::to_string(sessions_.size()));
}

void SessionManager::stopAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    LOG_INFO("Stopping all active sessions (" + std::to_string(sessions_.size()) + ")");
    for (auto& session : sessions_) {
        session->stop();
    }
    sessions_.clear();
}

size_t SessionManager::getActiveConnectionsCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.size();
}

} // namespace asyncnet
