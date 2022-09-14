#include "Process.hpp"

#include <unistd.h>
#include <string>

namespace Bloom
{
    ::pid_t Process::getProcessId() {
        return getpid();
    }

    ::pid_t Process::getParentProcessId() {
        return getppid();
    }

    bool Process::isProcessManagedByClion(::pid_t processId) {
        static auto cachedResult = std::optional<bool>();

        if (cachedResult.has_value()) {
            return cachedResult.value();
        }

        // Walk the process tree until we find CLion
        while (processId != 0) {
            const auto processInfo = Process::getProcessInfo(processId);

            if (!processInfo.has_value()) {
                break;
            }

            auto* processInfoPtr = processInfo.value();

            const auto commandLine = std::string(processInfoPtr->cmd);
            if (commandLine.find("clion.sh") != std::string::npos) {
                freeproc(processInfoPtr);
                cachedResult = true;
                return true;
            }

            processId = processInfoPtr->ppid;
            freeproc(processInfoPtr);
        }

        cachedResult = true;
        return false;
    }

    std::optional<::proc_t*> Process::getProcessInfo(::pid_t processId) {
        auto* proc = ::openproc(PROC_FILLSTAT | PROC_FILLARG | PROC_PID, &processId);
        auto* processInfo = ::readproc(proc, NULL);

        if (processInfo == NULL) {
            return std::nullopt;
        }

        closeproc(proc);
        return processInfo;
    }
}
