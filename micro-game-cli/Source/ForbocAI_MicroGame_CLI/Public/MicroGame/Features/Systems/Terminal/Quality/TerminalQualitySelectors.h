#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
/** User Story: As a game operator, I need model identity, metric gates, baseline state, and report path selected before presentation. @fn inline TArray<FTerminalLineViewModel> selectQualitySummaryViewModel(const FQualityReport &Report, const FString &Path) */
inline TArray<FTerminalLineViewModel>
selectQualitySummaryViewModel(const FQualityReport &Report,
                              const FString &Path) {
  const TArray<FTerminalLineViewModel> Heading{
      {false, qualityData().Messages.ReportHeading},
      {false,
       formatQualityTemplate(
           qualityData().Messages.ReportModel,
           {{qualityData().Tokens.Version, Report.Metadata.SlmVersion},
            {qualityData().Tokens.Artifact,
             Report.Metadata.SlmArtifactSha256},
            {qualityData().Tokens.ApiVersion, Report.Metadata.ApiVersion}})}};
  const TArray<FTerminalLineViewModel> Metrics =
      func::map_array<FString, FTerminalLineViewModel>(
          qualityData().MetricNames, [&Report](const FString &Metric) {
            const FQualityMetricSummary &Summary =
                Report.Summary.Metrics.FindChecked(Metric);
            return FTerminalLineViewModel{
                !Summary.bGatePassed,
                formatQualityTemplate(
                    qualityData().Messages.ReportMetric,
                    {{qualityData().Tokens.Metric, Metric},
                     {qualityData().Tokens.Passed,
                      FString::FromInt(Summary.Passed)},
                     {qualityData().Tokens.Total,
                      FString::FromInt(Summary.Total)},
                     {qualityData().Tokens.Rate,
                      FString::SanitizeFloat(
                          Summary.PassRate *
                          qualityData().Numbers.PercentageScale)}})};
          });
  const TArray<FTerminalLineViewModel> Verdict{
      {!Report.Summary.bRegressionGatePassed,
       formatQualityTemplate(
           qualityData().Messages.ReportBaseline,
           {{qualityData().Tokens.Status, Report.Summary.BaselineStatus}})},
      {!Report.Summary.bQualityGatePassed,
       formatQualityTemplate(
           qualityData().Messages.ReportGate,
           {{qualityData().Tokens.Status,
             Report.Summary.bQualityGatePassed
                 ? qualityData().Messages.Passed
                 : qualityData().Messages.Failed}})},
      {false,
       formatQualityTemplate(
           qualityData().Messages.ReportPath,
           {{qualityData().Tokens.Path, Path}})}};
  return func::concat_arrays<FTerminalLineViewModel>(
      {Heading, Metrics, Verdict});
}

} // namespace MicroGame
