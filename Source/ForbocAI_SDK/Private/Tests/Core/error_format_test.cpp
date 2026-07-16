#include "Features/Errors/ErrorsAdapters.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FForbocErrorFormatTest,
                                 "ForbocAI.Core.Errors.FormatHttpHtml",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

/** User Story: As a tests core consumer, I need to invoke run test through a stable signature so the tests core workflow remains explicit and composable. @fn bool FForbocErrorFormatTest::RunTest(const FString &Parameters) */
bool FForbocErrorFormatTest::RunTest(const FString &Parameters) {
  const FString Html =
      TEXT("<!DOCTYPE html><html><head><title>502: Service unavailable</title>")
      TEXT("</head><body>provider body</body></html>");

  TestEqual(TEXT("Summarizes status and title"),
            Errors::summarizeHttpError(502, Html),
            FString(TEXT("HTTP 502: Service unavailable")));
  TestEqual(TEXT("extractThunkErrorMessage summarizes html"),
            Errors::extractThunkErrorMessage(Html),
            FString(TEXT("HTTP 502: Service unavailable")));

  return true;
}
