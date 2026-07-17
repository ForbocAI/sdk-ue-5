#pragma once

#include "CoreMinimal.h"

namespace TestGame {

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

} // namespace TestGame
