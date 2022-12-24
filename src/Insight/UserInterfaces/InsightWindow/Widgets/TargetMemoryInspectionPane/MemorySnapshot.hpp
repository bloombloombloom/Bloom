#pragma once

#include <cstdint>
#include <QString>
#include <utility>
#include <vector>
#include <QUuid>
#include <QJsonObject>

#include "src/Targets/TargetMemory.hpp"
#include "src/Helpers/DateTime.hpp"

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

namespace Bloom
{
    struct MemorySnapshot
    {
    public:
        QString id = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
        QString name;
        QString description;
        Targets::TargetMemoryType memoryType;
        Targets::TargetMemoryBuffer data;
        Targets::TargetProgramCounter programCounter;
        QDateTime createdDate = DateTime::currentDateTime();

        std::vector<FocusedMemoryRegion> focusedRegions;
        std::vector<ExcludedMemoryRegion> excludedRegions;

        MemorySnapshot(
            const QString& name,
            const QString& description,
            Targets::TargetMemoryType memoryType,
            const Targets::TargetMemoryBuffer& data,
            Targets::TargetProgramCounter programCounter,
            const std::vector<FocusedMemoryRegion>& focusedRegions,
            const std::vector<ExcludedMemoryRegion>& excludedRegions
        );

        MemorySnapshot(const QJsonObject& jsonObject);

        QJsonObject toJson() const;

        bool isCompatible(const Targets::TargetMemoryDescriptor& memoryDescriptor) const;

        virtual ~MemorySnapshot() = default;

        MemorySnapshot(const MemorySnapshot& other) = default;
        MemorySnapshot(MemorySnapshot&& other) = default;

        MemorySnapshot& operator = (const MemorySnapshot& other) = default;
        MemorySnapshot& operator = (MemorySnapshot&& other) = default;
    };
}
