#pragma once

#include "CoreMinimal.h"

namespace Testing::API::Process {

struct FProcessCodecLabels {
  FString QueryVector;
  FString Finalize;
  FString RejectsAction;
  FString RejectsMemory;
  FString RejectsActor;
  FString RejectsInstruction;
};

struct FProcessCodecFixture {
  FString QueryVectorResponseJson;
  FString QueryVectorExpectedJson;
  FString FinalizeResponseJson;
  FString FinalizeExpectedJson;
  FString MalformedActionResponseJson;
  FString MalformedMemoryResponseJson;
  FString MalformedActorResponseJson;
  FString UnknownInstructionResponseJson;
  FProcessCodecLabels Labels;
};

} // namespace Testing::API::Process
