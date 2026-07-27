#pragma once

#include "MicroGame/Features/Systems/Quality/QualityTypes.h"

namespace MicroGame {

/** User Story: As a before/after evaluator, I need the exact command workload matched before result or latency comparisons are allowed. @fn inline bool qualityWorkloadMatches(const TArray<FQualitySample> &Samples, const FQualityReport &Baseline) */
inline bool qualityWorkloadMatches(const TArray<FQualitySample> &Samples,
                                   const FQualityReport &Baseline) {
  const TMap<FString, FQualitySample> BaselineById =
      func::fold_array<FQualitySample, TMap<FString, FQualitySample>>(
          Baseline.Samples, {},
          [](const TMap<FString, FQualitySample> &Acc,
             const FQualitySample &Sample) {
            TMap<FString, FQualitySample> Next = Acc;
            Next.Add(Sample.Id, Sample);
            return Next;
          });
  return Samples.Num() == Baseline.Samples.Num() &&
         func::all_array<FQualitySample>(
             Samples, [&BaselineById](const FQualitySample &Sample) {
               const FQualitySample *Parent = BaselineById.Find(Sample.Id);
               return Parent != nullptr && Parent->Category == Sample.Category &&
                      Parent->PairKey == Sample.PairKey &&
                      Parent->Command == Sample.Command &&
                      Parent->Reference == Sample.Reference;
             });
}

} // namespace MicroGame
