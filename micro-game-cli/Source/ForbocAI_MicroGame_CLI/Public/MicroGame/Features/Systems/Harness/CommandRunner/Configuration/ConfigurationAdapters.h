#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace MicroGame {
namespace CommandRunner {

/** User Story: As a systems harness command runner consumer, I need authored runner configuration decoded through a stable signature so command syntax, aliases, limits, and messages share one typed source. @fn inline FCommandRunnerData ReadCommandRunnerData() */
inline FCommandRunnerData ReadCommandRunnerData() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("harness/command-runner.json"));
  const TSharedRef<FJsonObject> Syntax =
      DataAdapters::ReadObjectField(Source, TEXT("syntax"));
  const TSharedRef<FJsonObject> Commands =
      DataAdapters::ReadObjectField(Source, TEXT("commands"));
  const TSharedRef<FJsonObject> Aliases =
      DataAdapters::ReadObjectField(Source, TEXT("aliases"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Quotes =
      DataAdapters::ReadObjectField(Source, TEXT("quotes"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  const TSharedRef<FJsonObject> TestNames =
      DataAdapters::ReadObjectField(Source, TEXT("testNames"));
  FCommandRunnerData Data;

#define FORBOCAI_READ_COMMAND_RUNNER_STRING(FieldType, FieldName)          \
  Data.syntax.FieldName =                                                  \
      DataAdapters::ReadStringField(Syntax, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_SYNTAX_FIELDS(
      FORBOCAI_READ_COMMAND_RUNNER_STRING)
#undef FORBOCAI_READ_COMMAND_RUNNER_STRING

#define FORBOCAI_READ_COMMAND_RUNNER_COMMAND(FieldType, FieldName)         \
  Data.commands.FieldName =                                                \
      DataAdapters::ReadStringField(Commands, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_COMMAND_FIELDS(
      FORBOCAI_READ_COMMAND_RUNNER_COMMAND)
#undef FORBOCAI_READ_COMMAND_RUNNER_COMMAND

#define FORBOCAI_READ_COMMAND_RUNNER_ALIAS(FieldType, FieldName)           \
  Data.aliases.FieldName =                                                 \
      DataAdapters::ReadStringField(Aliases, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_ALIAS_FIELDS(FORBOCAI_READ_COMMAND_RUNNER_ALIAS)
#undef FORBOCAI_READ_COMMAND_RUNNER_ALIAS

#define FORBOCAI_READ_COMMAND_RUNNER_LIMIT(FieldType, FieldName)           \
  Data.limits.FieldName =                                                  \
      DataAdapters::ReadNumberField(Limits, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_LIMIT_FIELDS(FORBOCAI_READ_COMMAND_RUNNER_LIMIT)
#undef FORBOCAI_READ_COMMAND_RUNNER_LIMIT

#define FORBOCAI_READ_COMMAND_RUNNER_QUOTE(FieldType, FieldName)           \
  Data.quotes.FieldName =                                                  \
      DataAdapters::ReadStringField(Quotes, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_QUOTE_FIELDS(FORBOCAI_READ_COMMAND_RUNNER_QUOTE)
#undef FORBOCAI_READ_COMMAND_RUNNER_QUOTE

#define FORBOCAI_READ_COMMAND_RUNNER_MESSAGE(FieldType, FieldName)         \
  Data.messages.FieldName =                                                \
      DataAdapters::ReadStringField(Messages, TEXT(#FieldName));
  FORBOCAI_COMMAND_RUNNER_MESSAGE_FIELDS(
      FORBOCAI_READ_COMMAND_RUNNER_MESSAGE)
#undef FORBOCAI_READ_COMMAND_RUNNER_MESSAGE

  Data.testNames.outputAssertions =
      DataAdapters::ReadStringField(TestNames, TEXT("outputAssertions"));

  return Data;
}

/** User Story: As a systems harness command runner consumer, I need immutable authored runner configuration through a stable signature so every command uses the same settings instance. @fn inline const FCommandRunnerData &CommandRunnerData() */
inline const FCommandRunnerData &CommandRunnerData() {
  static const FCommandRunnerData Data = ReadCommandRunnerData();
  return Data;
}

} // namespace CommandRunner
} // namespace MicroGame
