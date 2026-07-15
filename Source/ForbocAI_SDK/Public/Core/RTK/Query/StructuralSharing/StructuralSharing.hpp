#pragma once

#include "Core/RTK/Query/Response/Response.hpp"

namespace rtk {
template <typename T>
T copyWithStructuralSharing(const T &OldValue, const T &NewValue) {
  return detail::copyWithStructuralSharingImpl<T>(
      OldValue, NewValue, typename detail::HasEqualOperator<T>::type());
}
} // namespace rtk
