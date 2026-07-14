#pragma once
/**
 * Soul action creators own soul event construction.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

namespace SoulSlice {

using namespace rtk;

namespace Actions {

/**
 * Returns the cached action creator for remote soul export start events.
 * User Story: As soul export orchestration, I need a stable pending action
 * creator so reducers and thunks reuse one export-start contract.
 */
inline const ActionCreatorWithoutPayload &remoteExportSoulPendingActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("soul/remoteExportPending"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for successful soul exports.
 * User Story: As soul export completion flows, I need a reusable success
 * action creator so export results are stored consistently.
 */
inline const ActionCreator<FSoulExportResult> &
remoteExportSoulSuccessActionCreator() {
  static const ActionCreator<FSoulExportResult> ActionCreator =
      createAction<FSoulExportResult>(TEXT("soul/remoteExportSuccess"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for failed soul exports.
 * User Story: As soul export error handling, I need a reusable failure action
 * creator so errors propagate through one contract.
 */
inline const ActionCreator<FString> &remoteExportSoulFailedActionCreator() {
  static const ActionCreator<FString> ActionCreator =
      createAction<FString>(TEXT("soul/remoteExportFailed"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for soul import start events.
 * User Story: As soul import orchestration, I need one pending action creator
 * so reducers can track import startup consistently.
 */
inline const ActionCreatorWithoutPayload &importSoulPendingActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("soul/importPending"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for successful soul imports.
 * User Story: As soul import flows, I need a reusable success action creator
 * so imported payloads are stored the same way everywhere.
 */
inline const ActionCreator<FSoul> &importSoulSuccessActionCreator() {
  static const ActionCreator<FSoul> ActionCreator =
      createAction<FSoul>(TEXT("soul/importSuccess"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for failed soul imports.
 * User Story: As soul import error handling, I need a reusable failure action
 * creator so import errors stay consistent across callers.
 */
inline const ActionCreator<FString> &importSoulFailedActionCreator() {
  static const ActionCreator<FString> ActionCreator =
      createAction<FString>(TEXT("soul/importFailed"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for replacing the available soul list.
 * User Story: As soul catalog views, I need a stable action creator so remote
 * listings can populate the slice with one contract.
 */
inline const ActionCreator<TArray<FSoulListItem>> &setSoulListActionCreator() {
  static const ActionCreator<TArray<FSoulListItem>> ActionCreator =
      createAction<TArray<FSoulListItem>>(TEXT("soul/setSoulList"));
  return ActionCreator;
}

/**
 * Returns the cached action creator for resetting soul state.
 * User Story: As cleanup flows, I need a dedicated clear action creator so the
 * soul slice can return to defaults predictably.
 */
inline const ActionCreatorWithoutPayload &clearSoulStateActionCreator() {
  static const ActionCreatorWithoutPayload ActionCreator =
      createAction(TEXT("soul/clearSoulState"));
  return ActionCreator;
}

/**
 * Creates an action that marks a remote soul export as pending.
 * User Story: As export status tracking, I need pending state recorded so the
 * UI can show that an export is in flight.
 */
inline AnyAction remoteExportSoulPending() {
  return remoteExportSoulPendingActionCreator()();
}

/**
 * Creates an action that stores the latest remote soul export result.
 * User Story: As export result consumers, I need the final export metadata
 * saved so downstream flows can use the produced soul reference.
 */
inline AnyAction remoteExportSoulSuccess(const FSoulExportResult &Result) {
  return remoteExportSoulSuccessActionCreator()(Result);
}

/**
 * Creates an action that stores a remote soul export failure.
 * User Story: As export error handling, I need failures captured so callers
 * can explain why an export did not complete.
 */
inline AnyAction remoteExportSoulFailed(const FString &Error) {
  return remoteExportSoulFailedActionCreator()(Error);
}

/**
 * Creates an action that marks a soul import as pending.
 * User Story: As import status tracking, I need pending state stored so the UI
 * can reflect that a soul import is underway.
 */
inline AnyAction importSoulPending() {
  return importSoulPendingActionCreator()();
}

/**
 * Creates an action that stores the latest imported soul payload.
 * User Story: As import result consumers, I need the imported soul preserved
 * so later reducers and views can reuse it.
 */
inline AnyAction importSoulSuccess(const FSoul &Soul) {
  return importSoulSuccessActionCreator()(Soul);
}

/**
 * Creates an action that stores a soul import failure.
 * User Story: As import error handling, I need the failure recorded so callers
 * can surface useful feedback to the user.
 */
inline AnyAction importSoulFailed(const FString &Error) {
  return importSoulFailedActionCreator()(Error);
}

/**
 * Creates an action that replaces the available soul list.
 * User Story: As soul listing flows, I need the latest list stored so browsing
 * and selection use current remote data.
 */
inline AnyAction setSoulList(const TArray<FSoulListItem> &SoulList) {
  return setSoulListActionCreator()(SoulList);
}

/**
 * Creates an action that resets the soul slice to its initial state.
 * User Story: As cleanup flows, I need soul state cleared so later exports and
 * imports start from a known baseline.
 */
inline AnyAction clearSoulState() { return clearSoulStateActionCreator()(); }

} // namespace Actions

} // namespace SoulSlice
