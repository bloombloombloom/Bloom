#pragma once

#include <QLineEdit>
#include <QContextMenuEvent>

namespace Bloom::Widgets
{
    class TextInput: public QLineEdit
    {
        Q_OBJECT

    public:
        explicit TextInput(QWidget* parent = nullptr);

    protected:
        void contextMenuEvent(QContextMenuEvent* event) override;

    };
}
