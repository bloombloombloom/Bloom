#pragma once

#include <QString>
#include <unordered_map>
#include <vector>
#include <QPixmap>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "RegisterItem.hpp"
#include "src/Targets/TargetRegisterGroupDescriptor.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace Widgets
{
    class RegisterGroupItem: public ListItem
    {
    public:
        static constexpr int GROUP_LIST_ITEM_HEIGHT = 25;
        static inline std::optional<QPixmap> registerGroupIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> collapsedArrowIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> expandedArrowIconPixmap = std::nullopt;

        const Targets::TargetRegisterGroupDescriptor& registerGroupDescriptor;
        std::size_t nestedLevel;
        const Targets::TargetMemoryAddress startAddress;
        const QString name;
        const QString searchKeywords;
        std::vector<ListItem*> childItems;

        explicit RegisterGroupItem(
            const Targets::TargetRegisterGroupDescriptor& registerGroupDescriptor,
            std::size_t nestedLevel,
            std::unordered_map<Targets::TargetRegisterId, RegisterItem*>& flattenedRegisterItemsByRegisterIds,
            Targets::TargetRegisterDescriptors& flattenedRegisterDescriptors,
            QGraphicsItem* parent
        );

        [[nodiscard]] bool isEmpty() const;

        [[nodiscard]] bool isExpanded() const {
            return this->expanded;
        }
        void setExpanded(bool expanded);
        void setAllExpanded(bool expanded);

        void onGeometryChanged() override {
            return this->refreshGeometry();
        }
        void refreshGeometry();

        void applyFilter(const QString& keyword, bool displayEntirePeripheral);

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        bool expanded = false;
        void generatePixmaps() const;
    };
}
