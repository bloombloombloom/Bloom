#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>

namespace Bloom::Widgets
{
    class ErrorDialogue: public QDialog
    {
        Q_OBJECT

    public:
        ErrorDialogue(const QString& windowTitle, const QString& errorMessage, QWidget* parent);

    protected:
        void showEvent(QShowEvent* event) override;

    private:
        QWidget* container = nullptr;
        QLabel* errorMessageDescriptionLabel = nullptr;
        QPushButton* okButton = nullptr;
    };
}
