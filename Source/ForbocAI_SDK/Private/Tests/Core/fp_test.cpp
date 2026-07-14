/**
 * Tests for fp.hpp §19 Dispatcher, §20 multi_match, §21 from_nullable
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * Test: Dispatcher — key lookup returns just(handler())
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherKeyLookupTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.KeyLookup",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
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

/**
 * Test: Dispatcher — missing key returns nothing
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherMissingKeyTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.MissingKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDispatcherMissingKeyTest::RunTest(const FString &Parameters) {
  std::vector<std::pair<std::string, std::function<int()>>> entries;
  entries.push_back(std::make_pair(std::string("a"), []() { return 1; }));

  auto d = func::createDispatcher<std::string, int>(entries);

  auto result = func::dispatch(d, std::string("z"));
  TestFalse("Missing key returns nothing", result.hasValue);

  return true;
}

/**
 * Test: Dispatcher — has() and keys()
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherHasAndKeysTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.HasAndKeys",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
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

/**
 * Test: Dispatcher — strict Either miss
 * User Story: As a maintainer, I need strict dispatcher misses to surface as
 * typed errors instead of substitute behavior.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDispatcherEitherMissTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.EitherMiss",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
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

/**
 * Test: ArgDispatcher — strict argument dispatch
 * User Story: As ECS formatter code, I need argument dispatch to return Maybe
 * on misses so no hidden default branch is required.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FArgDispatcherStrictTest,
    "ForbocAI.Core.FunctionalCore.Dispatcher.ArgStrict",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
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

/**
 * Test: Cookbook collection helpers
 * User Story: As a maintainer, I need fold/filter/find/unique helpers covered
 * so feature code can reuse them instead of inventing local request wrappers.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookCollectionsTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.Collections",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FFunctionalCookbookCollectionsTest::RunTest(const FString &Parameters) {
  std::vector<int> values;
  values.push_back(1);
  values.push_back(2);
  values.push_back(2);
  values.push_back(3);

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

/**
 * Test: Cookbook Maybe/Either composition helpers
 * User Story: As data assembly code, I need traversal, sequence, lift, and
 * Either folding covered so missing data stays explicit.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookMaybeEitherTest,
    "ForbocAI.Core.FunctionalCore.Cookbook.MaybeEither",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FFunctionalCookbookMaybeEitherTest::RunTest(const FString &Parameters) {
  std::vector<int> values;
  values.push_back(2);
  values.push_back(4);

  auto traversed = func::traverse_maybe<int>(
      values, [](const int &value) { return func::just(value * 2); });
  TestTrue("traverse_maybe succeeds when all values exist",
           traversed.hasValue);
  TestEqual("traverse_maybe maps two values",
            static_cast<int>(traversed.value.size()), 2);
  TestEqual("traverse_maybe first mapped value", traversed.value[0], 4);

  std::vector<func::Maybe<int>> optionalValues;
  optionalValues.push_back(func::just(1));
  optionalValues.push_back(func::nothing<int>());
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

  const int indexedSum =
      func::fold_indexed(values, values.size(), 0,
                         [](const int &acc, const int &value) {
                           return acc + value;
                         });
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

/**
 * Test: TypeScript functional-core name parity
 * User Story: As SDK users moving between TS and UE, I need the public FP names
 * to read the same while preserving the UE implementations underneath them.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCoreTsNameParityTest,
    "ForbocAI.Core.FunctionalCore.TSNameParity",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
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

/**
 * Test: multi_match — predicate matching
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchPredicateTest,
    "ForbocAI.Core.FunctionalCore.MultiMatch.Predicate",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMultiMatchPredicateTest::RunTest(const FString &Parameters) {
  std::vector<func::MatchCase<int, std::string>> cases;
  cases.push_back(func::when<int, std::string>(
      [](const int &v) { return v < 0; },
      [](const int &) { return std::string("negative"); }));
  cases.push_back(func::when<int, std::string>(
      [](const int &v) { return v == 0; },
      [](const int &) { return std::string("zero"); }));
  cases.push_back(func::when<int, std::string>(
      [](const int &v) { return v > 0; },
      [](const int &) { return std::string("positive"); }));

  auto neg = func::multi_match<int, std::string>(-5, cases);
  TestTrue("Negative matched", neg.hasValue);
  TestEqual("Negative result", neg.value, std::string("negative"));

  auto zero = func::multi_match<int, std::string>(0, cases);
  TestTrue("Zero matched", zero.hasValue);
  TestEqual("Zero result", zero.value, std::string("zero"));

  auto pos = func::multi_match<int, std::string>(42, cases);
  TestTrue("Positive matched", pos.hasValue);
  TestEqual("Positive result", pos.value, std::string("positive"));

  return true;
}

/**
 * Test: multi_match — wildcard catches all
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchWildcardTest,
    "ForbocAI.Core.FunctionalCore.MultiMatch.Wildcard",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMultiMatchWildcardTest::RunTest(const FString &Parameters) {
  std::vector<func::MatchCase<int, std::string>> cases;
  cases.push_back(func::when<int, std::string>(
      func::equals<int>(1),
      [](const int &) { return std::string("one"); }));
  cases.push_back(func::when<int, std::string>(
      func::wildcard<int>(),
      [](const int &) { return std::string("default"); }));

  auto one = func::multi_match<int, std::string>(1, cases);
  TestTrue("Exact match found", one.hasValue);
  TestEqual("Exact match result", one.value, std::string("one"));

  auto other = func::multi_match<int, std::string>(99, cases);
  TestTrue("Wildcard matched", other.hasValue);
  TestEqual("Wildcard result", other.value, std::string("default"));

  return true;
}

/**
 * Test: multi_match — no match returns nothing
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchNoMatchTest,
    "ForbocAI.Core.FunctionalCore.MultiMatch.NoMatch",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMultiMatchNoMatchTest::RunTest(const FString &Parameters) {
  std::vector<func::MatchCase<int, std::string>> cases;
  cases.push_back(func::when<int, std::string>(
      func::equals<int>(1),
      [](const int &) { return std::string("one"); }));

  auto result = func::multi_match<int, std::string>(99, cases);
  TestFalse("No match returns nothing", result.hasValue);

  return true;
}

/**
 * Test: multi_match — value equality via equals()
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMultiMatchEqualsTest,
    "ForbocAI.Core.FunctionalCore.MultiMatch.Equals",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMultiMatchEqualsTest::RunTest(const FString &Parameters) {
  std::vector<func::MatchCase<std::string, int>> cases;
  cases.push_back(func::when<std::string, int>(
      func::equals<std::string>(std::string("alpha")),
      [](const std::string &) { return 1; }));
  cases.push_back(func::when<std::string, int>(
      func::equals<std::string>(std::string("beta")),
      [](const std::string &) { return 2; }));
  cases.push_back(func::when<std::string, int>(
      func::equals<std::string>(std::string("gamma")),
      [](const std::string &) { return 3; }));

  auto alpha = func::multi_match<std::string, int>(std::string("alpha"), cases);
  TestTrue("alpha matched", alpha.hasValue);
  TestEqual("alpha returns 1", alpha.value, 1);

  auto beta = func::multi_match<std::string, int>(std::string("beta"), cases);
  TestTrue("beta matched", beta.hasValue);
  TestEqual("beta returns 2", beta.value, 2);

  auto miss = func::multi_match<std::string, int>(std::string("delta"), cases);
  TestFalse("delta not matched", miss.hasValue);

  return true;
}

/**
 * Test: from_nullable — pointer
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFromNullablePtrTest,
    "ForbocAI.Core.FunctionalCore.FromNullable.Pointer",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FFromNullablePtrTest::RunTest(const FString &Parameters) {
  int val = 42;
  int *p = &val;
  auto m = func::from_nullable(p);
  TestTrue("Non-null pointer is just", m.hasValue);
  TestEqual("Value from pointer", m.value, 42);

  int *null_p = nullptr;
  auto n = func::from_nullable(null_p);
  TestFalse("Null pointer is nothing", n.hasValue);

  return true;
}

/**
 * Test: from_nullable_value — explicit validity flag
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFromNullableValueTest,
    "ForbocAI.Core.FunctionalCore.FromNullable.Value",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FFromNullableValueTest::RunTest(const FString &Parameters) {
  auto valid = func::from_nullable_value(std::string("hello"), true);
  TestTrue("Valid value is just", valid.hasValue);
  TestEqual("Valid value content", valid.value, std::string("hello"));

  auto invalid = func::from_nullable_value(std::string(""), false);
  TestFalse("Invalid value is nothing", invalid.hasValue);

  return true;
}

/**
 * Test: require_just — extracts or throws
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRequireJustTest,
    "ForbocAI.Core.FunctionalCore.RequireJust",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FRequireJustTest::RunTest(const FString &Parameters) {
  auto j = func::just(42);
  int extracted = func::require_just(j, "should not throw");
  TestEqual("require_just extracts value", extracted, 42);

  auto n = func::nothing<int>();
  bool threw = false;
  try {
    func::require_just(n, "expected failure");
  } catch (const std::runtime_error &e) {
    threw = true;
    TestEqual("Error message matches", std::string(e.what()),
              std::string("expected failure"));
  }
  TestTrue("require_just throws on nothing", threw);

  return true;
}
