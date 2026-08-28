#pragma once

#include "CoreMinimal.h"

namespace ProtocolConfiguration {

struct FIterationData {
  int32 InitialIndex;
  int32 Step;
};

struct FRuntimeTypes {
  FString Function;
  FString String;
};

struct FActionData {
  FString NpcProcess;
  FString GhostProcess;
};

struct FErrorMessages {
  FString MissingMemoryStore;
  FString MissingVectorMemory;
  FString UnknownInstruction;
  FString ValidationFailed;
  FString MaxTurnsExceeded;
  FString UnsupportedInstruction;
  FString MissingNpcId;
  FString MissingStructuredPersona;
  FString ProcessingFailed;
  FString NpcNotFoundTemplate;
  FString ApiStatusRequestFailed;
  FString ApiContractRequestFailed;
};

struct FLimitsData {
  int32 MaxTurns;
};

struct FClassificationData {
  FString QueryVector;
  FString Finalize;
};

struct FRoleData {
  FString User;
  FString Assistant;
};

struct FTokenData {
  FString NpcId;
};

struct FFormatData {
  FString RunIdSeparator;
};

struct FDebugLabelData {
  FString PreDispatch;
  FString RawAction;
  FString UnwrapThrow;
  FString UnwrapOk;
};

struct FDebugData {
  FString Environment;
  FString EnabledValue;
  FDebugLabelData Npc;
  FDebugLabelData Ghost;
};

struct FProtocolData {
  FIterationData Iteration;
  FRuntimeTypes RuntimeTypes;
  FActionData Actions;
  FErrorMessages Errors;
  FLimitsData Limits;
  FClassificationData Classifications;
  FRoleData Roles;
  FTokenData Tokens;
  FFormatData Formats;
  FDebugData Debug;
};

} // namespace ProtocolConfiguration
