#pragma once

#include "Core/rtk.hpp"
#include "Features/Dependencies/DependenciesThunks.h"
#include "Features/Dependencies/DependenciesTypes.h"

namespace DependenciesSlice {

inline FDependenciesState startWorkflow(const FDependenciesState &State,
                                      const FString &Status) {
  FDependenciesState Next = State;
  Next.Status = Status;
  Next.Error.Empty();
  return Next;
}

inline FDependenciesState completeDependencies(const FDependenciesState &State,
                                      const FDependenciesResult &Result) {
  FDependenciesState Next = State;
  Next.bHasResult = true;
  Next.Result = Result;
  Next.Status = Result.Vector.bOk && Result.Memory.bOk ? TEXT("ready")
                                                       : TEXT("failed");
  Next.Error = Next.Status == TEXT("failed")
                   ? TEXT("Native dependency setup did not complete")
                   : FString();
  return Next;
}

inline rtk::Slice<FDependenciesState> createDependenciesSlice() {
  return rtk::createSlice<FDependenciesState>(
      TEXT("dependencies"), FDependenciesState(),
      [](rtk::ActionReducerMapBuilder<FDependenciesState> &Builder) {
        Builder.addCase(
            rtk::checkNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<rtk::FEmptyPayload> &) {
              return startWorkflow(State, TEXT("checking"));
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
                                ? TEXT("ready")
                                : TEXT("failed");
              Next.Error = Next.Status == TEXT("failed")
                               ? TEXT("Native dependencies are not ready")
                               : FString();
              return Next;
            });
        Builder.addCase(
            rtk::checkNativeDependenciesThunk().rejected,
            [](const FDependenciesState &State,
               const rtk::Action<FString> &Action) {
              FDependenciesState Next = State;
              Next.Status = TEXT("failed");
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            rtk::setupNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesOptions> &) {
              return startWorkflow(State, TEXT("settingUp"));
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
              Next.Status = TEXT("failed");
              Next.Error = Action.PayloadValue;
              return Next;
            });
        Builder.addCase(
            rtk::refreshNativeDependenciesThunk().pending,
            [](const FDependenciesState &State,
               const rtk::Action<FDependenciesOptions> &) {
              return startWorkflow(State, TEXT("refreshing"));
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
              Next.Status = TEXT("failed");
              Next.Error = Action.PayloadValue;
              return Next;
            });
      });
}

} // namespace DependenciesSlice
