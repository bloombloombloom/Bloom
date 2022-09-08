#pragma once

#include <QObject>
#include <QDateTime>

#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom
{
    /**
     * Singleton class providing global signals to all Insight widgets that require them. The signals are emitted via
     * the Insight class and InsightWorkerTasks.
     */
    class InsightSignals: public QObject
    {
        Q_OBJECT

    public:
        static InsightSignals* instance() {
            static auto instance = InsightSignals();
            return &instance;
        }

        InsightSignals(const InsightSignals&) = delete;
        void operator = (const InsightSignals&) = delete;

    signals:
        void taskQueued();
        void taskProcessed();

        void targetStateUpdated(Bloom::Targets::TargetState newState);
        void targetReset();
        void targetRegistersWritten(const Bloom::Targets::TargetRegisters& targetRegisters, const QDateTime& timestamp);
        void targetControllerSuspended();
        void targetControllerResumed(const Bloom::Targets::TargetDescriptor& targetDescriptor);
        void programmingModeEnabled();
        void programmingModeDisabled();

    private:
        InsightSignals() = default;
    };
}
