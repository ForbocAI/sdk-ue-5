#pragma once

#include "Core/JsonInterop.h"
#include "Internationalization/Regex.h"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/DefinitionTypes.h"

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
  const FString Spoken = Output.TrimStartAndEnd();
  return Spoken.IsEmpty() ? Config.DialogueFallback : Spoken;
}

} // namespace MicroGame::TwoNpcChat
