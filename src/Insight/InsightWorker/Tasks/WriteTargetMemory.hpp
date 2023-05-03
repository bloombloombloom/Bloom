#pragma once

#include <vector>

#include "InsightWorkerTask.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"

namespace Bloom
{
    class WriteTargetMemory: public InsightWorkerTask
    {
        Q_OBJECT

    public:
        /*
         * A Block is just a block of contiguous data to write. A single WriteTargetMemory task can write multiple
         * blocks to a particular memory.
         */
        struct Block {
            Targets::TargetMemoryAddress startAddress;
            const Targets::TargetMemoryBuffer data;

            Block(
                Targets::TargetMemoryAddress startAddress,
                const Targets::TargetMemoryBuffer& data
            )
                : startAddress(startAddress)
                , data(data)
            {}
        };

        WriteTargetMemory(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            std::vector<Block>&& blocks
        )
            : memoryDescriptor(memoryDescriptor)
            , blocks(std::move(blocks))
        {}

        WriteTargetMemory(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& data
        )
            : WriteTargetMemory(memoryDescriptor, std::vector<Block>({{startAddress, data}}))
        {}

        QString brief() const override {
            return
                "Writing to target " + EnumToStringMappings::targetMemoryTypes.at(
                    this->memoryDescriptor.type
                ).toUpper();
        }

        TaskGroups taskGroups() const override {
            return TaskGroups({
                TaskGroup::USES_TARGET_CONTROLLER,
            });
        }

    signals:
        void targetMemoryWritten(Targets::TargetMemorySize bytesWritten);

    protected:
        void run(Services::TargetControllerService& targetControllerService) override;

    private:
        Targets::TargetMemoryDescriptor memoryDescriptor;
        std::vector<Block> blocks;
    };
}
