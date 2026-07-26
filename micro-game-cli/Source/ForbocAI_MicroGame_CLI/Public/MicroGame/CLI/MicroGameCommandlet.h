#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "MicroGameCommandlet.generated.h"

UCLASS()
class UForbocAIMicroGameCommandlet : public UCommandlet {
  GENERATED_BODY()

public:
  /** User Story: As a CLI consumer, I need to invoke the ForbocAI micro-game commandlet through a stable signature so the CLI workflow remains explicit and composable. @fn UForbocAIMicroGameCommandlet() */
  UForbocAIMicroGameCommandlet();

  /** User Story: As a cli consumer, I need to invoke main through a stable signature so the cli workflow remains explicit and composable. @fn virtual int32 Main(const FString &Params) override */
  virtual int32 Main(const FString &Params) override;
};
