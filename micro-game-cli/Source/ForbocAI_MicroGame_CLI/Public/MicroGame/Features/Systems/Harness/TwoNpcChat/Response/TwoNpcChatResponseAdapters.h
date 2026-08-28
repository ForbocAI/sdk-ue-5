#pragma once

#include "Core/JsonInterop.h"
#include "Internationalization/Regex.h"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/TwoNpcChatDefinitionTypes.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As a two-NPC chat adapter, I need the generated value read through the authored response field. @fn inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config,
                                  const FString &Output) {
  TSharedPtr<FJsonObject> Root;
  return (JsonInterop::ParseJsonObject(Output, Root) && Root.IsValid() &&
          Root->HasTypedField<EJson::String>(Config.ValueField))
             ? Root->GetStringField(Config.ValueField)
             : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need a created NPC id parsed with the authored pattern and index. @fn inline FString ReadNpcId(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadNpcId(const FTwoNpcChatConfig &Config,
                         const FString &Output) {
  FRegexMatcher Matcher(FRegexPattern(Config.IdPattern), Output);
  return Matcher.FindNext()
             ? Matcher.GetCaptureGroup(Config.Numbers.InitialIndex)
             : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need spoken dialogue normalized with the authored fallback. @fn inline FString ReadDialogue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadDialogue(const FTwoNpcChatConfig &Config,
                            const FString &Output) {
  TArray<FString> Lines;
  Output.ParseIntoArray(Lines, *Config.OutputLineSeparator, false);
  const FString Spoken =
      Lines.IsValidIndex(Config.Numbers.InitialIndex)
          ? Lines[Config.Numbers.InitialIndex].TrimStartAndEnd()
          : Config.EmptyContext;
  return Spoken.IsEmpty() || Spoken.StartsWith(Config.UnavailablePrefix)
             ? Config.DialogueFallback
             : Spoken;
}

/** User Story: As two-NPC thought diagnostics, I need the API-owned thought result selected from the thin CLI output without reinterpreting it. @fn inline FString ReadThoughtResult(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadThoughtResult(const FTwoNpcChatConfig &Config,
                                 const FString &Output) {
  TArray<FString> Lines;
  Output.ParseIntoArray(Lines, *Config.OutputLineSeparator, false);
  const FString Prefix =
      ForbocAI::CLI::NPC::readCliNpcState().Messages.ThoughtResultPrefix;
  const FString *Found = Lines.FindByPredicate(
      [&Prefix](const FString &Line) { return Line.StartsWith(Prefix); });
  return Found == nullptr ? Config.DialogueFallback
                          : Found->Mid(Prefix.Len());
}

} // namespace MicroGame::TwoNpcChat
