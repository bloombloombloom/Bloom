#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace Bloom::Widgets
{
    class ErrorDialogue: public QDialog
    {
        Q_OBJECT

    public:
        ErrorDialogue(const QString& windowTitle, const QString& errorMessage, QWidget* parent);

    private:
        QWidget* container = nullptr;
        QLabel* errorMessageLabel = nullptr;
        QPushButton* okButton = nullptr;
    };
}
