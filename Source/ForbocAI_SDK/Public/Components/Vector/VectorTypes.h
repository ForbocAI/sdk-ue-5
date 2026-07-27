#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"

struct FVectorState {
  FString Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92);
  bool bIsReady = false;
  FString Error;
};
