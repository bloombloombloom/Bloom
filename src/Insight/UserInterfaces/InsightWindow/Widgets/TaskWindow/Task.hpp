#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <QEvent>

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

namespace Bloom::Widgets
{
    class Task: public QWidget
    {
        Q_OBJECT

    public:
        Task(const QSharedPointer<InsightWorkerTask>& task, QWidget* parent);

    signals:
        void taskComplete(InsightWorkerTask::IdType taskId);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        QSharedPointer<InsightWorkerTask> task;

        void onTaskProgressUpdate();
        void onTaskStateChanged();
        void onTaskFinished();
    };
}
