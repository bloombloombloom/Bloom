#pragma once

#include <csignal>
#include <cassert>

#include "SyncSafe.hpp"

namespace Bloom
{
    enum class ThreadState
    {
        UNINITIALISED,
        READY,
        STOPPED,
        STARTING,
        SHUTDOWN_INITIATED,
    };

    class Thread
    {
    private:
        SyncSafe<ThreadState> state = SyncSafe<ThreadState>(ThreadState::UNINITIALISED);

    protected:
        virtual void setThreadState(ThreadState state) {
            this->state.setValue(state);
        };

        /**
         * Disables signal interrupts on current thread.
         */
        void blockAllSignalsOnCurrentThread() {
            sigset_t set = {};
            sigfillset(&set);
            sigprocmask(SIG_SETMASK, &set, NULL);
        };

        void setName(std::string name) {
            // POSIX thread names cannot exceed 16 characters, including the terminating null byte.
            assert(name.size() <= 15);

            pthread_setname_np(pthread_self(), name.c_str());
        }

    public:
        virtual ThreadState getThreadState() {
            return this->state.getValue();
        };
    };
}
