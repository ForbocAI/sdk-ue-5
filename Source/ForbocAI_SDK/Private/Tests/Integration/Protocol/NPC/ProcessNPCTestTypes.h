#pragma once

#include "Core/rtk.hpp"
#include "Components/Protocol/ProtocolTypes.h"
#include "Store.h"

struct FProcessNPCTestState {
  bool bCompleted = false;
  bool bSuccess = false;
  FString DatabaseName;
  FString Error;
  FAgentResponse Response;
  TSharedPtr<rtk::EnhancedStore<FRuntimeState>> Store;
};

struct FProcessNPCTestParams {
  FString NpcId;
  FString Input;
  FString Persona;
};
