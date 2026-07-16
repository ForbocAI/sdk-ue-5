#pragma once

#include "CoreMinimal.h"

namespace ProtocolConfiguration {

struct FHistoryData {
  int32 RecentInteractionLimit;
};

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

struct FDecisionActionTypes {
  FString Move;
  FString Interact;
  FString Speak;
};

struct FDecisionGoalData {
  FString Prefix;
  FString MemoryPrefix;
  FString MemorySuffix;
};

struct FDecisionMetadataFields {
  FString Source;
  FString Context;
  FString NpcState;
  FString Memories;
  FString RecentInteractions;
  FString Role;
  FString Content;
};

struct FDecisionData {
  TArray<FString> MoveVerbs;
  TArray<FString> InteractVerbs;
  TArray<FString> StopWords;
  FDecisionActionTypes ActionTypes;
  FDecisionGoalData Goal;
  FString MetadataSource;
  FDecisionMetadataFields MetadataFields;
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
  FHistoryData History;
  FIterationData Iteration;
  FRuntimeTypes RuntimeTypes;
  FErrorMessages Errors;
  FLimitsData Limits;
  FClassificationData Classifications;
  FDecisionData Decision;
  FRoleData Roles;
  FTextData Text;
  FFormatData Formats;
  FDebugData Debug;
};

} // namespace ProtocolConfiguration
