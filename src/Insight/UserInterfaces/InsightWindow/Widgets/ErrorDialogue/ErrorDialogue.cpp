#include "ErrorDialogue.hpp"

#include <QHBoxLayout>
#include <QFile>

#include "src/Insight/UserInterfaces/InsightWindow/UiLoader.hpp"
#include "src/Helpers/Paths.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::Widgets
{
    using Bloom::Exceptions::Exception;

    ErrorDialogue::ErrorDialogue(
        const QString& windowTitle,
        const QString& errorMessage,
        QWidget* parent
    )
        : QDialog(parent)
    {
        this->setObjectName("error-dialogue");
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        this->setWindowTitle(windowTitle);

        auto dialogueUiFile = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/UiFiles/ErrorDialogue.ui"
            )
        );

        auto dialogueStylesheet = QFile(
            QString::fromStdString(Paths::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/ErrorDialogue/Stylesheets/ErrorDialogue.qss"
            )
        );

        if (!dialogueUiFile.open(QFile::ReadOnly)) {
            throw Exception("Failed to open ErrorDialogue UI file");
        }

        if (!dialogueStylesheet.open(QFile::ReadOnly)) {
            throw Exception("Failed to open ErrorDialogue stylesheet file");
        }

        this->setStyleSheet(dialogueStylesheet.readAll());

        auto uiLoader = UiLoader(this);
        this->container = uiLoader.load(&dialogueUiFile, this);

        this->errorMessageDescriptionLabel = this->container->findChild<Label*>(
            "error-message-description-label"
        );
        this->okButton = this->container->findChild<QPushButton*>("ok-btn");

        this->container->setContentsMargins(15, 10, 15, 15);

        this->errorMessageDescriptionLabel->setText(errorMessage);

        QObject::connect(this->okButton, &QPushButton::clicked, this, &QDialog::close);
    }

    void ErrorDialogue::showEvent(QShowEvent* event) {
        const auto containerSize = this->container->sizeHint();
        const auto windowSize = QSize(
            std::max(containerSize.width(), 500),
            std::max(containerSize.height(), 100)
        );

        this->setFixedSize(windowSize);
        this->container->setFixedSize(windowSize);
    }
}
