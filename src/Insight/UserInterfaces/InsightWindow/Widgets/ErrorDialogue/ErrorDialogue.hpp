#pragma once

#include <QDialog>
#include <QPushButton>
#include <QShowEvent>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

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
        Label* errorMessageDescriptionLabel = nullptr;
        QPushButton* okButton = nullptr;
    };
}
