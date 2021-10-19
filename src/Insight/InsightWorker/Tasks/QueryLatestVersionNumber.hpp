#pragma once

#include "InsightWorkerTask.hpp"

#include "src/VersionNumber.hpp"

namespace Bloom
{
    class QueryLatestVersionNumber: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        QueryLatestVersionNumber(const VersionNumber& currentVersionNumber):
        InsightWorkerTask(), currentVersionNumber(currentVersionNumber) {}

    signals:
        void latestVersionNumberRetrieved(const VersionNumber& latestVersionNumber);

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    private:
        VersionNumber currentVersionNumber;
    };
}
