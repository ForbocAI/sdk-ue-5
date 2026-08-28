#pragma once

#include "CoreMinimal.h"

namespace ForbocAI::CLI::Ghost {

struct FCLIGhostState {
  FString DefaultSuite;
  int32 DefaultDuration;
  int32 HistoryLimit;
  int32 EmptyCount;
  FString RequiredSession;
  FString Starting;
  FString Started;
  FString Failed;
  FString Error;
  FString Results;
  FString Result;
  FString Passed;
  FString FailedTest;
  FString Session;
  FString Status;
  FString CompletedStatus;
  FString CompletedColor;
  FString ActiveColor;
  FString Progress;
  FString Errors;
  FString Duration;
  FString Stopping;
  FString Stopped;
  FString StopFailed;
  FString HistoryTitle;
  FString HistoryEmpty;
  FString History;
  FString NameOption;
  FString RoleOption;
  FString StateNameOption;
  FString StateRoleOption;
};

} // namespace ForbocAI::CLI::Ghost
