#pragma once

#include "Features/Async/AsyncAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractApi.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include <exception>

namespace TestGame {
namespace Contract {

/** User Story: As a test-game contract command, I need the API-owned contract resolved through the package RTK Query slice so CLI execution has one server-data path. @fn inline FContractQueryResult queryContractJson(FTestGameStore &Store, const FString &ApiUrl) */
inline FContractQueryResult queryContractJson(FTestGameStore &Store,
                                              const FString &ApiUrl) {
  FContractQueryResult Result;
  try {
    Result.Body = AsyncAdapters::waitForResult(
        Store.dispatch(ContractApi::getTestGameContractThunk(ApiUrl)),
        static_cast<double>(GameAdapters::GameData()
                                .numbers.contractTimeoutSeconds));
    Result.bSuccess = !Result.Body.IsEmpty();
    return Result;
  } catch (const std::exception &Error) {
    Result.Error = UTF8_TO_TCHAR(Error.what());
    return Result;
  }
}

} // namespace Contract
} // namespace TestGame
