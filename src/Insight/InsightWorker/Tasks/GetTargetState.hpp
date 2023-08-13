#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetState.hpp"

class GetTargetState: public InsightWorkerTask
{
    Q_OBJECT

public:
    GetTargetState() = default;

    QString brief() const override {
        return "Obtaining target state";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups({
            TaskGroup::USES_TARGET_CONTROLLER,
        });
    };

signals:
    void targetState(Targets::TargetState state);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;
};
