#include "UiLoader.hpp"

#include <QtUiTools>

// Custom widgets
#include "Widgets/Label.hpp"
#include "Widgets/RotatableLabel.hpp"
#include "Widgets/LabeledSeparator.hpp"
#include "Widgets/TextInput.hpp"
#include "Widgets/PlainTextEdit.hpp"
#include "Widgets/PushButton.hpp"
#include "Widgets/SvgWidget.hpp"
#include "Widgets/SvgToolButton.hpp"
#include "Widgets/ExpandingHeightScrollAreaWidget.hpp"

using namespace Widgets;

UiLoader::UiLoader(QObject* parent)
    : QUiLoader(parent)
{
    this->customWidgetConstructorsByWidgetName = {
        {
            "Label",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new Label{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "RotatableLabel",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new RotatableLabel{"", parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "LabeledSeparator",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new LabeledSeparator{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "TextInput",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new TextInput{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "PlainTextEdit",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new PlainTextEdit{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "PushButton",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new PushButton{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "ExpandingHeightScrollAreaWidget",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new ExpandingHeightScrollAreaWidget{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgWidget",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new SvgWidget{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
        {
            "SvgToolButton",
            [this] (QWidget* parent, const QString& name) {
                auto* widget = new SvgToolButton{parent};
                widget->setObjectName(name);
                widget->setStyleSheet(parent->styleSheet());
                return widget;
            }
        },
    };
}

QWidget* UiLoader::createWidget(const QString& className, QWidget* parent, const QString& name) {
    const auto widgetConstructorIt = this->customWidgetConstructorsByWidgetName.find(className);

    if (widgetConstructorIt != this->customWidgetConstructorsByWidgetName.end()) {
        // This is a custom widget - call the mapped constructor
        return widgetConstructorIt->second(parent, name);
    }

    return QUiLoader::createWidget(className, parent, name);
}
