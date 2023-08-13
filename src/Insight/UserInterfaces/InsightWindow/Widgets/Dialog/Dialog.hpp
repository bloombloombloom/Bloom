#pragma once

#include <QDialog>
#include <QHBoxLayout>
#include <QShowEvent>
#include <QPushButton>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Widgets
{
    class Dialog: public QDialog
    {
        Q_OBJECT

    public:
        Dialog(
            const QString& windowTitle,
            const QString& text,
            QWidget* parent
        );

    protected:
        void showEvent(QShowEvent* event) override;
        void addActionButton(QPushButton* button);

    private:
        QWidget* container = nullptr;
        Label* textLabel = nullptr;
        QHBoxLayout* actionLayout = nullptr;
    };
}
