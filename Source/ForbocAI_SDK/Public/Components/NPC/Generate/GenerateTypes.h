#pragma once

#include "CoreMinimal.h"

// Mirrors the TS SDK @forbocai/core NpcAttributeGenerate types: one granular,
// composable SLM attribute generated per round trip, conditioned on the prior
// attributes supplied (or recalled from the vector db) as context.

struct FNpcAttributeGenerateRequest {
  FString Context;
};

struct FNpcAttributeGenerateResponse {
  FString Attribute;
  FString Value;
};
