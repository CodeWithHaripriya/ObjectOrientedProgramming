#ifndef LOB_LOCK_FREE_QUEUE_HPP
#define LOB_LOCK_FREE_QUEUE_HPP

#include <atomic>
#include <vector>
#include <cstddef>
#include <new>

namespace lob {

// Cache line size padding constant (64 bytes for x86_64 / ARM)
#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

template <typename T, size_t Capacity>
class LockFreeSPSCQueue {
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for fast modulo masking.");

    LockFreeSPSCQueue() : buffer_(Capacity) {}

    ~LockFreeSPSCQueue() = default;

    LockFreeSPSCQueue(const LockFreeSPSCQueue&) = delete;
    LockFreeSPSCQueue& operator=(const LockFreeSPSCQueue&) = delete;

    template <typename... Args>
    bool emplace(Args&&... args) {
        const auto currentTail = tail_.load(std::memory_order_relaxed);
        const auto currentHead = head_.load(std::memory_order_acquire);

        if ((currentTail - currentHead) >= Capacity) [[unlikely]] {
            return false; // Queue full
        }

        buffer_[currentTail & MASK_] = T(std::forward<Args>(args)...);
        tail_.store(currentTail + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const auto currentHead = head_.load(std::memory_order_relaxed);
        const auto currentTail = tail_.load(std::memory_order_acquire);

        if (currentHead == currentTail) {
            return false; // Queue empty
        }

        item = std::move(buffer_[currentHead & MASK_]);
        head_.store(currentHead + 1, std::memory_order_release);
        return true;
    }

    size_t size() const noexcept {
        const auto currentHead = head_.load(std::memory_order_relaxed);
        const auto currentTail = tail_.load(std::memory_order_relaxed);
        return (currentTail >= currentHead) ? (currentTail - currentHead) : 0;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

private:
    static constexpr size_t MASK_ = Capacity - 1;
    std::vector<T> buffer_;

    // alignas to prevent false-sharing cache line invalidation between producer & consumer cores
    alignas(hardware_destructive_interference_size) std::atomic<size_t> head_{0};
    alignas(hardware_destructive_interference_size) std::atomic<size_t> tail_{0};
};

} // namespace lob

#endif // LOB_LOCK_FREE_QUEUE_HPP
