#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Systems/Dependencies/DependenciesThunks.h"
#include "Components/Dependencies/DependenciesTypes.h"

namespace DependenciesSlice {

/** User Story: As a features dependencies consumer, I need to invoke start workflow through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline FDependenciesState startWorkflow(const FDependenciesState &State, const FString &Status) */
inline FDependenciesState startWorkflow(const FDependenciesState &State,
                                      const FString &Status) {
  FDependenciesState Next = State;
  Next.Status = Status;
  Next.Error.Empty();
  return Next;
}

/** User Story: As a features dependencies consumer, I need to invoke complete dependencies through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline FDependenciesState completeDependencies(const FDependenciesState &State, const FDependenciesResult &Result) */
inline FDependenciesState completeDependencies(const FDependenciesState &State,
                                      const FDependenciesResult &Result) {
  FDependenciesState Next = State;
  Next.bHasResult = true;
  Next.Result = Result;
  Next.Status = Result.Vector.bOk && Result.Memory.bOk ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV05D2DE39A21B)
                                                       : TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
  Next.Error = Next.Status == TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB)
                   ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEAFEC76DA3FD)
                   : FString();
  return Next;
}

/** User Story: As a features dependencies consumer, I need to invoke create dependencies slice through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline rtk::Slice<FDependenciesState> createDependenciesSlice() */
inline rtk::Slice<FDependenciesState> createDependenciesSlice() {
  return rtk::createSlice<FDependenciesState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA421DFB052DB), FDependenciesState(),
      [](rtk::ActionReducerMapBuilder<FDependenciesState> &Builder) {
        Builder.addCase(
            rtk::checkNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              return startWorkflow(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5C694E6E05E0));
            });
        Builder.addCase(
            rtk::checkNativeDependenciesThunk().fulfilled,
            [](const FDependenciesState &State,
               const rtk::Action<FNativeDependenciesReport> &Action) {
              FDependenciesState Next = State;
              Next.bHasReport = true;
              Next.Report = Action.PayloadValue;
              Next.Status = Action.PayloadValue.Vectorizer.bAvailable &&
                                    Action.PayloadValue.VectorDb.bAvailable
                                ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV05D2DE39A21B)
                                : TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
              Next.Error = Next.Status == TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB)
                               ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1E91A3B8F63E)
                               : FString();
              return Next;
            });
        Builder.addCase(
            rtk::checkNativeDependenciesThunk().rejected,
            [](const FDependenciesState &State,
               const rtk::Action<FString> &Action) {
              FDependenciesState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            rtk::setupNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesOptions> &) {
              return startWorkflow(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF2A8803550F5));
            });
        Builder.addCase(
            rtk::setupNativeDependenciesThunk().fulfilled,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesResult> &Action) {
              return completeDependencies(State, Action.PayloadValue);
            });
        Builder.addCase(
            rtk::setupNativeDependenciesThunk().rejected,
            [](const FDependenciesState &State,
               const rtk::Action<FString> &Action) {
              FDependenciesState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            rtk::refreshNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesOptions> &) {
              return startWorkflow(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV311FECE34994));
            });
        Builder.addCase(
            rtk::refreshNativeDependenciesThunk().fulfilled,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesResult> &Action) {
              return completeDependencies(State, Action.PayloadValue);
            });
        Builder.addCase(
            rtk::refreshNativeDependenciesThunk().rejected,
            [](const FDependenciesState &State,
               const rtk::Action<FString> &Action) {
              FDependenciesState Next = State;
              Next.Status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFFFABC6923CB);
              Next.Error = Action.PayloadValue;
              return Next;
            });
      });
}

} // namespace DependenciesSlice
