#include "ProcessService.hpp"

#include <unistd.h>

namespace Services
{
    ::pid_t ProcessService::getProcessId() {
        return ::getpid();
    }

    ::pid_t ProcessService::getParentProcessId() {
        return ::getppid();
    }

    ::uid_t ProcessService::getEffectiveUserId() {
        return ::geteuid();
    }

    bool ProcessService::isRunningAsRoot() {
        return ProcessService::getEffectiveUserId() == 0;
    }
}
