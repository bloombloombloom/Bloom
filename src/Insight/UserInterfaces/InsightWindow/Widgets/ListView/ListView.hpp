#pragma once

#include <QGraphicsView>
#include <vector>
#include <QEvent>

#include "ListScene.hpp"
#include "ListItem.hpp"

namespace Widgets
{
    class ListView: public QGraphicsView
    {
        Q_OBJECT

    public:
        ListView(
            ListItem::ListItemSetType&& items,
            QWidget* parent
        );

        ListScene* listScene() const {
            return this->scene;
        }

    protected:
        ListScene* scene = nullptr;

        bool event(QEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;
    };
}
