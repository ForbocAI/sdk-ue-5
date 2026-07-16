#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "TestGameCommandlet.generated.h"

UCLASS()
class UForbocAITestGameCommandlet : public UCommandlet {
  GENERATED_BODY()

public:
  /** User Story: As a cli consumer, I need to invoke uforboc aitest game commandlet through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAITestGameCommandlet() */
  UForbocAITestGameCommandlet();

  /** User Story: As a cli consumer, I need to invoke main through a stable signature so the cli workflow remains explicit and composable. @fn virtual int32 Main(const FString &Params) override */
  virtual int32 Main(const FString &Params) override;
};
