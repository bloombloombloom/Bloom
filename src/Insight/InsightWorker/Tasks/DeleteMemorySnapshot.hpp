#pragma once

#include <QString>

#include "InsightWorkerTask.hpp"

class DeleteMemorySnapshot: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit DeleteMemorySnapshot(const QString& snapshotId);
    [[nodiscard]] QString brief() const override ;

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    QString snapshotId;
};
