#pragma once

#include <QGraphicsItem>
#include <QPainter>

#include "HexViewerItem.hpp"
#include "HexViewerSharedState.hpp"

namespace Bloom::Widgets
{
    class GraphicsItem: public QGraphicsItem
    {
    public:
        HexViewerItem* hexViewerItem = nullptr;

        GraphicsItem(const HexViewerSharedState* hexViewerState)
            : hexViewerState(hexViewerState)
        {
            this->setAcceptHoverEvents(true);
            this->setCacheMode(QGraphicsItem::CacheMode::NoCache);
        }

        ~GraphicsItem() {
            if (this->hexViewerItem != nullptr) {
                this->hexViewerItem->allocatedGraphicsItem = nullptr;
            }
        }

        void setHexViewerItem(HexViewerItem* item) {
            if (this->hexViewerItem != nullptr) {
                this->hexViewerItem->allocatedGraphicsItem = nullptr;
            }

            this->hexViewerItem = item;

            if (this->hexViewerItem == nullptr) {
                this->setVisible(false);
                return;
            }

            this->hexViewerItem->allocatedGraphicsItem = this;
            this->setPos(this->hexViewerItem->position());
            this->setVisible(true);
            this->update();
        }

        [[nodiscard]] QRectF boundingRect() const override {
            if (this->hexViewerItem != nullptr) {
                const auto itemSize = this->hexViewerItem->size();
                return QRectF(0, 0, itemSize.width(), itemSize.height());
            }

            return QRectF();
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
            if (this->hexViewerItem != nullptr) {
                return this->hexViewerItem->paint(painter, this->hexViewerState, this);
            }
        }

    private:
        const HexViewerSharedState* hexViewerState;
    };
}
