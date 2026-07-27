#include "Systems/Errors/ErrorsAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FForbocErrorFormatTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVD9FC55DC1247,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

/** User Story: As a tests core consumer, I need to invoke run test through a stable signature so the tests core workflow remains explicit and composable. @fn bool FForbocErrorFormatTest::RunTest(const FString &Parameters) */
bool FForbocErrorFormatTest::RunTest(const FString &Parameters) {
  const FString Html =
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV676E09248797)
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD53C14689D1E);

  TestEqual(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9E88E75B5A4B),
            Errors::summarizeHttpError(FORBOCAI_SDK_AUTHORED_NUMBERV23D9FF2601CF, Html),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV33ADEB10B828)));
  TestEqual(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2C20C8A3A857),
            Errors::extractThunkErrorMessage(Html),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV33ADEB10B828)));

  return true;
}
