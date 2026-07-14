#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Misc/AutomationTest.h"
#include "Features/Protocol/Requests/RequestsTypes.h"

using namespace rtk;
using namespace DirectiveSlice;

/**
 * Test: Full happy path — Started → Received → Validated
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveHappyPathTest,
                                 "ForbocAI.Slices.Directive.HappyPath",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveHappyPathTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  /**
   * Step 1: Start directive
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(
          TEXT("hp_1"), TEXT("npc_knight"), TEXT("Player attacks goblin")));

  func::Maybe<FDirectiveRun> Run = selectDirectiveById(State, TEXT("hp_1"));
  TestTrue("Run created", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("Status Running",
              static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Running));
    TestEqual("NpcId set", Run.value.NpcId, FString(TEXT("npc_knight")));
    TestEqual("Observation set", Run.value.Observation,
              FString(TEXT("Player attacks goblin")));
  }
  TestEqual("Active directive", State.ActiveDirectiveId,
            FString(TEXT("hp_1")));

  /**
   * Step 2: Directive received (memory recall instruction)
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FDirectiveResponse DirResponse;
  DirResponse.recallMemory.Query = TEXT("goblin encounters");
  DirResponse.recallMemory.Limit = 5;
  DirResponse.recallMemory.Threshold = 0.6f;

  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveReceived(TEXT("hp_1"), DirResponse));

  Run = selectDirectiveById(State, TEXT("hp_1"));
  TestTrue("Run still exists", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("MemoryRecallQuery set", Run.value.MemoryRecallQuery,
              FString(TEXT("goblin encounters")));
    TestEqual("MemoryRecallLimit", Run.value.MemoryRecallLimit, 5);
  }

  /**
   * Step 3: Verdict validated (valid)
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FVerdictResponse Verdict;
  Verdict.bValid = true;
  Verdict.Dialogue = TEXT("You swing your sword at the goblin!");
  Verdict.bHasAction = true;
  Verdict.Action.Type = TEXT("ATTACK");
  Verdict.Action.Target = TEXT("goblin");
  Verdict.Action.Reason = TEXT("Combat engagement");

  State = DirSlice.Reducer(
      State, DirectiveSlice::Actions::verdictValidated(TEXT("hp_1"), Verdict));

  Run = selectDirectiveById(State, TEXT("hp_1"));
  TestTrue("Run exists after verdict", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("Status Completed",
              static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Completed));
    TestTrue("Verdict valid", Run.value.bVerdictValid);
    TestEqual("Dialogue matches", Run.value.VerdictDialogue,
              FString(TEXT("You swing your sword at the goblin!")));
    TestEqual("Action type", Run.value.VerdictActionType,
              FString(TEXT("ATTACK")));
  }

  return true;
}

/**
 * Test: Multiple concurrent directives maintain independent state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveMultipleTest,
                                 "ForbocAI.Slices.Directive.MultipleConcurrent",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveMultipleTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  /**
   * Start two directives for different NPCs
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("d_a"), TEXT("npc_1"),
                                                   TEXT("obs_a")));
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("d_b"), TEXT("npc_2"),
                                                   TEXT("obs_b")));

  TestEqual("Two directives", selectAllDirectives(State).Num(), 2);
  TestEqual("Active is last started", State.ActiveDirectiveId,
            FString(TEXT("d_b")));

  /**
   * Fail first, second stays running
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunFailed(TEXT("d_a"),
                                                  TEXT("API timeout")));

  auto A = selectDirectiveById(State, TEXT("d_a"));
  auto B = selectDirectiveById(State, TEXT("d_b"));
  TestTrue("d_a exists", A.hasValue);
  TestTrue("d_b exists", B.hasValue);
  if (A.hasValue && B.hasValue) {
    TestEqual("d_a Failed",
              static_cast<int32>(A.value.Status),
              static_cast<int32>(EDirectiveStatus::Failed));
    TestEqual("d_b still Running",
              static_cast<int32>(B.value.Status),
              static_cast<int32>(EDirectiveStatus::Running));
  }

  return true;
}

/**
 * Test: clearDirectivesForNpc removes only matching NPC directives
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveClearForNpcTest,
                                 "ForbocAI.Slices.Directive.ClearForNpcSelective",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveClearForNpcTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("d1"),
                                                   TEXT("npc_target"),
                                                   TEXT("obs1")));
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("d2"),
                                                   TEXT("npc_keep"),
                                                   TEXT("obs2")));
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("d3"),
                                                   TEXT("npc_target"),
                                                   TEXT("obs3")));

  TestEqual("Three directives", selectAllDirectives(State).Num(), 3);

  /**
   * Clear only npc_target directives
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = DirSlice.Reducer(
      State, DirectiveSlice::Actions::clearDirectivesForNpc(TEXT("npc_target")));

  TestEqual("One directive remains", selectAllDirectives(State).Num(), 1);
  TestFalse("d1 removed", selectDirectiveById(State, TEXT("d1")).hasValue);
  TestTrue("d2 kept", selectDirectiveById(State, TEXT("d2")).hasValue);
  TestFalse("d3 removed", selectDirectiveById(State, TEXT("d3")).hasValue);

  return true;
}
