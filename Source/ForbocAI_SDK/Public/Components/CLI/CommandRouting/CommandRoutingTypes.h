#pragma once

#include "Components/State/StateTypes.h"
#include "Core/fp.hpp"
#include "Core/rtk.hpp"

namespace CLIOps {
namespace CommandRouting {

using Result = func::TestResult<void>;
using RouteResult = func::Maybe<Result>;

} // namespace CommandRouting
} // namespace CLIOps
