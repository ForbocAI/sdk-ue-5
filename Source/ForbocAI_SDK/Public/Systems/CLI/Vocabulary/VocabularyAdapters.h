#pragma once

#include "Components/CLI/CLITypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ForbocAI::CLI::Vocabulary {

/** User Story: As the CLI parser, I need token positions decoded from one vocabulary document so all command hosts normalize identically. @fn inline FCLIParsingSettings ReadParsingSettings( const TSharedRef<FJsonObject> &Parsing) */
inline FCLIParsingSettings ReadParsingSettings(
    const TSharedRef<FJsonObject> &Parsing) {
  return {
      DataAdapters::ReadNumberField(Parsing, TEXT("firstTokenIndex")),
      DataAdapters::ReadNumberField(Parsing, TEXT("secondTokenIndex")),
      DataAdapters::ReadNumberField(Parsing, TEXT("nextIndexOffset")),
      DataAdapters::ReadNumberField(Parsing, TEXT("directDropCount")),
      DataAdapters::ReadNumberField(Parsing, TEXT("subcommandDropCount")),
      DataAdapters::ReadNumberField(Parsing, TEXT("minimumTokenCount")),
  };
}

/** User Story: As the CLI invocation boundary, I need URL and credential option syntax decoded from the shared vocabulary document. @fn inline FCLIInvocationSettings ReadInvocationSettings( const TSharedRef<FJsonObject> &Invocation) */
inline FCLIInvocationSettings ReadInvocationSettings(
    const TSharedRef<FJsonObject> &Invocation) {
  return {
      DataAdapters::ReadStringArrayField(Invocation, TEXT("apiUrlOptions")),
      DataAdapters::ReadStringArrayField(Invocation, TEXT("apiKeyOptions")),
      DataAdapters::ReadStringField(Invocation, TEXT("assignmentSeparator")),
      DataAdapters::ReadNumberField(Invocation, TEXT("optionValueOffset")),
  };
}

} // namespace ForbocAI::CLI::Vocabulary
