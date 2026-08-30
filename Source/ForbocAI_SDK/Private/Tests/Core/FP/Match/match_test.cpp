#include "Core/fp.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/Testing/FP/Match/MatchAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchPredicateTest,
    FORBOCAI_SDK_AUTHORED_STRINGV7896674970EC,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FMultiMatchPredicateTest::RunTest(const FString &Parameters) */
bool FMultiMatchPredicateTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().Predicate;
  const std::string NegativeExpected =
      TCHAR_TO_UTF8(*Fixture.Negative.Expected);
  const std::string ZeroExpected = TCHAR_TO_UTF8(*Fixture.Zero.Expected);
  const std::string PositiveExpected =
      TCHAR_TO_UTF8(*Fixture.Positive.Expected);

  const std::vector<func::MatchCase<int, std::string>> Cases{
      func::when<int, std::string>(
          [&Fixture](const int &Value) { return Value < Fixture.Pivot; },
          [NegativeExpected](const int &) { return NegativeExpected; }),
      func::when<int, std::string>(
          [&Fixture](const int &Value) { return Value == Fixture.Pivot; },
          [ZeroExpected](const int &) { return ZeroExpected; }),
      func::when<int, std::string>(
          [&Fixture](const int &Value) { return Value > Fixture.Pivot; },
          [PositiveExpected](const int &) { return PositiveExpected; }),
  };

  const auto Negative =
      func::multi_match<int, std::string>(Fixture.Negative.Input, Cases);
  TestTrue(*Fixture.Negative.Labels.Present, Negative.hasValue);
  TestEqual(*Fixture.Negative.Labels.Value, Negative.value,
            NegativeExpected);

  const auto Zero =
      func::multi_match<int, std::string>(Fixture.Zero.Input, Cases);
  TestTrue(*Fixture.Zero.Labels.Present, Zero.hasValue);
  TestEqual(*Fixture.Zero.Labels.Value, Zero.value, ZeroExpected);

  const auto Positive =
      func::multi_match<int, std::string>(Fixture.Positive.Input, Cases);
  TestTrue(*Fixture.Positive.Labels.Present, Positive.hasValue);
  TestEqual(*Fixture.Positive.Labels.Value, Positive.value,
            PositiveExpected);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchWildcardTest,
    FORBOCAI_SDK_AUTHORED_STRINGV7327EDDEFFF8,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FMultiMatchWildcardTest::RunTest(const FString &Parameters) */
bool FMultiMatchWildcardTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().Wildcard;
  const std::string ExactExpected =
      TCHAR_TO_UTF8(*Fixture.ExactExpected);
  const std::string FallbackExpected =
      TCHAR_TO_UTF8(*Fixture.FallbackExpected);
  const std::vector<func::MatchCase<int, std::string>> Cases{
      func::when<int, std::string>(
          func::equals<int>(Fixture.ExactInput),
          [ExactExpected](const int &) { return ExactExpected; }),
      func::when<int, std::string>(
          func::wildcard<int>(),
          [FallbackExpected](const int &) { return FallbackExpected; }),
  };

  const auto Exact =
      func::multi_match<int, std::string>(Fixture.ExactInput, Cases);
  TestTrue(*Fixture.Labels.ExactPresent, Exact.hasValue);
  TestEqual(*Fixture.Labels.ExactValue, Exact.value, ExactExpected);

  const auto Fallback =
      func::multi_match<int, std::string>(Fixture.FallbackInput, Cases);
  TestTrue(*Fixture.Labels.FallbackPresent, Fallback.hasValue);
  TestEqual(*Fixture.Labels.FallbackValue, Fallback.value, FallbackExpected);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchNoMatchTest,
    FORBOCAI_SDK_AUTHORED_STRINGV94BBBBF39F00,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FMultiMatchNoMatchTest::RunTest(const FString &Parameters) */
bool FMultiMatchNoMatchTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().NoMatch;
  const std::string RegisteredResult =
      TCHAR_TO_UTF8(*Fixture.RegisteredResult);
  const std::vector<func::MatchCase<int, std::string>> Cases{
      func::when<int, std::string>(
          func::equals<int>(Fixture.RegisteredInput),
          [RegisteredResult](const int &) { return RegisteredResult; }),
  };
  const auto Result =
      func::multi_match<int, std::string>(Fixture.MissingInput, Cases);
  TestFalse(*Fixture.Label, Result.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchEqualsTest,
    FORBOCAI_SDK_AUTHORED_STRINGV705E5F24578E,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FMultiMatchEqualsTest::RunTest(const FString &Parameters) */
bool FMultiMatchEqualsTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().Equals;
  const std::string FirstInput = TCHAR_TO_UTF8(*Fixture.First.Input);
  const std::string SecondInput = TCHAR_TO_UTF8(*Fixture.Second.Input);
  const std::string ThirdInput = TCHAR_TO_UTF8(*Fixture.Third.Input);
  const std::string MissingInput = TCHAR_TO_UTF8(*Fixture.MissingInput);
  const std::vector<func::MatchCase<std::string, int>> Cases{
      func::when<std::string, int>(
          func::equals<std::string>(FirstInput),
          [&Fixture](const std::string &) { return Fixture.First.Result; }),
      func::when<std::string, int>(
          func::equals<std::string>(SecondInput),
          [&Fixture](const std::string &) { return Fixture.Second.Result; }),
      func::when<std::string, int>(
          func::equals<std::string>(ThirdInput),
          [&Fixture](const std::string &) { return Fixture.Third.Result; }),
  };

  const auto First = func::multi_match<std::string, int>(FirstInput, Cases);
  TestTrue(*Fixture.First.Labels.Present, First.hasValue);
  TestEqual(*Fixture.First.Labels.Value, First.value, Fixture.First.Result);

  const auto Second = func::multi_match<std::string, int>(SecondInput, Cases);
  TestTrue(*Fixture.Second.Labels.Present, Second.hasValue);
  TestEqual(*Fixture.Second.Labels.Value, Second.value, Fixture.Second.Result);

  const auto Missing =
      func::multi_match<std::string, int>(MissingInput, Cases);
  TestFalse(*Fixture.MissingLabel, Missing.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFromNullablePtrTest,
    FORBOCAI_SDK_AUTHORED_STRINGVE940B876AD53,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FFromNullablePtrTest::RunTest(const FString &Parameters) */
bool FFromNullablePtrTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().NullablePointer;
  int Value = Fixture.Value;
  int *Pointer = &Value;
  const auto Present = func::from_nullable(Pointer);
  TestTrue(*Fixture.Labels.Present, Present.hasValue);
  TestEqual(*Fixture.Labels.Value, Present.value, Fixture.Value);

  int *NullPointer = nullptr;
  const auto Missing = func::from_nullable(NullPointer);
  TestFalse(*Fixture.Labels.Missing, Missing.hasValue);

  const TSharedPtr<int> Shared = MakeShared<int>(Fixture.Value);
  const auto SharedPresent = func::from_shared(Shared);
  TestTrue(*Fixture.Labels.Present, SharedPresent.hasValue);
  TestEqual(*Fixture.Labels.Value, *SharedPresent.value, Fixture.Value);

  const TSharedPtr<int> MissingShared;
  const auto SharedMissing = func::from_shared(MissingShared);
  TestFalse(*Fixture.Labels.Missing, SharedMissing.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFromNullableValueTest,
    FORBOCAI_SDK_AUTHORED_STRINGVD21B85AD7E87,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FFromNullableValueTest::RunTest(const FString &Parameters) */
bool FFromNullableValueTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().NullableValue;
  const std::string ValidValue = TCHAR_TO_UTF8(*Fixture.ValidValue);
  const std::string EmptyValue = TCHAR_TO_UTF8(*Fixture.EmptyValue);
  const auto Present =
      func::from_nullable_value(ValidValue, Fixture.bValidFlag);
  TestTrue(*Fixture.Labels.Present, Present.hasValue);
  TestEqual(*Fixture.Labels.Value, Present.value, ValidValue);

  const auto Missing =
      func::from_nullable_value(EmptyValue, Fixture.bInvalidFlag);
  TestFalse(*Fixture.Labels.Missing, Missing.hasValue);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRequireJustTest, FORBOCAI_SDK_AUTHORED_STRINGVB10CA23A1256,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp match consumer, I need to invoke run test through a stable signature so the core fp match workflow remains explicit and composable. @fn bool FRequireJustTest::RunTest(const FString &Parameters) */
bool FRequireJustTest::RunTest(const FString &Parameters) {
  const auto &Fixture = Testing::FP::Match::MatchFixtures().RequireJust;
  const std::string SuccessMessage =
      TCHAR_TO_UTF8(*Fixture.SuccessMessage);
  const std::string FailureMessage =
      TCHAR_TO_UTF8(*Fixture.FailureMessage);
  const int Extracted =
      func::require_just(func::just(Fixture.Value), SuccessMessage);
  TestEqual(*Fixture.Labels.Value, Extracted, Fixture.Value);

  bool bThrew = false;
  try {
    func::require_just(func::nothing<int>(), FailureMessage);
  } catch (const std::runtime_error &Error) {
    bThrew = true;
    TestEqual(*Fixture.Labels.Error, std::string(Error.what()),
              FailureMessage);
  }
  TestTrue(*Fixture.Labels.Throws, bThrew);
  return true;
}
