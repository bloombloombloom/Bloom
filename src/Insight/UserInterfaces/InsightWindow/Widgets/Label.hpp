#pragma once

#include <QLabel>

namespace Bloom::Widgets
{
    class Label: public QLabel
    {
        Q_OBJECT

    public:
        explicit Label(QWidget* parent = nullptr)
            : QLabel(parent)
        {
            this->setTextFormat(Qt::TextFormat::PlainText);
        };

        Label(const QString& text, QWidget* parent = nullptr)
            : Label(parent)
        {
            this->setText(text);
        };
    };
}
