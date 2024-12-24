#pragma once

#include <QObject>
#include <QDateTime>
#include <QSharedPointer>

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

#include "InsightWorker/Tasks/InsightWorkerTask.hpp"

/**
 * Singleton class providing global signals to all Insight widgets that require them. The signals are emitted via
 * the Insight class and InsightWorkerTasks.
 */
class InsightSignals: public QObject
{
    Q_OBJECT

public:
    static InsightSignals* instance() {
        static auto instance = InsightSignals{};
        return &instance;
    }

    InsightSignals(const InsightSignals&) = delete;
    void operator = (const InsightSignals&) = delete;

signals:
    void taskQueued(QSharedPointer<InsightWorkerTask> task);
    void taskProcessed(QSharedPointer<InsightWorkerTask> task);

    void targetStateUpdated(Targets::TargetState newState, Targets::TargetState previousState);
    void targetReset();
    void targetRegistersWritten(const Targets::TargetRegisterDescriptorAndValuePairs& targetRegisters, const QDateTime& timestamp);
    void targetMemoryWritten(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddressRange addressRange
    );
    void programmingModeEnabled();
    void programmingModeDisabled();

private:
    InsightSignals() = default;
};
