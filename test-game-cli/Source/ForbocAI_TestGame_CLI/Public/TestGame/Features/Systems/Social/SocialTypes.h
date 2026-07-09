#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FTradeOffer {
  FString NpcId;
  FString Item;
  int32 Price;

  FTradeOffer() : Price(0) {}
};

struct FSocialState {
  FString ActiveDialogue;
  FTradeOffer ActiveTrade;
  bool bHasActiveTrade;

  FSocialState() : bHasActiveTrade(false) {}
};

} // namespace TestGame
