#pragma once

#include <cstdint>
#include <QEvent>
#include <QGraphicsItem>
#include <QFont>
#include <optional>
#include <set>

#include "src/Targets/TargetMemory.hpp"

#include "HexViewerWidgetSettings.hpp"
#include "AnnotationItem.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/FocusedMemoryRegion.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/ExcludedMemoryRegion.hpp"

namespace Bloom::Widgets
{
    class ByteItem: public QGraphicsItem
    {
    public:
        static constexpr int WIDTH = 25;
        static constexpr int HEIGHT = 20;

        static constexpr int RIGHT_MARGIN = 5;
        static constexpr int BOTTOM_MARGIN = 5;

        std::size_t byteIndex;
        Targets::TargetMemoryAddress address = 0x00;
        QString addressHex;
        QString relativeAddressHex;

        unsigned char value = 0x00;
        QString hexValue;

        std::size_t currentRowIndex = 0;
        std::size_t currentColumnIndex = 0;

        bool highlighted = false;
        bool selected = false;
        const FocusedMemoryRegion* focusedMemoryRegion = nullptr;
        const ExcludedMemoryRegion* excludedMemoryRegion = nullptr;

        ByteItem(
            std::size_t byteIndex,
            Targets::TargetMemoryAddress address,
            std::optional<Targets::TargetStackPointer>& currentStackPointer,
            ByteItem** hoveredByteItem,
            std::set<ByteItem*>& highlightedByteItems,
            const HexViewerWidgetSettings& settings
        );

        void setValue(unsigned char value);

        [[nodiscard]] QRectF boundingRect() const override {
            return QRectF(
                0,
                0,
                ByteItem::WIDTH,
                ByteItem::HEIGHT
            );
        }
        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        bool valueInitialised = false;

        const HexViewerWidgetSettings& settings;
        std::optional<QString> asciiValue;

        ByteItem** hoveredByteItem;
        std::optional<Targets::TargetStackPointer>& currentStackPointer;
        std::set<ByteItem*>& highlightedByteItems;

        const QColor* getBackgroundColor();
        const QColor* getTextColor();
    };
}
