#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/Directive/DirectiveSelectors.h"
#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/NPC/NPCActions.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolDirectiveLifecycleTest,
    FORBOCAI_SDK_AUTHORED_STRINGVC0BE17B16E6B,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol directive consumer, I need to invoke run test through a stable signature so the integration protocol directive workflow remains explicit and composable. @fn bool FProtocolDirectiveLifecycleTest::RunTest(const FString &Parameters) */
bool FProtocolDirectiveLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2604CA73BBC1);
  Npc.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV52D6E263F0B9);
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2604CA73BBC1)));
  TestStore.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2604CA73BBC1), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA59FEC8FA667)));

  FRuntimeState State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV167A86D29691, State.Directives.ActiveDirectiveId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0)));
  func::Maybe<FDirectiveRun> Run =
      DirectiveSlice::selectDirectiveById(State.Directives, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV9E6C5C93292A, Run.hasValue);
  if (Run.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVE094CB23E25B, static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Running));
  }

  FDirectiveResponse DirResponse;
  DirResponse.recallMemory.Query = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA9225A493166);
  DirResponse.recallMemory.Limit = FORBOCAI_SDK_AUTHORED_NUMBERV2B61CCD40B6E;
  DirResponse.recallMemory.Threshold = FORBOCAI_SDK_AUTHORED_NUMBERVA3CBFCEBD9ED;
  TestStore.dispatch(
      DirectiveSlice::Actions::directiveReceived(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0), DirResponse));

  FVerdictResponse Verdict;
  Verdict.bValid = true;
  Verdict.Dialogue = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63DA697143A6);
  Verdict.bHasAction = true;
  Verdict.Action.Type = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5EF270DC93FB);
  Verdict.Action.Target = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCC3E8E496F2C);
  TestStore.dispatch(
      DirectiveSlice::Actions::verdictValidated(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0), Verdict));

  State = TestStore.getState();
  Run = DirectiveSlice::selectDirectiveById(State.Directives, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV43E5E61159D0));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV07D039443FD5, Run.hasValue);
  if (Run.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAB20CD634973, static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Completed));
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVB8BAC0950C5D, Run.value.bVerdictValid);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV553DF39E998A, Run.value.VerdictDialogue,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63DA697143A6)));
  }
  return true;
}
