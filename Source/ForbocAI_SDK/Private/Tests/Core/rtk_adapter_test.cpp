#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkEntityAdapterTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV85BA1182C6D5,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkEntityAdapterTest::RunTest(const FString &Parameters)
 */
bool FRtkEntityAdapterTest::RunTest(const FString &Parameters) {
  auto Adapter = createEntityAdapter<FNpcFixtureState>(
      [](const FNpcFixtureState &E) { return E.Id; });
  auto State = Adapter.getInitialState();
  auto Selectors = Adapter.getSelectors();

  State = Adapter.addOne(State, FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEB2010F4451E), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831});
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV130ACCE710BF, Selectors.selectTotal(State), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  auto Ent1 = Selectors.selectById(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEB2010F4451E));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV1966732D3B9B, Ent1.hasValue);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV8FCD6B4B18BE, Ent1.value.Health, FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831);

  State = Adapter.addMany(
      State, {FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF1E6B42C3034), FORBOCAI_SDK_AUTHORED_NUMBERVEF1B5401B507}, FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9F798924E236), FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646}});
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV80799FB6EB06, Selectors.selectTotal(State), FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787);

  State = Adapter.updateOne(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9F798924E236), [](const FNpcFixtureState &E) {
    FNpcFixtureState Next = E;
    Next.Health = FORBOCAI_SDK_AUTHORED_NUMBERV9C481E234314;
    return Next;
  });

  auto Ent3 = Selectors.selectById(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9F798924E236));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVBD0082CEDF7D, Ent3.value.Health, FORBOCAI_SDK_AUTHORED_NUMBERV9C481E234314);

  State = Adapter.removeOne(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEB2010F4451E));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVCBF4F59CC46C, Selectors.selectTotal(State), FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVC7F69BC22267,
            Selectors.selectById(State, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEB2010F4451E)).hasValue);

  State = Adapter.setAll(State, {FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV38A44056EB4D), FORBOCAI_SDK_AUTHORED_NUMBERV6788D2DFBECD}});
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV68E081DDB38D, Selectors.selectTotal(State), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  return true;
}
