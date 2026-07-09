#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/NPCs/NPCsAdapters.h"

namespace TestGame {

struct FNPCsSliceState {
  rtk::EntityState<FGameNPC> Entities;

  FNPCsSliceState() : Entities(GetNPCAdapter().getInitialState()) {}

  bool operator==(const FNPCsSliceState &O) const {
    return Entities.ids == O.Entities.ids;
  }
};

namespace NPCsSelectors {
inline TArray<FGameNPC> SelectAllNpcs(const FNPCsSliceState &S) {
  return GetNPCAdapter().getSelectors().selectAll(S.Entities);
}

inline func::Maybe<FGameNPC> SelectNpcById(const FNPCsSliceState &S,
                                           const FString &Id) {
  return GetNPCAdapter().getSelectors().selectById(S.Entities, Id);
}

inline rtk::EntityState<FGameNPC> SelectNpcEntities(const FNPCsSliceState &S) {
  return S.Entities;
}

inline TArray<FString> SelectNpcIds(const FNPCsSliceState &S) {
  return GetNPCAdapter().getSelectors().selectIds(S.Entities);
}

inline int32 SelectNpcTotal(const FNPCsSliceState &S) {
  return GetNPCAdapter().getSelectors().selectTotal(S.Entities);
}
} // namespace NPCsSelectors

inline FGameNPC PatchNpc(const FGameNPC &Existing,
                         const NPCsActions::FPatchNPCChanges &Patch) {
  FGameNPC Updated = Existing;
  Updated.Name = Patch.bHasName ? Patch.Name : Updated.Name;
  Updated.Faction = Patch.bHasFaction ? Patch.Faction : Updated.Faction;
  Updated.Hp = Patch.bHasHp ? Patch.Hp : Updated.Hp;
  Updated.Suspicion =
      Patch.bHasSuspicion ? Patch.Suspicion : Updated.Suspicion;
  Updated.Inventory = Patch.bHasInventory ? Patch.Inventory : Updated.Inventory;
  Updated.KnownSecrets =
      Patch.bHasKnownSecrets ? Patch.KnownSecrets : Updated.KnownSecrets;
  Updated.Position = Patch.bHasPosition ? Patch.Position : Updated.Position;
  return Updated;
}

inline rtk::Slice<FNPCsSliceState> CreateNPCsSlice() {
  return rtk::createSlice<FNPCsSliceState>(
      TEXT("testgame/npcs"), FNPCsSliceState(),
      [](rtk::ActionReducerMapBuilder<FNPCsSliceState> &Builder) {
        Builder.addCase(
            NPCsActions::UpsertNPCActionCreator(),
            [](const FNPCsSliceState &S,
               const rtk::Action<FGameNPC> &A) -> FNPCsSliceState {
              FNPCsSliceState Next = S;
              Next.Entities =
                  GetNPCAdapter().upsertOne(Next.Entities, A.PayloadValue);
              return Next;
            });
        Builder.addCase(
            NPCsActions::MoveNPCActionCreator(),
            [](const FNPCsSliceState &S,
               const rtk::Action<NPCsActions::FMoveNPCPayload> &A)
                -> FNPCsSliceState {
              FNPCsSliceState Next = S;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, A.PayloadValue.Id,
                  [&A](const FGameNPC &Existing) {
                    FGameNPC Updated = Existing;
                    Updated.Position = A.PayloadValue.Position;
                    return Updated;
                  });
              return Next;
            });
        Builder.addCase(
            NPCsActions::PatchNPCActionCreator(),
            [](const FNPCsSliceState &S,
               const rtk::Action<NPCsActions::FPatchNPCPayload> &A)
                -> FNPCsSliceState {
              FNPCsSliceState Next = S;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, A.PayloadValue.Id,
                  [&A](const FGameNPC &Existing) {
                    return PatchNpc(Existing, A.PayloadValue.Patch);
                  });
              return Next;
            });
        Builder.addCase(
            NPCsActions::ApplyNpcVerdictActionCreator(),
            [](const FNPCsSliceState &S,
               const rtk::Action<NPCsActions::FApplyNpcVerdictPayload> &A)
                -> FNPCsSliceState {
              FNPCsSliceState Next = S;
              const auto &P = A.PayloadValue;
              Next.Entities = GetNPCAdapter().updateOne(
                  Next.Entities, P.Id, [&P](const FGameNPC &Existing) {
                    FGameNPC Updated = PatchNpc(Existing, P.StateDelta);
                    Updated.Position =
                        P.Action.Type == TEXT("MOVE") && P.Action.bHasTargetHex
                            ? P.Action.TargetHex
                            : Updated.Position;
                    return Updated;
                  });
              return Next;
            });
      });
}

} // namespace TestGame
