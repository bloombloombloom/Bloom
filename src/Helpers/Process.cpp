#include "Process.hpp"

#include <unistd.h>
#include <string>
#include <map>

#include "src/Exceptions/Exception.hpp"

namespace Bloom
{
    ::pid_t Process::getProcessId() {
        return getpid();
    }

    ::pid_t Process::getParentProcessId() {
        return getppid();
    }

    ::uid_t Process::getEffectiveUserId(std::optional<::pid_t> processId) {
        if (!processId.has_value()) {
            processId = Process::getProcessId();
        }

        const auto processInfo = Process::getProcessInfo(processId.value());

        if (!processInfo) {
            throw Exceptions::Exception(
                "Failed to fetch process info for process ID " + std::to_string(processId.value())
            );
        }

        return static_cast<::uid_t>(processInfo->euid);
    }

    bool Process::isRunningAsRoot(std::optional<::pid_t> processId) {
        return Process::getEffectiveUserId(processId) == 0;
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

    Process::Proc Process::getProcessInfo(::pid_t processId) {
        const auto proc = std::unique_ptr<::PROCTAB, decltype(&::closeproc)>(
            ::openproc(PROC_FILLSTAT | PROC_FILLARG | PROC_PID, &processId),
            ::closeproc
        );

        return Proc(::readproc(proc.get(), NULL), ::freeproc);
    }
}
