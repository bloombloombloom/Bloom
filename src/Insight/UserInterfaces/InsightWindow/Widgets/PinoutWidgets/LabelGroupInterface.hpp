#pragma once

#include <memory>

#include "Label.hpp"

namespace Widgets::PinoutWidgets
{
    class LabelGroupInterface
    {
    public:
        virtual ~LabelGroupInterface() = default;
        virtual void insertLabel(std::unique_ptr<Label>&& label) = 0;
    };
}
