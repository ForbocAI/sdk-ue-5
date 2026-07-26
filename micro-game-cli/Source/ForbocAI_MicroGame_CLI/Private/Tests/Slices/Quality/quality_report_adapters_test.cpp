#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Quality/Report/ReportAdapters.h"

using namespace MicroGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGameMissingQualityReportTest,
    qualityReportData().AutomationNames.MissingReport,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a quality harness maintainer, I need absent report files represented as Nothing so a first run cannot crash before producing its report. @fn bool FMicroGameMissingQualityReportTest::RunTest(const FString &Parameters) */
bool FMicroGameMissingQualityReportTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const func::Maybe<TSharedPtr<FJsonObject>> Report =
      QualityReportAdaptersDetail::readReportObject(FString());
  TestFalse(qualityReportData().Stories.MissingReport, Report.hasValue);
  return true;
}
