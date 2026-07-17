#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Quality/Report/ReportAdapters.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameMissingQualityReportTest,
    qualityReportData().AutomationNames.MissingReport,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a quality harness maintainer, I need absent report files represented as Nothing so a first run cannot crash before producing its report. @fn bool FTestGameMissingQualityReportTest::RunTest(const FString &Parameters) */
bool FTestGameMissingQualityReportTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const func::Maybe<TSharedPtr<FJsonObject>> Report =
      QualityReportAdaptersDetail::readReportObject(FString());
  TestFalse(qualityReportData().Stories.MissingReport, Report.hasValue);
  return true;
}
