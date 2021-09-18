#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <set>
#include <QSize>
#include <QString>
#include <QEvent>
#include <optional>

#include "src/Targets/TargetRegister.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetState.hpp"

#include "Item.hpp"
#include "CurrentItem.hpp"
#include "RegisterHistoryItem.hpp"

#include "src/Insight/InsightWorker/InsightWorker.hpp"

namespace Bloom::Widgets
{
    class RegisterHistoryWidget: public QWidget
    {
    Q_OBJECT
    private:
        Targets::TargetRegisterDescriptor registerDescriptor;
        InsightWorker& insightWorker;

        QWidget* container = nullptr;
        QWidget* itemContainer = nullptr;
        QVBoxLayout* itemContainerLayout = nullptr;

        Targets::TargetState targetState = Targets::TargetState::UNKNOWN;
        CurrentItem* currentItem = nullptr;
        Item* selectedItemWidget = nullptr;

    private slots:
        void onTargetStateChanged(Targets::TargetState newState);
        void onItemSelectionChange(Item* newlySelectedWidget);
        void onRegistersWritten(Targets::TargetRegisters targetRegisters, const QDateTime& changeDate);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    public:
        RegisterHistoryWidget(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            const Targets::TargetMemoryBuffer& currentValue,
            InsightWorker& insightWorker,
            QWidget* parent
        );

        void updateCurrentItemValue(const Targets::TargetMemoryBuffer& registerValue);
        void selectCurrentItem();

        bool isCurrentItemSelected() {
            return this->selectedItemWidget != nullptr && this->selectedItemWidget == this->currentItem;
        }

        void addItem(const Targets::TargetMemoryBuffer& registerValue, const QDateTime& changeDate);

    signals:
        void historyItemSelected(const Targets::TargetMemoryBuffer& registerValue);

    };
}
