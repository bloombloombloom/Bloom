#pragma once

#include <optional>
#include <memory>
#include <sys/types.h>
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
         * Returns the effective user ID of the given process.
         *
         * @param processId
         *  If not provided, this function will use the current process ID.
         *
         * @return
         */
        static ::uid_t getEffectiveUserId(std::optional<::pid_t> processId = std::nullopt);

        /**
         * Returns true if the given process is running as root.
         *
         * @param processId
         *  If not provided, this function will perform the check against the current process.
         *
         * @return
         */
        static bool isRunningAsRoot(std::optional<::pid_t> processId = std::nullopt);

        /**
         * Returns true if the given process is managed by CLion.
         *
         * @param processId
         *  If not provided, this function will perform the check against the current process.
         *
         * @return
         */
        static bool isManagedByClion(std::optional<::pid_t> processId = std::nullopt);

    private:
        using Proc = std::unique_ptr<::proc_t, decltype(&::freeproc)>;
        static Proc getProcessInfo(::pid_t processId);
    };
}
