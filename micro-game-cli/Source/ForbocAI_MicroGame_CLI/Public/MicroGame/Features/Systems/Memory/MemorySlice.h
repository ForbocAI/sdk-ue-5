#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Memory/MemoryActions.h"
#include "MicroGame/Features/Systems/Memory/MemoryAdapters.h"
#include "MicroGame/Features/Systems/Memory/MemoryTypes.h"

namespace MicroGame {

/** User Story: As a features systems memory consumer, I need to invoke create game memory slice through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::Slice<FGameMemorySliceState> CreateGameMemorySlice() */
inline rtk::Slice<FGameMemorySliceState> CreateGameMemorySlice() {
  return rtk::createSlice<FGameMemorySliceState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV98294ABF7F80), FGameMemorySliceState(),
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
              const TArray<FString> ToRemove = func::filter_array<FString>(
                  Next.Entities.ids, [&Next, &A](const FString &Id) {
                    return func::match(
                        func::find_map_value<FString, FMemoryRecord>(
                            Next.Entities.entities, Id),
                        [&A](const FMemoryRecord &Record) {
                          return Record.NpcId == A.PayloadValue;
                        },
                        []() { return false; });
                  });
              Next.Entities =
                  GetGameMemoryAdapter().removeMany(Next.Entities, ToRemove);
              return Next;
            });
      });
}

} // namespace MicroGame
