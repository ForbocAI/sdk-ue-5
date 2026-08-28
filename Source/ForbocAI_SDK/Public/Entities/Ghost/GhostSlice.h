#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Components/Ghost/State/GhostStateTypes.h"
#include "Entities/Ghost/GhostActions.h"
#include "Systems/Ghost/Configuration/GhostConfigurationAdapters.h"

namespace GhostSlice {

using namespace rtk;

/** User Story: As an isolated UE SDK store, I need fresh Ghost state with feature-authored policy. @fn inline FGhostSliceState createGhostInitialState() */
inline FGhostSliceState createGhostInitialState() {
  FGhostSliceState State;
  State.Configuration = GhostConfiguration::ghostConfiguration();
  State.Status = State.Configuration.IdleStatus;
  State.Progress = State.Configuration.EmptyProgress;
  State.Error = State.Configuration.EmptyMessage;
  return State;
}

/** User Story: As Ghost reducers, I need updates restricted to the currently active API session. @fn inline bool IsActiveSession(const FGhostSliceState &State, const FString &SessionId) */
inline bool IsActiveSession(const FGhostSliceState &State,
                            const FString &SessionId) {
  return !State.ActiveSessionId.IsEmpty() &&
         State.ActiveSessionId == SessionId;
}

/** User Story: As Ghost reducers, I need active-session transforms composed without mutation outside reducer ownership. @fn template <typename Transform> inline FGhostSliceState ReduceActiveSession(const FGhostSliceState &State, const FString &SessionId, Transform TransformState) */
template <typename Transform>
inline FGhostSliceState ReduceActiveSession(const FGhostSliceState &State,
                                            const FString &SessionId,
                                            Transform TransformState) {
  return IsActiveSession(State, SessionId) ? TransformState(State) : State;
}

/** User Story: As the package root store, I need one Ghost slice owning lifecycle, API identity, evidence, and history. @fn inline Slice<FGhostSliceState> createGhostSlice() */
inline Slice<FGhostSliceState> createGhostSlice() {
  const FGhostSliceState InitialState = createGhostInitialState();
  return rtk::createSlice<FGhostSliceState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV10CB016FAA00), InitialState,
      [InitialState](rtk::ActionReducerMapBuilder<FGhostSliceState> &Builder) {
        Builder.addCase(
            Actions::ghostSessionStartedActionCreator(),
            [](const FGhostSliceState &State,
               const Action<FGhostSessionStartedPayload> &Action) {
              FGhostSliceState Next = State;
              Next.ActiveSessionId = Action.PayloadValue.SessionId;
              Next.ActiveGhostName = func::just(Action.PayloadValue.GhostName);
              Next.RuntimeIdentity =
                  func::just(Action.PayloadValue.RuntimeIdentity);
              Next.Status = Action.PayloadValue.Status;
              Next.Progress = Next.Configuration.EmptyProgress;
              Next.Results = FGhostResults{};
              Next.bHasResults = false;
              Next.bLoading = false;
              Next.Error = Next.Configuration.EmptyMessage;
              return Next;
            });
        Builder.addCase(
            Actions::ghostSessionProgressActionCreator(),
            [](const FGhostSliceState &State,
               const Action<FGhostSessionProgressPayload> &Action) {
              return ReduceActiveSession(
                  State, Action.PayloadValue.SessionId,
                  [&Action](const FGhostSliceState &Active) {
                    FGhostSliceState Next = Active;
                    Next.Status = Action.PayloadValue.Status;
                    Next.Progress = Action.PayloadValue.Progress;
                    return Next;
                  });
            });
        Builder.addCase(
            Actions::ghostSessionCompletedActionCreator(),
            [](const FGhostSliceState &State,
               const Action<FGhostResults> &Action) {
              FGhostSliceState Next = State;
              Next.Results = Action.PayloadValue;
              Next.ActiveGhostName =
                  func::just(Action.PayloadValue.GhostName);
              Next.RuntimeIdentity =
                  func::just(Action.PayloadValue.RuntimeIdentity);
              Next.bHasResults = true;
              Next.Status = Next.Configuration.CompletedStatus;
              Next.Progress = Next.Configuration.CompleteProgress;
              Next.bLoading = false;
              return Next;
            });
        Builder.addCase(
            Actions::ghostSessionFailedActionCreator(),
            [](const FGhostSliceState &State,
               const Action<FGhostSessionFailedPayload> &Action) {
              return ReduceActiveSession(
                  State, Action.PayloadValue.SessionId,
                  [&Action](const FGhostSliceState &Active) {
                    FGhostSliceState Next = Active;
                    Next.Status = Next.Configuration.FailedStatus;
                    Next.bLoading = false;
                    Next.Error = Action.PayloadValue.Error;
                    return Next;
                  });
            });
        Builder.addCase(
            Actions::ghostHistoryLoadedActionCreator(),
            [](const FGhostSliceState &State,
               const Action<TArray<FGhostHistoryEntry>> &Action) {
              FGhostSliceState Next = State;
              Next.History = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::clearGhostSessionActionCreator(),
            [InitialState](const FGhostSliceState &,
                           const Action<rtk::FEmptyPayload> &) {
              return InitialState;
            });
      });
}

} // namespace GhostSlice
