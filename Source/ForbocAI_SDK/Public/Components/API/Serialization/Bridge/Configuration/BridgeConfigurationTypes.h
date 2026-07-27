#pragma once

#include "CoreMinimal.h"

namespace JsonInterop::BridgeConfiguration {

struct FContextFieldData {
  FString NpcState;
  FString WorldState;
  FString Constraints;
};

struct FValidateRequestFieldData {
  FString Action;
  FString Context;
};

struct FValidationResultFieldData {
  FString Valid;
  FString Reason;
  FString CorrectedAction;
};

struct FBridgeRuleFieldData {
  FString Id;
  FString Description;
  FString Actions;
};

struct FBridgeSerializationData {
  FContextFieldData Context;
  FValidateRequestFieldData ValidateRequest;
  FValidationResultFieldData ValidationResult;
  FBridgeRuleFieldData Rule;
};

} // namespace JsonInterop::BridgeConfiguration
