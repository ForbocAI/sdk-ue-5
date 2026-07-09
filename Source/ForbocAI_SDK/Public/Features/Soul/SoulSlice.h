#pragma once
/**
 * personas and souls; the essence of the machine
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"
#include "Features/Soul/SoulActions.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicSoulSoulSliceHOptionalDomainId;
} } }

namespace SoulSlice {

using namespace rtk;

struct FSoulSliceState {
  FString ExportStatus;
  FString ImportStatus;
  FSoulExportResult LastExport;
  bool bHasLastExport;
  FSoul LastImport;
  bool bHasLastImport;
  TArray<FSoulListItem> AvailableSouls;
  FString Error;

  FSoulSliceState()
      : ExportStatus(TEXT("idle")), ImportStatus(TEXT("idle")),
        bHasLastExport(false), bHasLastImport(false) {}
};

/**
 * Builds the soul slice reducer and initial state.
 * User Story: As soul runtime setup, I need one slice factory so export and
 * import actions are wired into the store consistently.
 */
inline Slice<FSoulSliceState> createSoulSlice() {
  return rtk::createSlice<FSoulSliceState>(
  TEXT("soul"), FSoulSliceState(),
  [](rtk::ActionReducerMapBuilder<FSoulSliceState> &Builder) {
    Builder.addCase(Actions::remoteExportSoulPendingActionCreator(),
      [](const FSoulSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FSoulSliceState {
                  FSoulSliceState Next = State;
                  Next.ExportStatus = TEXT("exporting");
                  Next.Error.Empty();
                  return Next;
                });
    Builder.addCase(Actions::remoteExportSoulSuccessActionCreator(),
      [](const FSoulSliceState &State,
                   const Action<FSoulExportResult> &Action) -> FSoulSliceState {
                  FSoulSliceState Next = State;
                  Next.ExportStatus = TEXT("success");
                  Next.LastExport = Action.PayloadValue;
                  Next.bHasLastExport = true;
                  return Next;
                });
    Builder.addCase(Actions::remoteExportSoulFailedActionCreator(),
      [](const FSoulSliceState &State,
                             const Action<FString> &Action) -> FSoulSliceState {
                            FSoulSliceState Next = State;
                            Next.ExportStatus = TEXT("failed");
                            Next.Error = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::importSoulPendingActionCreator(),
      [](const FSoulSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FSoulSliceState {
                  FSoulSliceState Next = State;
                  Next.ImportStatus = TEXT("importing");
                  Next.Error.Empty();
                  return Next;
                });
    Builder.addCase(Actions::importSoulSuccessActionCreator(),
      [](const FSoulSliceState &State,
                             const Action<FSoul> &Action) -> FSoulSliceState {
                            FSoulSliceState Next = State;
                            Next.ImportStatus = TEXT("success");
                            Next.LastImport = Action.PayloadValue;
                            Next.bHasLastImport = true;
                            return Next;
                          });
    Builder.addCase(Actions::importSoulFailedActionCreator(),
      [](const FSoulSliceState &State,
                             const Action<FString> &Action) -> FSoulSliceState {
                            FSoulSliceState Next = State;
                            Next.ImportStatus = TEXT("failed");
                            Next.Error = Action.PayloadValue;
                            return Next;
                          });
    Builder.addCase(Actions::setSoulListActionCreator(),
      [](const FSoulSliceState &State,
                   const Action<TArray<FSoulListItem>> &Action) -> FSoulSliceState {
                  FSoulSliceState Next = State;
                  Next.AvailableSouls = Action.PayloadValue;
                  return Next;
                });
    Builder.addCase(Actions::clearSoulStateActionCreator(),
      [](const FSoulSliceState &State,
                   const Action<rtk::FEmptyPayload> &Action) -> FSoulSliceState {
                  return FSoulSliceState();
                });
  });
}

} // namespace SoulSlice
