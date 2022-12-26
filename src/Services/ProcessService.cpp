#include "ProcessService.hpp"

#include <unistd.h>
#include <string>
#include <map>

#include "src/Exceptions/Exception.hpp"

namespace Bloom::Services
{
    ::pid_t ProcessService::getProcessId() {
        return getpid();
    }

    ::pid_t ProcessService::getParentProcessId() {
        return getppid();
    }

    ::uid_t ProcessService::getEffectiveUserId(std::optional<::pid_t> processId) {
        if (!processId.has_value()) {
            processId = ProcessService::getProcessId();
        }

        const auto processInfo = ProcessService::getProcessInfo(processId.value());

        if (!processInfo) {
            throw Exceptions::Exception(
                "Failed to fetch process info for process ID " + std::to_string(processId.value())
            );
        }

        return static_cast<::uid_t>(processInfo->euid);
    }

    bool ProcessService::isRunningAsRoot(std::optional<::pid_t> processId) {
        return ProcessService::getEffectiveUserId(processId) == 0;
    }

    bool ProcessService::isManagedByClion(std::optional<::pid_t> processId) {
        if (!processId.has_value()) {
            processId = ProcessService::getProcessId();
        }

        static auto cachedResultsByProcessId = std::map<::pid_t, bool>();
        const auto cachedResultIt = cachedResultsByProcessId.find(*processId);

        if (cachedResultIt != cachedResultsByProcessId.end()) {
            return cachedResultIt->second;
        }

        // Start with the parent process and walk the tree until we find CLion
        const auto processInfo = ProcessService::getProcessInfo(*processId);

        if (!processInfo) {
            cachedResultsByProcessId[*processId] = false;
            return false;
        }

        auto pid = processInfo->ppid;

        while (const auto processInfo = ProcessService::getProcessInfo(pid)) {
            const auto commandLine = std::string(processInfo->cmd);

            if (commandLine.find("clion.sh") != std::string::npos) {
                cachedResultsByProcessId[*processId] = true;
                return true;
            }

            pid = processInfo->ppid;
        }

        cachedResultsByProcessId[*processId] = false;
        return false;
    }

    ProcessService::Proc ProcessService::getProcessInfo(::pid_t processId) {
        const auto proc = std::unique_ptr<::PROCTAB, decltype(&::closeproc)>(
            ::openproc(PROC_FILLSTAT | PROC_FILLARG | PROC_PID, &processId),
            ::closeproc
        );

        return Proc(::readproc(proc.get(), NULL), ::freeproc);
    }
}
