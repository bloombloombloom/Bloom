#pragma once

#include <QWidget>
#include <QString>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>

namespace Widgets
{
    class LabeledSeparator: public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(QString title READ getTitle WRITE setTitle DESIGNABLE true)
        Q_PROPERTY(QColor lineColor READ getLineColor WRITE setLineColor DESIGNABLE true)
        Q_PROPERTY(int marginLeft READ getMarginLeft WRITE setMarginLeft DESIGNABLE true)
        Q_PROPERTY(int marginRight READ getMarginRight WRITE setMarginRight DESIGNABLE true)

    public:
        explicit LabeledSeparator(QString title = "", QWidget* parent = nullptr);
        explicit LabeledSeparator(QWidget* parent = nullptr): LabeledSeparator("", parent) {};

        [[nodiscard]] QString getTitle() const {
            return this->title;
        }

        void setTitle(const QString& title) {
            this->title = title;
        }

        [[nodiscard]] QColor getLineColor() const {
            return this->lineColor;
        }

        void setLineColor(QColor lineColor) {
            this->lineColor = lineColor;
        }

        [[nodiscard]] int getMarginLeft() const {
            return this->marginLeft;
        }

        void setMarginLeft(int marginLeft) {
            this->marginLeft = marginLeft;
        }

        [[nodiscard]] int getMarginRight() const {
            return this->marginRight;
        }

        void setMarginRight(int marginRight) {
            this->marginRight = marginRight;
        }

    protected:
        void paintEvent(QPaintEvent* event) override;
        void drawWidget(QPainter& painter);

    private:
        static constexpr int DEFAULT_HEIGHT = 20;

        QString title;
        QColor lineColor = QColor(0x4A, 0x4A, 0x4A);
        int marginLeft = 0;
        int marginRight = 10;
    };
}
