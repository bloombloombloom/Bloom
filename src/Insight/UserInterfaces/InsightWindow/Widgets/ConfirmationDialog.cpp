#include "ConfirmationDialog.hpp"

namespace Widgets
{
    ConfirmationDialog::ConfirmationDialog(
        const QString& windowTitle,
        const QString& text,
        const std::optional<QString>& confirmationButtonText,
        const std::optional<QString>& cancelButtonText,
        QWidget* parent
    )
        : Dialog(windowTitle, text, parent)
    {
        this->confirmButton->setStyleName("primary");

        this->addActionButton(this->confirmButton);
        this->addActionButton(this->cancelButton);

        this->confirmButton->setText(confirmationButtonText.value_or("Proceed"));
        this->cancelButton->setText(cancelButtonText.value_or("Cancel"));

        QObject::connect(this->confirmButton, &QPushButton::clicked, this, [this] {
            this->close();
            emit this->confirmed();
        });

        QObject::connect(this->cancelButton, &QPushButton::clicked, this, [this] {
            this->close();
            emit this->aborted();
        });
    }
}
