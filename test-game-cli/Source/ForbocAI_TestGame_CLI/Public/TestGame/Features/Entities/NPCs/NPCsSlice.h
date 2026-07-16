#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/NPCs/NPCsAdapters.h"

namespace TestGame {

/** User Story: As a features entities npcs consumer, I need to invoke create npcs slice through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline rtk::Slice<FNPCsSliceState> CreateNPCsSlice() */
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
                        IsMoveVerdictAction(P.Action.Type) && P.Action.bHasTargetHex
                            ? P.Action.TargetHex
                            : Updated.Position;
                    return Updated;
                  });
              return Next;
            });
      });
}

} // namespace TestGame
