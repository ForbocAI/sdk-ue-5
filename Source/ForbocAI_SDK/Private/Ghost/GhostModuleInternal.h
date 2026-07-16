#pragma once

#include "Ghost/GhostModule.h"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

namespace GhostInternal {

/**
 * Validates the test configuration.
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 * @fn GhostTypes::Either<FString, FGhostConfig> ValidateTestConfig(const FGhostConfig &Config)
 */
GhostTypes::Either<FString, FGhostConfig>
ValidateTestConfig(const FGhostConfig &Config);

/**
 * Generates a summary of the test report.
 * User Story: As a maintainer, I need this test note so I can see what behavior the surrounding case or helper is meant to cover.
 * @fn GhostTypes::Either<FString, FString> GenerateTestSummary(const FGhostTestReport &Report)
 */
GhostTypes::Either<FString, FString>
GenerateTestSummary(const FGhostTestReport &Report);

/**
 * Exports results to JSON string.
 * User Story: As a maintainer, I need this note so the surrounding API intent stays clear during maintenance and integration.
 * @fn GhostTypes::Either<FString, FString> ExportResultsToJson(const FGhostTestReport &Report)
 */
GhostTypes::Either<FString, FString>
ExportResultsToJson(const FGhostTestReport &Report);

/**
 * Exports results to CSV string.
 * User Story: As a maintainer, I need this note so the surrounding API intent stays clear during maintenance and integration.
 * @fn GhostTypes::Either<FString, FString> ExportResultsToCsv(const FGhostTestReport &Report)
 */
GhostTypes::Either<FString, FString>
ExportResultsToCsv(const FGhostTestReport &Report);

/**
 * Internal async test runner.
 * User Story: As a maintainer, I need this test note so I can see what behavior the surrounding case or helper is meant to cover.
 * @fn GhostTypes::AsyncResult<FGhostTestResult> RunScenarioTest(const FAgent &Agent, const FString &Scenario)
 */
GhostTypes::AsyncResult<FGhostTestResult>
RunScenarioTest(const FAgent &Agent, const FString &Scenario);

/**
 * Helper to run all tests sequentially.
 * User Story: As a maintainer, I need this note so the surrounding API intent stays clear during maintenance and integration.
 * @fn void RunTestsSequentially(const FGhost &Ghost, TSharedPtr<FGhostTestReport> Report, int32 Index, std::function<void(FGhostTestReport)> OnComplete, std::function<void(std::string)> OnError)
 */
void RunTestsSequentially(const FGhost &Ghost,
                          TSharedPtr<FGhostTestReport> Report, int32 Index,
                          std::function<void(FGhostTestReport)> OnComplete,
                          std::function<void(std::string)> OnError);

} // namespace GhostInternal
