#include "Misc/AutomationTest.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameNpcPatchAndSelectorsTest,
    "ForbocAI.Slices.TestGame.NpcPatchAndSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices npcs consumer, I need to invoke run test through a stable signature so the tests slices npcs workflow remains explicit and composable. @fn bool FTestGameNpcPatchAndSelectorsTest::RunTest(const FString &Parameters) */
bool FTestGameNpcPatchAndSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  FGameNPC Miller;
  Miller.Id = TEXT("miller");
  Miller.Name = TEXT("Miller");
  Miller.Faction = TEXT("Neutral");
  Miller.Hp = 100;
  Miller.Suspicion = 50;
  Miller.Position = FPosition(5, 12);

  const rtk::Slice<FNPCsSliceState> Slice = CreateNPCsSlice();
  FNPCsSliceState State =
      Slice.Reducer(FNPCsSliceState(), NPCsActions::UpsertNPC(Miller));

  NPCsActions::FPatchNPCPayload Patch;
  Patch.Id = TEXT("miller");
  Patch.Patch.Name = TEXT("Miller Prime");
  Patch.Patch.bHasName = true;
  Patch.Patch.Hp = 84;
  Patch.Patch.bHasHp = true;
  Patch.Patch.Suspicion = 75;
  Patch.Patch.bHasSuspicion = true;
  Patch.Patch.Inventory.Add(TEXT("medkit"));
  Patch.Patch.bHasInventory = true;
  Patch.Patch.Position = FPosition(6, 13);
  Patch.Patch.bHasPosition = true;

  State = Slice.Reducer(State, NPCsActions::PatchNPC(Patch));

  const func::Maybe<FGameNPC> MaybeNpc =
      NPCsSelectors::SelectNpcById(State, TEXT("miller"));
  TestTrue("NPC selector finds patched entity", MaybeNpc.hasValue);
  TestEqual("NPC name patches explicitly", MaybeNpc.value.Name,
            FString(TEXT("Miller Prime")));
  TestEqual("NPC hp patches explicitly", MaybeNpc.value.Hp, 84);
  TestEqual("NPC suspicion patches explicitly", MaybeNpc.value.Suspicion, 75);
  TestEqual("NPC inventory patches explicitly", MaybeNpc.value.Inventory.Num(),
            1);
  TestEqual("NPC position x patches explicitly", MaybeNpc.value.Position.X, 6);
  TestEqual("NPC total selector reads adapter state",
            NPCsSelectors::SelectNpcTotal(State), 1);
  TestEqual("NPC ids selector reads adapter state",
            NPCsSelectors::SelectNpcIds(State).Num(), 1);
  TestEqual("NPC all selector reads adapter state",
            NPCsSelectors::SelectAllNpcs(State).Num(), 1);

  NPCsActions::FApplyNpcVerdictPayload Verdict;
  Verdict.Id = TEXT("miller");
  Verdict.Action.Type = TEXT("MOVE");
  Verdict.Action.TargetHex = FPosition(7, 14);
  Verdict.Action.bHasTargetHex = true;
  Verdict.StateDelta.Suspicion = 55;
  Verdict.StateDelta.bHasSuspicion = true;
  State = Slice.Reducer(State, NPCsActions::ApplyNpcVerdict(Verdict));

  const func::Maybe<FGameNPC> VerdictNpc =
      NPCsSelectors::SelectNpcById(State, TEXT("miller"));
  TestTrue("NPC verdict selector finds entity", VerdictNpc.hasValue);
  TestEqual("NPC verdict applies state delta explicitly",
            VerdictNpc.value.Suspicion, 55);
  TestEqual("NPC verdict applies MOVE target", VerdictNpc.value.Position.X, 7);

  return true;
}
