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
#include "src/Targets/TargetState.hpp"

namespace Widgets
{
    class CreateSnapshotWindow: public QWidget
    {
        Q_OBJECT

    public:
        explicit CreateSnapshotWindow(
            Targets::TargetMemoryType memoryType,
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
        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

        bool captureEnabled();
        void resetForm();

        void issueCaptureRequest();
    };
}
