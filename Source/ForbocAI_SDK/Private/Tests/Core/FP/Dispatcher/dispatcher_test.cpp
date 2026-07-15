#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherKeyLookupTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.KeyLookup",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherKeyLookupTest::RunTest(const FString &Parameters) {
  std::vector<std::pair<std::string, std::function<int()>>> entries;
  entries.push_back(std::make_pair(std::string("a"), []() { return 1; }));
  entries.push_back(std::make_pair(std::string("b"), []() { return 2; }));
  entries.push_back(std::make_pair(std::string("c"), []() { return 3; }));

  auto d = func::createDispatcher<std::string, int>(entries);
  auto resultA = func::dispatch(d, std::string("a"));
  TestTrue("Key 'a' found", resultA.hasValue);
  TestEqual("Key 'a' returns 1", resultA.value, 1);
  auto resultB = func::dispatch(d, std::string("b"));
  TestTrue("Key 'b' found", resultB.hasValue);
  TestEqual("Key 'b' returns 2", resultB.value, 2);
  auto resultC = func::dispatch(d, std::string("c"));
  TestTrue("Key 'c' found", resultC.hasValue);
  TestEqual("Key 'c' returns 3", resultC.value, 3);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherMissingKeyTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.MissingKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherMissingKeyTest::RunTest(const FString &Parameters) {
  std::vector<std::pair<std::string, std::function<int()>>> entries;
  entries.push_back(std::make_pair(std::string("a"), []() { return 1; }));
  auto d = func::createDispatcher<std::string, int>(entries);
  auto result = func::dispatch(d, std::string("z"));
  TestFalse("Missing key returns nothing", result.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherHasAndKeysTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.HasAndKeys",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherHasAndKeysTest::RunTest(const FString &Parameters) {
  std::vector<std::pair<std::string, std::function<int()>>> entries;
  entries.push_back(std::make_pair(std::string("x"), []() { return 10; }));
  entries.push_back(std::make_pair(std::string("y"), []() { return 20; }));
  auto d = func::createDispatcher<std::string, int>(entries);
  TestTrue("has('x') is true", func::has(d, std::string("x")));
  TestTrue("has('y') is true", func::has(d, std::string("y")));
  TestFalse("has('z') is false", func::has(d, std::string("z")));
  auto k = func::keys(d);
  TestEqual("keys() has 2 entries", static_cast<int>(k.size()), 2);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherEitherMissTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.EitherMiss",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherEitherMissTest::RunTest(const FString &Parameters) {
  std::vector<std::pair<std::string, std::function<int()>>> entries;
  entries.push_back(std::make_pair(std::string("ready"), []() { return 7; }));
  auto dispatcher = func::createDispatcher<std::string, int>(entries);
  auto hit = func::dispatch_either<std::string, std::string, int>(
      dispatcher, std::string("ready"), std::string("missing"));
  TestFalse("Existing key returns Right", hit.isLeft);
  TestEqual("Existing key result", hit.right, 7);
  auto miss = func::dispatch_either<std::string, std::string, int>(
      dispatcher, std::string("absent"), std::string("missing"));
  TestTrue("Missing key returns Left", miss.isLeft);
  TestEqual("Missing key error", miss.left, std::string("missing"));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FArgDispatcherStrictTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.ArgStrict",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FArgDispatcherStrictTest::RunTest(const FString &Parameters) {
  typedef func::ArgDispatcher<std::string, int, std::string> FTestDispatcher;
  FTestDispatcher dispatcher =
      func::create_arg_dispatcher<std::string, int, std::string>();
  dispatcher = func::arg_dispatcher_register<std::string, int, std::string>(
      dispatcher, std::string("double"),
      [](const int &value) { return std::string(value == 4 ? "eight" : "x"); });
  std::string key = "double";
  int arg = 4;
  auto hit = func::arg_dispatcher_dispatch_maybe<std::string, int, std::string>(
      func::ArgDispatcherDispatch<std::string, int, std::string>{
          &dispatcher, &key, &arg});
  TestTrue("Argument dispatcher hit returns Just", hit.hasValue);
  TestEqual("Argument dispatcher hit value", hit.value, std::string("eight"));
  std::string missing = "triple";
  auto miss =
      func::arg_dispatcher_dispatch_either<std::string, std::string, int,
                                           std::string>(
          func::ArgDispatcherDispatch<std::string, int, std::string>{
              &dispatcher, &missing, &arg},
          std::string("missing-handler"));
  TestTrue("Argument dispatcher miss returns Left", miss.isLeft);
  TestEqual("Argument dispatcher miss error", miss.left,
            std::string("missing-handler"));
  return true;
}
