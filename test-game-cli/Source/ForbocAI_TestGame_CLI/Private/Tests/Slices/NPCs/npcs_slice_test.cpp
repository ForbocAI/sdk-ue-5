#include "Misc/AutomationTest.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameNpcPatchAndSelectorsTest,
    VerificationAdapters::ArchitectureTestData()
        .stateDomains.automationNames.npc,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices npcs consumer, I need to invoke run test through a stable signature so the tests slices npcs workflow remains explicit and composable. @fn bool FTestGameNpcPatchAndSelectorsTest::RunTest(const FString &Parameters) */
bool FTestGameNpcPatchAndSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const Verification::FStateDomainsVerificationData &Data =
      VerificationAdapters::ArchitectureTestData().stateDomains;
  FTestGameStore Store = createTestGameStore();
  Store.dispatch(NPCsActions::UpsertNPC(Data.npc.initial));

  NPCsActions::FPatchNPCPayload Patch;
  Patch.Id = Data.npc.initial.Id;
  Patch.Patch.Name = Data.npc.patch.name;
  Patch.Patch.bHasName = true;
  Patch.Patch.Hp = Data.npc.patch.hp;
  Patch.Patch.bHasHp = true;
  Patch.Patch.Suspicion = Data.npc.patch.suspicion;
  Patch.Patch.bHasSuspicion = true;
  Patch.Patch.Inventory = Data.npc.patch.inventory;
  Patch.Patch.bHasInventory = true;
  Patch.Patch.Position = Data.npc.patch.position;
  Patch.Patch.bHasPosition = true;

  Store.dispatch(NPCsActions::PatchNPC(Patch));

  const func::Maybe<FGameNPC> MaybeNpc =
      NPCsSelectors::SelectNpcById(Store.getState().NPCs,
                                   Data.npc.initial.Id);
  TestTrue(Data.stories.npc, MaybeNpc.hasValue);
  TestEqual(Data.stories.npc, MaybeNpc.value.Name, Data.npc.patch.name);
  TestEqual(Data.stories.npc, MaybeNpc.value.Hp, Data.npc.patch.hp);
  TestEqual(Data.stories.npc, MaybeNpc.value.Suspicion,
            Data.npc.patch.suspicion);
  TestEqual(Data.stories.npc, MaybeNpc.value.Inventory.Num(),
            Data.npc.patch.inventory.Num());
  TestEqual(Data.stories.npc, MaybeNpc.value.Position.X,
            Data.npc.patch.position.X);
  TestEqual(Data.stories.npc,
            NPCsSelectors::SelectNpcTotal(Store.getState().NPCs),
            Data.npc.expectedEntityCount);
  TestEqual(Data.stories.npc,
            NPCsSelectors::SelectNpcIds(Store.getState().NPCs).Num(),
            Data.npc.expectedEntityCount);
  TestEqual(Data.stories.npc,
            NPCsSelectors::SelectAllNpcs(Store.getState().NPCs).Num(),
            Data.npc.expectedEntityCount);

  NPCsActions::FApplyNpcVerdictPayload Verdict;
  Verdict.Id = Data.npc.initial.Id;
  Verdict.Action.Type = Data.npc.verdict.actionType;
  Verdict.Action.TargetHex = Data.npc.verdict.targetPosition;
  Verdict.Action.bHasTargetHex = true;
  Verdict.StateDelta.Suspicion = Data.npc.verdict.suspicion;
  Verdict.StateDelta.bHasSuspicion = true;
  Store.dispatch(NPCsActions::ApplyNpcVerdict(Verdict));

  const func::Maybe<FGameNPC> VerdictNpc =
      NPCsSelectors::SelectNpcById(Store.getState().NPCs,
                                   Data.npc.initial.Id);
  TestTrue(Data.stories.npc, VerdictNpc.hasValue);
  TestEqual(Data.stories.npc, VerdictNpc.value.Suspicion,
            Data.npc.verdict.suspicion);
  TestEqual(Data.stories.npc, VerdictNpc.value.Position.X,
            Data.npc.verdict.targetPosition.X);

  return true;
}
