#pragma once

#include "CoreMinimal.h"

// Mirrors the TS SDK @forbocai/core npc-generate data: field names and the
// error used to reject a malformed granular attribute-generation response.

namespace APISlice::Endpoints::NPCGenerateConfiguration {

struct FGenerateFields {
  FString Attribute;
  FString Value;
};

struct FGenerateErrors {
  FString InvalidResponse;
};

struct FGenerateConfigurationData {
  FGenerateFields Fields;
  FGenerateErrors Errors;
};

} // namespace APISlice::Endpoints::NPCGenerateConfiguration
