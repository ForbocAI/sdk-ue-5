#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FTradeOffer {
  FString NpcId;
  FString Item;
  int32 Price;

  /** User Story: As a features systems social consumer, I need to invoke ftrade offer through a stable signature so the features systems social workflow remains explicit and composable. @fn FTradeOffer() */
  FTradeOffer() : Price(0) {}
};

struct FSocialState {
  FString ActiveDialogue;
  FTradeOffer ActiveTrade;
  bool bHasActiveTrade;

  /** User Story: As a features systems social consumer, I need to invoke fsocial state through a stable signature so the features systems social workflow remains explicit and composable. @fn FSocialState() */
  FSocialState() : bHasActiveTrade(false) {}
};

} // namespace TestGame
