#pragma once

#include "Components/CLI/NPC/CLINPCTypes.h"

namespace ForbocAI {
namespace CLI {
struct FCLIState;
namespace NPC {

/** User Story: As a features cli npc consumer, I need to invoke select cli npc through a stable signature so the features cli npc workflow remains explicit and composable. @fn const FCLINPCState &selectCliNpc(const FCLIState &State) */
const FCLINPCState &selectCliNpc(const FCLIState &State);

/** User Story: As a features cli npc consumer, I need to invoke select dialogue through a stable signature so the features cli npc workflow remains explicit and composable. @fn inline FString selectDialogue(const FAgentResponse &Response, const FCLINPCState &State) */
inline FString selectDialogue(const FAgentResponse &Response,
                              const FCLINPCState &State) {
  return Response.Dialogue.IsEmpty() ? State.Messages.DialogueFallback
                                     : Response.Dialogue;
}

} // namespace NPC
} // namespace CLI
} // namespace ForbocAI
