#include "RetrieveMemorySnapshots.hpp"

#include <QFile>
#include <QDir>
#include <QStringList>
#include <QJsonDocument>

#include "src/Services/PathService.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

using Services::TargetControllerService;

RetrieveMemorySnapshots::RetrieveMemorySnapshots(Targets::TargetMemoryType memoryType)
    : memoryType(memoryType)
{}

void RetrieveMemorySnapshots::run(TargetControllerService& targetControllerService) {
    emit this->memorySnapshotsRetrieved(this->getSnapshots(this->memoryType));
}

std::vector<MemorySnapshot> RetrieveMemorySnapshots::getSnapshots(Targets::TargetMemoryType memoryType) {
    constexpr auto MAX_SNAPSHOTS = 30;
    auto snapshotDir = QDir(QString::fromStdString(Services::PathService::projectSettingsDirPath())
        + "/memory_snapshots/" + EnumToStringMappings::targetMemoryTypes.at(memoryType));

    if (!snapshotDir.exists()) {
        return {};
    }

    auto snapshots = std::vector<MemorySnapshot>();

    const auto snapshotFileEntries = snapshotDir.entryInfoList(
        QStringList("*.json"),
        QDir::Files,
        QDir::SortFlag::Time
    );

    for (const auto& snapshotFileEntry : snapshotFileEntries) {
        auto snapshotFile = QFile(snapshotFileEntry.absoluteFilePath());

        if (snapshots.size() >= MAX_SNAPSHOTS) {
            Logger::warning(
                "The total number of " + EnumToStringMappings::targetMemoryTypes.at(memoryType).toUpper().toStdString()
                    + " snapshots exceeds the hard limit of " + std::to_string(MAX_SNAPSHOTS)
                    + ". Only the most recent " + std::to_string(MAX_SNAPSHOTS) + " snapshots will be loaded."
            );
            break;
        }

        try {
            if (!snapshotFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw Exceptions::Exception("Failed to open snapshot file");
            }

            snapshots.emplace_back(QJsonDocument::fromJson(snapshotFile.readAll()).object());

        } catch (const Exceptions::Exception& exception) {
            Logger::error(
                "Failed to load snapshot " + snapshotFileEntry.absoluteFilePath().toStdString() + " - "
                    + exception.getMessage()
            );
        }

        snapshotFile.close();
    }

    return snapshots;
}
