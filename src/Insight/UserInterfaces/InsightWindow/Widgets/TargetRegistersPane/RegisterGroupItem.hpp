#pragma once

#include <QString>
#include <unordered_map>
#include <vector>
#include <QPixmap>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "RegisterItem.hpp"
#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Widgets
{
    class RegisterGroupItem: public ListItem
    {
    public:
        const QString groupName;
        std::vector<RegisterItem*> registerItems;

        explicit RegisterGroupItem(
            QString name,
            const std::set<Targets::TargetRegisterDescriptor>& registerDescriptors,
            std::unordered_map<Targets::TargetRegisterDescriptor, RegisterItem*>& registerItemsByDescriptor
        );

        bool isExpanded() const {
            return this->expanded;
        }

        void setExpanded(bool expanded);

        void onGeometryChanged() override {
            return this->refreshGeometry();
        }

        void refreshGeometry();

        bool operator < (const ListItem& rhs) const override {
            const auto& rhsRegisterGroupItem = dynamic_cast<const RegisterGroupItem&>(rhs);
            return this->groupName < rhsRegisterGroupItem.groupName;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int GROUP_LIST_ITEM_HEIGHT = 25;
        static inline std::optional<QPixmap> registerGroupIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> collapsedArrowIconPixmap = std::nullopt;
        static inline std::optional<QPixmap> expandedArrowIconPixmap = std::nullopt;

        bool expanded = false;

        void generatePixmaps() const;
    };
}
