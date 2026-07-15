#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Directive/DirectiveSelectors.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/NPC/NPCActions.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolDirectiveLifecycleTest,
    "ForbocAI.Integration.Protocol.DirectiveLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FProtocolDirectiveLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT("ag_dir_test");
  Npc.Persona = TEXT("A directive test NPC");
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT("ag_dir_test")));
  TestStore.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT("run_1"), TEXT("ag_dir_test"), TEXT("Player attacks goblin")));

  FRuntimeState State = TestStore.getState();
  TestEqual("Active directive", State.Directives.ActiveDirectiveId,
            FString(TEXT("run_1")));
  func::Maybe<FDirectiveRun> Run =
      DirectiveSlice::selectDirectiveById(State.Directives, TEXT("run_1"));
  TestTrue("Run exists", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("Run status running", static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Running));
  }

  FDirectiveResponse DirResponse;
  DirResponse.recallMemory.Query = TEXT("goblin encounter");
  DirResponse.recallMemory.Limit = 5;
  DirResponse.recallMemory.Threshold = 0.7f;
  TestStore.dispatch(
      DirectiveSlice::Actions::directiveReceived(TEXT("run_1"), DirResponse));

  FVerdictResponse Verdict;
  Verdict.bValid = true;
  Verdict.Dialogue = TEXT("You swing your sword at the goblin!");
  Verdict.bHasAction = true;
  Verdict.Action.Type = TEXT("ATTACK");
  Verdict.Action.Target = TEXT("goblin");
  TestStore.dispatch(
      DirectiveSlice::Actions::verdictValidated(TEXT("run_1"), Verdict));

  State = TestStore.getState();
  Run = DirectiveSlice::selectDirectiveById(State.Directives, TEXT("run_1"));
  TestTrue("Run exists after verdict", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("Run completed", static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Completed));
    TestTrue("Verdict valid", Run.value.bVerdictValid);
    TestEqual("Verdict dialogue", Run.value.VerdictDialogue,
              FString(TEXT("You swing your sword at the goblin!")));
  }
  return true;
}
