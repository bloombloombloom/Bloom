#include "RegisterGroupItem.hpp"

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
    RegisterGroupItem::RegisterGroupItem(
        const Targets::TargetRegisterGroupDescriptor& registerGroupDescriptor,
        std::size_t nestedLevel,
        std::unordered_map<Targets::TargetRegisterId, RegisterItem*>& flattenedRegisterItemsByRegisterIds,
        Targets::TargetRegisterDescriptors& flattenedRegisterDescriptors,
        QGraphicsItem* parent
    )
        : ListItem(parent)
        , registerGroupDescriptor(registerGroupDescriptor)
        , nestedLevel(nestedLevel)
        , startAddress(this->registerGroupDescriptor.startAddress())
        , name(QString::fromStdString(this->registerGroupDescriptor.name))
        , searchKeywords(
            this->name + " " + QString::fromStdString(this->registerGroupDescriptor.description.value_or(""))
        )
    {
        for (const auto& [groupKey, groupDescriptor] : this->registerGroupDescriptor.subgroupDescriptorsByKey) {
            auto* subgroupItem = new RegisterGroupItem{
                groupDescriptor,
                this->nestedLevel + 1,
                flattenedRegisterItemsByRegisterIds,
                flattenedRegisterDescriptors,
                this
            };

            if (subgroupItem->isEmpty()) {
                delete subgroupItem;
                continue;
            }

            subgroupItem->setVisible(this->expanded);
            this->childItems.emplace_back(subgroupItem);
        }

        for (const auto& [registerKey, registerDescriptor] : this->registerGroupDescriptor.registerDescriptorsByKey) {
            if (!registerDescriptor.access.readable) {
                continue;
            }

            auto* registerItem = new RegisterItem{registerDescriptor, this->nestedLevel + 1, this};
            registerItem->setVisible(this->expanded);

            this->childItems.emplace_back(registerItem);
            flattenedRegisterItemsByRegisterIds.emplace(registerDescriptor.id, registerItem);
            flattenedRegisterDescriptors.emplace_back(&registerDescriptor);
        }

        std::sort(
            this->childItems.begin(),
            this->childItems.end(),
            [] (const ListItem* itemA, const ListItem* itemB) {
                const auto* registerItemA = dynamic_cast<const RegisterItem*>(itemA);
                const auto startAddressA = registerItemA != nullptr
                    ? registerItemA->registerDescriptor.startAddress
                    : dynamic_cast<const RegisterGroupItem*>(itemA)->startAddress;

                const auto* registerItemB = dynamic_cast<const RegisterItem*>(itemB);
                const auto startAddressB = registerItemB != nullptr
                    ? registerItemB->registerDescriptor.startAddress
                    : dynamic_cast<const RegisterGroupItem*>(itemB)->startAddress;

                return startAddressA < startAddressB;
            }
        );

        if (!RegisterGroupItem::registerGroupIconPixmap.has_value()) {
            this->generatePixmaps();
        }
    }

    bool RegisterGroupItem::isEmpty() const {
        for (const auto* childItem : this->childItems) {
            const auto* subgroupItem = dynamic_cast<const RegisterGroupItem*>(childItem);
            if (subgroupItem != nullptr) {
                if (!subgroupItem->isEmpty()) {
                    return false;
                }

                continue;
            }

            const auto* registerItem = dynamic_cast<const RegisterItem*>(childItem);
            if (registerItem != nullptr) {
                return false;
            }
        }

        return true;
    }

    void RegisterGroupItem::setExpanded(bool expanded) {
        if (expanded == this->expanded) {
            return;
        }

        this->expanded = expanded;

        for (auto* childItem : this->childItems) {
            childItem->setVisible(expanded);
        }
    }

    void RegisterGroupItem::setAllExpanded(bool expanded) {
        this->expanded = expanded;

        for (auto* childItem : this->childItems) {
            childItem->setVisible(expanded);

            auto* groupItem = dynamic_cast<RegisterGroupItem*>(childItem);
            if (groupItem != nullptr) {
                groupItem->setAllExpanded(expanded);
            }
        }
    }

    void RegisterGroupItem::refreshGeometry() {
        const auto groupWidth = this->size.width();

        const auto startXPosition = 0;
        auto startYPosition = RegisterGroupItem::GROUP_LIST_ITEM_HEIGHT;

        if (this->expanded) {
            for (auto& childItem : this->childItems) {
                if (childItem->excluded || !childItem->isVisible()) {
                    continue;
                }

                auto* groupItem = dynamic_cast<RegisterGroupItem*>(childItem);
                if (groupItem != nullptr) {
                    groupItem->refreshGeometry();
                }

                childItem->size.setWidth(groupWidth);
                childItem->setPos(startXPosition, startYPosition);

                childItem->onGeometryChanged();

                startYPosition += childItem->size.height();
            }
        }

        this->size.setHeight(startYPosition);
    }

    void RegisterGroupItem::applyFilter(const QString& keyword, bool displayEntirePeripheral) {
        const auto displayEntireGroup = displayEntirePeripheral || keyword.isEmpty()
            || this->searchKeywords.contains(keyword, Qt::CaseInsensitive);

        auto visibleChildCount = std::size_t{0};
        for (auto* childItem : this->childItems) {
            auto* groupItem = dynamic_cast<RegisterGroupItem*>(childItem);
            if (groupItem != nullptr) {
                groupItem->applyFilter(keyword, displayEntirePeripheral);

                if (!groupItem->excluded) {
                    ++visibleChildCount;
                }
                continue;
            }

            auto* registerItem = dynamic_cast<RegisterItem*>(childItem);
            if (registerItem != nullptr) {
                registerItem->excluded = !displayEntireGroup
                    && !registerItem->searchKeywords.contains(keyword, Qt::CaseInsensitive);

                if (!registerItem->excluded) {
                    ++visibleChildCount;
                }
            }
        }

        this->excluded = !displayEntireGroup && visibleChildCount == 0 && !keyword.isEmpty();
        this->setExpanded((displayEntireGroup || visibleChildCount > 0) && !keyword.isEmpty());
    }

    void RegisterGroupItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        if (this->excluded) {
            return;
        }

        const auto& collapsedArrowIconPixmap = *(RegisterGroupItem::collapsedArrowIconPixmap);
        const auto& expandedArrowIconPixmap = *(RegisterGroupItem::expandedArrowIconPixmap);
        const auto& registerGroupIconPixmap = *(RegisterGroupItem::registerGroupIconPixmap);

        static constexpr auto selectedBackgroundColor = QColor{0x3C, 0x59, 0x5C};

        static constexpr auto itemTopPadding = 4;
        static constexpr auto iconLeftMargin = 5;
        static constexpr auto labelLeftMargin = 6;
        const auto itemLeftPadding = (
            (expandedArrowIconPixmap.width() + iconLeftMargin) * static_cast<int>(this->nestedLevel)
        ) + 3;

        static const auto nameLabelFont = QFont{"'Ubuntu', sans-serif", 11};
        static constexpr auto nameLabelFontColor = QColor{0xAF, 0xB1, 0xB3};

        if (this->selected) {
            painter->setBrush(selectedBackgroundColor);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->drawRect(0, 0, this->size.width(), RegisterGroupItem::GROUP_LIST_ITEM_HEIGHT);
        }

        const auto& arrowPixmap = this->isExpanded() ? expandedArrowIconPixmap : collapsedArrowIconPixmap;

        painter->drawPixmap(itemLeftPadding, itemTopPadding + 2, arrowPixmap);
        painter->drawPixmap(
            itemLeftPadding + arrowPixmap.width() + iconLeftMargin,
            itemTopPadding + 1,
            registerGroupIconPixmap
        );

        painter->setFont(nameLabelFont);
        painter->setPen(nameLabelFontColor);

        const auto fontMetrics = painter->fontMetrics();

        const auto nameLabelSize = fontMetrics.size(Qt::TextSingleLine, this->name);
        const auto nameLabelRect = QRect{
            itemLeftPadding + arrowPixmap.width() + iconLeftMargin + registerGroupIconPixmap.width() + labelLeftMargin,
            itemTopPadding,
            nameLabelSize.width(),
            nameLabelSize.height()
        };

        painter->drawText(nameLabelRect, Qt::AlignLeft, this->name);
    }

    void RegisterGroupItem::generatePixmaps() const {
        auto svgRenderer = QSvgRenderer{};

        {
            svgRenderer.load(QString::fromStdString(
                Services::PathService::compiledResourcesPath()
                    + "/src/Insight/UserInterfaces/InsightWindow/Widgets/TargetRegistersPane/Images/register-group.svg"
            ));

            auto registerGroupIconPixmap = QPixmap{svgRenderer.defaultSize()};
            registerGroupIconPixmap.fill(Qt::GlobalColor::transparent);

            auto painter = QPainter{&registerGroupIconPixmap};
            svgRenderer.render(&painter);

            RegisterGroupItem::registerGroupIconPixmap = registerGroupIconPixmap;
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
