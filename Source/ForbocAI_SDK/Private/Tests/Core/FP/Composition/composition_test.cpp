#include "Core/fp.hpp"
#include "Features/Testing/FP/Composition/CompositionAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookCollectionsTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.Collections",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp composition consumer, I need to invoke run test through a stable signature so the core fp composition workflow remains explicit and composable. @fn bool FFunctionalCookbookCollectionsTest::RunTest(const FString &Parameters) */
bool FFunctionalCookbookCollectionsTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Composition::CompositionFixtures().Collections;
  const std::vector<int> &Values = Fixture.Values;

  const int Sum = func::fold<int, int>(
      Values, Fixture.FoldSeed,
      [](const int Accumulator, const int &Value) {
        return Accumulator + Value;
      });
  TestEqual(*Fixture.Labels.Fold, Sum, Fixture.FoldExpected);

  const std::vector<int> Filtered = func::filter<int>(
      Values, [&Fixture](const int &Value) {
        return Value == Fixture.FilterValue;
      });
  TestEqual(*Fixture.Labels.Filter, static_cast<int>(Filtered.size()),
            Fixture.FilterExpectedCount);

  const auto Found = func::find_vector<int>(
      Values, [&Fixture](const int &Value) {
        return Value == Fixture.FindValue;
      });
  TestTrue(*Fixture.Labels.FindPresent, Found.hasValue);
  TestEqual(*Fixture.Labels.FindValue, Found.value, Fixture.FindValue);

  const std::vector<int> Unique = func::unique_by<int>(
      Values, [](const int &Value) { return Value; });
  TestEqual(*Fixture.Labels.Unique, static_cast<int>(Unique.size()),
            Fixture.UniqueExpectedCount);
  TestTrue(*Fixture.Labels.Contains,
           func::contains_value<int>(Unique, Fixture.ContainsValue));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookMaybeEitherTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.MaybeEither",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp composition consumer, I need to invoke run test through a stable signature so the core fp composition workflow remains explicit and composable. @fn bool FFunctionalCookbookMaybeEitherTest::RunTest(const FString &Parameters) */
bool FFunctionalCookbookMaybeEitherTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Composition::CompositionFixtures().MaybeEither;
  const std::vector<int> &Values = Fixture.Values;

  const auto Traversed = func::traverse<int>(
      Values, [&Fixture](const int &Value) {
        return func::just(Value * Fixture.TraverseMultiplier);
      });
  TestTrue(*Fixture.Labels.TraversePresent, Traversed.hasValue);
  TestEqual(*Fixture.Labels.TraverseCount,
            static_cast<int>(Traversed.value.size()),
            Fixture.TraverseExpectedCount);
  TestEqual(*Fixture.Labels.TraverseFirst,
            Traversed.value[Fixture.FirstIndex],
            Fixture.TraverseFirstExpected);

  const std::vector<func::Maybe<int>> OptionalValues{
      func::just(Fixture.OptionalPresent), func::nothing<int>()};
  const auto Sequenced = func::sequence_maybe<int>(OptionalValues);
  TestFalse(*Fixture.Labels.SequenceMissing, Sequenced.hasValue);

  const auto Lifted = func::lift3<int, int, int>(
      func::just(Fixture.LiftFirst), func::just(Fixture.LiftSecond),
      func::just(Fixture.LiftThird),
      [](const int First, const int Second, const int Third) {
        return First + Second + Third;
      });
  TestTrue(*Fixture.Labels.LiftPresent, Lifted.hasValue);
  TestEqual(*Fixture.Labels.LiftValue, Lifted.value, Fixture.LiftExpected);

  const auto Folded = func::fold_either<std::string, int, int>(
      Values, Fixture.FoldSeed,
      [&Fixture](const int Accumulator,
                 const int &Value) -> func::Either<std::string, int> {
        return Value < Fixture.FoldLimit
                   ? func::make_right<std::string, int>(Accumulator + Value)
                   : func::make_left<std::string, int>(
                         std::string(TCHAR_TO_UTF8(*Fixture.FoldError)));
      });
  TestFalse(*Fixture.Labels.FoldRight, Folded.isLeft);
  TestEqual(*Fixture.Labels.FoldValue, Folded.right, Fixture.FoldExpected);

  int IndexedEffectTotal = Fixture.IndexedSeed;
  func::for_each_indexed(
      Values, Values.size(), [&IndexedEffectTotal](const int &Value) {
        IndexedEffectTotal += Value;
      });
  TestEqual(*Fixture.Labels.ForEach, IndexedEffectTotal,
            Fixture.IndexedExpected);

  const int IndexedSum = func::fold_indexed(
      Values, Values.size(), Fixture.IndexedSeed,
      [](const int &Accumulator, const int &Value) {
        return Accumulator + Value;
      });
  TestEqual(*Fixture.Labels.FoldIndexed, IndexedSum,
            Fixture.IndexedExpected);

  const auto IndexedFind = func::find_indexed(
      Values, Values.size(), [&Fixture](const int &Value) {
        return Value == Fixture.IndexedFindValue;
      });
  TestTrue(*Fixture.Labels.FindPresent, IndexedFind.hasValue);
  TestEqual(*Fixture.Labels.FindValue, IndexedFind.value,
            Fixture.IndexedFindValue);
  TestTrue(*Fixture.Labels.Any,
           func::any_indexed(Values, Values.size(),
                             [&Fixture](const int &Value) {
                               return Value > Fixture.AnyThreshold;
                             }));
  TestTrue(*Fixture.Labels.All,
           func::all_indexed(Values, Values.size(),
                             [&Fixture](const int &Value) {
                               return Value > Fixture.AllThreshold;
                             }));

  const std::vector<int> Grid = func::map_grid<int>(
      Fixture.GridRows, Fixture.GridColumns,
      [&Fixture](const func::GridIndex &Index) {
        return static_cast<int>(Index.Row * Fixture.GridRowMultiplier +
                                Index.Column);
      });
  TestEqual(*Fixture.Labels.GridCount, static_cast<int>(Grid.size()),
            Fixture.GridExpectedCount);
  TestEqual(*Fixture.Labels.GridFirst, Grid[Fixture.GridFirstIndex],
            Fixture.GridFirstExpected);
  TestEqual(*Fixture.Labels.GridRowMajor, Grid[Fixture.GridRowMajorIndex],
            Fixture.GridRowMajorExpected);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCoreTsNameParityTest,
    "ForbocAI.Core.FunctionalCore.TSNameParity",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp composition consumer, I need to invoke run test through a stable signature so the core fp composition workflow remains explicit and composable. @fn bool FFunctionalCoreTsNameParityTest::RunTest(const FString &Parameters) */
bool FFunctionalCoreTsNameParityTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Composition::CompositionFixtures().NameParity;

  const auto Present = func::just(Fixture.PresentValue);
  const auto Missing = func::nothing<int>();
  TestTrue(*Fixture.Labels.IsJust, func::isJust(Present));
  TestFalse(*Fixture.Labels.IsJustMissing, func::isJust(Missing));
  TestTrue(*Fixture.Labels.IsNothing, func::isNothing(Missing));
  TestEqual(*Fixture.Labels.OrElse,
            func::orElse(Missing, Fixture.OrElseFallback),
            Fixture.OrElseFallback);
  TestEqual(*Fixture.Labels.RequireJust,
            func::requireJust(
                Present, std::string(TCHAR_TO_UTF8(*Fixture.MissingMessage))),
            Fixture.PresentValue);

  int Raw = Fixture.PointerValue;
  const auto PointerValue = func::fromNullable(&Raw);
  TestTrue(*Fixture.Labels.PointerPresent, func::isJust(PointerValue));
  TestEqual(*Fixture.Labels.PointerValue, PointerValue.value,
            Fixture.PointerValue);

  const std::string FlaggedText = TCHAR_TO_UTF8(*Fixture.FlaggedValue);
  const auto FlaggedValue =
      func::fromNullable(FlaggedText, Fixture.bFlaggedPresent);
  TestTrue(*Fixture.Labels.FlaggedPresent, func::isJust(FlaggedValue));
  TestEqual(*Fixture.Labels.FlaggedValue, FlaggedValue.value, FlaggedText);

  const std::string FlaggedMissingText =
      TCHAR_TO_UTF8(*Fixture.FlaggedMissingValue);
  const auto FlaggedMissing =
      func::fromNullable(FlaggedMissingText, Fixture.bFlaggedMissingPresent);
  TestTrue(*Fixture.Labels.FlaggedMissing, func::isNothing(FlaggedMissing));

  const std::string FailureText = TCHAR_TO_UTF8(*Fixture.FailureValue);
  const auto Failure = func::left<std::string, int>(FailureText);
  const auto Success = func::right<std::string, int>(Fixture.SuccessValue);
  TestTrue(*Fixture.Labels.IsLeft, func::isLeft(Failure));
  TestFalse(*Fixture.Labels.IsRightRejectsLeft, func::isRight(Failure));
  TestTrue(*Fixture.Labels.IsRight, func::isRight(Success));

  const auto Mapped = func::efmap<std::string, int>(
      Success, [&Fixture](const int Value) {
        return Value + Fixture.MapDelta;
      });
  TestTrue(*Fixture.Labels.EfmapRight, func::isRight(Mapped));
  TestEqual(*Fixture.Labels.EfmapValue, Mapped.right, Fixture.MapExpected);

  func::Predicate<int> IsEven = [&Fixture](const int &Value) {
    return Value % Fixture.EvenDivisor == Fixture.EvenRemainder;
  };
  const std::string PredicateText = TCHAR_TO_UTF8(*Fixture.PredicateValue);
  const auto PredicateCase = func::testCase<int, std::string>(
      Fixture.PredicateInput, IsEven,
      [PredicateText](const int &) { return PredicateText; });
  TestTrue(*Fixture.Labels.PredicatePresent, func::isJust(PredicateCase));
  TestEqual(*Fixture.Labels.PredicateValue, PredicateCase.value,
            PredicateText);

  const std::string LiteralText = TCHAR_TO_UTF8(*Fixture.LiteralValue);
  const auto LiteralCase = func::testCase<int, std::string>(
      Fixture.LiteralInput, Fixture.LiteralExpected,
      [LiteralText](const int &) { return LiteralText; });
  TestTrue(*Fixture.Labels.LiteralPresent, func::isJust(LiteralCase));

  const std::string WildcardPrefix = TCHAR_TO_UTF8(*Fixture.WildcardPrefix);
  const std::string WildcardExpected =
      TCHAR_TO_UTF8(*Fixture.WildcardExpected);
  const auto WildcardCase = func::testCase<int, std::string>(
      Fixture.WildcardInput, func::_,
      [WildcardPrefix](const int &Value) {
        return WildcardPrefix + std::to_string(Value);
      });
  TestTrue(*Fixture.Labels.WildcardPresent, func::isJust(WildcardCase));
  TestEqual(*Fixture.Labels.WildcardValue, WildcardCase.value,
            WildcardExpected);

  const std::string ExactText = TCHAR_TO_UTF8(*Fixture.ExactValue);
  const std::string FallbackText = TCHAR_TO_UTF8(*Fixture.FallbackValue);
  std::vector<func::MatchCase<int, std::string>> Cases;
  Cases.push_back(func::when<int, std::string>(
      func::equals<int>(Fixture.ExactInput),
      [ExactText](const int &) { return ExactText; }));
  Cases.push_back(func::when<int, std::string>(
      func::wildcard<int>(),
      [FallbackText](const int &) { return FallbackText; }));

  const auto Matched =
      func::multiMatch<int, std::string>(Fixture.MatchInput, Cases);
  TestTrue(*Fixture.Labels.MatchPresent, func::isJust(Matched));
  TestEqual(*Fixture.Labels.MatchValue, Matched.value, ExactText);

  const auto Fallback =
      func::multiMatch<int, std::string>(Fixture.FallbackInput, Cases);
  TestTrue(*Fixture.Labels.FallbackPresent, func::isJust(Fallback));
  TestEqual(*Fixture.Labels.FallbackValue, Fallback.value, FallbackText);
  return true;
}
