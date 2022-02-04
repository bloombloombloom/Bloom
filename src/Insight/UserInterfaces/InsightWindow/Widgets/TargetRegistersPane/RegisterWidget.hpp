#pragma once

#include <QWidget>
#include <unordered_set>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QSize>
#include <QEvent>
#include <QString>

#include "ItemWidget.hpp"
#include "src/Insight/InsightWorker/InsightWorker.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegisterInspector/TargetRegisterInspectorWindow.hpp"


namespace Bloom::Widgets
{
    class RegisterWidget: public ItemWidget
    {
        Q_OBJECT

    public:
        Targets::TargetRegisterDescriptor descriptor;
        QString searchKeywords;
        std::optional<Targets::TargetRegister> currentRegister;

        RegisterWidget(
            Targets::TargetRegisterDescriptor descriptor,
            InsightWorker& insightWorker,
            QWidget *parent
        );

        [[nodiscard]] QSize minimumSizeHint() const override {
            auto size = QSize(this->parentWidget()->width(), 25);
            return size;
        }

        void setRegisterValue(const Targets::TargetRegister& targetRegister);
        void clearInlineValue();

        void contextMenuEvent(QContextMenuEvent* event) override;

    public slots:
        void openInspectionWindow();
        void refreshValue();
        void copyName();
        void copyValueHex();
        void copyValueDecimal();
        void copyValueBinary();

    private:
        InsightWorker& insightWorker;
        QHBoxLayout* layout = new QHBoxLayout(this);
        SvgWidget* registerIcon = new SvgWidget(this);
        QLabel* nameLabel = new QLabel(this);
        QLabel* valueLabel = new QLabel(this);

        // Context-menu actions
        QAction* openInspectionWindowAction = new QAction("Inspect", this);
        QAction* refreshValueAction = new QAction("Refresh Value", this);
        QAction* copyValueNameAction = new QAction("Copy Register Name", this);
        QAction* copyValueHexAction = new QAction("Copy Hexadecimal Value", this);
        QAction* copyValueDecimalAction = new QAction("Copy Decimal Value", this);
        QAction* copyValueBinaryAction = new QAction("Copy Binary Value", this);

        TargetRegisterInspectorWindow* inspectWindow = nullptr;

        void postSetSelected(bool selected) override;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;

    private slots:
        void onTargetStateChange(Targets::TargetState newState);
    };
}
