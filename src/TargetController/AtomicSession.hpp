#pragma once

#include <atomic>
#include <cstdint>

namespace TargetController
{
    using AtomicSessionIdType = int;
    static_assert(std::atomic<AtomicSessionIdType>::is_always_lock_free);

    class AtomicSession
    {
    public:
        const AtomicSessionIdType id = ++(AtomicSession::lastSessionId);

        AtomicSession() = default;
        AtomicSession(const AtomicSession&) = delete;
        AtomicSession(const AtomicSession&&) = delete;
        AtomicSession& operator = (AtomicSession&) = delete;
        AtomicSession& operator = (AtomicSession&&) = delete;

    private:
        static inline std::atomic<AtomicSessionIdType> lastSessionId = 0;
    };
}
