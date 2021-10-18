#pragma once

#include "InsightWorkerTask.hpp"

namespace Bloom
{
    class QueryLatestVersionNumber: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        QueryLatestVersionNumber(const QString& currentVersionNumber):
        InsightWorkerTask(), currentVersionNumber(currentVersionNumber) {}

    signals:
        void latestVersionNumberRetrieved(const QString& latestVersionNumber);

    protected:
        void run(TargetControllerConsole& targetControllerConsole) override;

    private:
        QString currentVersionNumber;
    };
}
