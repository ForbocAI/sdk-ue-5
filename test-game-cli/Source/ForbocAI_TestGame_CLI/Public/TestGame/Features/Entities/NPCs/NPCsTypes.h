#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Entities/Spatial/SpatialTypes.h"

namespace TestGame {

struct FGameNPC {
  FString Id;
  FString Name;
  FString Faction;
  int32 Hp;
  int32 Suspicion;
  TArray<FString> Inventory;
  TArray<FString> KnownSecrets;
  FPosition Position;

  FGameNPC() : Hp(100), Suspicion(0) {}
};

typedef FGameNPC FNPC;

} // namespace TestGame
