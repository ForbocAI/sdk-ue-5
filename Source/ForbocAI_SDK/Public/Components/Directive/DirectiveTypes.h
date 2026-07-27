#pragma once

#include "Core/rtk.hpp"
#include "Components/Contracts/ContractsTypes.h"

namespace DirectiveSlice {

struct FDirectiveRunStartedPayload {
  FString Id;
  FString NpcId;
  FString Observation;
};

struct FDirectiveReceivedPayload {
  FString Id;
  FDirectiveResponse Response;
};

struct FVerdictValidatedPayload {
  FString Id;
  FVerdictResponse Verdict;
};

struct FDirectiveRunFailedPayload {
  FString Id;
  FString Error;
};

struct FDirectiveSliceState {
  rtk::EntityState<FDirectiveRun> Entities;
  FString ActiveDirectiveId;
};

} // namespace DirectiveSlice
