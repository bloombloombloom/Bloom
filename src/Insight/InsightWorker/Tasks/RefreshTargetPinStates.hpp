#pragma once

#include "InsightWorkerTask.hpp"
#include "src/Targets/TargetVariant.hpp"
#include "src/Targets/TargetPinDescriptor.hpp"

class RefreshTargetPinStates: public InsightWorkerTask
{
    Q_OBJECT

public:
    explicit RefreshTargetPinStates(int variantId)
        : variantId(variantId)
    {}

    QString brief() const override {
        return "Reading target pin states";
    }

    TaskGroups taskGroups() const override {
        return TaskGroups({
            TaskGroup::USES_TARGET_CONTROLLER,
        });
    };

signals:
    void targetPinStatesRetrieved(Targets::TargetPinStateMapping pinStatesByNumber);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;

private:
    int variantId;
};
