#pragma once

// Imperative local soul workflows live at this thunk boundary.

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Soul/SoulSlice.h"

namespace rtk {

inline ThunkAction<FSoul, FRuntimeState>
localExportSoulThunk(const FString &NpcId = TEXT("")) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoul> {
    const FString TargetNpcId =
        NpcId.IsEmpty() ? NPCSelectors::selectActiveNpcId(GetState().NPCs) : NpcId;
    const auto Npc = NPCSelectors::selectNPCById(GetState().NPCs, TargetNpcId);
    return !Npc.hasValue
        ? detail::RejectAsync<FSoul>(TEXT("NPC not found"))
        : detail::ResolveAsync(TypeFactory::Soul(
              TargetNpcId, TEXT("1.0.0"), TEXT("NPC"), Npc.value.Persona,
              Npc.value.State,
              MemorySlice::selectAllMemories(GetState().Memory)));
  };
}

inline ThunkAction<FSoul, FRuntimeState>
localImportSoulThunk(const FSoul &Soul) {
  return [Soul](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FSoul> {
    return Soul.Id.IsEmpty()
        ? detail::RejectAsync<FSoul>(TEXT("Soul ID is required"))
        : [&]() -> func::AsyncResult<FSoul> {
            FNPCInternalState Npc;
            Npc.Id = Soul.Id;
            Npc.Persona = Soul.Persona;
            Npc.State = Soul.State;
            Dispatch(NPCActions::setNPCInfo(Npc));
            Dispatch(SoulSlice::Actions::importSoulSuccess(Soul));
            return detail::ResolveAsync(Soul);
          }();
  };
}

} // namespace rtk
