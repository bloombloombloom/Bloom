#pragma once

#include <QWidget>
#include <QShowEvent>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TextInput.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PushButton.hpp"

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetState.hpp"

namespace Widgets
{
    class CreateSnapshotWindow: public QWidget
    {
        Q_OBJECT

    public:
        explicit CreateSnapshotWindow(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const Targets::TargetState& targetState,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            const bool& staleData,
            QWidget* parent = nullptr
        );

        void refreshForm();

    signals:
        void snapshotCaptureRequested(
            const QString& name,
            const QString& description,
            bool captureFocusedRegions,
            bool refreshBeforeCapture
        );

    protected:
        void showEvent(QShowEvent* event) override;
        void keyPressEvent(QKeyEvent* event) override;

    private:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        const Targets::TargetState& targetState;

        QWidget* container = nullptr;
        TextInput* nameInput = nullptr;
        QPlainTextEdit* descriptionInput = nullptr;

        QCheckBox* includeFocusedRegionsInput = nullptr;
        QCheckBox* captureDirectlyFromTargetInput = nullptr;

        QWidget* staleDataWarning = nullptr;

        PushButton* captureButton = nullptr;
        PushButton* closeButton = nullptr;

        const std::optional<Targets::TargetMemoryBuffer>& data;
        const bool& staleData;

        bool captureEnabled();
        void resetForm();

        void issueCaptureRequest();
    };
}
