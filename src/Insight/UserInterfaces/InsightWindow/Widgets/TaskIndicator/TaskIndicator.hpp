#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <unordered_map>
#include <QEvent>
#include <QMouseEvent>

#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TaskWindow/TaskWindow.hpp"

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
        void mousePressEvent(QMouseEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private:
        std::unordered_map<InsightWorkerTask::IdType, QSharedPointer<InsightWorkerTask>> activeTasksById;
        bool hovered = false;

        TaskWindow* taskWindow = nullptr;

        void onTaskQueued(QSharedPointer<InsightWorkerTask> task);
        void onTaskFinished(QSharedPointer<InsightWorkerTask> task);

        void updateToolTip();
    };
}
