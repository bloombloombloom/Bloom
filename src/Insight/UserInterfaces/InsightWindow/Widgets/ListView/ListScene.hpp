#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMargins>
#include <vector>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QPointF>

#include "ListItem.hpp"

namespace Bloom::Widgets
{
    class ListScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        QMargins margins = QMargins(0, 0, 0, 10);

        ListScene(
            std::vector<ListItem*> items,
            QGraphicsView* parent
        );

        void refreshGeometry();
        void setItems(const std::vector<ListItem*>& items);
        void addListItem(ListItem* item);
        void sortItems();
        void setEnabled(bool enabled);

    signals:
        void selectionChanged(ListItem* selectedItem);
        void itemClicked(ListItem* item);
        void itemDoubleClicked(ListItem* item);
        void itemContextMenu(ListItem* item, QPoint sourcePosition);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    private:
        std::vector<ListItem*> listItems;
        QGraphicsView* const parent;
        bool enabled = false;
        ListItem* selectedItem = nullptr;

        QWidget* viewport() const {
            const auto views = this->views();

            if (!views.empty()) {
                return views.first()->viewport();
            }

            return nullptr;
        }
    };
}
