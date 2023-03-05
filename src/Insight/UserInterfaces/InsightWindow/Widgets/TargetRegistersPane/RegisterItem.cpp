#include "RegisterItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSvgRenderer>
#include <QPainter>
#include <QString>
#include <QRect>

#include "src/Services/PathService.hpp"

namespace Bloom::Widgets
{
    RegisterItem::RegisterItem(
        const Targets::TargetRegisterDescriptor& registerDescriptor
    )
        : registerDescriptor(registerDescriptor)
        , registerName(QString::fromStdString(registerDescriptor.name.value()).toUpper())
        , searchKeywords(this->registerName + " " + QString::fromStdString(registerDescriptor.description.value_or("")))
    {
        this->size = QSize(0, RegisterItem::HEIGHT);

        if (!RegisterItem::registerIconPixmap.has_value()) {
            this->generatePixmaps();
        }
    }

    void RegisterItem::setValue(const Targets::TargetMemoryBuffer& value) {
        const auto valueByteArray = QByteArray(
            reinterpret_cast<const char*>(value.data()),
            static_cast<qsizetype>(value.size())
        );

        auto hexValueByteArray = valueByteArray.toHex();
        this->valueText = ": 0x" + QString(hexValueByteArray).toUpper()
            + " | " + QString::number(hexValueByteArray.toUInt(nullptr, 16));

        if (value.size() == 1 && value[0] >= 32 && value[0] <= 126) {
            this->valueText += " | '" + QString(valueByteArray) + "'";
        }
    }

    void RegisterItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        if (this->excluded) {
            return;
        }

        static constexpr auto selectedBackgroundColor = QColor(0x35, 0x5A, 0x80);

        static constexpr auto itemLeftPadding = 41;
        static constexpr auto itemTopPadding = 4;
        static constexpr auto labelLeftMargin = 5;

        static const auto nameLabelFont = QFont("'Ubuntu', sans-serif", 11);
        static constexpr auto nameLabelFontColor = QColor(0xAF, 0xB1, 0xB3);
        static const auto valueLabelFont = QFont("'Ubuntu', sans-serif", 10, -1, true);
        static constexpr auto valueLabelFontColor = QColor(0x8A, 0x8A, 0x8D);
        static constexpr auto highlightedValueLabelFontColor = QColor(0x54, 0x7F, 0xBA);

        if (this->selected) {
            painter->setBrush(selectedBackgroundColor);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawRect(QRect(QPoint(0, 0), this->size));
        }

        const auto& registerIconPixmap = *(RegisterItem::registerIconPixmap);
        painter->drawPixmap(itemLeftPadding, itemTopPadding + 4, registerIconPixmap);

        painter->setFont(nameLabelFont);
        painter->setPen(nameLabelFontColor);

        const auto fontMetrics = painter->fontMetrics();

        const auto nameLabelSize = fontMetrics.size(Qt::TextSingleLine, this->registerName);
        const auto nameLabelRect = QRect(
            registerIconPixmap.width() + itemLeftPadding + labelLeftMargin,
            itemTopPadding,
            nameLabelSize.width(),
            nameLabelSize.height()
        );

        painter->drawText(nameLabelRect, Qt::AlignLeft, this->registerName);

        if (!this->valueText.isEmpty()) {
            painter->setFont(valueLabelFont);
            painter->setPen(
                this->valueChanged && !this->selected ? highlightedValueLabelFontColor : valueLabelFontColor
            );

            const auto fontMetrics = painter->fontMetrics();

            const auto valueLabelSize = fontMetrics.size(Qt::TextSingleLine, this->valueText);
            const auto valueLabelRect = QRect(
                nameLabelRect.right() + labelLeftMargin,
                itemTopPadding + 1,
                valueLabelSize.width(),
                valueLabelSize.height()
            );

            painter->drawText(valueLabelRect, Qt::AlignLeft, this->valueText);
        }
    }

    void RegisterItem::generatePixmaps() const {
        auto svgRegisterIconRenderer = QSvgRenderer(QString::fromStdString(
            Services::PathService::compiledResourcesPath()
                + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register.svg"
        ));

        const auto registerIconSize = svgRegisterIconRenderer.defaultSize();

        auto registerIconPixmap = QPixmap(registerIconSize);
        registerIconPixmap.fill(Qt::GlobalColor::transparent);

        auto painter = QPainter(&registerIconPixmap);
        svgRegisterIconRenderer.render(&painter);

        RegisterItem::registerIconPixmap = registerIconPixmap;
    }
}
