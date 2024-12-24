#include "ListScene.hpp"

#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QByteArray>
#include <algorithm>

namespace Widgets
{
    ListScene::ListScene(const ListItem::ListItemSetType& items, QGraphicsView* parent)
        : QGraphicsScene(parent)
        , parent(parent)
    {
        this->setItemIndexMethod(QGraphicsScene::NoIndex);
        this->setItems(items);
    }

    void ListScene::setSelectionLimit(std::uint8_t selectionLimit) {
        this->selectionLimit = selectionLimit;
    }

    void ListScene::refreshGeometry() {
        const auto* viewport = this->viewport();
        const auto viewportWidth = viewport != nullptr ? viewport->width() : 0;
        const auto viewportHeight = viewport != nullptr ? viewport->height() : 0;

        const auto startXPosition = this->margins.left();
        auto startYPosition = this->margins.top();

        for (auto& listItem : this->listItems) {
            if (listItem->excluded || !listItem->isVisible()) {
                continue;
            }

            listItem->size.setWidth(viewportWidth - this->margins.left() - this->margins.right());
            listItem->setPos(startXPosition, startYPosition);

            listItem->onGeometryChanged();

            startYPosition += listItem->size.height();
        }

        /*
         * Due to a bug in Qt, we must subtract 2 from the scene rect width, to account for margins on the vertical
         * scrollbar.
         *
         * See https://bugreports.qt.io/browse/QTBUG-99189 for more on this.
         */
        this->setSceneRect(0, 0, viewportWidth - 2, std::max(viewportHeight, startYPosition));
        this->update();
    }

    void ListScene::setItems(const ListItem::ListItemSetType& items) {
        this->clearListItems();

        this->listItems = items;

        for (const auto& listItem : this->listItems) {
            this->addItem(listItem);
        }
    }

    void ListScene::addListItem(ListItem* item) {
        this->listItems.insert(item);
        this->addItem(item);
    }

    void ListScene::removeListItem(ListItem* item) {
        this->listItems.erase(item);
        this->selectedItems.remove(item);
        this->removeItem(item);
    }

    void ListScene::clearListItems() {
        this->selectedItems.clear();
        this->listItems.clear();
        this->clear();
    }

    void ListScene::setEnabled(bool enabled) {
        if (this->enabled == enabled) {
            return;
        }

        this->enabled = enabled;

        for (auto& item : this->listItems) {
            item->setEnabled(this->enabled);
        }

        this->update();
    }

    void ListScene::setKeyNavigationEnabled(bool enabled) {
        this->keyNavigationEnabled = enabled;
    }

    void ListScene::selectListItem(ListItem* item, bool append) {
        const auto selectedItemCount = this->selectedItems.size();
        if (selectedItemCount > 0) {
            if (!append || selectedItemCount >= this->selectionLimit) {
                const auto itemsToRemove = append
                    ? selectedItemCount - this->selectionLimit + 1
                    : selectedItemCount;

                auto itemIt = this->selectedItems.begin();
                while (
                    itemIt != this->selectedItems.end()
                    && (selectedItemCount - this->selectedItems.size()) < itemsToRemove
                ) {
                    auto& item = *itemIt;
                    item->selected = false;
                    item->update();

                    this->selectedItems.erase(itemIt++);
                }
            }
        }

        if (this->selectionLimit > 0) {
            this->selectedItems.push_back(item);
            item->selected = true;
            item->update();
            emit this->selectionChanged(this->selectedItems);
        }
    }

    void ListScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        const auto button = mouseEvent->button();

        const auto mousePosition = mouseEvent->buttonDownScenePos(button);

        const auto items = this->items(mousePosition);
        if (items.empty()) {
            return;
        }

        auto* clickedListItem = dynamic_cast<ListItem*>(items.first());
        if (clickedListItem == nullptr) {
            return;
        }

        if (clickedListItem->selected && button == Qt::MouseButton::RightButton) {
            return;
        }

        this->selectListItem(clickedListItem, (mouseEvent->modifiers() & Qt::ControlModifier) != 0);
        emit this->itemClicked(clickedListItem);
    }

    void ListScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent) {
        if (mouseEvent->button() != Qt::MouseButton::LeftButton) {
            return;
        }

        const auto mousePosition = mouseEvent->buttonDownScenePos(Qt::MouseButton::LeftButton);

        const auto items = this->items(mousePosition);
        if (items.empty()) {
            return;
        }

        auto* clickedListItem = dynamic_cast<ListItem*>(items.first());
        if (clickedListItem == nullptr) {
            return;
        }

        if (!clickedListItem->selected) {
            /*
             * Sometimes, QT won't trigger a press event when the user double clicks. This usually happens when the
             * first click closes a context menu.
             */
            this->mousePressEvent(mouseEvent);
        }

        emit this->itemDoubleClicked(clickedListItem);
    }

    void ListScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
        const auto items = this->items(event->scenePos());

        if (items.empty()) {
            return;
        }

        auto* listItem = dynamic_cast<ListItem*>(items.first());
        if (listItem == nullptr) {
            return;
        }

        emit this->itemContextMenu(listItem, event->screenPos());
    }

    void ListScene::keyPressEvent(QKeyEvent* keyEvent) {
        const auto key = keyEvent->key();

        if (
            this->keyNavigationEnabled
            && (key == Qt::Key_Up || key == Qt::Key_Down) && this->selectedItems.size() == 1
        ) {
            auto itemIt = this->listItems.find(this->selectedItems.front());
            if (
                itemIt != this->listItems.end()
                && (
                    (key == Qt::Key_Up && itemIt != this->listItems.begin())
                    || (key == Qt::Key_Down && itemIt != --(this->listItems.end()))
               )
            ) {
                auto* item = key == Qt::Key_Up ? *(--itemIt) : *(++itemIt);
                this->selectListItem(item, false);

                this->views().front()->ensureVisible(item);
                return;
            }
        }

        return QGraphicsScene::keyPressEvent(keyEvent);
    }
}
