#pragma once

#include <QDialog>
#include <QPushButton>
#include <QShowEvent>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Widgets
{
    /**
     * @deprecated
     * TODO: Bin this. Replace all usages with Widgets::Dialog.
     */
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
