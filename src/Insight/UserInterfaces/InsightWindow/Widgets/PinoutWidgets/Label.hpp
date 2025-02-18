#pragma once

#include <cstddef>
#include <QGraphicsItem>
#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>

namespace Widgets::PinoutWidgets
{
    class Label
    {
    public:
        static constexpr int HEIGHT = 22;
        static constexpr int FONT_SIZE_POINTS = 8;
        static constexpr int HORIZ_PADDING = 10;

        int minimumWidth = 0;
        int width = 0;

        /*
         * This flag isn't respected for pin number and pad name labels. Not a big deal, as we don't currently disable
         * them. Will fix later, if necessary.
         */
        bool enabled = true;

        bool changed = false;

        Label() = default;
        virtual ~Label() = default;

        [[nodiscard]] virtual const QString& text() const = 0;
        void refreshGeometry();
        virtual void paint(QPainter* painter, int startX, int startY) const;

    protected:
        static const inline auto FONT = QFont{"'Ubuntu', sans-serif", Label::FONT_SIZE_POINTS, QFont::Normal};
        static const inline auto FONT_METRICS = QFontMetrics{Label::FONT};

        [[nodiscard]] virtual const QColor& rectColor() const;
        [[nodiscard]] virtual const QColor& fontColor() const;
        [[nodiscard]] virtual const QColor& changedFontColor() const;
    };
}
