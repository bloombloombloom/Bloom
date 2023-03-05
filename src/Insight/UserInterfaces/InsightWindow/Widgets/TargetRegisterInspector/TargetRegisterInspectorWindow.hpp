#pragma once

#include <QWidget>
#include <QLineEdit>
#include <set>
#include <QSize>
#include <QString>
#include <QEvent>
#include <QScrollArea>
#include <optional>

#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetState.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PushButton.hpp"
#include "BitsetWidget/BitsetWidget.hpp"
#include "RegisterHistoryWidget/RegisterHistoryWidget.hpp"

namespace Bloom::Widgets
{
    class TargetRegisterInspectorWindow: public QWidget
    {
        Q_OBJECT

    public:
        TargetRegisterInspectorWindow(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            Targets::TargetState currentTargetState,
            QWidget* parent = nullptr
        );

        static bool registerSupported(const Targets::TargetRegisterDescriptor& descriptor);

        void setValue(const Targets::TargetMemoryBuffer& registerValue);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        Targets::TargetRegisterDescriptor registerDescriptor;
        Targets::TargetMemoryBuffer registerValue;

        QWidget* container = nullptr;
        Label* registerNameLabel = nullptr;
        Label* registerDescriptionLabel = nullptr;

        QScrollArea* contentContainer = nullptr;
        RegisterHistoryWidget* registerHistoryWidget = nullptr;

        QWidget* registerValueContainer = nullptr;
        QLineEdit* registerValueTextInput = nullptr;
        QWidget* registerValueBitsetWidgetContainer = nullptr;
        std::vector<BitsetWidget*> bitsetWidgets;

        PushButton* refreshValueButton = nullptr;
        PushButton* applyButton = nullptr;
        PushButton* helpButton = nullptr;
        PushButton* closeButton = nullptr;

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
