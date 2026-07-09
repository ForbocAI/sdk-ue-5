#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/Features/Systems/Memory/MemoryActions.h"
#include "TestGame/Features/Systems/Memory/MemoryAdapters.h"

namespace TestGame {

struct FGameMemorySliceState {
  rtk::EntityState<FMemoryRecord> Entities;

  FGameMemorySliceState()
      : Entities(GetGameMemoryAdapter().getInitialState()) {}

  bool operator==(const FGameMemorySliceState &O) const {
    return Entities.ids == O.Entities.ids;
  }
};

namespace GameMemorySelectors {
inline TArray<FMemoryRecord>
SelectAllMemories(const FGameMemorySliceState &S) {
  return GetGameMemoryAdapter().getSelectors().selectAll(S.Entities);
}

inline func::Maybe<FMemoryRecord>
SelectMemoryById(const FGameMemorySliceState &S, const FString &Id) {
  return GetGameMemoryAdapter().getSelectors().selectById(S.Entities, Id);
}

inline rtk::EntityState<FMemoryRecord>
SelectMemoryEntities(const FGameMemorySliceState &S) {
  return S.Entities;
}

inline TArray<FString> SelectMemoryIds(const FGameMemorySliceState &S) {
  return GetGameMemoryAdapter().getSelectors().selectIds(S.Entities);
}

inline int32 SelectMemoryTotal(const FGameMemorySliceState &S) {
  return GetGameMemoryAdapter().getSelectors().selectTotal(S.Entities);
}

inline TArray<FMemoryRecord>
SelectMemoriesByNpcId(const FGameMemorySliceState &S, const FString &NpcId) {
  struct CollectByNpc {
    static void apply(const TArray<FMemoryRecord> &Records,
                      const FString &TargetNpcId, TArray<FMemoryRecord> &Out,
                      int32 Index) {
      Index >= Records.Num()
          ? void()
          : (Records[Index].NpcId == TargetNpcId
                 ? (Out.Add(Records[Index]), void())
                 : void(),
             apply(Records, TargetNpcId, Out, Index + 1), void());
    }
  };
  TArray<FMemoryRecord> Records =
      GetGameMemoryAdapter().getSelectors().selectAll(S.Entities);
  TArray<FMemoryRecord> Results;
  CollectByNpc::apply(Records, NpcId, Results, 0);
  return Results;
}
} // namespace GameMemorySelectors

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
