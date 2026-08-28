#pragma once

#include "CoreMinimal.h"

struct FGhostConfiguration {
  int32 DefaultDuration;
  int32 DefaultHistoryLimit;
  float EmptyProgress;
  float CompleteProgress;
  FString IdleStatus;
  FString CompletedStatus;
  FString FailedStatus;
  FString EmptyMessage;
  FString NoActiveSession;
  FString StartFailed;
  FString StatusFailed;
  FString ResultsFailed;
  FString StopFailed;
  FString HistoryFailed;
  FString StopNotCompleted;
};
