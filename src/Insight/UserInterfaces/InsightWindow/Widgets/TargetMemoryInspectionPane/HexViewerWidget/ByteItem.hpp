#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <atomic>
#include <mutex>
#include <QPixmap>

#include "HexViewerItem.hpp"

namespace Bloom::Widgets
{
#pragma pack(push, 1)
    class ByteItem: public HexViewerItem
    {
    public:
        static constexpr int WIDTH = 27;
        static constexpr int HEIGHT = 21;

        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        bool selected = false;
        bool excluded = false;
        bool grouped = false;
        bool stackMemory = false;
        bool changed = false;

        explicit ByteItem(Targets::TargetMemoryAddress address);

        QSize size() const override {
            return QSize(ByteItem::WIDTH, ByteItem::HEIGHT);
        }

        void paint(
            QPainter* painter,
            const HexViewerSharedState* hexViewerState,
            const QGraphicsItem* graphicsItem
        ) const override;

    private:
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

        static void generatePixmapCaches();
    };
#pragma pack(pop)
}
