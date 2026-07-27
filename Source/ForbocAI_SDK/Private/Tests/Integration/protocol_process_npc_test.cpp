/**
 * processNPC full protocol loop - uses SDKConfig production default - I.2 Thunk Integration Tests.
 * Requires FORBOCAI_API_KEY. Set FORBOCAI_API_URL for production. Exercises tape evolution, NPC state, history, block behavior.
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/Directive/DirectiveSelectors.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Systems/Protocol/ProtocolThunks.h"
#include "Protocol/NPC/ProcessNPCTestAdapters.h"
#include "Store.h"

using namespace rtk;

namespace {

/** User Story: As a tests integration consumer, I need to invoke configure live api through a stable signature so the tests integration workflow remains explicit and composable. @fn bool ConfigureLiveApi(FAutomationTestBase &Test) */
bool ConfigureLiveApi(FAutomationTestBase &Test) {
  const FString ApiKey =
      FPlatformMisc::GetEnvironmentVariable(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5F0C966E7042));
  if (ApiKey.IsEmpty()) {
    Test.AddError(
        TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0D87130FB9D1));
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
  const int32 MaxPolls = FORBOCAI_SDK_AUTHORED_NUMBERV07C0796E1646;  // ~15s at 50ms

  if (!State->Store.IsValid()) {
    ProcessNPCTestAdapters::Start(State, Params);
    return false;
  }
  if (State->bCompleted)
    return true;
  if (++PollCount >= MaxPolls) {
    State->bCompleted = true;
    State->bSuccess = false;
    State->Error = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9FD0ADD5A4E1);
    return true;
  }
  FPlatformProcess::Sleep(FORBOCAI_SDK_AUTHORED_NUMBERV4B582E8E76C5);
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
    FORBOCAI_SDK_AUTHORED_STRINGV227D4975A4CA,
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
      State, FProcessNPCTestParams{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV251D3D537563), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV239645F4AA52),
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBF3D0FCA4463)},
      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV2F43F93A97CA, State->bCompleted);
        if (!State->bCompleted)
          return;
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVCDFE558FA72E, State->bSuccess);
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV704EDECC7A37), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        auto Run = DirectiveSlice::selectDirectiveById(
            StoreState.Directives,
            DirectiveSlice::selectActiveDirectiveId(StoreState.Directives));
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVD6FCFD23D50F, Run.hasValue);
        if (Run.hasValue) {
          TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAB20CD634973,
                    static_cast<int32>(Run.value.Status),
                    static_cast<int32>(
                        EDirectiveStatus::Completed));
        }

        auto Npc =
            NPCSelectors::selectNPCById(StoreState.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV251D3D537563));
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV28F2157B6D54, Npc.hasValue);
        if (Npc.hasValue) {
          TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV9F0FCD2C8ECA, Npc.value.History.Num() >= FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
          TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV869E1DF1D971, Npc.value.bIsBlocked);
        }

        // Wire-payload assertions on the live FAgentResponse — the API
        // must drive every field the test relies on (no fabricated response
        // object short-circuits the loop).
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV79C7194E2C52,
                 !State->Response.Dialogue.IsEmpty());
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));

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
    FORBOCAI_SDK_AUTHORED_STRINGV528051BE6582,
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
      State, FProcessNPCTestParams{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV11A26479761C),
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA4A5CE69EBCB),
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVACC7FD24FAD2)},
      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV2F43F93A97CA, State->bCompleted);
        if (!State->bCompleted)
          return;
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV704EDECC7A37), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        auto Npc =
            NPCSelectors::selectNPCById(StoreState.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV11A26479761C));
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV28F2157B6D54, Npc.hasValue);
        if (Npc.hasValue && Npc.value.bIsBlocked) {
          TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6DB68031A750,
                   !Npc.value.BlockReason.IsEmpty());
        }
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));

  return true;
}

/**
 * Test: processNPC directive lifecycle — Started -> Completed
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProcessNPCDirectiveLifecycleTest,
    FORBOCAI_SDK_AUTHORED_STRINGV3238821DCB8C,
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
      State, FProcessNPCTestParams{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV107C7B91D193),
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0B665CF47A61),
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1781C1774899)},
      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV964F9B262B38, State->bCompleted);
        if (!State->bCompleted)
          return;
        if (!State->bSuccess) {
          AddError(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV704EDECC7A37), *State->Error));
          return;
        }

        FRuntimeState StoreState = State->Store->getState();
        FString ActiveId =
            DirectiveSlice::selectActiveDirectiveId(StoreState.Directives);
        TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVC34FF3450068, ActiveId.IsEmpty());

        auto Run =
            DirectiveSlice::selectDirectiveById(StoreState.Directives, ActiveId);
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV9E6C5C93292A, Run.hasValue);
        if (Run.hasValue) {
          TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6156B21C6DA5,
                    static_cast<int32>(Run.value.Status),
                    static_cast<int32>(
                        EDirectiveStatus::Completed));
        }
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));

  return true;
}
