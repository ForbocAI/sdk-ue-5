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
  FString MissingApiDecisionIntent;
};

struct FLimitsData {
  int32 MaxTurns;
};

struct FClassificationData {
  FString IdentifyActor;
  FString QueryVector;
  FString Decision;
  FString Reasoning;
  FString Finalize;
};

struct FRoleData {
  FString User;
  FString Assistant;
};

struct FTextData {
  FString Empty;
  FString EmptyObject;
  FString WordSeparator;
};

struct FFormatData {
  FString RunIdSeparator;
};

struct FDebugData {
  FString Environment;
  FString EnabledValue;
  FString PreDispatch;
  FString RawAction;
  FString UnwrapThrow;
  FString UnwrapOk;
};

struct FProtocolData {
  FIterationData Iteration;
  FRuntimeTypes RuntimeTypes;
  FErrorMessages Errors;
  FLimitsData Limits;
  FClassificationData Classifications;
  FRoleData Roles;
  FTextData Text;
  FFormatData Formats;
  FDebugData Debug;
};

} // namespace ProtocolConfiguration
