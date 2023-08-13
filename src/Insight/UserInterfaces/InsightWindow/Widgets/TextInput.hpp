#pragma once

#include <QLineEdit>
#include <QContextMenuEvent>
#include <QFocusEvent>

namespace Widgets
{
    class TextInput: public QLineEdit
    {
        Q_OBJECT

    public:
        explicit TextInput(QWidget* parent = nullptr);

    signals:
        void focusChanged();

    protected:
        void contextMenuEvent(QContextMenuEvent* event) override;
        void focusInEvent(QFocusEvent* event) override;
        void focusOutEvent(QFocusEvent* event) override;

    };
}
