#include "PeripheralItem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSvgRenderer>
#include <QPainter>
#include <QString>
#include <QRect>
#include <algorithm>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListScene.hpp"
#include "src/Services/PathService.hpp"
#include "src/Logger/Logger.hpp"

namespace Widgets
{
    PeripheralItem::PeripheralItem(
        const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
        std::unordered_map<Targets::TargetRegisterId, RegisterItem*>& flattenedRegisterItemsByRegisterIds,
        Targets::TargetRegisterDescriptors& flattenedRegisterDescriptors
    )
        : peripheralDescriptor(peripheralDescriptor)
        , name(QString::fromStdString(this->peripheralDescriptor.name))
        , searchKeywords(this->name + " " + QString::fromStdString(this->peripheralDescriptor.description))
    {
        for (const auto& [groupKey, groupDescriptor] : this->peripheralDescriptor.registerGroupDescriptorsByKey) {
            auto* registerGroupItem = new RegisterGroupItem{
                groupDescriptor,
                1,
                flattenedRegisterItemsByRegisterIds,
                flattenedRegisterDescriptors,
                this
            };
            registerGroupItem->setVisible(this->expanded);

            this->registerGroupItems.push_back(registerGroupItem);
        }

        std::sort(
            this->registerGroupItems.begin(),
            this->registerGroupItems.end(),
            [] (const RegisterGroupItem* itemA, const RegisterGroupItem* itemB) {
                return itemA->startAddress < itemB->startAddress;
            }
        );

        if (!PeripheralItem::peripheralIconPixmap.has_value()) {
            this->generatePixmaps();
        }
    }

    void PeripheralItem::setExpanded(bool expanded) {
        if (expanded == this->expanded) {
            return;
        }

        this->expanded = expanded;

        for (auto& groupItem : this->registerGroupItems) {
            groupItem->setVisible(this->expanded);
        }
    }

    void PeripheralItem::setAllExpanded(bool expanded) {
        this->expanded = expanded;

        for (auto& groupItem : this->registerGroupItems) {
            groupItem->setVisible(this->expanded);
            groupItem->setAllExpanded(this->expanded);
        }
    }

    void PeripheralItem::refreshGeometry() {
        const auto groupWidth = this->size.width();

        const auto startXPosition = 0;
        auto startYPosition = PeripheralItem::LIST_ITEM_HEIGHT;

        if (this->expanded) {
            for (auto& groupItem : this->registerGroupItems) {
                if (groupItem->excluded || !groupItem->isVisible()) {
                    continue;
                }

                groupItem->size.setWidth(groupWidth);
                groupItem->setPos(startXPosition, startYPosition);

                groupItem->onGeometryChanged();

                startYPosition += groupItem->size.height();
            }
        }

        this->size.setHeight(startYPosition);
    }

    void PeripheralItem::applyFilter(const QString& keyword) {
        const auto displayEntirePeripheral = keyword.isEmpty()
            || this->searchKeywords.contains(keyword, Qt::CaseInsensitive);

        auto visibleChildItems = std::size_t{0};

        for (auto* groupItem : this->registerGroupItems) {
            groupItem->applyFilter(keyword, displayEntirePeripheral);

            if (!groupItem->excluded) {
                ++visibleChildItems;
            }
        }

        this->setExpanded(visibleChildItems > 0 && !keyword.isEmpty());
        this->excluded = visibleChildItems == 0 && !keyword.isEmpty();
    }

    void PeripheralItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        if (this->excluded) {
            return;
        }

        static constexpr auto selectedBackgroundColor = QColor{0x3C, 0x59, 0x5C};

        static constexpr auto itemLeftPadding = 3;
        static constexpr auto itemTopPadding = 4;
        static constexpr auto iconLeftMargin = 5;
        static constexpr auto labelLeftMargin = 6;

        static const auto nameLabelFont = QFont{"'Ubuntu', sans-serif", 11};
        static constexpr auto nameLabelFontColor = QColor{0xAF, 0xB1, 0xB3};

        if (this->selected) {
            painter->setBrush(selectedBackgroundColor);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawRect(0, 0, this->size.width(), PeripheralItem::LIST_ITEM_HEIGHT);
        }

        const auto& collapsedArrowIconPixmap = *(PeripheralItem::collapsedArrowIconPixmap);
        const auto& expandedArrowIconPixmap = *(PeripheralItem::expandedArrowIconPixmap);
        const auto& peripheralIconPixmap = *(PeripheralItem::peripheralIconPixmap);

        const QPixmap& arrowPixmap = this->expanded ? expandedArrowIconPixmap : collapsedArrowIconPixmap;

        painter->drawPixmap(itemLeftPadding, itemTopPadding + 2, arrowPixmap);
        painter->drawPixmap(
            arrowPixmap.width() + itemLeftPadding + iconLeftMargin,
            itemTopPadding + 1,
            peripheralIconPixmap
        );

        painter->setFont(nameLabelFont);
        painter->setPen(nameLabelFontColor);

        const auto fontMetrics = painter->fontMetrics();

        const auto nameLabelSize = fontMetrics.size(Qt::TextSingleLine, this->name);
        const auto nameLabelRect = QRect{
            arrowPixmap.width() + itemLeftPadding + peripheralIconPixmap.width() + itemLeftPadding + labelLeftMargin,
            itemTopPadding,
            nameLabelSize.width(),
            nameLabelSize.height()
        };

        painter->drawText(nameLabelRect, Qt::AlignLeft, this->name);
    }

    void PeripheralItem::generatePixmaps() const {
        auto svgRenderer = QSvgRenderer{};

        {
            svgRenderer.load(QString::fromStdString(
                Services::PathService::compiledResourcesPath()
                    + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register-group.svg"
            ));

            auto peripheralIconPixmap = QPixmap{svgRenderer.defaultSize()};
            peripheralIconPixmap.fill(Qt::GlobalColor::transparent);

            auto painter = QPainter{&peripheralIconPixmap};
            svgRenderer.render(&painter);

            PeripheralItem::peripheralIconPixmap = peripheralIconPixmap;
        }

        {
            svgRenderer.load(QString::fromStdString(
                Services::PathService::compiledResourcesPath()
                    + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/arrow.svg"
            ));

            const auto iconSize = svgRenderer.defaultSize();
            auto arrowIconPixmap = QPixmap{iconSize};
            arrowIconPixmap.fill(Qt::GlobalColor::transparent);

            auto painter = QPainter{&arrowIconPixmap};
            svgRenderer.render(&painter);

            PeripheralItem::collapsedArrowIconPixmap = arrowIconPixmap;

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

            PeripheralItem::expandedArrowIconPixmap = arrowIconPixmap;
        }
    }
}
