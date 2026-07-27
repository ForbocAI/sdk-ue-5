#pragma once

#include "CoreMinimal.h"

namespace JsonInterop::SoulConfiguration {

struct FSoulFieldData {
  FString Id;
  FString Version;
  FString Name;
  FString StructuredPersona;
  FString Memories;
  FString State;
};

struct FSoulSerializationData {
  FString Version;
  FSoulFieldData Soul;
  FString InvalidSoul;
  FString UnsupportedSoulVersion;
};

} // namespace JsonInterop::SoulConfiguration
