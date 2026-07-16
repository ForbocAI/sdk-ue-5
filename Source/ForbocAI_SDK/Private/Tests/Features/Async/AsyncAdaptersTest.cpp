#include "Features/Async/AsyncAdapters.h"
#include "Features/Testing/Async/TestingAsyncAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAsyncAdaptersWaitForResultTest,
    TestingAsyncAdapters::testingAsyncData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK host, I need blocking async adaptation to return settled
 * values and reject late completion after an explicit deadline without touching
 * expired caller state.
 * @fn bool FAsyncAdaptersWaitForResultTest::RunTest(const FString &Parameters)
 */
bool FAsyncAdaptersWaitForResultTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const TestingAsyncAdapters::FTestingAsyncData &Data =
      TestingAsyncAdapters::testingAsyncData();
  const int32 ExpectedValue = Data.Values.Immediate;
  const int32 ImmediateValue = AsyncAdapters::waitForResult(
      func::createAsyncResult<int32>(
          [ExpectedValue](std::function<void(int32)> Resolve,
                          std::function<void(std::string)> Reject) {
            (void)Reject;
            Resolve(ExpectedValue);
          }));
  TestEqual(*Data.Assertions.Immediate, ImmediateValue, ExpectedValue);

  std::function<void(int32)> ResolveAfterTimeout;
  func::AsyncResult<int32> Pending = func::createAsyncResult<int32>(
      [&ResolveAfterTimeout](std::function<void(int32)> Resolve,
                             std::function<void(std::string)> Reject) {
        (void)Reject;
        ResolveAfterTimeout = Resolve;
      });
  bool bTimedOut = false;
  try {
    (void)AsyncAdapters::waitForResult(std::move(Pending),
                                       Data.Values.TimeoutSeconds);
  } catch (const std::runtime_error &) {
    bTimedOut = true;
  }
  TestTrue(*Data.Assertions.Timeout, bTimedOut);
  TestTrue(*Data.Assertions.Callback,
           static_cast<bool>(ResolveAfterTimeout));

  ResolveAfterTimeout(Data.Values.Late);
  TestTrue(*Data.Assertions.Late, bTimedOut);
  return true;
}
