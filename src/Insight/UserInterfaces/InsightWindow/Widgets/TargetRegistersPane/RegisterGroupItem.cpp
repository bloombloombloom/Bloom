#include "RegisterGroupItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSvgRenderer>
#include <QPainter>
#include <QString>
#include <QRect>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListScene.hpp"
#include "src/Services/PathService.hpp"

namespace Bloom::Widgets
{
    RegisterGroupItem::RegisterGroupItem(
        QString name,
        const std::set<Targets::TargetRegisterDescriptor>& registerDescriptors,
        std::unordered_map<Targets::TargetRegisterDescriptorId, RegisterItem*>& registerItemsByDescriptorIds
    )
        : groupName(name)
    {
        for (const auto& registerDescriptor : registerDescriptors) {
            auto* registerItem = new RegisterItem(registerDescriptor);
            registerItem->setParentItem(this);
            registerItem->setVisible(this->isExpanded());

            this->registerItems.push_back(registerItem);
            registerItemsByDescriptorIds.insert(std::pair(registerDescriptor.id, registerItem));
        }

        if (!RegisterGroupItem::registerGroupIconPixmap.has_value()) {
            this->generatePixmaps();
        }
    }

    void RegisterGroupItem::setExpanded(bool expanded) {
        if (expanded == this->expanded) {
            return;
        }

        this->expanded = expanded;

        for (auto& registerItem : this->registerItems) {
            registerItem->setVisible(this->expanded);
        }
    }

    void RegisterGroupItem::refreshGeometry() {
        const auto groupWidth = this->size.width();

        const auto startXPosition = 0;
        auto startYPosition = RegisterGroupItem::GROUP_LIST_ITEM_HEIGHT;

        if (this->isExpanded()) {
            for (auto& registerItem : this->registerItems) {
                if (!registerItem->isVisible() || registerItem->excluded) {
                    continue;
                }

                registerItem->size.setWidth(groupWidth);
                registerItem->setPos(startXPosition, startYPosition);

                registerItem->onGeometryChanged();

                startYPosition += registerItem->size.height();
            }
        }

        this->size.setHeight(startYPosition);
    }

    void RegisterGroupItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        static constexpr auto selectedBackgroundColor = QColor(0x3C, 0x59, 0x5C);

        static constexpr auto itemLeftPadding = 3;
        static constexpr auto itemTopPadding = 4;
        static constexpr auto iconLeftMargin = 5;
        static constexpr auto labelLeftMargin = 6;

        static const auto nameLabelFont = QFont("'Ubuntu', sans-serif", 11);
        static constexpr auto nameLabelFontColor = QColor(0xAF, 0xB1, 0xB3);

        if (this->selected) {
            painter->setBrush(selectedBackgroundColor);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawRect(0, 0, this->size.width(), RegisterGroupItem::GROUP_LIST_ITEM_HEIGHT);
        }

        const auto& collapsedArrowIconPixmap = *(RegisterGroupItem::collapsedArrowIconPixmap);
        const auto& expandedArrowIconPixmap = *(RegisterGroupItem::expandedArrowIconPixmap);
        const auto& registerGroupIconPixmap = *(RegisterGroupItem::registerGroupIconPixmap);

        const QPixmap& arrowPixmap = this->isExpanded() ? expandedArrowIconPixmap : collapsedArrowIconPixmap;

        painter->drawPixmap(itemLeftPadding, itemTopPadding + 2, arrowPixmap);
        painter->drawPixmap(
            arrowPixmap.width() + itemLeftPadding + iconLeftMargin,
            itemTopPadding + 1,
            registerGroupIconPixmap
        );

        painter->setFont(nameLabelFont);
        painter->setPen(nameLabelFontColor);

        const auto fontMetrics = painter->fontMetrics();

        const auto nameLabelSize = fontMetrics.size(Qt::TextSingleLine, this->groupName);
        const auto nameLabelRect = QRect(
            arrowPixmap.width() + itemLeftPadding + registerGroupIconPixmap.width() + itemLeftPadding + labelLeftMargin,
            itemTopPadding,
            nameLabelSize.width(),
            nameLabelSize.height()
        );

        painter->drawText(nameLabelRect, Qt::AlignLeft, this->groupName);
    }

    void RegisterGroupItem::generatePixmaps() const {
        auto svgRenderer = QSvgRenderer();

        {
            svgRenderer.load(QString::fromStdString(
                Services::PathService::compiledResourcesPath()
                    + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register-group.svg"
            ));

            auto registerGroupIconPixmap = QPixmap(svgRenderer.defaultSize());
            registerGroupIconPixmap.fill(Qt::GlobalColor::transparent);

            auto painter = QPainter(&registerGroupIconPixmap);
            svgRenderer.render(&painter);

            RegisterGroupItem::registerGroupIconPixmap = registerGroupIconPixmap;
        }

        {
            svgRenderer.load(QString::fromStdString(
                Services::PathService::compiledResourcesPath()
                    + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/arrow.svg"
            ));

            const auto iconSize = svgRenderer.defaultSize();
            auto arrowIconPixmap = QPixmap(iconSize);
            arrowIconPixmap.fill(Qt::GlobalColor::transparent);

            auto painter = QPainter(&arrowIconPixmap);
            svgRenderer.render(&painter);

            RegisterGroupItem::collapsedArrowIconPixmap = arrowIconPixmap;

            painter.translate(
                std::ceil(static_cast<float>(iconSize.width() / 2)),
                std::ceil(static_cast<float>(iconSize.height() / 2))
            );
            painter.rotate(90);
            painter.translate(
                -std::ceil(static_cast<float>(iconSize.width() / 2)),
                -std::ceil(static_cast<float>(iconSize.height() / 2))
            );
            arrowIconPixmap.fill(Qt::GlobalColor::transparent);
            svgRenderer.render(&painter);

            RegisterGroupItem::expandedArrowIconPixmap = arrowIconPixmap;
        }
    }
}
