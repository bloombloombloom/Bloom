#pragma once

#include <QWidget>
#include <QSize>

namespace Bloom::Widgets
{
    class Q_WIDGETS_EXPORT ExpandingWidget: public QWidget
    {
    Q_OBJECT
    protected:
        [[nodiscard]] QSize minimumSizeHint() const override {
            auto parentWidget = this->parentWidget();

            if (parentWidget != nullptr) {
                return parentWidget->size();
            }

            return QWidget::size();
        };

        [[nodiscard]] QSize sizeHint() const override {
            return this->minimumSizeHint();
        };

    public:
        explicit ExpandingWidget(QWidget* parent): QWidget(parent) {};

    };
}
