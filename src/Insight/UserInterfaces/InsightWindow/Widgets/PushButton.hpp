#pragma once

#include <QPushButton>
#include <QString>

namespace Widgets
{
    class PushButton: public QPushButton
    {
        Q_OBJECT
        Q_PROPERTY(QString styleName READ getStyleName WRITE setStyleName DESIGNABLE true)

    public:
        explicit PushButton(QWidget* parent = nullptr);

        void setStyleName(const QString& styleName) {
            this->styleName = styleName;
        }

        [[nodiscard]] QString getStyleName() const {
            return this->styleName;
        }

    protected:
        QString styleName;
    };
}
