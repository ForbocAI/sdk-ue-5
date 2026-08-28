#pragma once

#include "CoreMinimal.h"

namespace MicroGame::CommandDiagnostics {

struct FCommandDiagnosticsData {
  FString Header;
  FString Footer;
  FString EnvironmentPrefix;
  FString ApiUrlEnvironmentKey;
  FString ApiKeyEnvironmentKey;
  FString ProtocolEnvironmentKey;
  FString PromptEnvironmentKey;
  FString EnabledValue;
  FString UnsetValue;
  FString ConfiguredValue;
  FString MissingValue;
  FString ApiUrlLabel;
  FString AuthLabel;
  FString CommandPrefix;
  FString GroupPrefix;
  FString RoutesPrefix;
  FString AssertionsPrefix;
  FString StatusPrefix;
  FString DurationPrefix;
  FString OutputPrefix;
  FString RoutePrefix;
  FString None;
  FString EmptyOutput;
  FString LineSeparator;
  FString ListSeparator;
  FString AssertionSeparator;
  FString BlankLine;
};

struct FCommandDebugEnvironment {
  FString Protocol;
  FString Prompt;
};

} // namespace MicroGame::CommandDiagnostics
