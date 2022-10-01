#pragma once

#include <optional>
#include <memory>
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
         * @param parentProcessId
         *  If not provided, this function will perform the check against the current process.
         *
         * @return
         */
        static bool isManagedByClion(std::optional<::pid_t> parentProcessId = std::nullopt);


    private:
        using Proc = std::unique_ptr<::proc_t, decltype(&::freeproc)>;
        static Proc getProcessInfo(::pid_t processId);
    };
}
