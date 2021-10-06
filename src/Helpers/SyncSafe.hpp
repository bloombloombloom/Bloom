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
     * @TODO Might be an idea to use an off-the-shelf solution for this, as there are a few available.
     *
     * @tparam Type
     */
    template<typename Type>
    class SyncSafe
    {
    public:
        SyncSafe() = default;

        explicit SyncSafe(Type value): value(value) {};

        void setValue(Type value) {
            auto lock = std::unique_lock(this->mutex);
            this->value = value;
        };

        Type getValue() {
            auto lock = std::unique_lock(this->mutex);
            return this->value;
        };

        Type& getReference() {
            return this->value;
        };

        void lock() {
            this->mutex.lock();
        };

        void unlock() {
            this->mutex.unlock();
        };

        std::unique_lock<std::mutex> acquireLock() {
            return std::unique_lock(this->mutex);
        };

    private:
        std::mutex mutex;
        Type value;
    };
}
