#pragma once

#include <cstdint>
#include <QString>
#include <utility>
#include <vector>
#include <QJsonObject>

#include "src/Targets/TargetMemory.hpp"
#include "src/Services/DateTimeService.hpp"

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

struct MemorySnapshot
{
public:
    QString id;
    QString name;
    QString description;
    Targets::TargetMemoryType memoryType;
    Targets::TargetMemoryBuffer data;
    Targets::TargetProgramCounter programCounter;
    Targets::TargetStackPointer stackPointer;
    QDateTime createdDate = Services::DateTimeService::currentDateTime();

    std::vector<FocusedMemoryRegion> focusedRegions;
    std::vector<ExcludedMemoryRegion> excludedRegions;

    MemorySnapshot(
        const QString& name,
        const QString& description,
        Targets::TargetMemoryType memoryType,
        const Targets::TargetMemoryBuffer& data,
        Targets::TargetProgramCounter programCounter,
        Targets::TargetStackPointer stackPointer,
        const std::vector<FocusedMemoryRegion>& focusedRegions,
        const std::vector<ExcludedMemoryRegion>& excludedRegions
    );

    MemorySnapshot(const QJsonObject& jsonObject);

    QJsonObject toJson() const;

    bool isCompatible(const Targets::TargetMemoryDescriptor& memoryDescriptor) const;
    std::set<Targets::TargetMemoryAddress> excludedAddresses() const;

    virtual ~MemorySnapshot() = default;

    MemorySnapshot(const MemorySnapshot& other) = default;
    MemorySnapshot(MemorySnapshot&& other) = default;

    MemorySnapshot& operator = (const MemorySnapshot& other) = default;
    MemorySnapshot& operator = (MemorySnapshot&& other) = default;
};
