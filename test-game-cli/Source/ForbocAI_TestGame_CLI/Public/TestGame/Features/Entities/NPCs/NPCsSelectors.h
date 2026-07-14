#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsAdapters.h"

namespace TestGame {
namespace NPCsSelectors {

inline TArray<FGameNPC> SelectAllNpcs(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectAll(State.Entities);
}

inline func::Maybe<FGameNPC> SelectNpcById(const FNPCsSliceState &State,
                                           const FString &Id) {
  return GetNPCAdapter().getSelectors().selectById(State.Entities, Id);
}

inline rtk::EntityState<FGameNPC>
SelectNpcEntities(const FNPCsSliceState &State) {
  return State.Entities;
}

inline TArray<FString> SelectNpcIds(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectIds(State.Entities);
}

inline int32 SelectNpcTotal(const FNPCsSliceState &State) {
  return GetNPCAdapter().getSelectors().selectTotal(State.Entities);
}

} // namespace NPCsSelectors
} // namespace TestGame
