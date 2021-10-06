#pragma once

#include <QtCore>
#include <QMainWindow>
#include <QtUiTools/QtUiTools>
#include <memory>

namespace Bloom
{
    class AboutWindow: public QObject
    {
        Q_OBJECT

    public:
        explicit AboutWindow(QWidget* parent);

        void show() {
            if (this->windowWidget != nullptr) {
                this->windowWidget->move(
                    this->windowWidget->parentWidget()->geometry().center() - this->windowWidget->rect().center()
                );
                this->windowWidget->show();
            }
        }

    private:
        QWidget* windowWidget = nullptr;
    };
}
