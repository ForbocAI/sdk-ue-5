#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/DefinitionTypes.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As a two-NPC chat adapter, I need generated values joined through the authored persona separator. @fn inline FString JoinPersona(const FTwoNpcChatConfig &Config, const TArray<FString> &Values) */
inline FString JoinPersona(const FTwoNpcChatConfig &Config,
                           const TArray<FString> &Values) {
  return FString::Join(Values, *Config.PersonaSeparator);
}

/** User Story: As a two-NPC chat adapter, I need generated context extended through the authored separator. @fn inline FString ExtendContext(const FTwoNpcChatConfig &Config, const FString &Context, const FString &Value) */
inline FString ExtendContext(const FTwoNpcChatConfig &Config,
                             const FString &Context, const FString &Value) {
  return Context.IsEmpty() ? Value
                           : Context + Config.ContextSeparator + Value;
}

/** User Story: As a two-NPC chat adapter, I need the opening seed selected by authored attribute name. @fn inline FString SeedValue(const FTwoNpcChatConfig &Config, const TArray<FString> &Values) */
inline FString SeedValue(const FTwoNpcChatConfig &Config,
                         const TArray<FString> &Values) {
  const int32 Index =
      Config.PersonaAttributes.IndexOfByKey(Config.SeedAttribute);
  return Values.IsValidIndex(Index) ? Values[Index] : Config.DialogueFallback;
}

} // namespace MicroGame::TwoNpcChat
