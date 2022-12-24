#pragma once

#include <QVBoxLayout>
#include <QComboBox>
#include <map>
#include <QString>
#include <QStringList>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ClickableWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/MemorySnapshot.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Widgets
{
    class MemorySnapshotItem: public ClickableWidget
    {
        Q_OBJECT

    public:
        const MemorySnapshot& memorySnapshot;

        MemorySnapshotItem(
            const MemorySnapshot& memorySnapshot,
            QWidget *parent
        );

        void setSelected(bool selected);

    signals:
        void selected(MemorySnapshotItem*);

    private:
        QVBoxLayout* layout = new QVBoxLayout(this);
        Label* nameLabel = new Label(this);
        Label* programCounterLabel = new Label(this);
        Label* createdDateLabel = new Label(this);
    };
}
