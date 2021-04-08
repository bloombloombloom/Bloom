#pragma once

#include <QtCore>
#include <QMainWindow>
#include <QtUiTools/QtUiTools>
#include <memory>
#include <optional>

#include "AboutWindow.hpp"
#include "src/ApplicationConfig.hpp"
#include "TargetWidgets/TargetPackageWidget.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetVariant.hpp"

namespace Bloom
{
    using InsightTargetWidgets::TargetPackageWidget;
    using InsightTargetWidgets::TargetPinWidget;

    using Targets::TargetDescriptor;
    using Targets::TargetVariant;
    using Targets::TargetPackage;
    using Targets::TargetState;
    using Targets::TargetPinState;

    class InsightWindow: public QObject
    {
    Q_OBJECT
    private:
        TargetDescriptor targetDescriptor;
        TargetState targetState = TargetState::UNKNOWN;

        QWidget* mainWindowWidget = nullptr;
        AboutWindow* aboutWindowWidget = nullptr;
        QMenuBar* mainMenuBar = nullptr;
        QMenu* variantMenu = nullptr;

        QWidget* header = nullptr;
        QToolButton* refreshIoInspectionButton = nullptr;

        QWidget* ioContainerWidget = nullptr;
        QWidget* ioUnavailableWidget = nullptr;
        TargetPackageWidget* targetPackageWidget = nullptr;

        QWidget* footer = nullptr;
        QLabel* targetStatusLabel = nullptr;
        QLabel* programCounterValueLabel = nullptr;

        std::vector<TargetVariant> supportedVariants;
        const TargetVariant* selectedVariant = nullptr;
        bool uiDisabled = false;

        bool isVariantSupported(const TargetVariant& variant);

        void selectVariant(const TargetVariant* variant);

        void toggleUi(bool disable) {
            this->uiDisabled = disable;

            if (this->refreshIoInspectionButton != nullptr) {
                this->refreshIoInspectionButton->setDisabled(disable);
                this->refreshIoInspectionButton->repaint();
            }

            if (this->ioContainerWidget != nullptr) {
                this->ioContainerWidget->setDisabled(disable);
                this->ioContainerWidget->repaint();
            }
        }

    public:
        InsightWindow() = default;

        void init(
            QApplication& application,
            Targets::TargetDescriptor targetDescriptor,
            const InsightConfig& config,
            const TargetConfig& targetConfig
        );

        void show();

    public slots:
        void onTargetPinStatesUpdate(int variantId, Bloom::Targets::TargetPinStateMappingType pinStatesByNumber);
        void onTargetStateUpdate(TargetState newState);
        void onTargetProgramCounterUpdate(quint32 programCounter);
        void onTargetIoPortsUpdate();
        void close();
        void openReportIssuesUrl();
        void openGettingStartedUrl();
        void openAboutWindow();
        void togglePinIoState(TargetPinWidget* pinWidget);

    signals:
        void refreshTargetPinStates(int variantId);
        void setTargetPinState(
            int variantId,
            Bloom::Targets::TargetPinDescriptor pinDescriptor,
            Bloom::Targets::TargetPinState pinState
        );
    };
}
