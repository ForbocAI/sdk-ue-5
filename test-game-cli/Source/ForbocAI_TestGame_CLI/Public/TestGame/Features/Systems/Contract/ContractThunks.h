#pragma once

#include "Features/Async/AsyncAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractApi.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include <exception>

namespace TestGame {
namespace Contract {

/** User Story: As a features systems contract consumer, I need to invoke get contract json through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FRawContractResponse GetContractJson(FTestGameStore &Store, const FString &ApiUrl) */
inline FRawContractResponse GetContractJson(FTestGameStore &Store,
                                            const FString &ApiUrl) {
  FRawContractResponse Response;
  try {
    Response.Body = AsyncAdapters::waitForResult(
        Store.dispatch(ContractApi::getTestGameContractThunk(ApiUrl)), 5.0);
    Response.bSuccess = !Response.Body.IsEmpty();
    return Response;
  } catch (const std::exception &Error) {
    Response.Error = UTF8_TO_TCHAR(Error.what());
    return Response;
  }
}

/** User Story: As a features systems contract consumer, I need to invoke get contract through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline FContractResponse GetContract(FTestGameStore &Store, const FString &ApiUrl) */
inline FContractResponse GetContract(FTestGameStore &Store,
                                     const FString &ApiUrl) {
  const FRawContractResponse Raw = GetContractJson(Store, ApiUrl);
  return Raw.bSuccess ? ParseContractJson(Raw.Body) : FContractResponse{};
}

/** User Story: As a features systems contract consumer, I need to invoke get contract scenario steps through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TArray<FScenarioStep> GetContractScenarioSteps( FTestGameStore &Store, const FString &ApiUrl) */
inline TArray<FScenarioStep> GetContractScenarioSteps(
    FTestGameStore &Store, const FString &ApiUrl) {
  const FContractResponse Contract = GetContract(Store, ApiUrl);
  return Contract.bValid ? ToScenarioSteps(Contract.Scenarios, 0, {})
                         : TArray<FScenarioStep>();
}

/** User Story: As a features systems contract consumer, I need to invoke validate contract parity through a stable signature so the features systems contract workflow remains explicit and composable. @fn inline TArray<FString> ValidateContractParity(FTestGameStore &Store, const FString &ApiUrl) */
inline TArray<FString> ValidateContractParity(FTestGameStore &Store,
                                              const FString &ApiUrl) {
  TArray<FString> Violations;
  return GetContract(Store, ApiUrl).bValid
             ? Violations
             : (Violations.Add(TEXT("Cannot reach API contract endpoint")),
                Violations);
}

} // namespace Contract
} // namespace TestGame
