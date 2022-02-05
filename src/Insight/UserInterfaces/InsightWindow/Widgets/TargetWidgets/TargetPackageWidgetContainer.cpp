#include "TargetPackageWidgetContainer.hpp"

#include <QLayout>

namespace Bloom::Widgets::InsightTargetWidgets
{
    TargetPackageWidgetContainer::TargetPackageWidgetContainer(QWidget* parent): QWidget(parent) {}

    void TargetPackageWidgetContainer::setPackageWidget(TargetPackageWidget* packageWidget) {
        this->packageWidget = packageWidget;

        if (packageWidget != nullptr) {
            this->layout()->addWidget(packageWidget);
        }
    }

    void TargetPackageWidgetContainer::resizeEvent(QResizeEvent* event) {
        if (this->packageWidget == nullptr) {
            return;
        }

        const auto packageSize = this->packageWidget->size();
        this->packageWidget->setGeometry(
            (this->width() / 2) - (packageSize.width() / 2),
            (this->height() / 2) - (packageSize.height() / 2),
            packageSize.width(),
            packageSize.height()
        );
    }
}
