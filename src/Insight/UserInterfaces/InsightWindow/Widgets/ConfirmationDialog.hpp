#pragma once

#include <optional>
#include <QString>

#include "Dialog/Dialog.hpp"
#include "PushButton.hpp"

namespace Widgets
{
    class ConfirmationDialog: public Dialog
    {
        Q_OBJECT

    public:
        explicit ConfirmationDialog(
            const QString& windowTitle,
            const QString& text,
            const std::optional<QString>& confirmationButtonText,
            const std::optional<QString>& cancelButtonText,
            QWidget* parent = nullptr
        );

    signals:
        void confirmed();
        void aborted();

    protected:
        PushButton* confirmButton = new PushButton{this};
        PushButton* cancelButton = new PushButton{this};
    };
}
