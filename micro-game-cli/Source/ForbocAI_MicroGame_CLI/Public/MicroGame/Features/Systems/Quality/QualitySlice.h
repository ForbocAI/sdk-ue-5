#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Quality/QualityActions.h"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"

namespace MicroGame {

/** User Story: As the quality domain owner, I need one RTK slice to own lifecycle, metadata, normalized samples, and baseline state. @fn inline rtk::Slice<FQualityState> CreateQualitySlice() */
inline rtk::Slice<FQualityState> CreateQualitySlice() {
  FQualityState Initial;
  Initial.Status = qualityData().RunStatuses.Idle;
  return rtk::createSlice<FQualityState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVECA306F5900F), Initial,
      [](rtk::ActionReducerMapBuilder<FQualityState> &Builder) {
        Builder.addCase(
            QualityActions::evaluationStartedActionCreator(),
            [](const FQualityState &,
               const rtk::Action<FQualityEvaluationStartedPayload> &Action) {
              FQualityState Next;
              Next.bRequired = true;
              Next.Status = qualityData().RunStatuses.Running;
              Next.Host = Action.PayloadValue.Host;
              Next.StartedAt = Action.PayloadValue.StartedAt;
              Next.Baseline = Action.PayloadValue.Baseline;
              return Next;
            });
        Builder.addCase(
            QualityActions::metadataCapturedActionCreator(),
            [](const FQualityState &State,
               const rtk::Action<FQualityModelMetadata> &Action) {
              FQualityState Next = State;
              Next.Metadata = func::just(Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            QualityActions::sampleRecordedActionCreator(),
            [](const FQualityState &State,
               const rtk::Action<FQualitySample> &Action) {
              FQualityState Next = State;
              Next.Samples.Add(Action.PayloadValue.Id,
                               Action.PayloadValue);
              return Next;
            });
        Builder.addCase(
            QualityActions::evaluationCompletedActionCreator(),
            [](const FQualityState &State,
               const rtk::Action<FQualityEvaluationCompletedPayload> &Action) {
              FQualityState Next = State;
              Next.Status = qualityData().RunStatuses.Completed;
              Next.CompletedAt = Action.PayloadValue.CompletedAt;
              Next.Error = qualityData().Output.Empty;
              return Next;
            });
        Builder.addCase(
            QualityActions::evaluationFailedActionCreator(),
            [](const FQualityState &State,
               const rtk::Action<FQualityEvaluationFailedPayload> &Action) {
              FQualityState Next = State;
              Next.Status = qualityData().RunStatuses.Failed;
              Next.CompletedAt = Action.PayloadValue.CompletedAt;
              Next.Error = Action.PayloadValue.Error;
              return Next;
            });
      });
}

} // namespace MicroGame
