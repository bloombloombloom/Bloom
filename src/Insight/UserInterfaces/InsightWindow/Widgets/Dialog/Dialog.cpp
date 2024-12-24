#include "Dialog.hpp"

#include <QFile>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Services/PathService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Widgets
{
    using Exceptions::Exception;

    Dialog::Dialog(
        const QString& windowTitle,
        const QString& text,
        QWidget* parent
    )
        : QDialog(parent)
    {
        this->setObjectName("dialog");
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        this->setWindowTitle(windowTitle);

        auto dialogUiFile = QFile{
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/Dialog/UiFiles/Dialog.ui"
            )
        };

        auto dialogStylesheet = QFile{
            QString::fromStdString(Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/Dialog/Stylesheets/Dialog.qss"
            )
        };

        if (!dialogUiFile.open(QFile::ReadOnly)) {
            throw Exception{"Failed to open Dialog UI file"};
        }

        if (!dialogStylesheet.open(QFile::ReadOnly)) {
            throw Exception{"Failed to open Dialog stylesheet file"};
        }

        this->setStyleSheet(dialogStylesheet.readAll());

        auto uiLoader = UiLoader{this};
        this->container = uiLoader.load(&dialogUiFile, this);

        this->textLabel = this->container->findChild<Label*>("text-label");
        this->actionLayout = this->container->findChild<QHBoxLayout*>("actions-layout");

        this->container->setContentsMargins(15, 10, 15, 15);

        this->textLabel->setTextFormat(Qt::TextFormat::RichText);
        this->textLabel->setText(text);
        this->setMinimumSize(500, 150);
    }

    void Dialog::showEvent(QShowEvent* event) {
        const auto containerSize = this->container->sizeHint();
        const auto windowSize = QSize{
            std::max(containerSize.width(), 500),
            std::max(containerSize.height(), 100)
        };

        this->setFixedSize(windowSize);
        this->container->setFixedSize(windowSize);
    }

    void Dialog::addActionButton(QPushButton* button) {
        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        this->actionLayout->insertWidget(1, button);
    }
}
