#pragma once

#include <memory>
#include <sys/types.h>

namespace Services
{
    class ProcessService
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
         * Returns the effective user ID of the current process.
         *
         * @return
         */
        static ::uid_t getEffectiveUserId();

        /**
         * Returns true if the current process is running as root.
         *
         * @return
         */
        static bool isRunningAsRoot();
    };
}
