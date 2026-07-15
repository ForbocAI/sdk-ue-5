#pragma once

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
 */
inline func::AsyncResult<FAgentResponse>
HandleDecision(const FNPCProcessResponse &Response,
               const FString &NpcId, const FString &Input,
               const FString &RunId, int32 Turn,
               const FProtocolHandlerContext &Runtime,
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const FRuntimeState &()> GetState) {
  FNPCProcessTape NextTape = Response.Tape;

  const FString ObsLower = Response.Tape.Observation.ToLower();
  const TArray<FString> ActionVerbs = {TEXT("attack"), TEXT("move"), TEXT("take"), TEXT("give"), TEXT("use"), TEXT("open"), TEXT("close"), TEXT("pick")};

  /**
   * Recursive predicate to detect action verbs without imperative loops.
   * User Story: As a maintainer, I need this note so the surrounding code intent
   * stays clear during maintenance and debugging.
   */
  const std::function<bool(int32)> ContainsActionVerb = [&](int32 Index) -> bool {
    return Index >= ActionVerbs.Num()
               ? false
               : (ObsLower.Contains(ActionVerbs[Index])
                      ? true
                      : ContainsActionVerb(Index + 1));
  };

  NextTape.DecisionIntent.ActionType = ContainsActionVerb(0) ? TEXT("INTERACT") : TEXT("SPEAK");

  const TArray<FString> TargetTokens = {TEXT("to"), TEXT("at"), TEXT("on"), TEXT("with")};

  /**
   * Recursive target extractor — walks the preposition list without an
   * imperative loop. Returns the trimmed substring after the first matching
   * " <token> " separator, or empty when no token matches.
   */
  const std::function<FString(const FString &, int32)> ExtractTargetRecursive =
      [&](const FString &Obs, int32 Index) -> FString {
    return Index >= TargetTokens.Num()
               ? FString(TEXT(""))
               : [&]() -> FString {
                   const FString &Token = TargetTokens[Index];
                   const int32 Pos = Obs.Find(TEXT(" ") + Token + TEXT(" "));
                   return Pos != INDEX_NONE
                              ? Obs.RightChop(Pos + Token.Len() + 2)
                                    .TrimStartAndEnd()
                              : ExtractTargetRecursive(Obs, Index + 1);
                 }();
  };

  NextTape.DecisionIntent.Target =
      NextTape.DecisionIntent.ActionType == TEXT("INTERACT")
          ? ExtractTargetRecursive(ObsLower, 0)
          : FString(TEXT(""));

  NextTape.DecisionIntent.Goal =
      Response.Tape.Memories.Num() > 0
          ? FString::Printf(
                TEXT("Respond to: %s (with %d recalled memories)"),
                *Response.Tape.Observation, Response.Tape.Memories.Num())
          : FString::Printf(TEXT("Respond to: %s"),
                            *Response.Tape.Observation);

  NextTape.bDecisionCompleted = true;

  return RunProtocolTurn(
      NpcId, Input, RunId, NextTape,
      SerializeDecisionResult(NextTape.DecisionIntent.Goal, NextTape.DecisionIntent.ActionType, NextTape.DecisionIntent.Target),
      true, Turn + 1, Runtime, Dispatch, GetState);
}

} // namespace rtk::detail
