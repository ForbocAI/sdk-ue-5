#pragma once

#include "MicroGame/Features/Systems/Harness/TwoNpcChat/Invocation/InvocationAdapters.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As a two-NPC chat adapter, I need one transcript line composed from authored speaker and dialogue tokens. @fn inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name, const FString &Dialogue) */
inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name,
                          const FString &Dialogue) {
  return ApplyToken(ApplyToken(Config.LineFormat, Config.Tokens.Name, Name),
                    Config.Tokens.Dialogue, Dialogue);
}

} // namespace MicroGame::TwoNpcChat
