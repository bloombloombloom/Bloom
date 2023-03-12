#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <unordered_map>
#include <QEvent>

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

namespace Bloom::Widgets
{
    class TaskIndicator: public QWidget
    {
        Q_OBJECT

    public:
        TaskIndicator(QWidget* parent);

    protected:
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private:
        std::unordered_map<InsightWorkerTask::IdType, QSharedPointer<InsightWorkerTask>> activeTasksById;
        bool hovered = false;

        void onTaskQueued(QSharedPointer<InsightWorkerTask> task);
        void onTaskFinished(QSharedPointer<InsightWorkerTask> task);

        void updateToolTip();
    };
}
