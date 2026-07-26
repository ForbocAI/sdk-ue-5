#pragma once

#include "CoreMinimal.h"

namespace MicroGame {

struct FChatCommandData {
  FString TextOption;
};

struct FChatNumbers {
  int32 EmptyCount{};
  int32 InitialIndex{};
  int32 NextIndex{};
};

struct FChatOutputData {
  FString Empty;
  FString Heading;
  FString User;
  FString Npc;
  FString Latency;
  FString Missing;
  FString Footer;
};

struct FChatTokens {
  FString Id;
  FString Prompt;
  FString Response;
  FString DurationMs;
};

struct FChatData {
  FString SessionId;
  TArray<FString> ProbeIds;
  FChatCommandData Command;
  FChatNumbers Numbers;
  FChatOutputData Output;
  FChatTokens Tokens;
};

struct FChatTranscriptTurn {
  FString Id;
  FString Prompt;
  FString Response;
  double DurationMs{};
  bool bAvailable{};
};

struct FChatTranscript {
  FString SessionId;
  TArray<FChatTranscriptTurn> Turns;
};

} // namespace MicroGame
