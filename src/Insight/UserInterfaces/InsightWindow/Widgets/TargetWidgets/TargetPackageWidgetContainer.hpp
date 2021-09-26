#pragma once

#include <QWidget>
#include <QResizeEvent>

#include "TargetPackageWidget.hpp"

namespace Bloom::Widgets::InsightTargetWidgets
{
    class TargetPackageWidgetContainer: public QWidget
    {
    Q_OBJECT
    private:
        TargetPackageWidget* packageWidget = nullptr;

    protected:
        void resizeEvent(QResizeEvent* event) override;

    public:
        TargetPackageWidgetContainer(QWidget* parent);

        void setPackageWidget(TargetPackageWidget* packageWidget);
    };
}
