#pragma once

#include <cstdint>
#include <csignal>
#include <cassert>
#include <atomic>

enum class ThreadState: std::uint8_t
{
    UNINITIALISED,
    READY,
    STOPPED,
    STARTING,
    SHUTDOWN_INITIATED,
};

class Thread
{
public:
    Thread() = default;
    virtual ~Thread() = default;

    Thread(const Thread& other) = delete;
    Thread(Thread&& other) = delete;

    Thread& operator = (const Thread& other) = delete;
    Thread& operator = (Thread&& other) = delete;

    ThreadState getThreadState() {
        return this->threadState;
    }

protected:
    std::atomic<ThreadState> threadState = ThreadState::UNINITIALISED;

    /**
     * Disables signal interrupts on current thread.
     */
    static void blockAllSignals() {
        sigset_t set = {};
        sigfillset(&set);
        sigprocmask(SIG_SETMASK, &set, NULL);
    }

    void setName(const std::string& name) {
        // POSIX thread names cannot exceed 16 characters, including the terminating null byte.
        assert(name.size() <= 15);

        pthread_setname_np(pthread_self(), name.c_str());
    }
};
