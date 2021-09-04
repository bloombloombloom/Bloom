#pragma once

#include "../ClickableWidget.hpp"

namespace Bloom::Widgets
{
    class ItemWidget: public ClickableWidget
    {
    Q_OBJECT
    protected:
        virtual void postSetSelected(bool selected) {};

    public:
        ItemWidget(QWidget *parent);

    public slots:
        void setSelected(bool selected);

    signals:
        void selected(ItemWidget*);
    };
}
