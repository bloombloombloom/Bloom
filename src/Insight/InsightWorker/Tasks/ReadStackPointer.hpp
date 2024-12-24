#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

class ReadStackPointer: public InsightWorkerTask
{
    Q_OBJECT

public:
    ReadStackPointer() = default;
    [[nodiscard]] QString brief() const override;
    [[nodiscard]] TaskGroups taskGroups() const override;

signals:
    void stackPointerRead(Targets::TargetStackPointer stackPointer);

protected:
    void run(Services::TargetControllerService& targetControllerService) override;
};
