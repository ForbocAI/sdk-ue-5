#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FQualityReportPaths {
  TArray<FString> Current;
  TArray<FString> Baseline;
};

struct FQualityReportSerialization {
  int32 Indentation{};
  FString LineEnding;
};

struct FQualityReportMessages {
  FString CurrentMissing;
  FString AbsoluteGateFailed;
  FString Promoted;
  FString ReadFailed;
  FString ParseFailed;
  FString WriteFailed;
};

struct FQualityReportTokens {
  FString Path;
};

struct FQualityReportAutomationNames {
  FString MissingReport;
};

struct FQualityReportStories {
  FString MissingReport;
};

struct FQualityReportData {
  FString Host;
  FQualityReportPaths Paths;
  FQualityReportSerialization Serialization;
  FQualityReportMessages Messages;
  FQualityReportTokens Tokens;
  FQualityReportAutomationNames AutomationNames;
  FQualityReportStories Stories;
};

} // namespace TestGame
