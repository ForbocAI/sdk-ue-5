#pragma once

#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Invocation/InvocationAdapters.h"

namespace MicroGame::TwoNpcChat {

/**
 * User Story: As a two-NPC chat adapter, I need one transcript line composed from authored speaker and dialogue tokens.
 * @fn inline FString FormatLine(const FTwoNpcChatConfig &Config, int32 Turn, const FString &Name, const FString &Dialogue)
 */
inline FString FormatLine(const FTwoNpcChatConfig &Config, int32 Turn,
                          const FString &Name, const FString &Dialogue) {
  return ApplyToken(
      ApplyToken(
          ApplyToken(Config.LineFormat, Config.Tokens.Turn,
                     FString::FromInt(Turn)),
          Config.Tokens.Name, Name),
      Config.Tokens.Dialogue, Dialogue);
}

/** User Story: As the two-NPC analytical transcript, I need the API-owned thought result paired with its speaker through authored tokens. @fn inline FString FormatThought(const FTwoNpcChatConfig &Config, const FString &Name, const FString &Thought) */
inline FString FormatThought(const FTwoNpcChatConfig &Config,
                             const FString &Name,
                             const FString &Thought) {
  return ApplyToken(
      ApplyToken(Config.ThoughtFormat, Config.Tokens.Name, Name),
      Config.Tokens.Thought, Thought);
}

} // namespace MicroGame::TwoNpcChat
