#pragma once

#include "Core/rtk.hpp"

struct FRuntimeState;

namespace APISlice {

namespace Endpoints {

template <typename T>
using Thunk = rtk::ThunkAction<T, FRuntimeState>;

} // namespace Endpoints

} // namespace APISlice
