#pragma once

#include "Components/Protocol/Requests/RequestsTypes.h"
#include "Components/Protocol/Process/ProtocolProcessTypes.h"

namespace ProtocolRequests {

/** User Story: As shared protocol execution, I need every caller-owned tape input copied without SDK interpretation. @fn inline FNPCProcessTape ProcessTape(const FProtocolProcessInput &Input, const FAgentState &NpcState) */
inline FNPCProcessTape ProcessTape(const FProtocolProcessInput &Input,
                                   const FAgentState &NpcState) {
  FNPCProcessTape Tape;
  Tape.Observation = Input.Observation;
  Tape.ContextJson = Input.ContextJson;
  Tape.NpcState = NpcState;
  Tape.Persona = Input.Persona;
  Tape.bHasStructuredPersona = !Input.Persona.IsEmpty();
  Tape.bHasThoughtProfile = Input.bHasThoughtProfile;
  Tape.ThoughtProfile = Input.ThoughtProfile;
  Tape.bHasLegalActions = Input.bHasLegalActions;
  Tape.LegalActions = Input.LegalActions;
  Tape.bHasVisitedActions = Input.bHasVisitedActions;
  Tape.VisitedActions = Input.VisitedActions;
  Tape.bHasAvoidActions = Input.bHasAvoidActions;
  Tape.AvoidActions = Input.AvoidActions;
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
