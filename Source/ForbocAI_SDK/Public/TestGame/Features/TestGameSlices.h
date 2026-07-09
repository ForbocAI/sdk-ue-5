#pragma once
/**
 * Test-game slice definitions — mirrors TS test-game feature slices
 * 13 slices across 5 domains: entities, mechanics, store, terminal, autoplay
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/Features/TestGameTypes.h"
#include "TestGame/Features/TestGameActions.h"
#include "TestGame/Features/TestGameAdapters.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicTestGameTestGameSlicesHOptionalDomainId;
} } }

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

/**
 * Builds the NPC slice for the test game.
 * User Story: As test-game store setup, I need an NPC slice factory so entity
 * actions update normalized NPC state predictably.
 */
inline rtk::Slice<FNPCsSliceState> CreateNPCsSlice() {
  return rtk::createSlice<FNPCsSliceState>(
  TEXT("testgame/npcs"),
                                         FNPCsSliceState(),
  [](rtk::ActionReducerMapBuilder<FNPCsSliceState> &Builder) {
    Builder.addCase(NPCsActions::UpsertNPCActionCreator(),
      [](const FNPCsSliceState &S,
                   const rtk::Action<FGameNPC> &A) -> FNPCsSliceState {
                  FNPCsSliceState Next = S;
                  Next.Entities =
                      GetNPCAdapter().upsertOne(Next.Entities, A.PayloadValue);
                  return Next;
                });
    Builder.addCase(NPCsActions::MoveNPCActionCreator(),
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
    Builder.addCase(NPCsActions::PatchNPCActionCreator(),
      [](const FNPCsSliceState &S,
                   const rtk::Action<NPCsActions::FPatchNPCPayload> &A)
                    -> FNPCsSliceState {
                  FNPCsSliceState Next = S;
                  Next.Entities = GetNPCAdapter().updateOne(
                      Next.Entities, A.PayloadValue.Id,
                      [&A](const FGameNPC &Existing) {
                        FGameNPC Updated = Existing;
                        Updated.Name = A.PayloadValue.Patch.bHasName
                                           ? A.PayloadValue.Patch.Name
                                           : Updated.Name;
                        Updated.Faction = A.PayloadValue.Patch.bHasFaction
                                              ? A.PayloadValue.Patch.Faction
                                              : Updated.Faction;
                        Updated.Hp = A.PayloadValue.Patch.bHasHp
                                         ? A.PayloadValue.Patch.Hp
                                         : Updated.Hp;
                        Updated.Suspicion = A.PayloadValue.Patch.bHasSuspicion
                            ? A.PayloadValue.Patch.Suspicion
                            : Updated.Suspicion;
                        Updated.Inventory = A.PayloadValue.Patch.bHasInventory
                                                ? A.PayloadValue.Patch.Inventory
                                                : Updated.Inventory;
                        Updated.KnownSecrets = A.PayloadValue.Patch.bHasKnownSecrets
                                                   ? A.PayloadValue.Patch.KnownSecrets
                                                   : Updated.KnownSecrets;
                        Updated.Position = A.PayloadValue.Patch.bHasPosition
                                               ? A.PayloadValue.Patch.Position
                                               : Updated.Position;
                        return Updated;
                      });
                  return Next;
                });
    Builder.addCase(NPCsActions::ApplyNpcVerdictActionCreator(),
      [](const FNPCsSliceState &S,
                   const rtk::Action<NPCsActions::FApplyNpcVerdictPayload> &A)
                    -> FNPCsSliceState {
                  FNPCsSliceState Next = S;
                  const auto &P = A.PayloadValue;
                  Next.Entities = GetNPCAdapter().updateOne(
                      Next.Entities, P.Id, [&P](const FGameNPC &Existing) {
                        FGameNPC Updated = Existing;
                        Updated.Name = P.StateDelta.bHasName
                                           ? P.StateDelta.Name
                                           : Updated.Name;
                        Updated.Faction = P.StateDelta.bHasFaction
                                              ? P.StateDelta.Faction
                                              : Updated.Faction;
                        Updated.Hp = P.StateDelta.bHasHp
                                         ? P.StateDelta.Hp
                                         : Updated.Hp;
                        Updated.Suspicion = P.StateDelta.bHasSuspicion
                                                ? P.StateDelta.Suspicion
                                                : Updated.Suspicion;
                        Updated.Inventory = P.StateDelta.bHasInventory
                                                ? P.StateDelta.Inventory
                                                : Updated.Inventory;
                        Updated.KnownSecrets = P.StateDelta.bHasKnownSecrets
                                                   ? P.StateDelta.KnownSecrets
                                                   : Updated.KnownSecrets;
                        Updated.Position =
                            P.Action.Type == TEXT("MOVE") && P.Action.bHasTargetHex
                                ? P.Action.TargetHex
                                : (P.StateDelta.bHasPosition
                                       ? P.StateDelta.Position
                                       : Updated.Position);
                        return Updated;
                      });
                  return Next;
                });
  });
}

/**
 * Returns the cached action creator for player position updates.
 * User Story: As player movement state, I need a reusable action creator so
 * the player position can be updated through the store.
 */
inline rtk::ActionCreator<FPosition> SetPositionActionCreator() {
  static auto C =
      rtk::createAction<FPosition>(TEXT("testgame/player/setPosition"));
  return C;
}

/**
 * Returns the cached action creator for player visibility changes.
 * User Story: As stealth state updates, I need a reusable action creator so
 * hidden status can be toggled consistently.
 */
inline rtk::ActionCreator<bool> SetHiddenActionCreator() {
  static auto C = rtk::createAction<bool>(TEXT("testgame/player/setHidden"));
  return C;
}

struct FPatchPlayerPayload {
  FString Name;
  bool bHasName;
  int32 Hp;
  bool bHasHp;
  bool bHidden;
  bool bHasHidden;
  FPosition Position;
  bool bHasPosition;
  TArray<FString> Inventory;
  bool bHasInventory;
  FPatchPlayerPayload()
      : bHasName(false), Hp(0), bHasHp(false), bHidden(false),
        bHasHidden(false), bHasPosition(false), bHasInventory(false) {}
};

/**
 * Returns the cached action creator for targeted player updates.
 * User Story: As RTK-style player reducers, I need explicit field patch payloads
 * so the reducer owns the state shape instead of accepting arbitrary merges.
 */
inline rtk::ActionCreator<FPatchPlayerPayload> PatchPlayerActionCreator() {
  static auto C =
      rtk::createAction<FPatchPlayerPayload>(TEXT("testgame/player/patchPlayer"));
  return C;
}

namespace PlayerSelectors {
inline FString SelectPlayerName(const FPlayerState &S) { return S.Name; }
inline int32 SelectPlayerHp(const FPlayerState &S) { return S.Hp; }
inline bool SelectPlayerHidden(const FPlayerState &S) { return S.bHidden; }
inline FPosition SelectPlayerPosition(const FPlayerState &S) {
  return S.Position;
}
inline TArray<FString> SelectPlayerInventory(const FPlayerState &S) {
  return S.Inventory;
}
} // namespace PlayerSelectors

/**
 * Builds the player slice for the test game.
 * User Story: As test-game store setup, I need a player slice factory so
 * movement and hidden-state actions update one canonical player state.
 */
inline rtk::Slice<FPlayerState> CreatePlayerSlice() {
  return rtk::createSlice<FPlayerState>(
  TEXT("testgame/player"), FPlayerState(),
  [](rtk::ActionReducerMapBuilder<FPlayerState> &Builder) {
    Builder.addCase(SetPositionActionCreator(),
      [](const FPlayerState &S,
                                   const rtk::Action<FPosition> &A)
                                    -> FPlayerState {
                                  FPlayerState Next = S;
                                  Next.Position = A.PayloadValue;
                                  return Next;
                                });
    Builder.addCase(SetHiddenActionCreator(),
      [](const FPlayerState &S, const rtk::Action<bool> &A)
                                    -> FPlayerState {
                                  FPlayerState Next = S;
                                  Next.bHidden = A.PayloadValue;
                                  return Next;
                                });
    Builder.addCase(PatchPlayerActionCreator(),
      [](const FPlayerState &S,
                                   const rtk::Action<FPatchPlayerPayload> &A)
                                    -> FPlayerState {
                                  FPlayerState Next = S;
                                  Next.Name = A.PayloadValue.bHasName
                                                  ? A.PayloadValue.Name
                                                  : Next.Name;
                                  Next.Hp = A.PayloadValue.bHasHp
                                                ? A.PayloadValue.Hp
                                                : Next.Hp;
                                  Next.bHidden = A.PayloadValue.bHasHidden
                                                     ? A.PayloadValue.bHidden
                                                     : Next.bHidden;
                                  Next.Position = A.PayloadValue.bHasPosition
                                                      ? A.PayloadValue.Position
                                                      : Next.Position;
                                  Next.Inventory = A.PayloadValue.bHasInventory
                                                       ? A.PayloadValue.Inventory
                                                       : Next.Inventory;
                                  return Next;
                                });
  });
}

/**
 * --- Grid Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

struct FSetGridSizePayload {
  int32 Width;
  int32 Height;
};

namespace GridSelectors {
inline int32 SelectGridWidth(const FGridState &S) { return S.Width; }
inline int32 SelectGridHeight(const FGridState &S) { return S.Height; }
inline TArray<FPosition> SelectGridBlocked(const FGridState &S) {
  return S.Blocked;
}
} // namespace GridSelectors

/**
 * Builds the grid slice for the test game.
 * User Story: As test-game store setup, I need a grid slice factory so map
 * dimensions and blocked tiles are managed by one reducer.
 */
inline rtk::Slice<FGridState> CreateGridSlice() {
  return rtk::createSlice<FGridState>(
  TEXT("testgame/grid"), FGridState(),
  [](rtk::ActionReducerMapBuilder<FGridState> &Builder) {
    Builder.addCase(GridActions::SetGridSizeActionCreator(),
      [](const FGridState &S,
                   const rtk::Action<FSetGridSizePayload> &A) -> FGridState {
                  FGridState Next = S;
                  Next.Width = A.PayloadValue.Width;
                  Next.Height = A.PayloadValue.Height;
                  return Next;
                });
    Builder.addCase(GridActions::SetBlockedActionCreator(),
      [](const FGridState &S,
                   const rtk::Action<TArray<FPosition>> &A) -> FGridState {
                  FGridState Next = S;
                  Next.Blocked = A.PayloadValue;
                  return Next;
                });
  });
}

/**
 * --- Stealth Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace StealthSelectors {
inline bool SelectStealthDoorOpen(const FStealthState &S) {
  return S.bDoorOpen;
}
inline int32 SelectStealthAlertLevel(const FStealthState &S) {
  return S.AlertLevel;
}
} // namespace StealthSelectors

/**
 * Builds the stealth slice for the test game.
 * User Story: As test-game store setup, I need a stealth slice factory so door
 * and alert changes flow through one state reducer.
 */
inline rtk::Slice<FStealthState> CreateStealthSlice() {
  return rtk::createSlice<FStealthState>(
  TEXT("testgame/stealth"),
                                       FStealthState(),
  [](rtk::ActionReducerMapBuilder<FStealthState> &Builder) {
    Builder.addCase(StealthActions::SetDoorOpenActionCreator(),
      [](const FStealthState &S,
                                   const rtk::Action<bool> &A)
                                    -> FStealthState {
                                  FStealthState Next = S;
                                  Next.bDoorOpen = A.PayloadValue;
                                  return Next;
                                });
    Builder.addCase(StealthActions::BumpAlertActionCreator(),
      [](const FStealthState &S,
                                   const rtk::Action<int32> &A)
                                    -> FStealthState {
                                  FStealthState Next = S;
                                  int32 Raw = Next.AlertLevel + A.PayloadValue;
                                  Next.AlertLevel =
                                      FMath::Clamp(Raw, 0, 100);
                                  return Next;
                                });
  });
}

/**
 * --- Social Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace SocialSelectors {
inline FString SelectSocialActiveDialogue(const FSocialState &S) {
  return S.ActiveDialogue;
}
inline func::Maybe<FTradeOffer> SelectSocialActiveTrade(const FSocialState &S) {
  return S.bHasActiveTrade ? func::just<FTradeOffer>(S.ActiveTrade)
                           : func::nothing<FTradeOffer>();
}
} // namespace SocialSelectors

/**
 * Builds the social slice for the test game.
 * User Story: As test-game store setup, I need a social slice factory so
 * dialogue and trade actions update one canonical social state.
 */
inline rtk::Slice<FSocialState> CreateSocialSlice() {
  return rtk::createSlice<FSocialState>(
  TEXT("testgame/social"), FSocialState(),
  [](rtk::ActionReducerMapBuilder<FSocialState> &Builder) {
    Builder.addCase(SocialActions::SetDialogueActionCreator(),
      [](const FSocialState &S,
                                   const rtk::Action<FString> &A)
                                    -> FSocialState {
                                  FSocialState Next = S;
                                  Next.ActiveDialogue = A.PayloadValue;
                                  return Next;
                                });
    Builder.addCase(SocialActions::SetTradeOfferActionCreator(),
      [](const FSocialState &S,
                                   const rtk::Action<FTradeOffer> &A)
                                    -> FSocialState {
                                  FSocialState Next = S;
                                  Next.ActiveTrade = A.PayloadValue;
                                  Next.bHasActiveTrade = true;
                                  return Next;
                                });
    Builder.addCase(SocialActions::ClearSocialStateActionCreator(),
      [](const FSocialState &S,
                                   const rtk::Action<rtk::FEmptyPayload> &)
                                    -> FSocialState { return FSocialState(); });
  });
}

/**
 * --- Bridge Slice (game-specific, not SDK BridgeSlice) ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace GameBridgeSelectors {
inline int32 SelectBridgeMaxJumpForce(const FBridgeRulesState &S) {
  return S.MaxJumpForce;
}
inline int32 SelectBridgeMaxMoveDistance(const FBridgeRulesState &S) {
  return S.MaxMoveDistance;
}
inline FString SelectBridgeActivePreset(const FBridgeRulesState &S) {
  return S.ActivePreset;
}
} // namespace GameBridgeSelectors

/**
 * Builds the game-specific bridge slice.
 * User Story: As test-game store setup, I need a bridge slice factory so local
 * bridge presets and rules are reduced in one place.
 */
inline rtk::Slice<FBridgeRulesState> CreateGameBridgeSlice() {
  return rtk::createSlice<FBridgeRulesState>(
  TEXT("testgame/bridge"),
                                           FBridgeRulesState(),
  [](rtk::ActionReducerMapBuilder<FBridgeRulesState> &Builder) {
    Builder.addCase(GameBridgeActions::SetBridgeRulesActionCreator(),
      [](const FBridgeRulesState &S,
                   const rtk::Action<GameBridgeActions::FSetBridgeRulesPayload> &A)
                      -> FBridgeRulesState {
                  FBridgeRulesState Next = S;
                  Next.MaxJumpForce = A.PayloadValue.bHasMaxJumpForce
                                          ? A.PayloadValue.MaxJumpForce
                                          : Next.MaxJumpForce;
                  Next.MaxMoveDistance = A.PayloadValue.bHasMaxMoveDistance
                                             ? A.PayloadValue.MaxMoveDistance
                                             : Next.MaxMoveDistance;
                  Next.ActivePreset = A.PayloadValue.bHasActivePreset
                                          ? A.PayloadValue.ActivePreset
                                          : Next.ActivePreset;
                  return Next;
                });
    Builder.addCase(GameBridgeActions::LoadBridgePresetActionCreator(),
      [](const FBridgeRulesState &S,
                   const rtk::Action<FString> &A) -> FBridgeRulesState {
                  FBridgeRulesState Next = S;
                  Next.ActivePreset = A.PayloadValue;
                  Next.MaxMoveDistance =
                      A.PayloadValue == TEXT("social") ? 1
                      : A.PayloadValue == TEXT("default") ? 2
                      : Next.MaxMoveDistance;
                  return Next;
                });
  });
}

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
    static void apply(
        const TArray<FMemoryRecord> &Records,
        const FString &TargetNpcId,
        TArray<FMemoryRecord> &Out,
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

/**
 * Builds the game-specific memory slice.
 * User Story: As test-game store setup, I need a memory slice factory so local
 * memory records can be stored and cleared through one reducer.
 */
inline rtk::Slice<FGameMemorySliceState> CreateGameMemorySlice() {
  return rtk::createSlice<FGameMemorySliceState>(
  TEXT("testgame/memory"),
                                               FGameMemorySliceState(),
  [](rtk::ActionReducerMapBuilder<FGameMemorySliceState> &Builder) {
    Builder.addCase(GameMemoryActions::StoreMemoryActionCreator(),
      [](const FGameMemorySliceState &S,
                   const rtk::Action<FMemoryRecord> &A) -> FGameMemorySliceState {
                  FGameMemorySliceState Next = S;
                  Next.Entities =
                      GetGameMemoryAdapter().addOne(Next.Entities, A.PayloadValue);
                  return Next;
                });
    Builder.addCase(GameMemoryActions::ClearMemoryForNpcActionCreator(),
      [](const FGameMemorySliceState &S,
                   const rtk::Action<FString> &A) -> FGameMemorySliceState {
                  FGameMemorySliceState Next = S;
                  struct CollectIds {
                    static void apply(
                        const rtk::EntityState<FMemoryRecord> &E,
                        const FString &NpcId,
                        TArray<FString> &Out,
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

/**
 * --- Inventory Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

struct FSetOwnerInventoryPayload {
  FString OwnerId;
  TArray<FString> Items;
};

namespace InventorySelectors {
inline TMap<FString, TArray<FString>>
SelectInventoryByOwner(const FInventoryState &S) {
  return S.ByOwner;
}
inline TArray<FString> SelectOwnerInventory(const FInventoryState &S,
                                            const FString &OwnerId) {
  const TArray<FString> *Items = S.ByOwner.Find(OwnerId);
  return Items == nullptr ? TArray<FString>() : *Items;
}
} // namespace InventorySelectors

/**
 * Builds the inventory slice for the test game.
 * User Story: As test-game store setup, I need an inventory slice factory so
 * owner item lists are reduced in one place.
 */
inline rtk::Slice<FInventoryState> CreateInventorySlice() {
  return rtk::createSlice<FInventoryState>(
  TEXT("testgame/inventory"),
                                         FInventoryState(),
  [](rtk::ActionReducerMapBuilder<FInventoryState> &Builder) {
    Builder.addCase(InventoryActions::SetOwnerInventoryActionCreator(),
      [](const FInventoryState &S,
                   const rtk::Action<FSetOwnerInventoryPayload> &A)
                    -> FInventoryState {
                  FInventoryState Next = S;
                  Next.ByOwner.Add(A.PayloadValue.OwnerId, A.PayloadValue.Items);
                  return Next;
                });
  });
}

/**
 * --- Soul Tracking Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

struct FMarkSoulExportedPayload {
  FString NpcId;
  FString TxId;
};

namespace GameSoulSelectors {
inline TMap<FString, FString>
SelectSoulExportsByNpc(const FSoulTrackingState &S) {
  return S.ExportsByNpc;
}
inline TArray<FString> SelectImportedSoulTxIds(const FSoulTrackingState &S) {
  return S.ImportedSoulTxIds;
}
inline func::Maybe<FString> SelectSoulExportTxId(
    const FSoulTrackingState &S, const FString &NpcId) {
  const FString *TxId = S.ExportsByNpc.Find(NpcId);
  return TxId == nullptr ? func::nothing<FString>()
                         : func::just<FString>(*TxId);
}
} // namespace GameSoulSelectors

/**
 * Builds the game-specific soul tracking slice.
 * User Story: As test-game store setup, I need a soul slice factory so export
 * and import tracking live in one reducer.
 */
inline rtk::Slice<FSoulTrackingState> CreateGameSoulSlice() {
  return rtk::createSlice<FSoulTrackingState>(
  TEXT("testgame/soul"),
                                            FSoulTrackingState(),
  [](rtk::ActionReducerMapBuilder<FSoulTrackingState> &Builder) {
    Builder.addCase(GameSoulActions::MarkSoulExportedActionCreator(),
      [](const FSoulTrackingState &S,
                   const rtk::Action<FMarkSoulExportedPayload> &A)
                    -> FSoulTrackingState {
                  FSoulTrackingState Next = S;
                  Next.ExportsByNpc.Add(A.PayloadValue.NpcId, A.PayloadValue.TxId);
                  return Next;
                });
    Builder.addCase(GameSoulActions::MarkSoulImportedActionCreator(),
      [](const FSoulTrackingState &S,
                                   const rtk::Action<FString> &A)
                                    -> FSoulTrackingState {
                                  FSoulTrackingState Next = S;
                                  Next.ImportedSoulTxIds.Add(A.PayloadValue);
                                  return Next;
                                });
  });
}

/**
 * --- UI Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace UISelectors {
inline EPlayMode SelectUiMode(const FUIState &S) { return S.Mode; }
inline TArray<FString> SelectUiMessages(const FUIState &S) {
  return S.Messages;
}
} // namespace UISelectors

/**
 * Builds the UI slice for the test game.
 * User Story: As test-game store setup, I need a UI slice factory so mode and
 * message updates flow through one reducer.
 */
inline rtk::Slice<FUIState> CreateUISlice() {
  return rtk::createSlice<FUIState>(
  TEXT("testgame/ui"), FUIState(),
  [](rtk::ActionReducerMapBuilder<FUIState> &Builder) {
    Builder.addCase(UIActions::SetModeActionCreator(),
      [](const FUIState &S,
                                   const rtk::Action<EPlayMode> &A)
                                    -> FUIState {
                                  FUIState Next = S;
                                  Next.Mode = A.PayloadValue;
                                  return Next;
                                });
    Builder.addCase(UIActions::AddMessageActionCreator(),
      [](const FUIState &S,
                                   const rtk::Action<FString> &A) -> FUIState {
                                  FUIState Next = S;
                                  Next.Messages.Add(A.PayloadValue);
                                  return Next;
                                });
  });
}

/**
 * --- Transcript Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace TranscriptSelectors {
inline TArray<FTranscriptEntry>
SelectTranscriptEntries(const FTranscriptState &S) {
  return S.Entries;
}

inline int32 SelectTranscriptErrorCountRecursive(
    const TArray<FTranscriptEntry> &Entries, int32 Index) {
  return Index >= Entries.Num()
             ? 0
             : ((Entries[Index].Status == ETranscriptStatus::Error ? 1 : 0) +
                SelectTranscriptErrorCountRecursive(Entries, Index + 1));
}

inline int32 SelectTranscriptErrorCount(const FTranscriptState &S) {
  return SelectTranscriptErrorCountRecursive(S.Entries, 0);
}
} // namespace TranscriptSelectors

/**
 * Builds the transcript slice for the test game.
 * User Story: As test-game store setup, I need a transcript slice factory so
 * command history is recorded through one reducer.
 */
inline rtk::Slice<FTranscriptState> CreateTranscriptSlice() {
  return rtk::createSlice<FTranscriptState>(
  TEXT("testgame/transcript"),
                                          FTranscriptState(),
  [](rtk::ActionReducerMapBuilder<FTranscriptState> &Builder) {
    Builder.addCase(TranscriptActions::RecordTranscriptActionCreator(),
      [](const FTranscriptState &S,
                   const rtk::Action<TranscriptActions::FRecordTranscriptPayload> &A)
                    -> FTranscriptState {
                  FTranscriptState Next = S;
                  FTranscriptEntry E;
                  E.Id = FString::Printf(TEXT("%lld-%d"),
                                         FDateTime::Now().GetTicks(),
                                         FMath::Rand());
                  E.ScenarioId = A.PayloadValue.ScenarioId;
                  E.CommandGroup = A.PayloadValue.CommandGroup;
                  E.Command = A.PayloadValue.Command;
                  E.ExpectedRoutes = A.PayloadValue.ExpectedRoutes;
                  E.Status = A.PayloadValue.Status;
                  E.Output = A.PayloadValue.Output;
                  E.Timestamp = FDateTime::Now().ToIso8601();
                  Next.Entries.Add(E);
                  return Next;
                });
    Builder.addCase(TranscriptActions::ResetTranscriptActionCreator(),
      [](const FTranscriptState &S,
                   const rtk::Action<rtk::FEmptyPayload> &) -> FTranscriptState {
                  return FTranscriptState();
                });
  });
}

/**
 * --- Harness Slice ---
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

/**
 * Builds the harness slice for the test game.
 * User Story: As test-game store setup, I need a harness slice factory so CLI
 * coverage updates are reduced in one place.
 */
inline rtk::Slice<FHarnessState> CreateHarnessSlice() {
  return rtk::createSlice<FHarnessState>(
  TEXT("testgame/harness"),
                                       FHarnessState(),
  [](rtk::ActionReducerMapBuilder<FHarnessState> &Builder) {
    Builder.addCase(HarnessActions::MarkCoveredActionCreator(),
      [](const FHarnessState &S,
                                   const rtk::Action<ECommandGroup> &A)
                                    -> FHarnessState {
                                  FHarnessState Next = S;
                                  Next.Covered.Add(A.PayloadValue, true);
                                  return Next;
                                });
    Builder.addCase(HarnessActions::ResetCoverageActionCreator(),
      [](const FHarnessState &S,
                   const rtk::Action<rtk::FEmptyPayload> &) -> FHarnessState {
                  return FHarnessState();
                });
  });
}

/**
 * --- Scenario Slice (read-only static data) ---
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

struct FScenarioSliceState {
  TArray<FScenarioStep> Steps;
  bool operator==(const FScenarioSliceState &O) const {
    return Steps.Num() == O.Steps.Num();
  }
};

typedef FScenarioSliceState FScenarioState;

/**
 * Returns the default scenario step list for the test game.
 * User Story: As scenario slice bootstrapping, I need the canonical step list
 * so store initialization can seed the default scenario sequence.
 */

/**
 * Builds the read-only scenario slice for the test game.
 * User Story: As scenario bootstrapping, I need a slice factory that loads the
 * default scenario list into store state once.
 */
inline rtk::Slice<FScenarioSliceState> CreateScenarioSlice() {
  FScenarioSliceState Initial;
  Initial.Steps = {};
  return rtk::createSlice<FScenarioSliceState>(
  TEXT("testgame/scenario"),
                                             Initial,
  [](rtk::ActionReducerMapBuilder<FScenarioSliceState> &Builder) {
  });
}

namespace ScenarioSelectors {
inline TArray<FScenarioStep> SelectScenarioSteps(const FScenarioSliceState &S) {
  return S.Steps;
}
} // namespace ScenarioSelectors

/**
 * Returns the command groups that have not been covered yet.
 * User Story: As harness reporting, I need the missing group list so the final
 * run result can explain which CLI areas were not exercised.
 */
inline TArray<ECommandGroup>
SelectMissingGroups(const TMap<ECommandGroup, bool> &Covered, const TArray<ECommandGroup> &Groups) {
  struct CollectMissing {
    static void apply(
        const TMap<ECommandGroup, bool> &C,
        const TArray<ECommandGroup> &G,
        TArray<ECommandGroup> &Out,
        int32 Idx) {
      Idx >= G.Num()
          ? void()
          : ((!C.Contains(G[Idx]) || !(*C.Find(G[Idx])))
                 ? (Out.Add(G[Idx]), void())
                 : void(),
             apply(C, G, Out, Idx + 1), void());
    }
  };
  TArray<ECommandGroup> Missing;
  CollectMissing::apply(Covered, Groups, Missing, 0);
  return Missing;
}

namespace HarnessSelectors {
inline TMap<ECommandGroup, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}
inline TArray<ECommandGroup> SelectHarnessMissingGroups(
    const FHarnessState &S, const TArray<ECommandGroup> &Groups) {
  return SelectMissingGroups(S.Covered, Groups);
}
} // namespace HarnessSelectors

} // namespace TestGame
