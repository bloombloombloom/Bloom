#include "TaskProgressIndicator.hpp"

#include <QPainter>
#include <QColor>
#include <algorithm>

namespace Widgets
{
    TaskProgressIndicator::TaskProgressIndicator(QWidget* parent)
        : QWidget(parent)
    {
        this->setObjectName("task-progress-indicator");
        this->setFixedHeight(26);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        this->clearCompletedTasksTimer->setSingleShot(true);
        this->clearCompletedTasksTimer->setInterval(2500);

        QObject::connect(
            this->clearCompletedTasksTimer,
            &QTimer::timeout,
            this,
            &TaskProgressIndicator::clearCompletedTasks
        );
    }

    void TaskProgressIndicator::addTask(const QSharedPointer<InsightWorkerTask>& task) {
        if (this->tasksById.contains(task->id)) {
            return;
        }

        this->tasksById.emplace(task->id, task);

        QObject::connect(
            task.get(),
            &InsightWorkerTask::started,
            this,
            &TaskProgressIndicator::refreshProgressPercentage
        );

        QObject::connect(
            task.get(),
            &InsightWorkerTask::finished,
            this,
            &TaskProgressIndicator::refreshProgressPercentage
        );

        QObject::connect(
            task.get(),
            &InsightWorkerTask::failed,
            this,
            &TaskProgressIndicator::refreshProgressPercentage
        );

        QObject::connect(
            task.get(),
            &InsightWorkerTask::finished,
            this,
            &TaskProgressIndicator::onTaskFinished
        );

        QObject::connect(
            task.get(),
            &InsightWorkerTask::progressUpdate,
            this,
            &TaskProgressIndicator::refreshProgressPercentage
        );

        this->refreshProgressPercentage();
    }

    void TaskProgressIndicator::paintEvent(QPaintEvent* event) {
        const auto taskCount = this->tasksById.size();

        if (taskCount < 1) {
            return;
        }

        auto painter = QPainter(this);

        const auto size = this->size();

        static constexpr auto backgroundBarColor = QColor(0x8E, 0x8B, 0x83, 40);
        static constexpr auto barColor = QColor(0x8E, 0x8B, 0x83, 90);
        static constexpr auto fontColor = QColor(0x99, 0x9a, 0x9d);

        static const auto font = QFont("'Ubuntu', sans-serif", 9);
        painter.setFont(font);
        painter.setPen(fontColor);

        static constexpr auto barHeight = 3;
        const auto barYPosition = size.height() - barHeight - 3;

        const auto percentage = std::max(static_cast<int>(this->progressPercentage), 2);

        if (taskCount == 1) {
            auto* task = this->tasksById.begin()->second.get();
            const auto status = QString(
                task->state == InsightWorkerTaskState::FAILED
                    ? " - Failed"
                    : task->state == InsightWorkerTaskState::COMPLETED
                        ? " - Completed"
                        : ""
            );

            painter.drawText(
                0,
                barYPosition - 5,
                task->brief() + status
            );

        } else {
            painter.drawText(
                0,
                barYPosition - 5,
                "Processing " + QString::number(taskCount) + " tasks"
            );
        }

        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(backgroundBarColor);

        painter.drawRect(
            0,
            barYPosition,
            size.width(),
            barHeight
        );

        painter.setBrush(barColor);

        painter.drawRect(
            0,
            barYPosition,
            static_cast<int>(static_cast<float>(size.width()) * (static_cast<float>(percentage) / 100)),
            barHeight
        );
    }

    void TaskProgressIndicator::clearCompletedTasks() {
        std::erase_if(
            this->tasksById,
            [] (const decltype(this->tasksById)::value_type& pair) {
                return
                    pair.second->state == InsightWorkerTaskState::COMPLETED
                    || pair.second->state == InsightWorkerTaskState::FAILED;
            }
        );

        this->refreshProgressPercentage();
    }

    void TaskProgressIndicator::refreshProgressPercentage() {
        unsigned int percentageSum = 0;

        for (const auto& [taskId, task] : this->tasksById) {
            percentageSum += task->progressPercentage;
        }

        this->progressPercentage = static_cast<std::uint8_t>(
            static_cast<float>(percentageSum) / static_cast<float>(this->tasksById.size())
        );

        this->update();
    }

    void TaskProgressIndicator::onTaskFinished() {
        if (this->tasksById.size() > 1) {
            this->clearCompletedTasks();
            return;
        }

        this->clearCompletedTasksTimer->start();
    }
}
