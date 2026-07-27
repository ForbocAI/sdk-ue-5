#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkSelectorTest, FORBOCAI_SDK_AUTHORED_STRINGV5F2BA5EC957D,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkSelectorTest::RunTest(const FString &Parameters)
 */
bool FRtkSelectorTest::RunTest(const FString &Parameters) {
  struct FTestState {
    int32 A;
    int32 B;

    bool operator==(const FTestState &Other) const {
      return A == Other.A && B == Other.B;
    }
  };

  auto SelectA = [](const FTestState &State) { return State.A; };
  auto SelectB = [](const FTestState &State) { return State.B; };

  int32 Computations = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;

  auto ComplexSelector = createSelector<FTestState, int32>(
      std::make_tuple(SelectA, SelectB), [&Computations](int32 A, int32 B) {
        Computations++;
        return A + B;
      });

  FTestState State1{FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F, FORBOCAI_SDK_AUTHORED_NUMBERV60102E9346C8};

  /**
   * First call computes
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV853C607AFC86, ComplexSelector(State1), FORBOCAI_SDK_AUTHORED_NUMBERV3A02326BFC02);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV3A2989202497, Computations, FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  /**
   * Second call with same state (by value equality) hits cache
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FTestState State2{FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F, FORBOCAI_SDK_AUTHORED_NUMBERV60102E9346C8};
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF353C97EC47A, ComplexSelector(State2), FORBOCAI_SDK_AUTHORED_NUMBERV3A02326BFC02);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVD5294C46EEF5, Computations, FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  /**
   * Third call with new state computes
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FTestState State3{FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F, FORBOCAI_SDK_AUTHORED_NUMBERV258E2447D900};
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV2D35FD293626, ComplexSelector(State3), FORBOCAI_SDK_AUTHORED_NUMBERV096A9CA942F7);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV57E9AC7D39AD, Computations, FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);

  return true;
}
