#include "CaptureMemorySnapshot.hpp"

#include <QFile>
#include <QDir>
#include <QJsonDocument>

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    CaptureMemorySnapshot::CaptureMemorySnapshot(
        const QString& name,
        const QString& description,
        Targets::TargetMemoryType memoryType,
        const std::vector<FocusedMemoryRegion>& focusedRegions,
        const std::vector<ExcludedMemoryRegion>& excludedRegions,
        const std::optional<Targets::TargetMemoryBuffer>& data
    )
        : name(name)
        , description(description)
        , memoryType(memoryType)
        , focusedRegions(focusedRegions)
        , excludedRegions(excludedRegions)
        , data(data)
    {}

    void CaptureMemorySnapshot::run(TargetControllerService& targetControllerService) {
        using Targets::TargetMemorySize;

        Logger::info("Capturing snapshot");

        const auto& targetDescriptor = targetControllerService.getTargetDescriptor();
        const auto memoryDescriptorIt = targetDescriptor.memoryDescriptorsByType.find(this->memoryType);

        if (memoryDescriptorIt == targetDescriptor.memoryDescriptorsByType.end()) {
            throw Exceptions::Exception("Invalid memory type");
        }

        const auto& memoryDescriptor = memoryDescriptorIt->second;
        const auto memorySize = memoryDescriptor.size();

        if (!this->data.has_value()) {
            Logger::info("Reading data for snapshot capture");

            this->data = Targets::TargetMemoryBuffer();
            this->data->reserve(memorySize);

            const auto readSize = std::max(
                TargetMemorySize(256),
                memoryDescriptor.pageSize.value_or(TargetMemorySize(0))
            );
            const auto readsRequired = static_cast<std::uint32_t>(
                std::ceil(static_cast<float>(memorySize) / static_cast<float>(readSize))
            );

            for (std::uint32_t i = 0; i < readsRequired; i++) {
                auto dataSegment = targetControllerService.readMemory(
                    this->memoryType,
                    memoryDescriptor.addressRange.startAddress + static_cast<Targets::TargetMemoryAddress>(readSize * i),
                    (memorySize - this->data->size()) >= readSize
                        ? readSize
                        : static_cast<Targets::TargetMemorySize>(memorySize - this->data->size()),
                    {}
                );

                std::move(dataSegment.begin(), dataSegment.end(), std::back_inserter(*this->data));
            }
        }

        assert(this->data->size() == memorySize);

        auto snapshot = MemorySnapshot(
            std::move(this->name),
            std::move(this->description),
            this->memoryType,
            std::move(*this->data),
            targetControllerService.getProgramCounter(),
            std::move(this->focusedRegions),
            std::move(this->excludedRegions)
        );

        const auto snapshotDirPath = QString::fromStdString(Services::PathService::projectSettingsDirPath())
            + "/memory_snapshots/" + EnumToStringMappings::targetMemoryTypes.at(snapshot.memoryType);

        QDir().mkpath(snapshotDirPath);

        const auto snapshotFilePath = snapshotDirPath + "/" + snapshot.id + ".json";

        auto outputFile = QFile(snapshotFilePath);

        if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
            Logger::error("Failed to save snapshot - cannot open " + snapshotFilePath.toStdString());
            return;
        }

        outputFile.write(QJsonDocument(snapshot.toJson()).toJson(QJsonDocument::JsonFormat::Compact));
        outputFile.close();

        Logger::info("Snapshot captured - UUID: " + snapshot.id.toStdString());

        emit this->memorySnapshotCaptured(std::move(snapshot));
    }
}
