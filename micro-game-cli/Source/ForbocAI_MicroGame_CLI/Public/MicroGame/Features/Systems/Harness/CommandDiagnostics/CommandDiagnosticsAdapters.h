#pragma once

#include "MicroGame/Features/Systems/Harness/CommandDiagnostics/Definition/CommandDiagnosticsDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Definition/TwoNpcChatDefinitionAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/Vocabulary/VerificationVocabularyAdapters.h"

namespace MicroGame::CommandDiagnostics {

/** User Story: As four diagnostic modes, I need granular tracing selected from the shared authored mode sets. @fn inline bool IsCommandDebugMode(const FString &Mode) */
inline bool IsCommandDebugMode(const FString &Mode) {
  const FGameRuntimeData &Runtime =
      VerificationVocabularyAdapters::GameRuntimeData();
  return Runtime.autoplayDebugModes.Contains(Mode) ||
         Runtime.twoNpcChatDebugModes.Contains(Mode) ||
         Runtime.mazeDebugModes.Contains(Mode) ||
         Runtime.mazeGhostDebugModes.Contains(Mode);
}

/** User Story: As mode-specific diagnostics, I need the authored trace header selected without duplicating command execution. @fn inline FString SelectDebugHeader(const FString &Mode) */
inline FString SelectDebugHeader(const FString &Mode) {
  const FGameRuntimeData &Runtime =
      VerificationVocabularyAdapters::GameRuntimeData();
  return Runtime.autoplayDebugModes.Contains(Mode)
             ? CommandDiagnosticsData().Header
         : Runtime.twoNpcChatDebugModes.Contains(Mode)
             ? TwoNpcChat::TwoNpcChatConfig().DebugHeader
         : Runtime.mazeDebugModes.Contains(Mode) ||
                   Runtime.mazeGhostDebugModes.Contains(Mode)
             ? Maze::MazeConfig().DebugHeader
             : CommandDiagnosticsData().BlankLine;
}

/** User Story: As mode-specific diagnostics, I need the authored trace footer selected from the same mode domain as its header. @fn inline FString SelectDebugFooter(const FString &Mode) */
inline FString SelectDebugFooter(const FString &Mode) {
  const FGameRuntimeData &Runtime =
      VerificationVocabularyAdapters::GameRuntimeData();
  return Runtime.autoplayDebugModes.Contains(Mode)
             ? CommandDiagnosticsData().Footer
         : Runtime.twoNpcChatDebugModes.Contains(Mode)
             ? TwoNpcChat::TwoNpcChatConfig().DebugFooter
         : Runtime.mazeDebugModes.Contains(Mode) ||
                   Runtime.mazeGhostDebugModes.Contains(Mode)
             ? Maze::MazeConfig().DebugFooter
             : CommandDiagnosticsData().BlankLine;
}

/** User Story: As secure diagnostics, I need endpoint configuration and credential presence shown without exposing credential values. @fn inline FString FormatDebugEnvironment() */
inline FString FormatDebugEnvironment() {
  const FCommandDiagnosticsData &Data = CommandDiagnosticsData();
  const FString Url =
      FPlatformMisc::GetEnvironmentVariable(*Data.ApiUrlEnvironmentKey);
  const FString Key =
      FPlatformMisc::GetEnvironmentVariable(*Data.ApiKeyEnvironmentKey);
  return Data.EnvironmentPrefix + Data.ApiUrlLabel +
         (Url.IsEmpty() ? Data.UnsetValue : Url) + Data.AuthLabel +
         (Key.IsEmpty() ? Data.MissingValue : Data.ConfiguredValue);
}

/**
 * User Story: As isolated debug execution, I need temporary protocol and prompt tracing enabled while preserving caller-owned values.
 * @fn inline FCommandDebugEnvironment BeginCommandDiagnostics( const FString &Mode)
 */
inline FCommandDebugEnvironment BeginCommandDiagnostics(
    const FString &Mode) {
  const FCommandDiagnosticsData &Data = CommandDiagnosticsData();
  const FCommandDebugEnvironment Previous{
      FPlatformMisc::GetEnvironmentVariable(*Data.ProtocolEnvironmentKey),
      FPlatformMisc::GetEnvironmentVariable(*Data.PromptEnvironmentKey)};
  IsCommandDebugMode(Mode)
      ? (FPlatformMisc::SetEnvironmentVar(*Data.ProtocolEnvironmentKey,
                                          *Data.EnabledValue),
         FPlatformMisc::SetEnvironmentVar(*Data.PromptEnvironmentKey,
                                          *Data.EnabledValue),
         GLog != nullptr ? GLog->Log(*SelectDebugHeader(Mode)) : void(),
         GLog != nullptr ? GLog->Log(*FormatDebugEnvironment()) : void(),
         void())
      : void();
  return Previous;
}

/**
 * User Story: As isolated debug execution, I need temporary tracing values restored and the matching footer emitted after every mode result.
 * @fn inline void EndCommandDiagnostics( const FString &Mode, const FCommandDebugEnvironment &Previous)
 */
inline void EndCommandDiagnostics(
    const FString &Mode, const FCommandDebugEnvironment &Previous) {
  const FCommandDiagnosticsData &Data = CommandDiagnosticsData();
  IsCommandDebugMode(Mode)
      ? (FPlatformMisc::SetEnvironmentVar(*Data.ProtocolEnvironmentKey,
                                          *Previous.Protocol),
         FPlatformMisc::SetEnvironmentVar(*Data.PromptEnvironmentKey,
                                          *Previous.Prompt),
         GLog != nullptr ? GLog->Log(*SelectDebugFooter(Mode)) : void(),
         void())
      : void();
}

/**
 * User Story: As assertion diagnostics, I need authored kinds and values rendered recursively for the exact command contract.
 * @fn inline TArray<FString> FormatAssertions( const TArray<FOutputAssertion> &Assertions, int32 Index, TArray<FString> Values)
 */
inline TArray<FString> FormatAssertions(
    const TArray<FOutputAssertion> &Assertions, int32 Index,
    TArray<FString> Values) {
  return Index >= Assertions.Num()
             ? Values
             : (Values.Add(Assertions[Index].Kind +
                           CommandDiagnosticsData().AssertionSeparator +
                           Assertions[Index].Value),
                FormatAssertions(
                    Assertions,
                    Index + VerificationVocabularyAdapters::GameRuntimeData()
                                .numbers.nextIndex,
                    MoveTemp(Values)));
}

/** User Story: As raw-output diagnostics, I need each returned line visibly attributed to the command that produced it. @fn inline TArray<FString> FormatOutputLines(const FString &Output) */
inline TArray<FString> FormatOutputLines(const FString &Output) {
  const FCommandDiagnosticsData &Data = CommandDiagnosticsData();
  TArray<FString> Lines;
  Output.TrimStartAndEnd().ParseIntoArray(
      Lines, *Data.LineSeparator, false);
  return Lines.IsEmpty()
             ? TArray<FString>{Data.OutputPrefix + Data.EmptyOutput}
             : func::map_array<FString, FString>(
                   Lines, [&Data](const FString &Line) {
                     return Data.OutputPrefix + Line;
                   });
}

/**
 * User Story: As API and SLM troubleshooting, I need command, group, routes, assertions, status, timing, SDK route, and raw output projected together.
 * @fn inline TArray<FString> FormatCommandDiagnostics( const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result)
 */
inline TArray<FString> FormatCommandDiagnostics(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result) {
  const FCommandDiagnosticsData &Data = CommandDiagnosticsData();
  const TArray<FString> Assertions = FormatAssertions(
      Command.OutputAssertions,
      VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount,
      {});
  return func::concat_arrays<FString>(
      {{Data.CommandPrefix + Command.Command,
        Data.GroupPrefix + Command.Group,
        Data.RoutesPrefix +
            (Command.ExpectedRoutes.IsEmpty()
                 ? Data.None
                 : FString::Join(Command.ExpectedRoutes,
                                 *Data.ListSeparator)),
        Data.AssertionsPrefix +
            (Assertions.IsEmpty()
                 ? Data.None
                 : FString::Join(Assertions, *Data.ListSeparator)),
        Data.StatusPrefix + Result.Status,
        Data.DurationPrefix + LexToString(Result.DurationMs),
        Data.RoutePrefix +
            (Result.RoutedThrough.IsEmpty() ? Data.None
                                            : Result.RoutedThrough)},
       FormatOutputLines(Result.Output), {Data.BlankLine}});
}

/**
 * User Story: As a shared debug decorator, I need diagnostics emitted around the real result without creating another execution path.
 * @fn inline void TraceCommandResult( const FString &Mode, const FCommandSpec &Command, const CommandRunner::FCommandOutput &Result)
 */
inline void TraceCommandResult(
    const FString &Mode, const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &Result) {
  IsCommandDebugMode(Mode) && GLog != nullptr
      ? func::for_each_array<FString>(
            FormatCommandDiagnostics(Command, Result),
            [](const FString &Line) { GLog->Log(*Line); })
      : void();
}

} // namespace MicroGame::CommandDiagnostics
