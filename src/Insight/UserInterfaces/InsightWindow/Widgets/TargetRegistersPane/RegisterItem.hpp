#pragma once

#include <QString>
#include <QPixmap>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Targets/TargetRegisterDescriptor.hpp"

namespace Widgets
{
    class RegisterItem: public ListItem
    {
    public:
        const Targets::TargetRegisterDescriptor& registerDescriptor;
        std::size_t nestedLevel;
        const QString registerName;
        const QString searchKeywords;
        bool valueChanged = false;

        explicit RegisterItem(
            const Targets::TargetRegisterDescriptor& registerDescriptor,
            std::size_t nestedLevel,
            QGraphicsItem* parent
        );

        void setValue(const Targets::TargetMemoryBuffer& value);

        void clearValue() {
            this->valueText.clear();
        }

        bool operator < (const ListItem& rhs) const override {
            const auto& rhsRegisterItem = dynamic_cast<const RegisterItem&>(rhs);
            return this->registerDescriptor.startAddress < rhsRegisterItem.registerDescriptor.startAddress;
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 25;
        static inline std::optional<QPixmap> registerIconPixmap = std::nullopt;

        QString valueText = {};

        void generatePixmaps() const;
    };
}
