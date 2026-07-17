#pragma once

#include "Features/Config/ConfigSelectors.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"

namespace TestGame::Contract::Execution {

/** User Story: As the test-game contract endpoint, I need its API origin and credential selected from the current package root state immediately before request execution. @fn inline func::AsyncResult<rtk::QueryReturnValue<FString>> executeContractQuery(const rtk::ApiContext<FTestGameState> &Context) */
inline func::AsyncResult<rtk::QueryReturnValue<FString>>
executeContractQuery(const rtk::ApiContext<FTestGameState> &Context) {
  const FTestGameState &State = Context.getState();
  return executeTestGameContractAdapter(createTestGameContractRequest(
      ConfigSelectors::selectApiUrl(State),
      ConfigSelectors::selectApiKey(State)));
}

} // namespace TestGame::Contract::Execution
