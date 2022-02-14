#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <set>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QScrollArea>
#include <optional>

#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetState.hpp"

#include "BitsetWidget/BitsetWidget.hpp"
#include "RegisterHistoryWidget/RegisterHistoryWidget.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

namespace Bloom::Widgets
{
    class TargetRegisterInspectorWindow: public QWidget
    {
        Q_OBJECT

    public:
        TargetRegisterInspectorWindow(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            InsightWorker& insightWorker,
            Targets::TargetState currentTargetState,
            const std::optional<Targets::TargetMemoryBuffer>& registerValue = std::nullopt,
            QWidget* parent = nullptr
        );

        static bool registerSupported(const Targets::TargetRegisterDescriptor& descriptor);

        void setValue(const Targets::TargetMemoryBuffer& registerValue);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        Targets::TargetRegisterDescriptor registerDescriptor;
        Targets::TargetMemoryBuffer registerValue;
        InsightWorker& insightWorker;

        QWidget* container = nullptr;
        QLabel* registerNameLabel = nullptr;
        QLabel* registerDescriptionLabel = nullptr;

        QScrollArea* contentContainer = nullptr;
        RegisterHistoryWidget* registerHistoryWidget = nullptr;

        QWidget* registerValueContainer = nullptr;
        QLineEdit* registerValueTextInput = nullptr;
        QWidget* registerValueBitsetWidgetContainer = nullptr;
        std::vector<BitsetWidget*> bitsetWidgets;

        QPushButton* refreshValueButton = nullptr;
        QPushButton* applyButton = nullptr;
        QPushButton* helpButton = nullptr;
        QPushButton* closeButton = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    private slots:
        void onValueTextInputChanged(QString text);
        void onTargetStateChanged(Targets::TargetState newState);
        void onHistoryItemSelected(const Targets::TargetMemoryBuffer& selectedRegisterValue);
        void updateRegisterValueInputField();
        void updateRegisterValueBitsetWidgets();
        void updateValue();
        void refreshRegisterValue();
        void applyChanges();
        void openHelpPage();
    };
}
