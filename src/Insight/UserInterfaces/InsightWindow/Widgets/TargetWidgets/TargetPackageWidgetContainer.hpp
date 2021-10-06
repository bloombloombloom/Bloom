#pragma once

#include <QWidget>
#include <QResizeEvent>

#include "TargetPackageWidget.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    class TargetPackageWidgetContainer: public QWidget
    {
        Q_OBJECT

    public:
        TargetPackageWidgetContainer(QWidget* parent);

        void setPackageWidget(TargetPackageWidget* packageWidget);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        TargetPackageWidget* packageWidget = nullptr;
    };
}
