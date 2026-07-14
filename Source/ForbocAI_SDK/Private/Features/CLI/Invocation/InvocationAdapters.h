#pragma once

#include "Features/CLI/Invocation/InvocationTypes.h"

namespace CommandletInvocation {

FInvocation ResolveInvocation(const FString &Params);

} // namespace CommandletInvocation
