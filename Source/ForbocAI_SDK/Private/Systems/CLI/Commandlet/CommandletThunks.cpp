#include "Systems/CLI/Commandlet/CommandletThunks.h"

/** User Story: As a cli consumer, I need to invoke uforboc aicommandlet through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAICommandlet::UForbocAICommandlet() */
UForbocAICommandlet::UForbocAICommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/** User Story: As a cli consumer, I need to invoke main through a stable signature so the cli workflow remains explicit and composable. @fn int32 UForbocAICommandlet::Main(const FString &Params) */
int32 UForbocAICommandlet::Main(const FString &Params) {
  return CLIOps::ExecuteInvocation(Params);
}
