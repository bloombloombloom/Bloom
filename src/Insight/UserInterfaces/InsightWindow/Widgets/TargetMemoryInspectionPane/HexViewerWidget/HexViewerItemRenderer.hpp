#pragma once

#include <QGraphicsItem>
#include <QSize>
#include <QGraphicsView>
#include <QWidget>
#include <QPainter>
#include <atomic>
#include <mutex>
#include <QPixmap>
#include <vector>
#include <optional>

#include "HexViewerItemIndex.hpp"
#include "HexViewerSharedState.hpp"
#include "HexViewerItem.hpp"
#include "TopLevelGroupItem.hpp"
#include "ByteItem.hpp"
#include "FocusedRegionGroupItem.hpp"
#include "StackMemoryGroupItem.hpp"

namespace Bloom::Widgets
{
    /**
     * Renders hex viewer items in a QGraphicsScene.
     */
    class HexViewerItemRenderer: public QGraphicsItem
    {
    public:
        QSize size;

        HexViewerItemRenderer(
            const HexViewerSharedState& hexViewerState,
            const HexViewerItemIndex& itemIndex,
            const QGraphicsView* view
        );

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(0, 0, this->size.width(), this->size.height());
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    protected:
        const HexViewerSharedState& hexViewerState;
        const HexViewerItemIndex& itemIndex;

        const QGraphicsView* view;
        const QWidget* viewport;

        static inline std::atomic<bool> pixmapCachesGenerated = false;
        static inline std::mutex pixmapCacheMutex;

        static inline std::vector<QPixmap> standardPixmapsByValue = {};
        static inline std::vector<QPixmap> selectedPixmapsByValue = {};
        static inline std::vector<QPixmap> groupedPixmapsByValue = {};
        static inline std::vector<QPixmap> stackMemoryPixmapsByValue = {};
        static inline std::vector<QPixmap> changedMemoryPixmapsByValue = {};
        static inline std::vector<QPixmap> standardAsciiPixmapsByValue = {};
        static inline std::vector<QPixmap> selectedAsciiPixmapsByValue = {};
        static inline std::vector<QPixmap> groupedAsciiPixmapsByValue = {};
        static inline std::vector<QPixmap> stackMemoryAsciiPixmapsByValue = {};
        static inline std::vector<QPixmap> changedMemoryAsciiPixmapsByValue = {};
        static inline std::vector<QPixmap> hoveredPrimaryPixmapsByValue = {};
        static inline std::vector<QPixmap> hoveredPrimaryAsciiPixmapsByValue = {};
        static inline std::optional<QPixmap> missingDataPixmap = {};
        static inline std::optional<QPixmap> selectedMissingDataPixmap = {};

        void paintItem(const HexViewerItem* item, QPainter* painter);
        inline void paintByteItem(const ByteItem* item, QPainter* painter) __attribute__((__always_inline__));
        inline void paintFocusedRegionGroupItem(
            const FocusedRegionGroupItem* item,
            QPainter* painter
        ) __attribute__((__always_inline__));
        inline void paintStackMemoryGroupItem(
            const StackMemoryGroupItem* item,
            QPainter* painter
        ) __attribute__((__always_inline__));

        static void generatePixmapCaches();
    };
}
