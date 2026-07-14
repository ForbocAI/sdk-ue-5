#include "CLI/CliHandlers.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "CLI/RuntimeCommandlet.h"

// @covers:cli:setup
// @covers:cli:setup_check

/**
 * Test: setup_check passes commandlet validation and executes.
 * User Story: As CLI automation, I need setup commands accepted by the
 * commandlet validation layer so the supported entrypoint can run them.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSetupCommandletValidationTest,
    "ForbocAI.Integration.Setup.CommandletValidation",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSetupCommandletValidationTest::RunTest(const FString &Parameters) {
  UForbocAICommandlet *Commandlet = NewObject<UForbocAICommandlet>();
  bool bCompleted = false;
  FString Error;

  Commandlet->createCommandPipeline(TEXT("setup_check"), TArray<FString>())
      .then([&bCompleted]() { bCompleted = true; })
      .catch_([&Error](std::string Message) {
        Error = UTF8_TO_TCHAR(Message.c_str());
      })
      .execute();

  TestTrue("setup_check completed through commandlet pipeline", bCompleted);
  TestTrue("setup_check did not fail validation", Error.IsEmpty());
  return true;
}
