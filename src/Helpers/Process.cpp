#include "Process.hpp"

#include <unistd.h>
#include <string>
#include <map>

namespace Bloom
{
    ::pid_t Process::getProcessId() {
        return getpid();
    }

    ::pid_t Process::getParentProcessId() {
        return getppid();
    }

    bool Process::isManagedByClion(std::optional<::pid_t> parentProcessId) {
        if (!parentProcessId.has_value()) {
            parentProcessId = Process::getParentProcessId();
        }

        static auto cachedResultsByProcessId = std::map<::pid_t, bool>();

        if (cachedResultsByProcessId.contains(*parentProcessId)) {
            return cachedResultsByProcessId.at(*parentProcessId);
        }

        // Walk the process tree until we find CLion
        auto processId = *parentProcessId;
        while (processId != 0) {
            const auto processInfo = Process::getProcessInfo(processId);

            if (!processInfo) {
                break;
            }

            const auto commandLine = std::string(processInfo->cmd);
            if (commandLine.find("clion.sh") != std::string::npos) {
                cachedResultsByProcessId[*parentProcessId] = true;
                return true;
            }

            processId = processInfo->ppid;
        }

        cachedResultsByProcessId[*parentProcessId] = false;
        return false;
    }

    Process::ProcT Process::getProcessInfo(::pid_t processId) {
        auto proc = std::unique_ptr<::PROCTAB, decltype(&::closeproc)>(
            ::openproc(PROC_FILLSTAT | PROC_FILLARG | PROC_PID, &processId),
            ::closeproc
        );
        auto processInfo = ProcT(::readproc(proc.get(), NULL), ::freeproc);

        if (processInfo == NULL) {
            return ProcT(nullptr, ::freeproc);
        }

        return processInfo;
    }
}
