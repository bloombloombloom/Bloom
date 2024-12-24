#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMargins>
#include <set>
#include <list>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include <QGraphicsRectItem>
#include <QPointF>

#include "ListItem.hpp"

namespace Widgets
{
    class ListScene: public QGraphicsScene
    {
        Q_OBJECT

    public:
        QMargins margins = {0, 0, 0, 10};

        ListScene(
            const ListItem::ListItemSetType& items,
            QGraphicsView* parent
        );

        void refreshGeometry();
        void setSelectionLimit(std::uint8_t selectionLimit);
        void setItems(const ListItem::ListItemSetType& items);
        void addListItem(ListItem* item);
        void removeListItem(ListItem* item);
        void clearListItems();
        void setEnabled(bool enabled);
        void setKeyNavigationEnabled(bool enabled);
        void selectListItem(ListItem* item, bool append);

    signals:
        void selectionChanged(const std::list<ListItem*>& selectedItems);
        void itemClicked(ListItem* item);
        void itemDoubleClicked(ListItem* item);
        void itemContextMenu(ListItem* item, QPoint sourcePosition);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) override;
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;
        void keyPressEvent(QKeyEvent* keyEvent) override;

    private:
        ListItem::ListItemSetType listItems;
        QGraphicsView* const parent;
        bool enabled = false;
        bool keyNavigationEnabled = true;
        std::list<ListItem*> selectedItems;
        std::uint8_t selectionLimit = 1;

        QWidget* viewport() const {
            const auto views = this->views();

            if (!views.empty()) {
                return views.first()->viewport();
            }

            return nullptr;
        }
    };
}
