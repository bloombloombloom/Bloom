#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QSize>

#include "src/Logger/Logger.hpp"

namespace Bloom::Widgets
{
    class Q_WIDGETS_EXPORT ExpandingHeightScrollAreaWidget: public QScrollArea
    {
    Q_OBJECT
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


    public:
        explicit ExpandingHeightScrollAreaWidget(QWidget* parent): QScrollArea(parent) {};
    };
}
