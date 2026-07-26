#pragma once

#include "CoreMinimal.h"

namespace MicroGame::Contract::Parsing {

template <typename Value> struct TParseResult {
  bool bValid{};
  Value Parsed{};
};

} // namespace MicroGame::Contract::Parsing
