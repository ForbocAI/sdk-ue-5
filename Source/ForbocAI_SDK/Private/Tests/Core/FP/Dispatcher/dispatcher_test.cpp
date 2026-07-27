#include "Core/fp.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/Testing/FP/Dispatcher/DispatcherAdapters.h"
#include "Misc/AutomationTest.h"

namespace {

/** User Story: As a core fp dispatcher consumer, I need to invoke build entries through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn template <typename EntryType> std::vector<std::pair<std::string, std::function<int()>>> BuildEntries(const TArray<EntryType> &Fixtures) */
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

/** User Story: As a core fp dispatcher consumer, I need to invoke build fstring entries through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn template <typename EntryType> std::vector<std::pair<FString, std::function<int()>>> BuildFStringEntries(const TArray<EntryType> &Fixtures) */
template <typename EntryType>
std::vector<std::pair<FString, std::function<int()>>>
BuildFStringEntries(const TArray<EntryType> &Fixtures) {
  std::vector<std::pair<FString, std::function<int()>>> Entries;
  for (const EntryType &Fixture : Fixtures) {
    const int Value = Fixture.Value;
    Entries.push_back(std::make_pair(
        Fixture.Key, [Value]() { return Value; }));
  }
  return Entries;
}

/** User Story: As a core fp dispatcher consumer, I need to invoke build entry through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn std::vector<std::pair<std::string, std::function<int()>>> BuildEntry(const Testing::FP::Dispatcher::FEntryFixture &Fixture) */
std::vector<std::pair<std::string, std::function<int()>>>
BuildEntry(const Testing::FP::Dispatcher::FEntryFixture &Fixture) {
  const std::string Key = TCHAR_TO_UTF8(*Fixture.Key);
  const int Value = Fixture.Value;
  return {std::make_pair(Key, [Value]() { return Value; })};
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherKeyLookupTest,
    FORBOCAI_SDK_AUTHORED_STRINGV72E71E940CC8,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FDispatcherKeyLookupTest::RunTest(const FString &Parameters) */
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
    FDispatcherFStringKeyLookupTest,
    FORBOCAI_SDK_AUTHORED_STRINGV72B49578D2F9,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FDispatcherFStringKeyLookupTest::RunTest(const FString &Parameters) */
bool FDispatcherFStringKeyLookupTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Dispatcher::DispatcherFixtures().KeyLookup;
  const auto Dispatcher = func::createDispatcher<FString, int>(
      BuildFStringEntries(Fixture.Entries));
  for (const auto &Entry : Fixture.Entries) {
    const auto Result = func::dispatch(Dispatcher, Entry.Key);
    TestTrue(*Entry.PresentLabel, Result.hasValue);
    TestEqual(*Entry.ValueLabel, Result.value, Entry.Value);
  }
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherMissingKeyTest,
    FORBOCAI_SDK_AUTHORED_STRINGV4AD42FB9D4EE,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FDispatcherMissingKeyTest::RunTest(const FString &Parameters) */
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
    FORBOCAI_SDK_AUTHORED_STRINGVDAF0D9E66F2C,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FDispatcherHasAndKeysTest::RunTest(const FString &Parameters) */
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
    FORBOCAI_SDK_AUTHORED_STRINGVB29C2546D86D,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FDispatcherEitherMissTest::RunTest(const FString &Parameters) */
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
    FORBOCAI_SDK_AUTHORED_STRINGV03F206DF49E9,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp dispatcher consumer, I need to invoke run test through a stable signature so the core fp dispatcher workflow remains explicit and composable. @fn bool FArgDispatcherStrictTest::RunTest(const FString &Parameters) */
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
