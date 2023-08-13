#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

class ReadStackPointer: public InsightWorkerTask
{
    Q_OBJECT

public:
    ReadStackPointer() = default;

    QString brief() const override {
        return "Reading stack pointer";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups({
            TaskGroup::USES_TARGET_CONTROLLER,
        });
    };

signals:
    void stackPointerRead(Targets::TargetStackPointer stackPointer);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;
};
