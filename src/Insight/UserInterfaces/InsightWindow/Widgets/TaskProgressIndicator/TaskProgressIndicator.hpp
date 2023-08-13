#pragma once

#include <QWidget>
#include <unordered_map>
#include <QSharedPointer>
#include <cstdint>
#include <QEvent>
#include <QTimer>

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

namespace Widgets
{
    class TaskProgressIndicator: public QWidget
    {
        Q_OBJECT

    public:
        TaskProgressIndicator(QWidget* parent);

        void addTask(const QSharedPointer<InsightWorkerTask>& task);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        std::unordered_map<InsightWorkerTask::IdType, QSharedPointer<InsightWorkerTask>> tasksById;

        std::uint8_t progressPercentage = 0;
        QTimer* clearCompletedTasksTimer = new QTimer(this);

        void clearCompletedTasks();
        void refreshProgressPercentage();
        void onTaskFinished();
    };
}
