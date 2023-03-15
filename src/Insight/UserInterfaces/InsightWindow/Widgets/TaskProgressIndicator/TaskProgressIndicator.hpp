#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <cstdint>
#include <unordered_map>
#include <QEvent>

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

namespace Bloom::Widgets
{
    class TaskProgressIndicator: public QWidget
    {
        Q_OBJECT

    public:
        TaskProgressIndicator(const QSharedPointer<InsightWorkerTask>& task, QWidget* parent);

    signals:
        void taskComplete();

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        QSharedPointer<InsightWorkerTask> task;
        int taskProgressPercentage = 0;
        bool taskFailed = false;

        void onTaskProgressUpdate(int progressPercentage);
        void onTaskFailed();
        void onTaskFinished();
    };
}
