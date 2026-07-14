#pragma once

#include "CLI/CliOperations.h"
#include "TestGame/Features/Systems/Contract/ContractAdapters.h"
#include "TestGame/Features/Systems/Contract/ContractApi.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include <exception>

namespace TestGame {
namespace Contract {

inline FRawContractResponse GetContractJson(FTestGameStore &Store,
                                            const FString &ApiUrl) {
  FRawContractResponse Response;
  try {
    Response.Body = Ops::waitForResult(
        Store.dispatch(ContractApi::getTestGameContractThunk(ApiUrl)), 5.0);
    Response.bSuccess = !Response.Body.IsEmpty();
    return Response;
  } catch (const std::exception &Error) {
    Response.Error = UTF8_TO_TCHAR(Error.what());
    return Response;
  }
}

inline FContractResponse GetContract(FTestGameStore &Store,
                                     const FString &ApiUrl) {
  const FRawContractResponse Raw = GetContractJson(Store, ApiUrl);
  return Raw.bSuccess ? ParseContractJson(Raw.Body) : FContractResponse{};
}

inline TArray<FScenarioStep> GetContractScenarioSteps(
    FTestGameStore &Store, const FString &ApiUrl) {
  const FContractResponse Contract = GetContract(Store, ApiUrl);
  return Contract.bValid ? ToScenarioSteps(Contract.Scenarios, 0, {})
                         : TArray<FScenarioStep>();
}

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
