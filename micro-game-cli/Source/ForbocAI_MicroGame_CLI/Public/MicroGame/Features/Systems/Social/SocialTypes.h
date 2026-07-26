#pragma once

#include "CoreMinimal.h"

namespace MicroGame {

struct FTradeOffer {
  FString NpcId;
  FString Item;
  int32 Price{};
};

struct FSocialState {
  FString ActiveDialogue;
  FTradeOffer ActiveTrade;
  bool bHasActiveTrade{};
};

} // namespace MicroGame
