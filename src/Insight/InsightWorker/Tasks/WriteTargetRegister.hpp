#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetRegister.hpp"

class WriteTargetRegister: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit WriteTargetRegister(const Targets::TargetRegister& targetRegister)
        : targetRegister(targetRegister)
    {}

    QString brief() const override {
        return "Writing target register";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups({
            TaskGroup::USES_TARGET_CONTROLLER,
        });
    };

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    Targets::TargetRegister targetRegister;
};
