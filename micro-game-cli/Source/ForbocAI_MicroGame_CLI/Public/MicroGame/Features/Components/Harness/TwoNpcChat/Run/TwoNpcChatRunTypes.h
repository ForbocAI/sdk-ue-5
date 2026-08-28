#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Participant/ParticipantTypes.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace MicroGame::TwoNpcChat {

struct FChatCommandResult {
  bool bSuccess{};
  CommandRunner::FCommandOutput Output;
  FString Failure;
};

struct FChatValuesResult {
  bool bSuccess{};
  TArray<FString> Values;
  FString Failure;
};

struct FChatOperationResult {
  bool bSuccess{};
  FString Failure;
};

struct FChatNpcResult {
  bool bSuccess{};
  FChatNpc Npc;
  FString Failure;
};

struct FChatTurnsResult {
  bool bSuccess{};
  TArray<FString> Lines;
  FString Failure;
};

struct FChatRunResult {
  bool bSuccess{};
  TArray<FString> Lines;
  FString Failure;
};

} // namespace MicroGame::TwoNpcChat
