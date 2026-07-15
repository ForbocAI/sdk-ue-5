#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookCollectionsTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.Collections",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FFunctionalCookbookCollectionsTest::RunTest(const FString &Parameters) {
  std::vector<int> values{1, 2, 2, 3};
  int sum = func::fold_vector<int, int>(
      values, 0, [](int acc, const int &value) { return acc + value; });
  TestEqual("fold_vector sums values", sum, 8);

  std::vector<int> twos = func::filter_vector<int>(
      values, [](const int &value) { return value == 2; });
  TestEqual("filter_vector keeps matching values",
            static_cast<int>(twos.size()), 2);
  auto found = func::find_vector<int>(
      values, [](const int &value) { return value == 3; });
  TestTrue("find_vector returns Just on hit", found.hasValue);
  TestEqual("find_vector hit value", found.value, 3);
  std::vector<int> unique = func::unique_by<int>(
      values, [](const int &value) { return value; });
  TestEqual("unique_by keeps first values", static_cast<int>(unique.size()),
            3);
  TestTrue("contains_value finds existing value",
           func::contains_value<int>(unique, 2));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookMaybeEitherTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.MaybeEither",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FFunctionalCookbookMaybeEitherTest::RunTest(const FString &Parameters) {
  std::vector<int> values{2, 4};
  auto traversed = func::traverse_maybe<int>(
      values, [](const int &value) { return func::just(value * 2); });
  TestTrue("traverse_maybe succeeds when all values exist",
           traversed.hasValue);
  TestEqual("traverse_maybe maps two values",
            static_cast<int>(traversed.value.size()), 2);
  TestEqual("traverse_maybe first mapped value", traversed.value[0], 4);

  std::vector<func::Maybe<int>> optionalValues{
      func::just(1), func::nothing<int>()};
  auto sequenced = func::sequence_maybe<int>(optionalValues);
  TestFalse("sequence_maybe fails when any value is Nothing",
            sequenced.hasValue);
  auto lifted = func::lift3<int, int, int>(
      func::just(1), func::just(2), func::just(3),
      [](int a, int b, int c) { return a + b + c; });
  TestTrue("lift3 succeeds with all values", lifted.hasValue);
  TestEqual("lift3 combines values", lifted.value, 6);

  auto folded = func::fold_either<std::string, int, int>(
      values, 0,
      [](int acc, const int &value) -> func::Either<std::string, int> {
        return value < 5 ? func::make_right<std::string, int>(acc + value)
                         : func::make_left<std::string, int>(
                               std::string("too-large"));
      });
  TestFalse("fold_either succeeds while steps succeed", folded.isLeft);
  TestEqual("fold_either accumulated value", folded.right, 6);

  int indexedEffectTotal = 0;
  func::for_each_indexed(values, values.size(),
                         [&indexedEffectTotal](const int &value) {
                           indexedEffectTotal += value;
                         });
  TestEqual("for_each_indexed visits values", indexedEffectTotal, 6);
  const int indexedSum = func::fold_indexed(
      values, values.size(), 0,
      [](const int &acc, const int &value) { return acc + value; });
  TestEqual("fold_indexed accumulates values", indexedSum, 6);

  auto indexedFind = func::find_indexed(
      values, values.size(), [](const int &value) { return value == 2; });
  TestTrue("find_indexed finds matching value", indexedFind.hasValue);
  TestEqual("find_indexed returns matching value", indexedFind.value, 2);
  TestTrue("any_indexed finds matching predicate",
           func::any_indexed(values, values.size(),
                             [](const int &value) { return value > 2; }));
  TestTrue("all_indexed validates all values",
           func::all_indexed(values, values.size(),
                             [](const int &value) { return value > 0; }));

  const std::vector<int> grid =
      func::map_grid<int>(2, 3, [](const func::GridIndex &index) {
        return static_cast<int>(index.Row * 10 + index.Column);
      });
  TestEqual("map_grid output size", static_cast<int>(grid.size()), 6);
  TestEqual("map_grid first value", grid[0], 0);
  TestEqual("map_grid row-major value", grid[4], 11);
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCoreTsNameParityTest,
    "ForbocAI.Core.FunctionalCore.TSNameParity",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
bool FFunctionalCoreTsNameParityTest::RunTest(const FString &Parameters) {
  auto present = func::just(5);
  auto missing = func::nothing<int>();
  TestTrue("isJust mirrors TS", func::isJust(present));
  TestFalse("isJust rejects missing values", func::isJust(missing));
  TestTrue("isNothing mirrors TS", func::isNothing(missing));
  TestEqual("orElse mirrors TS", func::orElse(missing, 9), 9);
  TestEqual("requireJust mirrors TS", func::requireJust(present, "missing"), 5);

  int raw = 11;
  auto pointerValue = func::fromNullable(&raw);
  TestTrue("fromNullable pointer returns Just", func::isJust(pointerValue));
  TestEqual("fromNullable pointer value", pointerValue.value, 11);
  auto flaggedValue = func::fromNullable(std::string("ready"), true);
  TestTrue("fromNullable value flag returns Just", func::isJust(flaggedValue));
  TestEqual("fromNullable value flag content", flaggedValue.value,
            std::string("ready"));
  auto flaggedMissing = func::fromNullable(std::string(""), false);
  TestTrue("fromNullable false flag returns Nothing",
           func::isNothing(flaggedMissing));

  auto failure = func::left<std::string, int>(std::string("bad"));
  auto success = func::right<std::string, int>(3);
  TestTrue("isLeft mirrors TS", func::isLeft(failure));
  TestFalse("isRight rejects Left", func::isRight(failure));
  TestTrue("isRight mirrors TS", func::isRight(success));
  auto mapped = func::efmap<std::string, int>(
      success, [](int value) { return value + 2; });
  TestTrue("efmap keeps Right", func::isRight(mapped));
  TestEqual("efmap maps right payload", mapped.right, 5);

  func::Predicate<int> isEven = [](const int &value) {
    return value % 2 == 0;
  };
  auto predicateCase = func::testCase<int, std::string>(
      6, isEven, [](const int &) { return std::string("even"); });
  TestTrue("testCase predicate returns Just", func::isJust(predicateCase));
  TestEqual("testCase predicate payload", predicateCase.value,
            std::string("even"));
  auto literalCase = func::testCase<int, std::string>(
      4, 4, [](const int &) { return std::string("literal"); });
  TestTrue("testCase literal returns Just", func::isJust(literalCase));
  auto wildcardCase = func::testCase<int, std::string>(
      9, func::_, [](const int &value) {
        return std::string("got ") + std::to_string(value);
      });
  TestTrue("testCase wildcard returns Just", func::isJust(wildcardCase));
  TestEqual("testCase wildcard payload", wildcardCase.value,
            std::string("got 9"));

  std::vector<func::MatchCase<int, std::string>> cases;
  cases.push_back(func::when<int, std::string>(
      func::equals<int>(2), [](const int &) { return std::string("two"); }));
  cases.push_back(func::when<int, std::string>(
      func::wildcard<int>(),
      [](const int &) { return std::string("fallback"); }));
  auto matched = func::multiMatch<int, std::string>(2, cases);
  TestTrue("multiMatch returns Just", func::isJust(matched));
  TestEqual("multiMatch exact payload", matched.value, std::string("two"));
  auto fallback = func::multiMatch<int, std::string>(7, cases);
  TestTrue("multiMatch wildcard returns Just", func::isJust(fallback));
  TestEqual("multiMatch fallback payload", fallback.value,
            std::string("fallback"));
  return true;
}
