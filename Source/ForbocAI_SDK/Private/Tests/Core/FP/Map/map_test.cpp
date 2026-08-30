#include "Core/fp.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/Testing/FP/Composition/CompositionAdapters.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FFunctionalCookbookMapLawsTest,
    FORBOCAI_SDK_AUTHORED_STRINGVBAAC0F36A0F6,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/** User Story: As a core fp map consumer, I need the right-biased combination laws proven from authored fixtures so domain folds can depend on stable algebra. @fn bool FFunctionalCookbookMapLawsTest::RunTest(const FString &Parameters) */
bool FFunctionalCookbookMapLawsTest::RunTest(const FString &Parameters) {
  const auto &Fixture =
      Testing::FP::Composition::CompositionFixtures().Map;
  const TMap<FString, int32> Empty;
  const TMap<FString, int32> Left{
      {Fixture.LeftOnlyKey, Fixture.LeftOnlyValue},
      {Fixture.SharedKey, Fixture.LeftSharedValue},
  };
  const TMap<FString, int32> Right{
      {Fixture.SharedKey, Fixture.RightSharedValue},
      {Fixture.RightOnlyKey, Fixture.RightOnlyValue},
  };
  const TMap<FString, int32> Third{
      {Fixture.LeftOnlyKey, Fixture.RightOnlyValue},
  };
  const auto EqualValues = [](const int32 &LeftValue,
                              const int32 &RightValue) {
    return LeftValue == RightValue;
  };
  const auto EqualMaps = [EqualValues](const TMap<FString, int32> &LeftMap,
                                        const TMap<FString, int32> &RightMap) {
    return func::map_values_equal(LeftMap, RightMap, EqualValues);
  };

  TestTrue(*Fixture.Labels.LeftIdentity,
           EqualMaps(func::merge_maps_right(Empty, Left), Left));
  TestTrue(*Fixture.Labels.RightIdentity,
           EqualMaps(func::merge_maps_right(Left, Empty), Left));
  TestTrue(
      *Fixture.Labels.Associative,
      EqualMaps(func::merge_maps_right(
                    func::merge_maps_right(Left, Right), Third),
                func::merge_maps_right(
                    Left, func::merge_maps_right(Right, Third))));

  const TMap<FString, int32> Combined =
      func::merge_maps_right(Left, Right);
  TestEqual(*Fixture.Labels.RightBias,
            func::map_value_or(Combined, Fixture.SharedKey,
                               Fixture.LeftSharedValue),
            Fixture.RightSharedValue);
  TestEqual(*Fixture.Labels.PreservesLeft,
            func::map_value_or(Combined, Fixture.LeftOnlyKey,
                               Fixture.RightOnlyValue),
            Fixture.LeftOnlyValue);
  TestEqual(*Fixture.Labels.PreservesRight,
            func::map_value_or(Combined, Fixture.RightOnlyKey,
                               Fixture.LeftOnlyValue),
            Fixture.RightOnlyValue);
  TestEqual(*Fixture.Labels.Count, Combined.Num(), Fixture.ExpectedCount);
  return true;
}
