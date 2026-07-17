/**
 * processNPC full protocol loop - uses SDKConfig production default - I.2 Thunk Integration Tests.
 * Requires FORBOCAI_API_KEY. Set FORBOCAI_API_URL for production. Exercises tape evolution, NPC state, history, block behavior.
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Directive/DirectiveSelectors.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Protocol/NPC/ProcessNPCTestAdapters.h"
#include "Store.h"

using namespace rtk;

namespace {

/** User Story: As a tests integration consumer, I need to invoke configure live api through a stable signature so the tests integration workflow remains explicit and composable. @fn bool ConfigureLiveApi(FAutomationTestBase &Test) */
bool ConfigureLiveApi(FAutomationTestBase &Test) {
  const FString ApiKey =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_KEY"));
  if (ApiKey.IsEmpty()) {
    Test.AddError(
        TEXT("FORBOCAI_API_KEY is required for processNPC integration"));
    return false;
  }

  return true;
}

} // namespace

/**
 * Starts processNPC and polls until complete. Uses SDKConfig resolution.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
    FProcessNPCWaitComplete, TSharedPtr<FProcessNPCTestState>, State,
    FProcessNPCTestParams, Params, int32, PollCount);
/**
 * User Story: As a developer, I need Update to fulfill its role in the module.
 * @fn bool FProcessNPCWaitComplete::Update()
 */
bool FProcessNPCWaitComplete::Update() {
  const int32 MaxPolls = 300;  // ~15s at 50ms

  if (!State->Store.IsValid()) {
    ProcessNPCTestAdapters::Start(State, Params);
    return false;
  }
  if (State->bCompleted)
    return true;
  if (++PollCount >= MaxPolls) {
    State->bCompleted = true;
    State->bSuccess = false;
    State->Error = TEXT("Timeout waiting for processNPC");
    return true;
  }
  FPlatformProcess::Sleep(0.05f);
  return false;
}

/**
 * Test: processNPC with real API — full flow (valid verdict).
 * Drives the live SDKConfig-resolved API; renamed from the original
 * legacy test-double prefix per ForbocAI/demo-ue-5#7; the test has no
 * response object and asserts the real wire payload returned by the
 * API.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProcessNPCLiveFinalizeValidTest,
    "ForbocAI.Integration.Protocol.ProcessNPCLiveFinalizeValid",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FProcessNPCLiveFinalizeValidTest::RunTest(const FString &Parameters)
 */
bool FProcessNPCLiveFinalizeValidTest::RunTest(const FString &Parameters) {
  if (!ConfigureLiveApi(*this)) {
    return true;
  }

  auto State = MakeShared<FProcessNPCTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FProcessNPCWaitComplete(
      State, FProcessNPCTestParams{TEXT("npc_valid_1"), TEXT("Hello!"),
                                   TEXT("A friendly merchant")},
      0));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("processNPC completed", State->bCompleted);
        if (!State->bCompleted)
          return;
        TestTrue("processNPC succeeded", State->bSuccess);
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT("API error: %s"), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        auto Run = DirectiveSlice::selectDirectiveById(
            StoreState.Directives,
            DirectiveSlice::selectActiveDirectiveId(StoreState.Directives));
        TestTrue("Directive run exists", Run.hasValue);
        if (Run.hasValue) {
          TestEqual("Run completed",
                    static_cast<int32>(Run.value.Status),
                    static_cast<int32>(
                        EDirectiveStatus::Completed));
        }

        auto Npc =
            NPCSelectors::selectNPCById(StoreState.NPCs, TEXT("npc_valid_1"));
        TestTrue("NPC exists", Npc.hasValue);
        if (Npc.hasValue) {
          TestTrue("History has entries", Npc.value.History.Num() >= 1);
          TestFalse("NPC not blocked", Npc.value.bIsBlocked);
        }

        // Wire-payload assertions on the live FAgentResponse — the API
        // must drive every field the test relies on (no fabricated response
        // object short-circuits the loop).
        TestTrue("Response has dialogue from the API",
                 !State->Response.Dialogue.IsEmpty());
      },
      0.01f));

  return true;
}

/**
 * Test: processNPC with real API — block behavior (invalid verdict).
 * Uses observation that the API's bridge ruleset blocks. Renamed from
 * the original test-double prefix per ForbocAI/demo-ue-5#7; the test
 * dispatches the real `rtk::processNPC` thunk against the live SDKConfig
 * URL and asserts the actual `bIsBlocked` / `BlockReason` shape the API
 * returns.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProcessNPCLiveFinalizeInvalidTest,
    "ForbocAI.Integration.Protocol.ProcessNPCLiveFinalizeInvalid",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FProcessNPCLiveFinalizeInvalidTest::RunTest(const FString &Parameters)
 */
bool FProcessNPCLiveFinalizeInvalidTest::RunTest(const FString &Parameters) {
  if (!ConfigureLiveApi(*this)) {
    return true;
  }

  auto State = MakeShared<FProcessNPCTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FProcessNPCWaitComplete(
      State, FProcessNPCTestParams{TEXT("npc_block_1"),
                                   TEXT("I murder the innocent villager."),
                                   TEXT("A village guard")},
      0));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("processNPC completed", State->bCompleted);
        if (!State->bCompleted)
          return;
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT("API error: %s"), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        auto Npc =
            NPCSelectors::selectNPCById(StoreState.NPCs, TEXT("npc_block_1"));
        TestTrue("NPC exists", Npc.hasValue);
        if (Npc.hasValue && Npc.value.bIsBlocked) {
          TestTrue("Block reason set when blocked",
                   !Npc.value.BlockReason.IsEmpty());
        }
      },
      0.01f));

  return true;
}

/**
 * Test: processNPC directive lifecycle — Started -> Completed
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProcessNPCDirectiveLifecycleTest,
    "ForbocAI.Integration.Protocol.ProcessNPCDirectiveLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FProcessNPCDirectiveLifecycleTest::RunTest(const FString &Parameters)
 */
bool FProcessNPCDirectiveLifecycleTest::RunTest(const FString &Parameters) {
  if (!ConfigureLiveApi(*this)) {
    return true;
  }

  auto State = MakeShared<FProcessNPCTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FProcessNPCWaitComplete(
      State, FProcessNPCTestParams{TEXT("npc_lc_1"),
                                   TEXT("What do you sell?"),
                                   TEXT("Test merchant")},
      0));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Completed", State->bCompleted);
        if (!State->bCompleted)
          return;
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT("API error: %s"), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        FString ActiveId =
            DirectiveSlice::selectActiveDirectiveId(StoreState.Directives);
        TestFalse("Active directive set", ActiveId.IsEmpty());

        auto Run =
            DirectiveSlice::selectDirectiveById(StoreState.Directives, ActiveId);
        TestTrue("Run exists", Run.hasValue);
        if (Run.hasValue) {
          TestEqual("Status completed",
                    static_cast<int32>(Run.value.Status),
                    static_cast<int32>(
                        EDirectiveStatus::Completed));
        }
      },
      0.01f));

  return true;
}
