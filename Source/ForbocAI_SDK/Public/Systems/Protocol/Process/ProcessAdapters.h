#pragma once

#include "Components/Protocol/Process/ProcessTypes.h"

namespace ProtocolProcess {

/** User Story: As a protocol caller, I need the required actor and observation composed before optional transport fields are attached. @fn inline FProtocolProcessInput ProcessInput(const FString &NpcId, const FString &Observation) */
inline FProtocolProcessInput ProcessInput(const FString &NpcId,
                                          const FString &Observation) {
  FProtocolProcessInput Input;
  Input.NpcId = NpcId;
  Input.Observation = Observation;
  return Input;
}

} // namespace ProtocolProcess
