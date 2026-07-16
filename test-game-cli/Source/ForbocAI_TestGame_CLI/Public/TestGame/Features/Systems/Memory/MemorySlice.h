#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Memory/MemoryActions.h"
#include "TestGame/Features/Systems/Memory/MemoryAdapters.h"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {

/** User Story: As a features systems memory consumer, I need to invoke create game memory slice through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::Slice<FGameMemorySliceState> CreateGameMemorySlice() */
inline rtk::Slice<FGameMemorySliceState> CreateGameMemorySlice() {
  return rtk::createSlice<FGameMemorySliceState>(
      TEXT("testgame/memory"), FGameMemorySliceState(),
      [](rtk::ActionReducerMapBuilder<FGameMemorySliceState> &Builder) {
        Builder.addCase(
            GameMemoryActions::storeMemoryActionCreator(),
            [](const FGameMemorySliceState &S,
               const rtk::Action<FMemoryRecord> &A) -> FGameMemorySliceState {
              FGameMemorySliceState Next = S;
              Next.Entities =
                  GetGameMemoryAdapter().addOne(Next.Entities, A.PayloadValue);
              return Next;
            });
        Builder.addCase(
            GameMemoryActions::clearMemoryForNpcActionCreator(),
            [](const FGameMemorySliceState &S,
               const rtk::Action<FString> &A) -> FGameMemorySliceState {
              FGameMemorySliceState Next = S;
              struct CollectIds {
                static void apply(const rtk::EntityState<FMemoryRecord> &E,
                                  const FString &NpcId, TArray<FString> &Out,
                                  int32 Idx) {
                  Idx >= E.ids.Num()
                      ? void()
                      : ((E.entities.Find(E.ids[Idx]) != nullptr &&
                          E.entities.Find(E.ids[Idx])->NpcId == NpcId)
                             ? (Out.Add(E.ids[Idx]), void())
                             : void(),
                         apply(E, NpcId, Out, Idx + 1), void());
                }
              };
              TArray<FString> ToRemove;
              CollectIds::apply(Next.Entities, A.PayloadValue, ToRemove, 0);
              Next.Entities =
                  GetGameMemoryAdapter().removeMany(Next.Entities, ToRemove);
              return Next;
            });
      });
}

} // namespace TestGame
