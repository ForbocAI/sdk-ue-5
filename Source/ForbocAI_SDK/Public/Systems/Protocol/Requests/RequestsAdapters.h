#pragma once

#include "Components/Protocol/Requests/RequestsTypes.h"

namespace ProtocolRequests {

/** User Story: As a features protocol requests consumer, I need to invoke process tape through a stable signature so the features protocol requests workflow remains explicit and composable. @fn inline FNPCProcessTape ProcessTape(const FString &Observation, const FString &ContextJson, const FAgentState &NpcState, const FString &Persona) */
inline FNPCProcessTape ProcessTape(const FString &Observation,
                                   const FString &ContextJson,
                                   const FAgentState &NpcState,
                                   const FString &Persona) {
  FNPCProcessTape Tape;
  Tape.Observation = Observation;
  Tape.ContextJson = ContextJson;
  Tape.NpcState = NpcState;
  Tape.Persona = Persona;
  Tape.bHasStructuredPersona = !Persona.IsEmpty();
  return Tape;
}

/** User Story: As a features protocol requests consumer, I need to invoke directive request through a stable signature so the features protocol requests workflow remains explicit and composable. @fn inline FDirectiveRequest DirectiveRequest(const FString &Observation, const FAgentState &NpcState, const FString &ContextJson) */
inline FDirectiveRequest DirectiveRequest(const FString &Observation,
                                          const FAgentState &NpcState,
                                          const FString &ContextJson) {
  FDirectiveRequest Request;
  Request.Observation = Observation;
  Request.NpcState = NpcState;
  Request.ContextJson = ContextJson;
  return Request;
}

} // namespace ProtocolRequests
