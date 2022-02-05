#include "BloomProxyStyle.hpp"

namespace Bloom
{
    int BloomProxyStyle::styleHint(
        StyleHint hint,
        const QStyleOption* option,
        const QWidget* widget,
        QStyleHintReturn* returnData
    ) const {
        if (hint == QStyle::SH_ComboBox_Popup) {
            return 0;
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}
