#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Components/Ghost/Identity/IdentityTypes.h"
#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Components/Ghost/Results/ResultsTypes.h"

namespace GhostSlice {

using namespace rtk;

struct FGhostSessionStartedPayload {
  FString SessionId;
  FString Status;
  FString GhostName;
  FGhostRuntimeIdentity RuntimeIdentity;
};

struct FGhostSessionProgressPayload {
  FString SessionId;
  FString Status;
  float Progress{};
};

struct FGhostSessionFailedPayload {
  FString SessionId;
  FString Error;
};

namespace Actions {

/**
 * User Story: As Ghost startup orchestration, I need one stable action contract carrying API identity.
 * @fn inline const ActionCreator<FGhostSessionStartedPayload> & ghostSessionStartedActionCreator()
 */
inline const ActionCreator<FGhostSessionStartedPayload> &
ghostSessionStartedActionCreator() {
  static const ActionCreator<FGhostSessionStartedPayload> Creator =
      createAction<FGhostSessionStartedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV70F10EBA107E));
  return Creator;
}

/**
 * User Story: As Ghost progress orchestration, I need one stable action contract for active-session updates.
 * @fn inline const ActionCreator<FGhostSessionProgressPayload> & ghostSessionProgressActionCreator()
 */
inline const ActionCreator<FGhostSessionProgressPayload> &
ghostSessionProgressActionCreator() {
  static const ActionCreator<FGhostSessionProgressPayload> Creator =
      createAction<FGhostSessionProgressPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC98C5C03A7C5));
  return Creator;
}

/**
 * User Story: As Ghost result ownership, I need the API result object carried unchanged into reducer state.
 * @fn inline const ActionCreator<FGhostResults> & ghostSessionCompletedActionCreator()
 */
inline const ActionCreator<FGhostResults> &
ghostSessionCompletedActionCreator() {
  static const ActionCreator<FGhostResults> Creator =
      createAction<FGhostResults>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4B495246B593));
  return Creator;
}

/**
 * User Story: As Ghost failure orchestration, I need one stable active-session failure contract.
 * @fn inline const ActionCreator<FGhostSessionFailedPayload> & ghostSessionFailedActionCreator()
 */
inline const ActionCreator<FGhostSessionFailedPayload> &
ghostSessionFailedActionCreator() {
  static const ActionCreator<FGhostSessionFailedPayload> Creator =
      createAction<FGhostSessionFailedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7CA0AB8FFB6C));
  return Creator;
}

/**
 * User Story: As Ghost history ownership, I need one stable action contract replacing the cached API history.
 * @fn inline const ActionCreator<TArray<FGhostHistoryEntry>> & ghostHistoryLoadedActionCreator()
 */
inline const ActionCreator<TArray<FGhostHistoryEntry>> &
ghostHistoryLoadedActionCreator() {
  static const ActionCreator<TArray<FGhostHistoryEntry>> Creator =
      createAction<TArray<FGhostHistoryEntry>>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3F9F89777C50));
  return Creator;
}

/** User Story: As Ghost teardown, I need one stable action restoring feature initial state. @fn inline const ActionCreatorWithoutPayload &clearGhostSessionActionCreator() */
inline const ActionCreatorWithoutPayload &clearGhostSessionActionCreator() {
  static const ActionCreatorWithoutPayload Creator =
      createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV540B3266BB82));
  return Creator;
}

/**
 * User Story: As Ghost startup callers, I need API identity assembled into one event without fallback values.
 * @fn inline AnyAction ghostSessionStarted( const FString &SessionId, const FString &Status, const FString &GhostName, const FGhostRuntimeIdentity &RuntimeIdentity)
 */
inline AnyAction ghostSessionStarted(
    const FString &SessionId, const FString &Status,
    const FString &GhostName,
    const FGhostRuntimeIdentity &RuntimeIdentity) {
  return ghostSessionStartedActionCreator()(
      FGhostSessionStartedPayload{SessionId, Status, GhostName,
                                  RuntimeIdentity});
}

/** User Story: As Ghost polling callers, I need active-session progress assembled into one event. @fn inline AnyAction ghostSessionProgress(const FString &SessionId, const FString &Status, float Progress) */
inline AnyAction ghostSessionProgress(const FString &SessionId,
                                      const FString &Status,
                                      float Progress) {
  return ghostSessionProgressActionCreator()(
      FGhostSessionProgressPayload{SessionId, Status, Progress});
}

/** User Story: As Ghost result callers, I need the exact API result object dispatched without reshaping. @fn inline AnyAction ghostSessionCompleted(const FGhostResults &Results) */
inline AnyAction ghostSessionCompleted(const FGhostResults &Results) {
  return ghostSessionCompletedActionCreator()(Results);
}

/** User Story: As Ghost failure callers, I need active session failures assembled into one event. @fn inline AnyAction ghostSessionFailed(const FString &SessionId, const FString &Error) */
inline AnyAction ghostSessionFailed(const FString &SessionId,
                                    const FString &Error) {
  return ghostSessionFailedActionCreator()(
      FGhostSessionFailedPayload{SessionId, Error});
}

/**
 * User Story: As Ghost history callers, I need API history dispatched as one immutable collection.
 * @fn inline AnyAction ghostHistoryLoaded( const TArray<FGhostHistoryEntry> &History)
 */
inline AnyAction ghostHistoryLoaded(
    const TArray<FGhostHistoryEntry> &History) {
  return ghostHistoryLoadedActionCreator()(History);
}

/** User Story: As Ghost teardown callers, I need one event restoring feature initial state. @fn inline AnyAction clearGhostSession() */
inline AnyAction clearGhostSession() {
  return clearGhostSessionActionCreator()();
}

} // namespace Actions
} // namespace GhostSlice
