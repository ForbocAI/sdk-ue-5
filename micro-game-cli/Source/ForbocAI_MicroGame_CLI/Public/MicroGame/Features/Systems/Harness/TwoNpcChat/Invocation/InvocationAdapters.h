#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/DefinitionTypes.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As a two-NPC chat adapter, I need one authored token replaced without duplicating template syntax. @fn inline FString ApplyToken(const FString &Template, const FString &Token, const FString &Value) */
inline FString ApplyToken(const FString &Template, const FString &Token,
                          const FString &Value) {
  return Template.Replace(*Token, *Value);
}

/** User Story: As a two-NPC chat adapter, I need a granular generate command composed from authored tokens. @fn inline FString FormatGenerate(const FTwoNpcChatConfig &Config, const FString &Attribute, const FString &Context) */
inline FString FormatGenerate(const FTwoNpcChatConfig &Config,
                              const FString &Attribute,
                              const FString &Context) {
  const FString Quoted = Context.IsEmpty()
                             ? Config.EmptyContext
                             : Config.ContextQuote + Context + Config.ContextQuote;
  return ApplyToken(
      ApplyToken(Config.GenerateCommand, Config.Tokens.Attribute, Attribute),
      Config.Tokens.Context, Quoted);
}

/** User Story: As a two-NPC chat adapter, I need an NPC create command composed from an authored persona token. @fn inline FString FormatCreate(const FTwoNpcChatConfig &Config, const FString &Persona) */
inline FString FormatCreate(const FTwoNpcChatConfig &Config,
                            const FString &Persona) {
  return ApplyToken(Config.CreateCommand, Config.Tokens.Persona, Persona);
}

/** User Story: As a two-NPC chat adapter, I need a decide command composed from authored identity and line tokens. @fn inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Line) */
inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id,
                            const FString &Line) {
  return ApplyToken(ApplyToken(Config.DecideCommand, Config.Tokens.Id, Id),
                    Config.Tokens.Line, Line);
}

/** User Story: As a two-NPC chat adapter, I need a memory command composed from authored identity and memory tokens. @fn inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Memory) */
inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config,
                                 const FString &Id, const FString &Memory) {
  return ApplyToken(
      ApplyToken(Config.MemoryStoreCommand, Config.Tokens.Id, Id),
      Config.Tokens.Memory, Memory);
}

} // namespace MicroGame::TwoNpcChat
