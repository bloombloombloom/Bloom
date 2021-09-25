#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>

namespace Bloom::Widgets
{
    class Q_WIDGETS_EXPORT ErrorDialogue: public QDialog
    {
    Q_OBJECT
        QWidget* container = nullptr;
        QLabel* errorMessageLabel = nullptr;
        QPushButton* okButton = nullptr;

    public:
        ErrorDialogue(const QString& windowTitle, const QString& errorMessage, QWidget* parent);
    };
}
