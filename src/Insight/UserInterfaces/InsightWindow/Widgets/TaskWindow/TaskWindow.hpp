#pragma once

#include <QWidget>
#include <QSharedPointer>
#include <QVBoxLayout>
#include <unordered_map>
#include <QEvent>

#include "Task.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"
#include "src/Insight/InsightWorker/Tasks/InsightWorkerTask.hpp"

namespace Bloom::Widgets
{
    class TaskWindow: public QWidget
    {
        Q_OBJECT

    public:
        TaskWindow(QWidget* parent);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        QWidget* container = nullptr;
        QVBoxLayout* taskWidgetLayout = nullptr;

        Label* taskPlaceholderLabel = nullptr;

        std::unordered_map<InsightWorkerTask::IdType, Task*> taskWidgetsByTaskId;

        void onTaskQueued(const QSharedPointer<InsightWorkerTask>& task);
    };
}
