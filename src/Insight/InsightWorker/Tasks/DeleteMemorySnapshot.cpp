#include "DeleteMemorySnapshot.hpp"

#include <QFile>

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    DeleteMemorySnapshot::DeleteMemorySnapshot(
        const QString& snapshotId,
        Targets::TargetMemoryType memoryType
    )
        : snapshotId(snapshotId)
        , memoryType(memoryType)
    {}

    void DeleteMemorySnapshot::run(TargetControllerService&) {
        using Targets::TargetMemorySize;

        Logger::info("Deleting snapshot " + this->snapshotId.toStdString());

        const auto snapshotFilePath = QString::fromStdString(Services::PathService::projectSettingsDirPath())
            + "/memory_snapshots/" + EnumToStringMappings::targetMemoryTypes.at(this->memoryType) + "/"
                + this->snapshotId + ".json";

        auto snapshotFile = QFile(snapshotFilePath);

        if (!snapshotFile.exists()) {
            Logger::warning(
                "Could not find snapshot file for " + this->snapshotId.toStdString() + " - expected path: "
                    + snapshotFilePath.toStdString()
            );
            return;
        }

        snapshotFile.remove();
    }
}
