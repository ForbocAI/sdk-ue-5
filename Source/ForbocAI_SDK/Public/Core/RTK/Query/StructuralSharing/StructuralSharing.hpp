#pragma once

#include "Core/RTK/Query/Response/Response.hpp"

namespace rtk {
/** User Story: As a rtk query structural sharing consumer, I need to invoke copy with structural sharing through a stable signature so the rtk query structural sharing workflow remains explicit and composable. @fn template <typename T> T copyWithStructuralSharing(const T &OldValue, const T &NewValue) */
template <typename T>
T copyWithStructuralSharing(const T &OldValue, const T &NewValue) {
  return detail::copyWithStructuralSharingImpl<T>(
      OldValue, NewValue, typename detail::HasEqualOperator<T>::type());
}
} // namespace rtk
