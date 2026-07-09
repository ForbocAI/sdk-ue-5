#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Misc/AutomationTest.h"
#include "Protocol/ProtocolRequestTypes.h"

using namespace rtk;
using namespace DirectiveSlice;

/**
 * Test: Rapid sequential failures on different directives
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveRapidFailuresTest,
                                 "ForbocAI.Slices.Directive.RapidSequentialFailures",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveRapidFailuresTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  /**
   * Start 5 directives
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  for (int32 i = 0; i < 5; ++i) {
    const FString Id = FString::Printf(TEXT("rapid_%d"), i);
    State = DirSlice.Reducer(
        State,
        DirectiveSlice::Actions::directiveRunStarted(Id, TEXT("npc_stress"),
                                                     TEXT("obs")));
  }
  TestEqual("Five directives", selectAllDirectives(State).Num(), 5);

  /**
   * Fail them all in rapid succession
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  for (int32 i = 0; i < 5; ++i) {
    const FString Id = FString::Printf(TEXT("rapid_%d"), i);
    const FString Error =
        FString::Printf(TEXT("Timeout after %d ms"), (i + 1) * 1000);
    State = DirSlice.Reducer(
        State, DirectiveSlice::Actions::directiveRunFailed(Id, Error));
  }

  /**
   * Verify each has its own error and status
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  for (int32 i = 0; i < 5; ++i) {
    const FString Id = FString::Printf(TEXT("rapid_%d"), i);
    auto Run = selectDirectiveById(State, Id);
    TestTrue(FString::Printf(TEXT("rapid_%d exists"), i), Run.hasValue);
    if (Run.hasValue) {
      TestEqual(FString::Printf(TEXT("rapid_%d failed"), i),
                static_cast<int32>(Run.value.Status),
                static_cast<int32>(EDirectiveStatus::Failed));
      const FString ExpectedError =
          FString::Printf(TEXT("Timeout after %d ms"), (i + 1) * 1000);
      TestEqual(FString::Printf(TEXT("rapid_%d error"), i), Run.value.Error,
                ExpectedError);
    }
  }

  return true;
}

/**
 * Test: clearDirectivesForNpc clears ActiveDirectiveId when active is removed
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveClearActiveLostTest,
                                 "ForbocAI.Slices.Directive.ClearActiveLost",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveClearActiveLostTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("keep"), TEXT("npc_a"),
                                                   TEXT("obs1")));
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("remove"),
                                                   TEXT("npc_b"),
                                                   TEXT("obs2")));

  /**
   * Active is the last started
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Active is remove", State.ActiveDirectiveId,
            FString(TEXT("remove")));

  /**
   * Clear npc_b directives — should clear ActiveDirectiveId
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = DirSlice.Reducer(
      State, DirectiveSlice::Actions::clearDirectivesForNpc(TEXT("npc_b")));

  TestTrue("ActiveDirectiveId cleared", State.ActiveDirectiveId.IsEmpty());
  TestEqual("One directive remains", selectAllDirectives(State).Num(), 1);
  TestTrue("keep survives", selectDirectiveById(State, TEXT("keep")).hasValue);

  return true;
}

/**
 * Test: clearDirectivesForNpc preserves ActiveDirectiveId when active is kept
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveClearActivePreservedTest,
                                 "ForbocAI.Slices.Directive.ClearActivePreserved",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveClearActivePreservedTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("other"),
                                                   TEXT("npc_x"),
                                                   TEXT("obs1")));
  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("active"),
                                                   TEXT("npc_y"),
                                                   TEXT("obs2")));

  TestEqual("Active is active", State.ActiveDirectiveId,
            FString(TEXT("active")));

  /**
   * Clear npc_x — active should stay
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = DirSlice.Reducer(
      State, DirectiveSlice::Actions::clearDirectivesForNpc(TEXT("npc_x")));

  TestEqual("Active preserved", State.ActiveDirectiveId,
            FString(TEXT("active")));
  TestEqual("One remains", selectAllDirectives(State).Num(), 1);

  return true;
}

/**
 * Test: Verdict with no action (dialogue only)
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveVerdictDialogueOnlyTest,
                                 "ForbocAI.Slices.Directive.VerdictDialogueOnly",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveVerdictDialogueOnlyTest::RunTest(const FString &Parameters) {
  Slice<FDirectiveSliceState> DirSlice = createDirectiveSlice();
  FDirectiveSliceState State;

  State = DirSlice.Reducer(
      State,
      DirectiveSlice::Actions::directiveRunStarted(TEXT("dg"), TEXT("npc1"),
                                                   TEXT("obs")));

  FVerdictResponse Verdict;
  Verdict.bValid = true;
  Verdict.Dialogue = TEXT("I greet you warmly, traveler.");
  Verdict.bHasAction = false;

  State = DirSlice.Reducer(
      State, DirectiveSlice::Actions::verdictValidated(TEXT("dg"), Verdict));

  auto Run = selectDirectiveById(State, TEXT("dg"));
  TestTrue("Run exists", Run.hasValue);
  if (Run.hasValue) {
    TestEqual("Status Completed",
              static_cast<int32>(Run.value.Status),
              static_cast<int32>(EDirectiveStatus::Completed));
    TestTrue("Valid", Run.value.bVerdictValid);
    TestEqual("Dialogue set", Run.value.VerdictDialogue,
              FString(TEXT("I greet you warmly, traveler.")));
    TestTrue("No action type", Run.value.VerdictActionType.IsEmpty());
  }

  return true;
}

/**
 * Test: selectActiveDirective returns nothing when no active
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDirectiveSelectActiveEmptyTest,
                                 "ForbocAI.Slices.Directive.SelectActiveEmpty",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FDirectiveSelectActiveEmptyTest::RunTest(const FString &Parameters) {
  FDirectiveSliceState State;

  TestTrue("ActiveDirectiveId empty", State.ActiveDirectiveId.IsEmpty());
  TestFalse("selectActiveDirective returns nothing",
            selectActiveDirective(State).hasValue);

  return true;
}
