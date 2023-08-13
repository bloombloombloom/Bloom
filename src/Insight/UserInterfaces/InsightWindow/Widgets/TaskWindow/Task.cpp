#include "Task.hpp"

#include <QPainter>
#include <QColor>
#include <QTimer>

namespace Widgets
{
    Task::Task(
        const QSharedPointer<InsightWorkerTask>& task,
        QWidget* parent
    )
        : QWidget(parent)
        , task(task)
    {
        this->setObjectName("task");
        this->setFixedHeight(80);
        this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        this->setContentsMargins(15, 0, 15, 0);

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::started,
            this,
            &Task::onTaskStateChanged
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::finished,
            this,
            &Task::onTaskStateChanged
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::failed,
            this,
            &Task::onTaskStateChanged
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::finished,
            this,
            &Task::onTaskFinished
        );

        QObject::connect(
            this->task.get(),
            &InsightWorkerTask::progressUpdate,
            this,
            &Task::onTaskProgressUpdate
        );
    }

    void Task::paintEvent(QPaintEvent* event) {
        auto painter = QPainter(this);

        const auto margins = this->contentsMargins();
        const auto size = this->size();

        static constexpr auto backgroundBarColor = QColor(0x8E, 0x8B, 0x83, 40);
        static constexpr auto barColor = QColor(0x8E, 0x8B, 0x83, 90);
        static constexpr auto fontColor = QColor(0x99, 0x9a, 0x9d);
        static constexpr auto statusFontColor = QColor(0x99, 0x9a, 0x9d, 200);

        static auto font = QFont("'Ubuntu', sans-serif");
        font.setPixelSize(14);

        static auto statusFont = QFont("'Ubuntu', sans-serif");
        statusFont.setPixelSize(12);

        painter.setFont(font);
        painter.setPen(fontColor);

        static constexpr auto barHeight = 5;
        const auto barYPosition = 35;

        painter.drawText(
            margins.left(),
            barYPosition - 12,
            this->task->brief()
        );

        const auto status = QString(
            this->task->state == InsightWorkerTaskState::FAILED
                ? "Failed"
                : this->task->state == InsightWorkerTaskState::COMPLETED
                    ? "Completed"
                    : this->task->state == InsightWorkerTaskState::STARTED
                        ? "Running"
                        : "Queued"
        );

        painter.setFont(statusFont);
        painter.setPen(statusFontColor);
        painter.drawText(
            margins.left(),
            barYPosition + barHeight + 20,
            status
        );

        painter.setPen(Qt::PenStyle::NoPen);
        painter.setBrush(backgroundBarColor);

        painter.drawRect(
            margins.left(),
            barYPosition,
            size.width() - margins.left() - margins.right(),
            barHeight
        );

        painter.setBrush(barColor);

        painter.drawRect(
            margins.left(),
            barYPosition,
            static_cast<int>(
                static_cast<float>(size.width() - margins.left() - margins.right())
                    * (static_cast<float>(this->task->progressPercentage) / 100)
            ),
            barHeight
        );

        painter.setPen(backgroundBarColor);

        painter.drawLine(
            0,
            size.height() - 1,
            size.width(),
            size.height() - 1
        );
    }

    void Task::onTaskProgressUpdate() {
        this->update();
    }

    void Task::onTaskStateChanged() {
        this->update();
    }

    void Task::onTaskFinished() {
        emit this->taskComplete(this->task->id);
    }
}
