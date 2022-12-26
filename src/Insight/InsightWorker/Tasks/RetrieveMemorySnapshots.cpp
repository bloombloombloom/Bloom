#include "RetrieveMemorySnapshots.hpp"

#include <QFile>
#include <QDir>
#include <QStringList>
#include <QJsonDocument>

#include "src/Helpers/Paths.hpp"
#include "src/Helpers/EnumToStringMappings.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Logger/Logger.hpp"

namespace Bloom
{
    using Services::TargetControllerService;

    RetrieveMemorySnapshots::RetrieveMemorySnapshots(Targets::TargetMemoryType memoryType)
        : memoryType(memoryType)
    {}

    void RetrieveMemorySnapshots::run(TargetControllerService& targetControllerService) {
        emit this->memorySnapshotsRetrieved(this->getSnapshots(this->memoryType));
    }

    std::vector<MemorySnapshot> RetrieveMemorySnapshots::getSnapshots(Targets::TargetMemoryType memoryType) {
        auto snapshotDir = QDir(QString::fromStdString(Paths::projectSettingsDirPath())
            + "/memory_snapshots/" + EnumToStringMappings::targetMemoryTypes.at(memoryType));

        if (!snapshotDir.exists()) {
            return {};
        }

        auto snapshots = std::vector<MemorySnapshot>();

        const auto snapshotFileEntries = snapshotDir.entryInfoList(QStringList("*.json"), QDir::Files);
        for (const auto& snapshotFileEntry : snapshotFileEntries) {
            auto snapshotFile = QFile(snapshotFileEntry.absoluteFilePath());

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
}
