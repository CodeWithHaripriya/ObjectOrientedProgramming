#ifndef LOB_OBJECT_POOL_HPP
#define LOB_OBJECT_POOL_HPP

#include <vector>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace lob {

template <typename T, size_t Capacity>
class ObjectPool {
public:
    ObjectPool() {
        poolStorage_.resize(Capacity);
        freeList_.reserve(Capacity);
        for (size_t i = 0; i < Capacity; ++i) {
            freeList_.push_back(&poolStorage_[i]);
        }
    }

    ~ObjectPool() = default;

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    template <typename... Args>
    T* allocate(Args&&... args) {
        if (freeList_.empty()) [[unlikely]] {
            throw std::runtime_error("ObjectPool capacity exhausted!");
        }

        T* ptr = freeList_.back();
        freeList_.pop_back();
        
        // Placement new to construct object in pre-allocated memory slot
        new (ptr) T(std::forward<Args>(args)...);
        return ptr;
    }

    void deallocate(T* ptr) {
        if (!ptr) return;
        
        // Call destructor explicitly
        ptr->~T();
        freeList_.push_back(ptr);
    }

    size_t available() const noexcept {
        return freeList_.size();
    }

    size_t capacity() const noexcept {
        return Capacity;
    }

private:
    std::vector<T> poolStorage_;
    std::vector<T*> freeList_;
};

} // namespace lob

#endif // LOB_OBJECT_POOL_HPP
