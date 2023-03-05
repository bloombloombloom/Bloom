#pragma once

#include <QString>
#include <QPixmap>
#include <optional>

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/ListView/ListItem.hpp"

#include "src/Targets/TargetRegister.hpp"

namespace Bloom::Widgets
{
    class RegisterItem: public ListItem
    {
    public:
        const Targets::TargetRegisterDescriptor registerDescriptor;
        const QString registerName;
        const QString searchKeywords;
        bool excluded = false;
        bool valueChanged = false;

        explicit RegisterItem(
            const Targets::TargetRegisterDescriptor& registerDescriptor
        );

        void setValue(const Targets::TargetMemoryBuffer& value);

        void clearValue() {
            this->valueText.clear();
        }

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    private:
        static constexpr int HEIGHT = 25;
        static inline std::optional<QPixmap> registerIconPixmap = std::nullopt;

        QString valueText = "";

        void generatePixmaps() const;
    };
}
