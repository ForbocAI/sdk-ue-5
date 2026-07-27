#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Entities/Soul/SoulActions.h"
#include "Systems/Soul/SoulThunks.h"
#include "Components/Soul/SoulTypes.h"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"

namespace SoulSlice {

/**
 * User Story: As a Soul reducer, I need export startup represented without side effects so status is derived from generated RTK events.
 * @fn inline FSoulSliceState startSoulExportReducer( const FSoulSliceState &State, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState startSoulExportReducer(
    const FSoulSliceState &State,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ExportStatus = Lifecycle.Exporting;
  Next.Error.Empty();
  return Next;
}

/**
 * User Story: As a Soul reducer, I need successful export metadata stored as one immutable state transition.
 * @fn inline FSoulSliceState completeSoulExportReducer( const FSoulSliceState &State, const FSoulExportResult &Result, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState completeSoulExportReducer(
    const FSoulSliceState &State, const FSoulExportResult &Result,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ExportStatus = Lifecycle.Succeeded;
  Next.ExportResult = Result;
  Next.bHasExportResult = true;
  return Next;
}

/**
 * User Story: As a Soul reducer, I need export failures stored without performing transport work in the reducer.
 * @fn inline FSoulSliceState failSoulExportReducer(const FSoulSliceState &State, const FString &Error, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState failSoulExportReducer(const FSoulSliceState &State,
                                             const FString &Error,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ExportStatus = Lifecycle.Failed;
  Next.Error = Error;
  return Next;
}

/**
 * User Story: As a Soul reducer, I need import startup represented without side effects so status is derived from generated RTK events.
 * @fn inline FSoulSliceState startSoulImportReducer( const FSoulSliceState &State, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState startSoulImportReducer(
    const FSoulSliceState &State,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ImportStatus = Lifecycle.Importing;
  Next.Error.Empty();
  return Next;
}

/**
 * User Story: As a Soul reducer, I need authenticated imports stored as one immutable state transition.
 * @fn inline FSoulSliceState completeSoulImportReducer(const FSoulSliceState &State, const FSoul &Soul, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState completeSoulImportReducer(const FSoulSliceState &State,
                                                 const FSoul &Soul,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ImportStatus = Lifecycle.Succeeded;
  Next.ImportedSoul = Soul;
  Next.bHasImportedSoul = true;
  return Next;
}

/**
 * User Story: As a Soul reducer, I need import failures stored without performing provider work in the reducer.
 * @fn inline FSoulSliceState failSoulImportReducer(const FSoulSliceState &State, const FString &Error, const SoulStorage::Configuration::FLifecycleData &Lifecycle)
 */
inline FSoulSliceState failSoulImportReducer(const FSoulSliceState &State,
                                             const FString &Error,
    const SoulStorage::Configuration::FLifecycleData &Lifecycle) {
  FSoulSliceState Next = State;
  Next.ImportStatus = Lifecycle.Failed;
  Next.Error = Error;
  return Next;
}

/**
 * User Story: As the package root store, I need generated Soul thunk lifecycle
 * events reduced through one feature slice.
 * @fn inline rtk::Slice<FSoulSliceState> createSoulSlice()
 */
inline rtk::Slice<FSoulSliceState> createSoulSlice() {
  const SoulStorage::Configuration::FLifecycleData Lifecycle =
      SoulStorage::Configuration::soulStorageData().Lifecycle;
  FSoulSliceState InitialState;
  InitialState.ExportStatus = Lifecycle.Idle;
  InitialState.ImportStatus = Lifecycle.Idle;
  return rtk::createSlice<FSoulSliceState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGVED5418A84438), InitialState,
      [Lifecycle, InitialState](
          rtk::ActionReducerMapBuilder<FSoulSliceState> &Builder) {
        Builder.addCase(
            rtk::exportSoulThunk().pending,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FString> &) {
              return startSoulExportReducer(State, Lifecycle);
            });
        Builder.addCase(
            rtk::exportSoulThunk().fulfilled,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FSoulExportResult> &Action) {
              return completeSoulExportReducer(State, Action.PayloadValue,
                                               Lifecycle);
            });
        Builder.addCase(
            rtk::exportSoulThunk().rejected,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FString> &Action) {
              return failSoulExportReducer(State, Action.PayloadValue,
                                           Lifecycle);
            });
        Builder.addCase(
            rtk::importSoulThunk().pending,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FString> &) {
              return startSoulImportReducer(State, Lifecycle);
            });
        Builder.addCase(
            rtk::importSoulThunk().fulfilled,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FSoul> &Action) {
              return completeSoulImportReducer(State, Action.PayloadValue,
                                               Lifecycle);
            });
        Builder.addCase(
            rtk::importSoulThunk().rejected,
            [Lifecycle](const FSoulSliceState &State,
               const rtk::Action<FString> &Action) {
              return failSoulImportReducer(State, Action.PayloadValue,
                                           Lifecycle);
            });
        Builder.addCase(
            rtk::listSoulsThunk().pending,
            [](const FSoulSliceState &State, const rtk::Action<int32> &) {
              FSoulSliceState Next = State;
              Next.bListing = true;
              return Next;
            });
        Builder.addCase(
            rtk::listSoulsThunk().fulfilled,
            [](const FSoulSliceState &State,
               const rtk::Action<TArray<FSoulListItem>> &Action) {
              FSoulSliceState Next = State;
              Next.AvailableSouls = Action.PayloadValue;
              Next.bListing = false;
              return Next;
            });
        Builder.addCase(
            rtk::listSoulsThunk().rejected,
            [](const FSoulSliceState &State,
               const rtk::Action<FString> &Action) {
              FSoulSliceState Next = State;
              Next.bListing = false;
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            rtk::verifySoulThunk().pending,
            [](const FSoulSliceState &State,
               const rtk::Action<FString> &Action) {
              FSoulSliceState Next = State;
              Next.bVerifying = true;
              return Next;
            });
        Builder.addCase(
            rtk::verifySoulThunk().fulfilled,
            [](const FSoulSliceState &State,
               const rtk::Action<FSoulVerifyResult> &) {
              FSoulSliceState Next = State;
              Next.bVerifying = false;
              return Next;
            });
        Builder.addCase(
            rtk::verifySoulThunk().rejected,
            [](const FSoulSliceState &State,
               const rtk::Action<FString> &Action) {
              FSoulSliceState Next = State;
              Next.bVerifying = false;
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            Actions::clearSoulStateActionCreator(),
            [InitialState](const FSoulSliceState &,
               const rtk::Action<rtk::FEmptyPayload> &) {
              return InitialState;
            });
      });
}

} // namespace SoulSlice
