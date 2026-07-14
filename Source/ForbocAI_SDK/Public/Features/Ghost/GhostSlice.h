#pragma once
/**
 * ᚷ ghost traffic stays traceable even when it feels supernatural
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"
#include "Features/Ghost/GhostActions.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicGhostGhostSliceHOptionalDomainId;
} } }

namespace GhostSlice {

using namespace rtk;

;

struct FGhostSessionProgressPayload {
  FString SessionId;
  FString Status;
  float Progress;

  FGhostSessionProgressPayload() : Progress(0.0f) {}
};

struct FGhostSessionFailedPayload {
  FString SessionId;
  FString Error;
};

struct FGhostSliceState {
  FString ActiveSessionId;
  FString Status;
  float Progress;
  FGhostTestReport Results;
  bool bHasResults;
  TArray<FGhostHistoryEntry> History;
  bool bLoading;
  FString Error;

  FGhostSliceState()
      : Status(TEXT("idle")), Progress(0.0f), bHasResults(false),
        bLoading(false) {}
};

namespace Actions {

/**
 * Returns the cached action creator for ghost session start events.
 * User Story: As ghost session orchestration, I need a stable action creator
 * so reducers and middleware can reuse the same start contract.
 */
inline const ActionCreator<FGhostSessionStartedPayload> &
ghostSessionStartedActionCreator() {
  static const ActionCreator<FGhostSessionStartedPayload> ActionCreator =
      createAction<FGhostSessionStartedPayload>(TEXT("ghost/sessionStarted"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for ghost session progress events.
 * User Story: As ghost progress tracking, I need a reusable action creator so
 * progress updates stay consistent across dispatch sites.
 */
inline const ActionCreator<FGhostSessionProgressPayload> &
ghostSessionProgressActionCreator() {
  static const ActionCreator<FGhostSessionProgressPayload> ActionCreator =
      createAction<FGhostSessionProgressPayload>(TEXT("ghost/sessionProgress"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for completed ghost sessions.
 * User Story: As ghost reporting, I need a reusable completion action creator
 * so finished runs can be stored with one contract.
 */
inline const ActionCreator<FGhostTestReport> &
ghostSessionCompletedActionCreator() {
  static const ActionCreator<FGhostTestReport> ActionCreator =
      createAction<FGhostTestReport>(TEXT("ghost/sessionCompleted"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for failed ghost sessions.
 * User Story: As ghost failure handling, I need a reusable failure action
 * creator so session errors can be reported consistently.
 */
inline const ActionCreator<FGhostSessionFailedPayload> &
ghostSessionFailedActionCreator() {
  static const ActionCreator<FGhostSessionFailedPayload> ActionCreator =
      createAction<FGhostSessionFailedPayload>(TEXT("ghost/sessionFailed"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for loading ghost history.
 * User Story: As ghost history views, I need a stable action creator so prior
 * runs can be loaded without custom action wiring.
 */
inline const ActionCreator<TArray<FGhostHistoryEntry>> &
ghostHistoryLoadedActionCreator() {
  static const ActionCreator<TArray<FGhostHistoryEntry>> ActionCreator =
      createAction<TArray<FGhostHistoryEntry>>(TEXT("ghost/historyLoaded"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for clearing ghost state.
 * User Story: As ghost session reset flows, I need one clear action creator so
 * teardown can restore the slice predictably.
 */
inline const ActionCreatorWithoutPayload &clearGhostSessionActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("ghost/clearGhostSession"));
  return ActionCreator;
}

/**
 * Creates an action that opens a new ghost test session.
 * User Story: As ghost run startup, I need session metadata captured so the UI
 * and reducers know which run is active.
 */
inline AnyAction ghostSessionStarted(const FString &SessionId,
                                     const FString &Status = TEXT("running")) {
  return ghostSessionStartedActionCreator()(
      FGhostSessionStartedPayload{SessionId, Status});
}

/**
 * Creates an action that updates ghost session progress state.
 * User Story: As ghost progress reporting, I need each progress tick recorded
 * so observers can render current status and percentage.
 */
inline AnyAction ghostSessionProgress(const FString &SessionId,
                                      const FString &Status, float Progress) {
  FGhostSessionProgressPayload Payload;
  Payload.SessionId = SessionId;
  Payload.Status = Status;
  Payload.Progress = Progress;
  return ghostSessionProgressActionCreator()(Payload);
}

/**
 * Creates an action that stores a completed ghost test report.
 * User Story: As ghost result consumers, I need the finished report preserved
 * so results can be reviewed after execution.
 */
inline AnyAction ghostSessionCompleted(const FGhostTestReport &Report) {
  return ghostSessionCompletedActionCreator()(Report);
}

/**
 * Creates an action that stores a ghost session failure.
 * User Story: As ghost error handling, I need failed sessions recorded so the
 * UI can explain why a run stopped.
 */
inline AnyAction ghostSessionFailed(const FString &SessionId,
                                    const FString &Error) {
  return ghostSessionFailedActionCreator()(
      FGhostSessionFailedPayload{SessionId, Error});
}

/**
 * Creates an action that replaces the cached ghost history.
 * User Story: As history views, I need the latest run history loaded so users
 * can inspect recent ghost sessions.
 */
inline AnyAction ghostHistoryLoaded(const TArray<FGhostHistoryEntry> &History) {
  return ghostHistoryLoadedActionCreator()(History);
}

/**
 * Creates an action that resets ghost session state.
 * User Story: As cleanup flows, I need ghost state cleared so a new run starts
 * from a known baseline.
 */
inline AnyAction clearGhostSession() {
  return clearGhostSessionActionCreator()();
}

} // namespace Actions

inline bool IsActiveSession(const FGhostSliceState &State,
                            const FString &SessionId) {
  return !State.ActiveSessionId.IsEmpty() && State.ActiveSessionId == SessionId;
}

template <typename Transform>
inline FGhostSliceState ReduceActiveSession(const FGhostSliceState &State,
                                            const FString &SessionId,
                                            Transform TransformState) {
  return IsActiveSession(State, SessionId) ? TransformState(State) : State;
}

/**
 * Builds the ghost slice reducer and initial state.
 * User Story: As ghost runtime setup, I need one slice factory so store
 * creation wires ghost actions and state transitions consistently.
 */
inline Slice<FGhostSliceState> createGhostSlice() {
  return rtk::createSlice<FGhostSliceState>(
  TEXT("ghost"),
                                                   FGhostSliceState(),
  [](rtk::ActionReducerMapBuilder<FGhostSliceState> &Builder) {
    Builder.addCase(Actions::ghostSessionStartedActionCreator(),
      [](const FGhostSliceState &State,
                             const Action<FGhostSessionStartedPayload> &Action)
                              -> FGhostSliceState {
                            FGhostSliceState Next = State;
                            Next.ActiveSessionId = Action.PayloadValue.SessionId;
                            Next.Status = Action.PayloadValue.Status;
                            Next.Progress = 0.0f;
                            Next.bLoading = false;
                            Next.Error.Empty();
                            Next.bHasResults = false;
                            return Next;
                          });
    Builder.addCase(Actions::ghostSessionProgressActionCreator(),
      [](const FGhostSliceState &State,
                             const Action<FGhostSessionProgressPayload> &Action)
                              -> FGhostSliceState {
                            return ReduceActiveSession(
                                State, Action.PayloadValue.SessionId,
                                [&Action](const FGhostSliceState &Active) {
                                  FGhostSliceState Next = Active;
                                  Next.Status = Action.PayloadValue.Status;
                                  Next.Progress = Action.PayloadValue.Progress;
                                  return Next;
                                });
                          });
    Builder.addCase(Actions::ghostSessionCompletedActionCreator(),
      [](const FGhostSliceState &State,
                   const Action<FGhostTestReport> &Action) -> FGhostSliceState {
                  FGhostSliceState Next = State;
                  Next.Results = Action.PayloadValue;
                  Next.bHasResults = true;
                  Next.Status = TEXT("completed");
                  Next.Progress = 1.0f;
                  Next.bLoading = false;
                  return Next;
                });
    Builder.addCase(Actions::ghostSessionFailedActionCreator(),
      [](const FGhostSliceState &State,
                             const Action<FGhostSessionFailedPayload> &Action)
                              -> FGhostSliceState {
                            return ReduceActiveSession(
                                State, Action.PayloadValue.SessionId,
                                [&Action](const FGhostSliceState &Active) {
                                  FGhostSliceState Next = Active;
                                  Next.Status = TEXT("failed");
                                  Next.bLoading = false;
                                  Next.Error = Action.PayloadValue.Error;
                                  return Next;
                                });
                          });
    Builder.addCase(Actions::ghostHistoryLoadedActionCreator(),
      [](const FGhostSliceState &State,
                             const Action<TArray<FGhostHistoryEntry>> &Action)
                              -> FGhostSliceState {
                            FGhostSliceState Next = State;
                            Next.History = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::clearGhostSessionActionCreator(),
      [](const FGhostSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FGhostSliceState {
                  return FGhostSliceState();
                });
  });
}

} // namespace GhostSlice
