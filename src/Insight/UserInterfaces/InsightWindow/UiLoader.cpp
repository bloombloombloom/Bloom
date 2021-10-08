#include "UiLoader.hpp"

#include <QtUiTools>

// Custom widgets
#include "Widgets/PanelWidget.hpp"
#include "Widgets/RotatableLabel.hpp"
#include "Widgets/SvgWidget.hpp"
#include "Widgets/SvgToolButton.hpp"
#include "Widgets/ExpandingHeightScrollAreaWidget.hpp"
#include "Widgets/TargetWidgets/TargetPackageWidgetContainer.hpp"

using namespace Bloom;
using namespace Bloom::Widgets;

UiLoader::UiLoader(QObject* parent): QUiLoader(parent) {
    this->customWidgetConstructorsByWidgetName = decltype(this->customWidgetConstructorsByWidgetName) {
        {
            "PanelWidget",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new PanelWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "RotatableLabel",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new RotatableLabel("", parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "ExpandingHeightScrollAreaWidget",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new ExpandingHeightScrollAreaWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgWidget",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new SvgWidget(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgToolButton",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new SvgToolButton(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "TargetPackageWidgetContainer",
            [this] (QWidget* parent, const QString& name) {
                auto widget = new InsightTargetWidgets::TargetPackageWidgetContainer(parent);
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
    };
}

QWidget* UiLoader::createWidget(const QString& className, QWidget* parent, const QString& name) {
    if (this->customWidgetConstructorsByWidgetName.contains(className)) {
        // This is a custom widget - call the mapped constructor
        return this->customWidgetConstructorsByWidgetName.at(className)(parent, name);
    }

    return QUiLoader::createWidget(className, parent, name);
}
