#include "CaptureMemorySnapshot.hpp"

#include <QFile>
#include <QDir>
#include <QJsonDocument>

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Logger/Logger.hpp"

using Services::TargetControllerService;

CaptureMemorySnapshot::CaptureMemorySnapshot(
    const QString& name,
    const QString& description,
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
    const std::vector<FocusedMemoryRegion>& focusedRegions,
    const std::vector<ExcludedMemoryRegion>& excludedRegions,
    const std::optional<Targets::TargetMemoryBuffer>& data
)
    : name(name)
    , description(description)
    , addressSpaceDescriptor(addressSpaceDescriptor)
    , memorySegmentDescriptor(memorySegmentDescriptor)
    , focusedRegions(focusedRegions)
    , excludedRegions(excludedRegions)
    , data(data)
{}

QString CaptureMemorySnapshot::brief() const {
    return "Capturing memory snapshot";
}

TaskGroups CaptureMemorySnapshot::taskGroups() const {
    return {
        TaskGroup::USES_TARGET_CONTROLLER,
    };
}

void CaptureMemorySnapshot::run(TargetControllerService& targetControllerService) {
    using Targets::TargetMemorySize;

    Logger::info("Capturing snapshot");

    const auto memorySize = this->memorySegmentDescriptor.size();

    if (!this->data.has_value()) {
        Logger::info("Reading data for snapshot capture");

        this->data = Targets::TargetMemoryBuffer{};
        this->data->reserve(memorySize);

        const auto readSize = std::max(
            TargetMemorySize{256},
            this->memorySegmentDescriptor.pageSize.value_or(TargetMemorySize{0})
        );
        const auto readsRequired = static_cast<std::uint32_t>(
            std::ceil(static_cast<float>(memorySize) / static_cast<float>(readSize))
        );

        for (auto i = std::size_t{0}; i < readsRequired; i++) {
            auto dataSegment = targetControllerService.readMemory(
                this->addressSpaceDescriptor,
                this->memorySegmentDescriptor,
                this->memorySegmentDescriptor.addressRange.startAddress
                    + static_cast<Targets::TargetMemoryAddress>(readSize * i),
                (memorySize - this->data->size()) >= readSize
                    ? readSize
                    : static_cast<Targets::TargetMemorySize>(memorySize - this->data->size()),
                true,
                {}
            );

            std::move(dataSegment.begin(), dataSegment.end(), std::back_inserter(*this->data));
            this->setProgressPercentage(static_cast<std::uint8_t>(
                (static_cast<float>(i) + 1) / (static_cast<float>(readsRequired + 1) / 100)
            ));
        }
    }

    assert(this->data->size() == memorySize);

    auto snapshot = MemorySnapshot{
        this->name,
        this->description,
        QString::fromStdString(this->addressSpaceDescriptor.key),
        QString::fromStdString(this->memorySegmentDescriptor.key),
        *this->data,
        targetControllerService.getProgramCounter(),
        targetControllerService.getStackPointer(),
        this->focusedRegions,
        this->excludedRegions
    };

    const auto snapshotDirPath = QString::fromStdString(Services::PathService::memorySnapshotsPath());
    QDir().mkpath(snapshotDirPath);

    const auto snapshotFilePath = snapshotDirPath + "/" + snapshot.id + ".json";

    auto outputFile = QFile{snapshotFilePath};

    if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        Logger::error("Failed to save snapshot - cannot open " + snapshotFilePath.toStdString());
        return;
    }

    outputFile.write(QJsonDocument{snapshot.toJson()}.toJson(QJsonDocument::JsonFormat::Compact));
    outputFile.close();

    Logger::info("Snapshot captured - UUID: " + snapshot.id.toStdString());

    emit this->memorySnapshotCaptured(std::move(snapshot));
}
