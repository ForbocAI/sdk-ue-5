#pragma once

#include "CoreMinimal.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

#define FORBOCAI_COMMAND_RUNNER_SYNTAX_FIELDS(X)                           \
  X(FString, unknownCommandKey)                                            \
  X(FString, textOption)                                                   \
  X(FString, sdkRootCommand)                                               \
  X(FString, invalidCommandMessage)

#define FORBOCAI_COMMAND_RUNNER_COMMAND_FIELDS(X)                          \
  X(FString, npcPrefix)                                                    \
  X(FString, memoryPrefix)                                                 \
  X(FString, soulPrefix)                                                   \
  X(FString, ghostPrefix)                                                  \
  X(FString, ghostDomain)                                                  \
  X(FString, ghostMemoryStoreAction)                                       \
  X(FString, ghostMemoryStore)                                             \
  X(FString, npcCreate)                                                    \
  X(FString, ghostRun)                                                     \
  X(FString, soulExport)                                                   \
  X(FString, soulImport)

#define FORBOCAI_COMMAND_RUNNER_ALIAS_FIELDS(X)                            \
  X(FString, ghostSession)                                                 \
  X(FString, soulTransaction)

#define FORBOCAI_COMMAND_RUNNER_LIMIT_FIELDS(X)                            \
  X(int32, rootTokenCount)                                                 \
  X(int32, firstTokenIndex)                                                \
  X(int32, domainTokenIndex)                                               \
  X(int32, firstArgumentIndex)                                             \
  X(int32, expectedResolvedArgumentCount)                                  \
  X(int32, nextIndex)

#define FORBOCAI_COMMAND_RUNNER_QUOTE_FIELDS(X)                            \
  X(FString, doubleQuote)                                                  \
  X(FString, singleQuote)

#define FORBOCAI_COMMAND_RUNNER_EVIDENCE_FIELDS(X)                         \
  X(FString, structuredRequired)                                           \
  X(FString, structuredMatchingOutput)                                     \
  X(FString, structuredMismatchingOutput)                                  \
  X(FString, memoryObservation)

#define FORBOCAI_COMMAND_RUNNER_MESSAGE_FIELDS(X)                          \
  X(FString, unresolvedAlias)                                              \
  X(FString, capturedValuePreservesSuccess)                                \
  X(FString, missingCapturedValueFails)                                    \
  X(FString, requiredLiteralPreservesSuccess)                              \
  X(FString, missingLiteralFails)                                          \
  X(FString, absentForbiddenLiteralPreservesSuccess)                       \
  X(FString, presentForbiddenLiteralFails)                                 \
  X(FString, structuredJsonPreservesSuccess)                               \
  X(FString, structuredJsonMismatchFails)                                  \
  X(FString, missingAliasFails)                                            \
  X(FString, unresolvedIdentifierRejected)                                 \
  X(FString, capturedIdentifierDispatchable)                               \
  X(FString, sdkCatalogMatchesHyphenatedCommand)                           \
  X(FString, sdkCatalogPreservesCommandArguments)

namespace MicroGame {
namespace CommandRunner {

#define FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD(Type, Name) Type Name{};

struct FCommandRunnerSyntax {
  FORBOCAI_COMMAND_RUNNER_SYNTAX_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerCommands {
  FORBOCAI_COMMAND_RUNNER_COMMAND_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerAliases {
  FORBOCAI_COMMAND_RUNNER_ALIAS_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerLimits {
  FORBOCAI_COMMAND_RUNNER_LIMIT_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerQuotes {
  FORBOCAI_COMMAND_RUNNER_QUOTE_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerEvidence {
  FORBOCAI_COMMAND_RUNNER_EVIDENCE_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerMessages {
  FORBOCAI_COMMAND_RUNNER_MESSAGE_FIELDS(
      FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD)
};

struct FCommandRunnerTestNames {
  FString outputAssertions;
  FString commandResolution;
};

#undef FORBOCAI_DECLARE_COMMAND_RUNNER_FIELD

struct FCommandRunnerData {
  FCommandRunnerSyntax syntax;
  FCommandRunnerCommands commands;
  FCommandRunnerAliases aliases;
  FCommandRunnerLimits limits;
  FCommandRunnerQuotes quotes;
  FCommandRunnerEvidence evidence;
  FCommandRunnerMessages messages;
  FCommandRunnerTestNames testNames;
};

struct FCommandAliasState {
  TMap<FString, FString> NpcAliases;
  TMap<FString, FString> GhostSessionAliases;
  TMap<FString, FString> SoulTransactionAliases;
};

struct FCommandAliasUpdate {
  TMap<FString, FString> NpcAliases;
  TMap<FString, FString> GhostSessionAliases;
  TMap<FString, FString> SoulTransactionAliases;
};

struct FCommandOutput {
  FString Status;
  FString Output;
  FString RoutedThrough;
  FCommandAliasUpdate AliasUpdate;
  double DurationMs{};

  /** User Story: As a command runner consumer, I need an empty transport value so progress records can be assembled before execution. @fn FCommandOutput() = default */
  FCommandOutput() = default;
  /** User Story: As a systems harness command runner consumer, I need to invoke fcommand output through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn FCommandOutput(FString InStatus, FString InOutput, FString InRoutedThrough, FCommandAliasUpdate InAliasUpdate) */
  FCommandOutput(FString InStatus, FString InOutput,
                 FString InRoutedThrough, FCommandAliasUpdate InAliasUpdate)
      : Status(InStatus), Output(MoveTemp(InOutput)),
        RoutedThrough(MoveTemp(InRoutedThrough)),
        AliasUpdate(MoveTemp(InAliasUpdate)) {}
};

} // namespace CommandRunner
} // namespace MicroGame
