#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkEntityAdapterTest,
                                 "ForbocAI.Core.RTK.EntityAdapter",
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

  State = Adapter.addOne(State, FNpcFixtureState{TEXT("1"), 100});
  TestEqual("addOne total count", Selectors.selectTotal(State), 1);

  auto Ent1 = Selectors.selectById(State, TEXT("1"));
  TestTrue("selectById finds entity", Ent1.hasValue);
  TestEqual("selectById accurate health", Ent1.value.Health, 100);

  State = Adapter.addMany(
      State, {FNpcFixtureState{TEXT("2"), 200}, FNpcFixtureState{TEXT("3"), 300}});
  TestEqual("addMany total count", Selectors.selectTotal(State), 3);

  State = Adapter.updateOne(State, TEXT("3"), [](const FNpcFixtureState &E) {
    FNpcFixtureState Next = E;
    Next.Health = 350;
    return Next;
  });

  auto Ent3 = Selectors.selectById(State, TEXT("3"));
  TestEqual("updateOne payload accurate", Ent3.value.Health, 350);

  State = Adapter.removeOne(State, TEXT("1"));
  TestEqual("removeOne total count", Selectors.selectTotal(State), 2);
  TestFalse("removeOne removed entity from lookup",
            Selectors.selectById(State, TEXT("1")).hasValue);

  State = Adapter.setAll(State, {FNpcFixtureState{TEXT("4"), 400}});
  TestEqual("setAll total count", Selectors.selectTotal(State), 1);

  return true;
}
