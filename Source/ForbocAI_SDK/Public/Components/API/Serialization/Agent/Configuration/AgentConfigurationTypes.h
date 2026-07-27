#pragma once

#include "CoreMinimal.h"

namespace JsonInterop::AgentConfiguration {

struct FPersonaFields {
  FString Traits;
  FString Goals;
  FString Relationships;
  FString World;
  FString SpeakingStyle;
  FString Constraints;
};

struct FActionFields {
  FString Type;
  FString Target;
  FString Reason;
  FString Confidence;
  FString Signature;
  FString Payload;
};

struct FAgentContractData {
  FPersonaFields Persona;
  FActionFields Action;
  float DefaultConfidence;
  FString EmptyObject;
  int32 MinimumTypeLength;
};

} // namespace JsonInterop::AgentConfiguration
