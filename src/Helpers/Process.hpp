#pragma once

#include <optional>
#include <proc/readproc.h>

namespace Bloom
{
    class Process
    {
    public:
        /**
         * Returns the process ID of the current process.
         *
         * @return
         */
        static ::pid_t getProcessId();

        /**
         * Returns the process ID of the current process's parent.
         *
         * @return
         */
        static ::pid_t getParentProcessId();

        /**
         * Returns true if the given process is managed by CLion.
         *
         * @param processId
         * @return
         */
        static bool isProcessManagedByClion(::pid_t processId);

    private:
        static std::optional<::proc_t*> getProcessInfo(::pid_t processId);
    };
}
