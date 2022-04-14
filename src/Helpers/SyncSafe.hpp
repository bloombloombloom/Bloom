#pragma once

#include <mutex>

namespace Bloom
{
    /**
     * Template for synchronization safe types.
     *
     * Just a convenient template that allows us to create thread safe types without having to write
     * the bloat of mutexes, unique_locks, etc etc.
     *
     * @tparam Type
     */
    template<typename Type>
    class SyncSafe
    {
    public:
        SyncSafe() = default;

        explicit SyncSafe(Type value)
            : value(value)
        {}

        void setValue(const Type& value) {
            auto lock = std::unique_lock(this->mutex);
            this->value = value;
        }

        Type& getValue() {
            return this->value;
        }

        std::unique_lock<std::mutex> acquireLock() {
            return std::unique_lock(this->mutex);
        }

    private:
        Type value;
        std::mutex mutex;
    };
}
