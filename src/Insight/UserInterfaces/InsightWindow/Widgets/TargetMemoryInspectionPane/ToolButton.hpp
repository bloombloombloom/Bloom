#pragma once

#include <QToolButton>
#include <QString>
#include <QHBoxLayout>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/SvgWidget.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/Label.hpp"

namespace Widgets
{
    class ToolButton: public QToolButton
    {
        Q_OBJECT

    public:
        ToolButton(QString&& label, QWidget* parent);
        QSize minimumSizeHint() const override;

    private:
        QHBoxLayout* layout;
        SvgWidget* icon;
        Label* label;
    };
}
