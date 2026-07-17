#pragma once

#include "CoreMinimal.h"

namespace TestGame::Contract::Parsing {

template <typename Value> struct TParseResult {
  bool bValid{};
  Value Parsed{};
};

} // namespace TestGame::Contract::Parsing
