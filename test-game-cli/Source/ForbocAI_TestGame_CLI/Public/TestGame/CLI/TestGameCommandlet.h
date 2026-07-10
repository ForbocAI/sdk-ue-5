#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "TestGameCommandlet.generated.h"

UCLASS()
class UForbocAITestGameCommandlet : public UCommandlet {
  GENERATED_BODY()

public:
  UForbocAITestGameCommandlet();

  virtual int32 Main(const FString &Params) override;
};
