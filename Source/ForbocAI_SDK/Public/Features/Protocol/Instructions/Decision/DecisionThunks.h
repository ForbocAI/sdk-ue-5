#pragma once

#include "Features/NPC/NPCSelectors.h"
#include "Features/Protocol/Instructions/Decision/DecisionAdapters.h"
#include "Features/Protocol/Turn/TurnAdapters.h"

namespace rtk::detail {

/**
 * Handles the Decision protocol instruction by returning a decision intent.
 * User Story: As protocol instruction dispatch, I need the Decision step
 * handled so the multi-round loop can advance past the decision phase
 * without stalling. The API issues DecisionInstruction after QueryVector;
 * the SDK must return a DecisionResult with goal and actionType so the
 * tape includes decisionIntent on subsequent /process calls.
 *
 * Implementation (2026-04-28): Derives goal and actionType from observation
 * and memories, mirroring the API Orchestrator.hs logic.
 * @fn template <typename RuntimeState> inline func::AsyncResult<FAgentResponse> HandleDecision(const FNPCProcessResponse &Response, const FString &NpcId, const FString &Input, const FString &RunId, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<FAgentResponse>
HandleDecision(const FNPCProcessResponse &Response,
               const FString &NpcId, const FString &Input,
               const FString &RunId, int32 Turn,
               const FProtocolHandlerContext &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const RuntimeState &()> GetState) {
  const auto &Data = ProtocolConfiguration::protocolData();
  const func::Maybe<TArray<FNPCHistoryEntry>> History =
      NPCSelectors::selectNPCHistory(GetState().NPCs, NpcId);
  FNPCProcessTape NextTape = Response.Tape;
  NextTape.DecisionIntent = buildDecisionIntent(
      Response.Tape,
      func::is_just(History) ? History.value : TArray<FNPCHistoryEntry>());
  NextTape.bDecisionCompleted = true;

  return RunProtocolTurn(
      NpcId, Input, RunId, NextTape,
      SerializeDecisionResult(NextTape.DecisionIntent.Goal, NextTape.DecisionIntent.ActionType, NextTape.DecisionIntent.Target),
      true, Turn + Data.Iteration.Step, Runtime, Dispatch, GetState);
}

} // namespace rtk::detail
