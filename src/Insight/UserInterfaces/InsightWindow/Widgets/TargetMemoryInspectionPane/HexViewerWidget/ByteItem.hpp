#pragma once

#include <cstdint>
#include <QEvent>
#include <QGraphicsItem>
#include <optional>

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
        std::uint32_t address = 0x00;

        QString addressHex;
        QString relativeAddressHex;

        std::size_t currentRowIndex = 0;
        std::size_t currentColumnIndex = 0;

        const FocusedMemoryRegion* focusedMemoryRegion = nullptr;
        const ExcludedMemoryRegion* excludedMemoryRegion = nullptr;

        ByteItem(
            std::size_t byteIndex,
            std::uint32_t address,
            std::optional<ByteItem*>& hoveredByteItem,
            std::optional<AnnotationItem*>& hoveredAnnotationItem,
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
        unsigned char value = 0x00;
        bool valueInitialised = false;
        bool valueChanged = false;

        const HexViewerWidgetSettings& settings;

        QString hexValue;
        std::optional<QString> asciiValue;

        std::optional<ByteItem*>& hoveredByteItem;
        std::optional<AnnotationItem*>& hoveredAnnotationItem;
    };
}
