#include "DeleteMemorySnapshot.hpp"

#include <QFile>

#include "src/Services/PathService.hpp"
#include "src/Logger/Logger.hpp"

using Services::TargetControllerService;

DeleteMemorySnapshot::DeleteMemorySnapshot(const QString& snapshotId)
    : snapshotId(snapshotId)
{}

QString DeleteMemorySnapshot::brief() const {
    return "Deleting memory snapshot " + this->snapshotId;
}

void DeleteMemorySnapshot::run(TargetControllerService&) {
    Logger::info("Deleting snapshot " + this->snapshotId.toStdString());

    const auto snapshotFilePath = QString::fromStdString(Services::PathService::memorySnapshotsPath())
        + this->snapshotId + ".json";

    auto snapshotFile = QFile{snapshotFilePath};
    if (!snapshotFile.exists()) {
        Logger::warning(
            "Could not find snapshot file for " + this->snapshotId.toStdString() + " - expected path: "
                + snapshotFilePath.toStdString()
        );
        return;
    }

    snapshotFile.remove();
}
