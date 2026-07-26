#pragma once

#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
namespace TerminalQualitySelectorsDetail {

/** User Story: As a quality presenter, I need metric lines selected recursively from stable authored names. @fn inline void appendMetricLines(const FQualityReport &Report, int32 Index, TArray<FTerminalLineViewModel> &Lines) */
inline void appendMetricLines(const FQualityReport &Report, int32 Index,
                              TArray<FTerminalLineViewModel> &Lines) {
  Index >= qualityData().MetricNames.Num()
      ? void()
      : [&]() {
          const FString &Metric = qualityData().MetricNames[Index];
          const FQualityMetricSummary &Summary =
              Report.Summary.Metrics.FindChecked(Metric);
          Lines.Add(FTerminalLineViewModel{
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
                        qualityData().Numbers.PercentageScale)}})});
          appendMetricLines(
              Report, Index + qualityData().Numbers.NextIndex, Lines);
        }();
}

} // namespace TerminalQualitySelectorsDetail

/** User Story: As a game operator, I need model identity, metric gates, baseline state, and report path selected before presentation. @fn inline TArray<FTerminalLineViewModel> selectQualitySummaryViewModel(const FQualityReport &Report, const FString &Path) */
inline TArray<FTerminalLineViewModel>
selectQualitySummaryViewModel(const FQualityReport &Report,
                              const FString &Path) {
  TArray<FTerminalLineViewModel> Lines{
      {false, qualityData().Messages.ReportHeading},
      {false,
       formatQualityTemplate(
           qualityData().Messages.ReportModel,
           {{qualityData().Tokens.Version, Report.Metadata.SlmVersion},
            {qualityData().Tokens.Artifact,
             Report.Metadata.SlmArtifactSha256},
            {qualityData().Tokens.ApiVersion, Report.Metadata.ApiVersion}})}};
  TerminalQualitySelectorsDetail::appendMetricLines(
      Report, qualityData().Numbers.InitialIndex, Lines);
  Lines.Add(FTerminalLineViewModel{
      !Report.Summary.bRegressionGatePassed,
      formatQualityTemplate(
          qualityData().Messages.ReportBaseline,
          {{qualityData().Tokens.Status,
            Report.Summary.BaselineStatus}})});
  Lines.Add(FTerminalLineViewModel{
      !Report.Summary.bQualityGatePassed,
      formatQualityTemplate(
          qualityData().Messages.ReportGate,
          {{qualityData().Tokens.Status,
            Report.Summary.bQualityGatePassed
                ? qualityData().Messages.Passed
                : qualityData().Messages.Failed}})});
  Lines.Add(FTerminalLineViewModel{
      false,
      formatQualityTemplate(
          qualityData().Messages.ReportPath,
          {{qualityData().Tokens.Path, Path}})});
  return Lines;
}

} // namespace MicroGame
