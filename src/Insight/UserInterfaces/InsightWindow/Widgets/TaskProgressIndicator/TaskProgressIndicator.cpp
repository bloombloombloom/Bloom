#include "TaskProgressIndicator.hpp"

#include <QPainter>
#include <QColor>
#include <algorithm>
#include <QTimer>

namespace Bloom::Widgets
{
    TaskProgressIndicator::TaskProgressIndicator(
        const QSharedPointer<InsightWorkerTask>& task,
        QWidget* parent
    )
        : QWidget(parent)
        , task(task)
    {
        this->setObjectName("task-progress-indicator");
        this->setFixedHeight(26);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::progressUpdate,
            this,
            &TaskProgressIndicator::onTaskProgressUpdate
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::failed,
            this,
            &TaskProgressIndicator::onTaskFailed
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::finished,
            this,
            &TaskProgressIndicator::onTaskFinished
        );
    }

    void TaskProgressIndicator::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);

        const auto size = this->size();

        static constexpr auto backgroundBarColor = QColor(0x8E, 0x8B, 0x83, 40);
        static constexpr auto barColor = QColor(0x8E, 0x8B, 0x83, 90);
        static constexpr auto fontColor = QColor(0x99, 0x9a, 0x9d);

        static auto font = QFont("'Ubuntu', sans-serif", 9);
        painter.setFont(font);
        painter.setPen(fontColor);

        static constexpr auto barHeight = 3;
        const auto barYPosition = size.height() - barHeight - 3;

        const auto percentage = std::max(this->taskProgressPercentage, 2);

        painter.drawText(
            0,
            barYPosition - 5,
            this->task->brief() + QString(this->taskFailed ? " - Failed" : (percentage == 100 ? " - Completed" : ""))
        );

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

    void TaskProgressIndicator::onTaskProgressUpdate(int taskProgressPercentage) {
        this->taskProgressPercentage = taskProgressPercentage;
        this->update();
    }

    void TaskProgressIndicator::onTaskFailed() {
        this->taskFailed = true;
        this->update();
    }

    void TaskProgressIndicator::onTaskFinished() {
        this->taskProgressPercentage = 100;
        this->update();

        auto* finishedSignalTimer = new QTimer();
        finishedSignalTimer->setSingleShot(true);
        finishedSignalTimer->setInterval(2500);

        QObject::connect(finishedSignalTimer, &QTimer::timeout, this, [this] {
            emit this->taskComplete();
        });

        finishedSignalTimer->start();
    }
}
