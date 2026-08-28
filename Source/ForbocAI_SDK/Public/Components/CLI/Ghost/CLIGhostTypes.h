#pragma once

#include "CoreMinimal.h"

namespace ForbocAI::CLI::Ghost {

struct FCLIGhostState {
  FString DefaultSuite;
  int32 DefaultDuration;
  FString DurationOption;
  int32 HistoryLimit;
  int32 EmptyCount;
  FString RequiredSession;
  FString Starting;
  FString Started;
  FString Identity;
  FString Runtime;
  FString Suite;
  FString Failed;
  FString Error;
  FString Results;
  FString Verdict;
  FString Coverage;
  FString Result;
  FString Metric;
  FString Evidence;
  FString Integrity;
  FString Exploration;
  FString EvidenceDimensions;
  FString EvaluationDimensions;
  FString DimensionSeparator;
  FString Summary;
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
  FString HistoryRuntime;
  FString NameOption;
  FString RoleOption;
  FString StateNameOption;
  FString StateRoleOption;
};

} // namespace ForbocAI::CLI::Ghost
