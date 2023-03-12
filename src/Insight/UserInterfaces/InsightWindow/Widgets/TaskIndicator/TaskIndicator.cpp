#include "TaskIndicator.hpp"

#include <QPainter>
#include <QColor>

#include "src/Insight/InsightSignals.hpp"

namespace Bloom::Widgets
{
    TaskIndicator::TaskIndicator(QWidget* parent)
        : QWidget(parent)
    {
        this->setObjectName("task-indicator");
        this->setFixedSize(50, 26);

        auto* insightSignals = InsightSignals::instance();

        QObject::connect(
            insightSignals,
            &InsightSignals::taskQueued,
            this,
            &TaskIndicator::onTaskQueued
        );

        QObject::connect(
            insightSignals,
            &InsightSignals::taskProcessed,
            this,
            &TaskIndicator::onTaskFinished
        );

        this->updateToolTip();
    }

    void TaskIndicator::enterEvent(QEnterEvent* event) {
        this->hovered = true;
        this->update();
    }

    void TaskIndicator::leaveEvent(QEvent* event) {
        this->hovered = false;
        this->update();
    }

    void TaskIndicator::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);
        painter.setPen(Qt::PenStyle::NoPen);

        const auto size = this->size();

        static constexpr auto activeItemColor = QColor(0x7C, 0x5D, 0x3B);
        static constexpr auto inactiveItemColor = QColor(0x83, 0x83, 0x82);
        static constexpr auto itemSize = QSize(3, 3);

        static constexpr auto rowCount = 3;
        static constexpr auto itemsPerRow = 6;

        static constexpr auto itemRightMargin = 2;
        static constexpr auto itemBottomMargin = 2;

        if (this->hovered) {
            static constexpr auto hoveredBackgroundColor = QColor(0x45, 0x45, 0x41);
            painter.setBrush(hoveredBackgroundColor);
            painter.drawRect(0, 1, size.width(), size.height() - 1);
        }

        const auto activeTaskCount = this->activeTasksById.size();

        // Position the items at the center of the drawing space
        auto startX = (size.width() / 2) - (((itemSize.width() + itemRightMargin) * itemsPerRow - itemRightMargin) / 2);
        auto startY = (size.height() / 2) - (((itemSize.height() + itemBottomMargin) * rowCount - itemBottomMargin) / 2);

        // The itemIndex goes in reverse
        auto itemIndex = rowCount * itemsPerRow;

        for (auto rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
            for (auto rowItemIndex = 0; rowItemIndex < itemsPerRow; ++rowItemIndex) {
                painter.setBrush(itemIndex <= activeTaskCount ? activeItemColor : inactiveItemColor);

                painter.drawRect(
                    startX + ((itemSize.width() + itemRightMargin) * rowItemIndex),
                    startY + ((itemSize.height() + itemBottomMargin) * rowIndex),
                    itemSize.width(),
                    itemSize.height()
                );

                --itemIndex;
            }
        }
    }

    void TaskIndicator::onTaskQueued(QSharedPointer<InsightWorkerTask> task) {
        this->activeTasksById[task->id] = task;
        this->updateToolTip();
        this->update();
    }

    void TaskIndicator::onTaskFinished(QSharedPointer<InsightWorkerTask> task) {
        this->activeTasksById.erase(task->id);
        this->updateToolTip();
        this->update();
    }

    void TaskIndicator::updateToolTip() {
        const auto taskCount = this->activeTasksById.size();
        if (taskCount > 0) {
            this->setToolTip("Processing " + QString::number(taskCount) + (taskCount == 1 ? " task" : " tasks"));
            return;
        }

        this->setToolTip("Open Task Window");
    }
}
