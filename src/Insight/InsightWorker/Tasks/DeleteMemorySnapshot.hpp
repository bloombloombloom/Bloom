#pragma once

#include <QString>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

class DeleteMemorySnapshot: public InsightWorkerTask
{
    Q_OBJECT

public:
    DeleteMemorySnapshot(const QString& snapshotId, Targets::TargetMemoryType memoryType);

    QString brief() const override {
        return "Deleting memory snapshot " + this->snapshotId;
    }

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    QString snapshotId;
    Targets::TargetMemoryType memoryType;
};
