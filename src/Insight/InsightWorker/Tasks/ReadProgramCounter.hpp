#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

class ReadProgramCounter: public InsightWorkerTask
{
    Q_OBJECT

public:
    ReadProgramCounter() = default;

    QString brief() const override {
        return "Reading program counter";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups({
            TaskGroup::USES_TARGET_CONTROLLER,
        });
    };

signals:
    void programCounterRead(Targets::TargetProgramCounter programCounter);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;
};
