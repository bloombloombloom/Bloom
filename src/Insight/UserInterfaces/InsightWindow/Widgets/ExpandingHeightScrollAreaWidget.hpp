#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QSize>

namespace Widgets
{
    class ExpandingHeightScrollAreaWidget: public QScrollArea
    {
        Q_OBJECT

    public:
        explicit ExpandingHeightScrollAreaWidget(QWidget* parent): QScrollArea(parent) {};

    protected:
        [[nodiscard]] QSize scrollAreaSize() const {
            auto parentWidget = this->parentWidget();
            auto widget = this->widget();

            if (parentWidget != nullptr) {
                auto size = parentWidget->size();

                if (widget != nullptr) {
                    size.setWidth(widget->width() - 1);
                }

                return size;
            }

            return QScrollArea::size();
        };

        [[nodiscard]] QSize sizeHint() const override {
            return this->scrollAreaSize();
        };

        [[nodiscard]] QSize minimumSizeHint() const override {
            return this->scrollAreaSize();
        };
    };
}
