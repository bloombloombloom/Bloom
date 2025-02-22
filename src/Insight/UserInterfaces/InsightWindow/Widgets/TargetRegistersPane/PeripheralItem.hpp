#pragma once

#include <QString>
#include <unordered_map>
#include <vector>
#include <QPixmap>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "RegisterGroupItem.hpp"
#include "src/Targets/TargetPeripheralDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace Widgets
{
    class PeripheralItem: public ListItem
    {
    public:
        const Targets::TargetPeripheralDescriptor& peripheralDescriptor;
        const QString name;
        const QString searchKeywords;
        std::vector<RegisterGroupItem*> registerGroupItems;

        PeripheralItem(
            const Targets::TargetPeripheralDescriptor& peripheralDescriptor,
            std::unordered_map<Targets::TargetRegisterId, RegisterItem*>& flattenedRegisterItemsByRegisterIds,
            Targets::TargetRegisterDescriptors& flattenedRegisterDescriptors
        );

        [[nodiscard]] bool isEmpty() const;

        bool isExpanded() const {
            return this->expanded;
        }
        void setExpanded(bool expanded);
        void setAllExpanded(bool expanded);

        void onGeometryChanged() override {
            return this->refreshGeometry();
        }
        void refreshGeometry();

        bool operator < (const ListItem& rhs) const override {
            return this->name < dynamic_cast<const PeripheralItem&>(rhs).name;
        }

        void applyFilter(const QString& keyword);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int LIST_ITEM_HEIGHT = 25;
        static inline std::optional<QPixmap> peripheralIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> collapsedArrowIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> expandedArrowIconPixmap = std::nullopt;

        bool expanded = false;

        void generatePixmaps() const;
    };
}
