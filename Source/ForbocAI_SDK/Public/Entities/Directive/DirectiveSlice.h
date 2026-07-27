#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
/**
 * directive wire carries intent; static is not an excuse
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Components/Contracts/ContractsTypes.h"
#include "Entities/Directive/DirectiveActions.h"
#include "Systems/Directive/DirectiveAdapters.h"
#include "Components/Directive/DirectiveTypes.h"

namespace DirectiveSlice {

using namespace rtk;
using namespace func;

/** User Story: As a features directive consumer, I need to invoke create directive initial state through a stable signature so the features directive workflow remains explicit and composable. @fn inline FDirectiveSliceState createDirectiveInitialState() */
inline FDirectiveSliceState createDirectiveInitialState() {
  FDirectiveSliceState State;
  State.Entities = GetDirectiveAdapter().getInitialState();
  return State;
}

/**
 * Builds the directive slice reducer and initial state.
 * User Story: As directive runtime setup, I need one slice factory so all
 * directive lifecycle actions share a consistent reducer.
 * @fn inline Slice<FDirectiveSliceState> createDirectiveSlice()
 */
inline Slice<FDirectiveSliceState> createDirectiveSlice() {
  return rtk::createSlice<FDirectiveSliceState>(
  TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFDB85247065),
                                                       createDirectiveInitialState(),
  [](rtk::ActionReducerMapBuilder<FDirectiveSliceState> &Builder) {
    Builder.addCase(Actions::directiveRunStartedActionCreator(),
      [](const FDirectiveSliceState &State,
                             const Action<FDirectiveRunStartedPayload> &Action)
                              -> FDirectiveSliceState {
                            FDirectiveSliceState Next = State;
                            FDirectiveRun Run;
                            Run.Id = Action.PayloadValue.Id;
                            Run.NpcId = Action.PayloadValue.NpcId;
                            Run.Observation = Action.PayloadValue.Observation;
                            Run.Status = EDirectiveStatus::Running;
                            Run.StartedAt = FDateTime::UtcNow().ToUnixTimestamp();
                            Next.Entities =
                                GetDirectiveAdapter().upsertOne(Next.Entities, Run);
                            Next.ActiveDirectiveId = Run.Id;
                            return Next;
                          });
    Builder.addCase(Actions::directiveReceivedActionCreator(),
      [](const FDirectiveSliceState &State,
                             const Action<FDirectiveReceivedPayload> &Action)
                              -> FDirectiveSliceState {
                            FDirectiveSliceState Next = State;
                            const FDirectiveReceivedPayload &Payload =
                                Action.PayloadValue;
                            Next.Entities = GetDirectiveAdapter().updateOne(
                                Next.Entities, Payload.Id,
                                [Payload](const FDirectiveRun &Existing) {
                                  FDirectiveRun Updated = Existing;
                                  Updated.MemoryRecallQuery =
                                      Payload.Response.recallMemory.Query;
                                  Updated.MemoryRecallLimit =
                                      Payload.Response.recallMemory.Limit;
                                  Updated.MemoryRecallThreshold =
                                      Payload.Response.recallMemory.Threshold;
                                  return Updated;
                                });
                            return Next;
                          });
    Builder.addCase(Actions::verdictValidatedActionCreator(),
      [](const FDirectiveSliceState &State,
                   const Action<FVerdictValidatedPayload> &Action)
                    -> FDirectiveSliceState {
                  FDirectiveSliceState Next = State;
                  const FVerdictValidatedPayload &Payload = Action.PayloadValue;
                  Next.Entities = GetDirectiveAdapter().updateOne(
                      Next.Entities, Payload.Id,
                      [Payload](const FDirectiveRun &Existing) {
                        FDirectiveRun Updated = Existing;
                        Updated.Status = EDirectiveStatus::Completed;
                        Updated.CompletedAt = FDateTime::UtcNow().ToUnixTimestamp();
                        Updated.bVerdictValid = Payload.Verdict.bValid;
                        Updated.VerdictDialogue = Payload.Verdict.Dialogue;
                        Updated.VerdictActionType = Payload.Verdict.bHasAction
                                                        ? Payload.Verdict.Action.Type
                                                        : TEXT("");
                        return Updated;
                      });
                  return Next;
                });
    Builder.addCase(Actions::directiveRunFailedActionCreator(),
      [](const FDirectiveSliceState &State,
                             const Action<FDirectiveRunFailedPayload> &Action)
                              -> FDirectiveSliceState {
                            FDirectiveSliceState Next = State;
                            const FDirectiveRunFailedPayload &Payload =
                                Action.PayloadValue;
                            Next.Entities = GetDirectiveAdapter().updateOne(
                                Next.Entities, Payload.Id,
                                [Payload](const FDirectiveRun &Existing) {
                                  FDirectiveRun Updated = Existing;
                                  Updated.Status = EDirectiveStatus::Failed;
                                  Updated.CompletedAt =
                                      FDateTime::UtcNow().ToUnixTimestamp();
                                  Updated.Error = Payload.Error;
                                  return Updated;
                                });
                            return Next;
                          });
    Builder.addCase(Actions::clearDirectivesForNpcActionCreator(),
      [](const FDirectiveSliceState &State,
                   const Action<FString> &Action) -> FDirectiveSliceState {
                  FDirectiveSliceState Next = State;
                  const TArray<FDirectiveRun> Runs =
                      GetDirectiveAdapter().getSelectors().selectAll(Next.Entities);
                  struct CollectIds {
                    static void apply(
                        const TArray<FDirectiveRun> &R,
                        const FString &NpcId,
                        TArray<FString> &Out,
                        int32 Idx) {
                      Idx >= R.Num()
                          ? void()
                          : (R[Idx].NpcId == NpcId
                                 ? (Out.Add(R[Idx].Id), void())
                                 : void(),
                             apply(R, NpcId, Out, Idx + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4), void());
                    }
                  };
                  TArray<FString> IdsToRemove;
                  CollectIds::apply(Runs, Action.PayloadValue, IdsToRemove, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
                  Next.Entities =
                      GetDirectiveAdapter().removeMany(Next.Entities, IdsToRemove);
                  IdsToRemove.Contains(Next.ActiveDirectiveId)
                      ? (Next.ActiveDirectiveId.Empty(), void())
                      : void();
                  return Next;
                });
  });
}

} // namespace DirectiveSlice
