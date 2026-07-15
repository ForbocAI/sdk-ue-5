#include "Core/fp.hpp"
#include "Features/Testing/FP/Dispatcher/DispatcherAdapters.h"
#include "Misc/AutomationTest.h"

namespace {

template <typename EntryType>
std::vector<std::pair<std::string, std::function<int()>>>
BuildEntries(const TArray<EntryType> &Fixtures) {
  std::vector<std::pair<std::string, std::function<int()>>> Entries;
  for (const EntryType &Fixture : Fixtures) {
    const std::string Key = TCHAR_TO_UTF8(*Fixture.Key);
    const int Value = Fixture.Value;
    Entries.push_back(std::make_pair(
        Key, [Value]() { return Value; }));
  }
  return Entries;
}

std::vector<std::pair<std::string, std::function<int()>>>
BuildEntry(const Testing::FP::Dispatcher::FEntryFixture &Fixture) {
  const std::string Key = TCHAR_TO_UTF8(*Fixture.Key);
  const int Value = Fixture.Value;
  return {std::make_pair(Key, [Value]() { return Value; })};
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherKeyLookupTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.KeyLookup",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherKeyLookupTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().KeyLookup;
  const auto Dispatcher = func::createDispatcher<std::string, int>(
      BuildEntries(Fixture.Entries));
  for (const auto &Entry : Fixture.Entries) {
    const auto Result =
        func::dispatch(Dispatcher, std::string(TCHAR_TO_UTF8(*Entry.Key)));
    TestTrue(*Entry.PresentLabel, Result.hasValue);
    TestEqual(*Entry.ValueLabel, Result.value, Entry.Value);
  }
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherMissingKeyTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.MissingKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherMissingKeyTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().MissingKey;
  const auto Dispatcher =
      func::createDispatcher<std::string, int>(BuildEntry(Fixture.Entry));
  const auto Result = func::dispatch(
      Dispatcher, std::string(TCHAR_TO_UTF8(*Fixture.MissingKey)));
  TestFalse(*Fixture.Label, Result.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherHasAndKeysTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.HasAndKeys",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherHasAndKeysTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().HasAndKeys;
  const auto Dispatcher = func::createDispatcher<std::string, int>(
      BuildEntries(Fixture.Entries));
  for (const auto &Entry : Fixture.Entries) {
    TestTrue(*Entry.PresentLabel,
             func::has(Dispatcher,
                       std::string(TCHAR_TO_UTF8(*Entry.Key))));
  }
  TestFalse(*Fixture.MissingLabel,
            func::has(Dispatcher,
                      std::string(TCHAR_TO_UTF8(*Fixture.MissingKey))));
  TestEqual(*Fixture.CountLabel,
            static_cast<int>(func::keys(Dispatcher).size()),
            Fixture.ExpectedCount);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherEitherMissTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.EitherMiss",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FDispatcherEitherMissTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().EitherMiss;
  const auto Dispatcher =
      func::createDispatcher<std::string, int>(BuildEntry(Fixture.Entry));
  const std::string Error = TCHAR_TO_UTF8(*Fixture.Error);
  const auto Hit = func::dispatch_either<std::string, std::string, int>(
      Dispatcher, std::string(TCHAR_TO_UTF8(*Fixture.Entry.Key)), Error);
  TestFalse(*Fixture.Labels.HitSide, Hit.isLeft);
  TestEqual(*Fixture.Labels.HitValue, Hit.right, Fixture.Entry.Value);

  const auto Miss = func::dispatch_either<std::string, std::string, int>(
      Dispatcher, std::string(TCHAR_TO_UTF8(*Fixture.MissingKey)), Error);
  TestTrue(*Fixture.Labels.MissSide, Miss.isLeft);
  TestEqual(*Fixture.Labels.MissError, Miss.left, Error);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FArgDispatcherStrictTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.ArgStrict",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FArgDispatcherStrictTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().ArgStrict;
  typedef func::ArgDispatcher<std::string, int, std::string> FTestDispatcher;
  FTestDispatcher Dispatcher =
      func::create_arg_dispatcher<std::string, int, std::string>();
  const std::string RegisteredKey =
      TCHAR_TO_UTF8(*Fixture.RegisteredKey);
  const std::string MatchedOutput =
      TCHAR_TO_UTF8(*Fixture.MatchedOutput);
  const std::string OtherOutput = TCHAR_TO_UTF8(*Fixture.OtherOutput);
  Dispatcher = func::arg_dispatcher_register<std::string, int, std::string>(
      Dispatcher, RegisteredKey,
      [&Fixture, MatchedOutput, OtherOutput](const int &Value) {
        return Value == Fixture.MatchedInput ? MatchedOutput : OtherOutput;
      });

  std::string Key = RegisteredKey;
  int Argument = Fixture.Input;
  const auto Hit =
      func::arg_dispatcher_dispatch_maybe<std::string, int, std::string>(
          func::ArgDispatcherDispatch<std::string, int, std::string>{
              &Dispatcher, &Key, &Argument});
  TestTrue(*Fixture.Labels.HitPresent, Hit.hasValue);
  TestEqual(*Fixture.Labels.HitValue, Hit.value, MatchedOutput);

  std::string MissingKey = TCHAR_TO_UTF8(*Fixture.MissingKey);
  const std::string MissingError =
      TCHAR_TO_UTF8(*Fixture.MissingError);
  const auto Miss =
      func::arg_dispatcher_dispatch_either<std::string, std::string, int,
                                           std::string>(
          func::ArgDispatcherDispatch<std::string, int, std::string>{
              &Dispatcher, &MissingKey, &Argument},
          MissingError);
  TestTrue(*Fixture.Labels.MissSide, Miss.isLeft);
  TestEqual(*Fixture.Labels.MissError, Miss.left, MissingError);
  return true;
}
