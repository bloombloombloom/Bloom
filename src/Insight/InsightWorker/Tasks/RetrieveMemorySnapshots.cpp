#include "RetrieveMemorySnapshots.hpp"

#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <algorithm>
#include <utility>

#include "src/Services/PathService.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

using Services::TargetControllerService;

RetrieveMemorySnapshots::RetrieveMemorySnapshots(
    const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
    const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
    const Targets::TargetDescriptor& targetDescriptor
)
    : addressSpaceDescriptor(addressSpaceDescriptor)
    , memorySegmentDescriptor(memorySegmentDescriptor)
    , targetDescriptor(targetDescriptor)
{}

QString RetrieveMemorySnapshots::brief() const {
    return "Loading \"" + QString::fromStdString(this->memorySegmentDescriptor.name)
        + "\" memory snapshots";
}

void RetrieveMemorySnapshots::run(TargetControllerService& targetControllerService) {
    if (
        this->targetDescriptor.family == Targets::TargetFamily::AVR_8
        && (
            this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::FLASH
            || this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::EEPROM
            || this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::RAM
       )
    ) {
        /*
         * Support for RISC-V targets was introduced in the same release as the new snapshots, so all old snapshots
         * will be for AVR targets. This is why we only need to perform snapshot migration for AVR targets.
         */
        this->migrateOldSnapshotFiles();
    }

    emit this->memorySnapshotsRetrieved(this->getSnapshots());
}

std::vector<MemorySnapshot> RetrieveMemorySnapshots::getSnapshots() {
    using Services::StringService;

    constexpr auto MAX_SNAPSHOTS = 30;

    const auto snapshotDir = QDir{QString::fromStdString(Services::PathService::memorySnapshotsPath())};
    if (!snapshotDir.exists()) {
        return {};
    }

    const auto snapshotFileEntries = snapshotDir.entryInfoList(
        {"*.json"},
        QDir::Files,
        QDir::SortFlag::Time
    );

    auto snapshots = std::vector<MemorySnapshot>{};
    for (const auto& snapshotFileEntry : snapshotFileEntries) {
        auto snapshotFile = QFile{snapshotFileEntry.absoluteFilePath()};

        if (snapshots.size() >= MAX_SNAPSHOTS) {
            Logger::warning(
                "The total number of `" +this->memorySegmentDescriptor.key
                    + "` snapshots exceeds the hard limit of " + std::to_string(MAX_SNAPSHOTS)
                    + ". Only the most recent " + std::to_string(MAX_SNAPSHOTS) + " snapshots will be loaded."
            );
            break;
        }

        try {
            if (!snapshotFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw Exceptions::Exception{"Failed to open snapshot file"};
            }

            auto snapshot = MemorySnapshot{QJsonDocument::fromJson(snapshotFile.readAll()).object(), targetDescriptor};
            if (
                snapshot.addressSpaceKey != QString::fromStdString(this->addressSpaceDescriptor.key)
                || snapshot.memorySegmentKey != QString::fromStdString(this->memorySegmentDescriptor.key)
            ) {
                continue;
            }

            snapshots.emplace_back(std::move(snapshot));

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

void RetrieveMemorySnapshots::migrateOldSnapshotFiles() {
    const auto newSnapshotDirPath = QString::fromStdString(Services::PathService::memorySnapshotsPath());
    auto oldSnapshotDir = QDir{
        this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::FLASH
            ? QString::fromStdString(Services::PathService::memorySnapshotsPath()) + "/flash/"
            : this->memorySegmentDescriptor.type == Targets::TargetMemorySegmentType::EEPROM
                ? QString::fromStdString(Services::PathService::memorySnapshotsPath()) + "/eeprom/"
                : QString::fromStdString(Services::PathService::memorySnapshotsPath()) + "/ram/"
    };

    if (!oldSnapshotDir.exists()) {
        return;
    }

    const auto snapshotFileEntries = oldSnapshotDir.entryInfoList(
        {"*.json"},
        QDir::Files,
        QDir::SortFlag::Time
    );

    for (const auto& snapshotFileEntry : snapshotFileEntries) {
        auto snapshotFile = QFile{snapshotFileEntry.absoluteFilePath()};
        Logger::info("Migrating memory snapshot file \"" + snapshotFileEntry.absoluteFilePath().toStdString() + "\"");

        try {
            if (!snapshotFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                throw Exceptions::Exception{"Failed to open snapshot file"};
            }

            auto snapshot = MemorySnapshot{QJsonDocument::fromJson(snapshotFile.readAll()).object(), targetDescriptor};
            if (
                snapshot.addressSpaceKey != QString::fromStdString(this->addressSpaceDescriptor.key)
                || snapshot.memorySegmentKey != QString::fromStdString(this->memorySegmentDescriptor.key)
            ) {
                continue;
            }

            const auto snapshotFilePath = newSnapshotDirPath + "/" + snapshot.id + ".json";
            Logger::info("Saving new memory snapshot file to \"" + snapshotFilePath.toStdString() + "\"");

            auto outputFile = QFile{snapshotFilePath};
            if (!outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
                Logger::error("Failed to save snapshot - cannot open " + snapshotFilePath.toStdString());
                return;
            }

            outputFile.write(QJsonDocument{snapshot.toJson()}.toJson(QJsonDocument::JsonFormat::Compact));
            outputFile.close();

            snapshotFile.remove();

        } catch (const Exceptions::Exception& exception) {
            Logger::error(
                "Failed to migrate snapshot file " + snapshotFileEntry.absoluteFilePath().toStdString() + " - "
                    + exception.getMessage()
            );
        }

        snapshotFile.close();
    }

//    if (oldSnapshotDir.isEmpty()) {
//        Logger::info("Deleting old snapshot directory: \"" + oldSnapshotDir.absolutePath().toStdString() + "\"");
//        oldSnapshotDir.removeRecursively();
//    }
}
