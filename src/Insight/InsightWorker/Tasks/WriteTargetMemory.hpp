#pragma once

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom
{
    class WriteTargetMemory: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        WriteTargetMemory(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& data
        )
            : memoryDescriptor(memoryDescriptor)
            , startAddress(startAddress)
            , data(data)
        {}

        QString brief() const override {
            return
                "Writing to target "
                + QString(this->memoryDescriptor.type == Targets::TargetMemoryType::EEPROM ? "EEPROM" : "RAM");
        }

        TaskGroups taskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        };

    signals:
        void targetMemoryWritten(Targets::TargetMemorySize bytesWritten);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;

    private:
        Targets::TargetMemoryDescriptor memoryDescriptor;
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryBuffer data;
    };
}
