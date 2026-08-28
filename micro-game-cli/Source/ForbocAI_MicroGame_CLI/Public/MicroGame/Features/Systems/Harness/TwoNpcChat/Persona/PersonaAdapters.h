#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/TwoNpcChatDefinitionTypes.h"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/Memory/TwoNpcChatMemoryTypes.h"
#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Invocation/InvocationAdapters.h"

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

/** User Story: As two-NPC identity presentation, I need the generated role selected by authored attribute name. @fn inline FString RoleValue(const FTwoNpcChatConfig &Config, const TArray<FString> &Values) */
inline FString RoleValue(const FTwoNpcChatConfig &Config,
                         const TArray<FString> &Values) {
  const int32 Index =
      Config.PersonaAttributes.IndexOfByKey(Config.RoleAttribute);
  return Values.IsValidIndex(Index) ? Values[Index]
                                    : Config.DialogueFallback;
}

/** User Story: As two-NPC transcript identity, I need a generated name and role rendered through the authored title template. @fn inline FString FormatDisplayName(const FTwoNpcChatConfig &Config, const FString &Name, const FString &Role) */
inline FString FormatDisplayName(const FTwoNpcChatConfig &Config,
                                 const FString &Name,
                                 const FString &Role) {
  return Role.TrimStartAndEnd().IsEmpty()
             ? Name
             : ApplyToken(
                   ApplyToken(Config.TitleFormat, Config.Tokens.Name, Name),
                   Config.Tokens.Role, Role);
}

/**
 * User Story: As structured persona persistence, I need scene, typed attributes, and a known peer projected into typed memory records without storing identity fields as traits.
 * @fn inline TArray<FTypedMemory> PersonaMemories( const FTwoNpcChatConfig &Config, const FString &Scene, const TArray<FString> &Values, const FString &Peer)
 */
inline TArray<FTypedMemory> PersonaMemories(
    const FTwoNpcChatConfig &Config, const FString &Scene,
    const TArray<FString> &Values, const FString &Peer) {
  const TArray<FTypedMemory> Attributes =
      func::filter_array<FTypedMemory>(
          func::map_array<FString, FTypedMemory>(
              Config.PersonaAttributes,
              [&Config, &Values](const FString &Attribute) {
                const int32 Index =
                    Config.PersonaAttributes.IndexOfByKey(Attribute);
                const FString *Type =
                    Config.AttributeMemoryTypes.Find(Attribute);
                return FTypedMemory{
                    Values.IsValidIndex(Index) ? Values[Index]
                                               : Config.EmptyContext,
                    Type == nullptr ? Config.EmptyContext : *Type};
              }),
          [](const FTypedMemory &Memory) {
            return !Memory.Text.IsEmpty() && !Memory.Type.IsEmpty();
          });
  return func::filter_array<FTypedMemory>(
      func::concat_arrays<FTypedMemory>({
          {{Scene, Config.SceneMemoryType}},
          Attributes,
          Peer.IsEmpty()
              ? TArray<FTypedMemory>()
              : TArray<FTypedMemory>{
                    {Peer, Config.RelationshipMemoryType}},
      }),
      [](const FTypedMemory &Memory) { return !Memory.Text.IsEmpty(); });
}

} // namespace MicroGame::TwoNpcChat
