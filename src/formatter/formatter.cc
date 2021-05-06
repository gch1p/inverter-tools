// SPDX-License-Identifier: BSD-3-Clause

#include "formatter.h"

namespace formatter {

std::ostream& operator<<(std::ostream& os, Unit val) {
    switch (val) {
        case Unit::V:
            return os << "V";

        case Unit::A:
            return os << "A";

        case Unit::Wh:
            return os << "Wh";

        case Unit::VA:
            return os << "VA";

        case Unit::Hz:
            return os << "Hz";

        case Unit::Percentage:
            return os << "%";

        case Unit::Celsius:
            return os << "°C";

        default:
            break;
    };

    return os;
}


}