#pragma once

#include <vector>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"

namespace Bloom
{
    class RetrieveMemorySnapshots: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        RetrieveMemorySnapshots(Targets::TargetMemoryType memoryType);

        QString brief() const override {
            return "Loading saved " + QString(this->memoryType == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM")
                + " memory snapshots";
        }

    signals:
        void memorySnapshotsRetrieved(std::vector<MemorySnapshot> snapshots);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;

    private:
        Targets::TargetMemoryType memoryType;

        std::vector<MemorySnapshot> getSnapshots(Targets::TargetMemoryType memoryType);
    };
}
