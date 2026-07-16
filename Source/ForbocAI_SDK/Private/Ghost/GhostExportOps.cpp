// User Story: As a developer, I need this module to function.
#include "Ghost/GhostModuleInternal.h"

namespace GhostInternal {

/** User Story: As a ghost consumer, I need to invoke validate test config through a stable signature so the ghost workflow remains explicit and composable. @fn GhostTypes::Either<FString, FGhostConfig> ValidateTestConfig(const FGhostConfig &Config) */
GhostTypes::Either<FString, FGhostConfig>
ValidateTestConfig(const FGhostConfig &Config) {
  return Config.Scenarios.Num() == 0
             ? GhostTypes::make_left(
                   FString(TEXT("No test scenarios provided")),
                   FGhostConfig{})
         : Config.MaxIterations < 1
             ? GhostTypes::make_left(
                   FString(TEXT("Max iterations must be >= 1")),
                   FGhostConfig{})
             : GhostTypes::make_right(FString(), Config);
}

/** User Story: As a ghost consumer, I need to invoke generate test summary through a stable signature so the ghost workflow remains explicit and composable. @fn GhostTypes::Either<FString, FString> GenerateTestSummary(const FGhostTestReport &Report) */
GhostTypes::Either<FString, FString>
GenerateTestSummary(const FGhostTestReport &Report) {
  FString Summary = FString::Printf(TEXT("Ghost Test Summary for Agent: %s\n"),
                                    *Report.Config.Agent.Id);
  Summary += FString::Printf(TEXT("Total Tests: %d\n"), Report.TotalTests);
  Summary += FString::Printf(TEXT("Passed: %d\n"), Report.PassedTests);
  Summary += FString::Printf(TEXT("Failed: %d\n"), Report.FailedTests);
  Summary += FString::Printf(TEXT("Success Rate: %.1f%%\n"),
                             Report.SuccessRate * 100.0f);
  return GhostTypes::make_right(FString(), Summary);
}

/** User Story: As a ghost consumer, I need to invoke export results to json through a stable signature so the ghost workflow remains explicit and composable. @fn GhostTypes::Either<FString, FString> ExportResultsToJson(const FGhostTestReport &Report) */
GhostTypes::Either<FString, FString>
ExportResultsToJson(const FGhostTestReport &Report) {
  struct AppendResults {
    static FString apply(const TArray<FGhostTestResult> &Results,
                         int32 Idx, const FString &Acc) {
      return Idx >= Results.Num()
                 ? Acc
                 : apply(
                       Results, Idx + 1,
                       Acc +
                           FString::Printf(
                               TEXT("    { \"scenario\": \"%s\", "
                                    "\"passed\": %s }"),
                               *Results[Idx].Scenario,
                               Results[Idx].bPassed ? TEXT("true")
                                                    : TEXT("false")) +
                           (Idx < Results.Num() - 1 ? TEXT(",\n")
                                                    : TEXT("\n")));
    }
  };
  const FString Header = TEXT("{\n  \"agent\": \"") +
                          Report.Config.Agent.Id +
                          TEXT("\",\n  \"results\": [\n");
  return GhostTypes::make_right(
      FString(),
      AppendResults::apply(Report.Results, 0, Header) + TEXT("  ]\n}"));
}

/** User Story: As a ghost consumer, I need to invoke export results to csv through a stable signature so the ghost workflow remains explicit and composable. @fn GhostTypes::Either<FString, FString> ExportResultsToCsv(const FGhostTestReport &Report) */
GhostTypes::Either<FString, FString>
ExportResultsToCsv(const FGhostTestReport &Report) {
  struct AppendRows {
    static FString apply(const TArray<FGhostTestResult> &Results,
                         int32 Idx, const FString &Acc) {
      return Idx >= Results.Num()
                 ? Acc
                 : apply(
                       Results, Idx + 1,
                       Acc +
                           FString::Printf(
                               TEXT("\"%s\",%s,\"%s\",\"%s\"\n"),
                               *Results[Idx].Scenario.Replace(
                                   TEXT("\""), TEXT("\"\"")),
                               Results[Idx].bPassed ? TEXT("True")
                                                    : TEXT("False"),
                               *Results[Idx].ActualResponse.Replace(
                                   TEXT("\""), TEXT("\"\"")),
                               *Results[Idx].ErrorMessage.Replace(
                                   TEXT("\""), TEXT("\"\""))));
    }
  };
  return GhostTypes::make_right(
      FString(),
      AppendRows::apply(
          Report.Results, 0,
          FString(TEXT("Scenario,Passed,ActualResponse,ErrorMessage\n"))));
}

} // namespace GhostInternal
