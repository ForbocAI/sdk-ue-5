#pragma once
/**
 * Test-game slice definitions — mirrors TS test-game feature slices
 * 13 slices across 5 domains: entities, mechanics, store, terminal, autoplay
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/TestGameTypes.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicTestGameTestGameSlicesHOptionalDomainId;
} } }

namespace TestGame {

/**
 * Returns the entity adapter used for test-game NPC state.
 * User Story: As test-game entity reducers, I need one shared adapter so NPC
 * CRUD actions update and query a consistent normalized state shape.
 */
inline rtk::EntityAdapter<FGameNPC> &GetNPCAdapter() {
  static rtk::EntityAdapter<FGameNPC> Adapter =
      rtk::createEntityAdapter<FGameNPC>(
          [](const FGameNPC &N) { return N.Id; });
  return Adapter;
}

struct FNPCsSliceState {
  rtk::EntityState<FGameNPC> Entities;
  FNPCsSliceState() : Entities(GetNPCAdapter().getInitialState()) {}
  bool operator==(const FNPCsSliceState &O) const {
    return Entities.ids == O.Entities.ids;
  }
};

namespace NPCsActions {

/**
 * Returns the cached action creator for upserting NPCs.
 * User Story: As scenario setup and runtime updates, I need one action creator
 * so NPC records can be inserted or replaced consistently.
 */
inline rtk::ActionCreator<FGameNPC> UpsertNPCActionCreator() {
  static auto C = rtk::createAction<FGameNPC>(TEXT("testgame/npcs/upsertNPC"));
  return C;
}

struct FMoveNPCPayload {
  FString Id;
  FPosition Position;
};

/**
 * Returns the cached action creator for moving NPCs.
 * User Story: As verdict-driven movement, I need a reusable action creator so
 * NPC position updates follow one contract.
 */
inline rtk::ActionCreator<FMoveNPCPayload> MoveNPCActionCreator() {
  static auto C =
      rtk::createAction<FMoveNPCPayload>(TEXT("testgame/npcs/moveNPC"));
  return C;
}

struct FPatchNPCChanges {
  FString Name;
  bool bHasName;
  FString Faction;
  bool bHasFaction;
  int32 Hp;
  bool bHasHp;
  int32 Suspicion;
  bool bHasSuspicion;
  TArray<FString> Inventory;
  bool bHasInventory;
  TArray<FString> KnownSecrets;
  bool bHasKnownSecrets;
  FPosition Position;
  bool bHasPosition;
  FPatchNPCChanges()
      : bHasName(false), bHasFaction(false), Hp(0), bHasHp(false),
        Suspicion(0), bHasSuspicion(false), bHasInventory(false),
        bHasKnownSecrets(false), bHasPosition(false) {}
};

struct FPatchNPCPayload {
  FString Id;
  FPatchNPCChanges Patch;
};

/**
 * Returns the cached action creator for patching NPC state.
 * User Story: As scenario state tweaks, I need a reusable patch action creator
 * so targeted NPC fields can change without replacing the whole entity.
 */
inline rtk::ActionCreator<FPatchNPCPayload> PatchNPCActionCreator() {
  static auto C =
      rtk::createAction<FPatchNPCPayload>(TEXT("testgame/npcs/patchNPC"));
  return C;
}

struct FNpcVerdictAction {
  FString Type;
  FPosition TargetHex;
  bool bHasTargetHex;
  FNpcVerdictAction() : bHasTargetHex(false) {}
};

struct FApplyNpcVerdictPayload {
  FString Id;
  FNpcVerdictAction Action;
  FPatchNPCChanges StateDelta;
};

/**
 * Returns the cached action creator for applying parsed verdicts.
 * User Story: As CLI output replay, I need a reusable verdict action creator
 * so parsed actions can update NPC state through the store.
 */
inline rtk::ActionCreator<FApplyNpcVerdictPayload>
ApplyNpcVerdictActionCreator() {
  static auto C = rtk::createAction<FApplyNpcVerdictPayload>(
      TEXT("testgame/npcs/applyNpcVerdict"));
  return C;
}

/**
 * Creates an action that inserts or updates one NPC entity.
 * User Story: As scenario orchestration, I need NPC upserts so the test world
 * can seed and refresh actors during a run.
 */
inline rtk::AnyAction UpsertNPC(const FGameNPC &N) {
  return UpsertNPCActionCreator()(N);
}
/**
 * Creates an action that moves one NPC to a new position.
 * User Story: As verdict replay, I need movement actions so NPC positions can
 * track the last parsed command outcome.
 */
inline rtk::AnyAction MoveNPC(const FMoveNPCPayload &P) {
  return MoveNPCActionCreator()(P);
}
/**
 * Creates an action that patches selected NPC fields.
 * User Story: As scenario setup, I need targeted NPC patches so suspicion and
 * similar values can change without rewriting the whole record.
 */
inline rtk::AnyAction PatchNPC(const FPatchNPCPayload &P) {
  return PatchNPCActionCreator()(P);
}
/**
 * Creates an action that applies a parsed verdict to an NPC.
 * User Story: As CLI transcript replay, I need verdict actions so parsed
 * output can drive NPC movement and suspicion changes.
 */
inline rtk::AnyAction ApplyNpcVerdict(const FApplyNpcVerdictPayload &P) {
  return ApplyNpcVerdictActionCreator()(P);
}

} // namespace NPCsActions

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

namespace PlayerActions {
/**
 * Creates an action that moves the player.
 * User Story: As test-game player control, I need position actions so the
 * player avatar can be relocated by scenarios and commands.
 */
inline rtk::AnyAction SetPosition(const FPosition &P) {
  return SetPositionActionCreator()(P);
}
/**
 * Creates an action that updates player visibility.
 * User Story: As stealth scenarios, I need hidden-state actions so the player
 * can be shown or concealed in the game state.
 */
inline rtk::AnyAction SetHidden(bool H) {
  return SetHiddenActionCreator()(H);
}
/**
 * Creates an action that patches selected player fields.
 * User Story: As scenario setup, I need targeted player patches so optional
 * updates do not replace the whole player state shape.
 */
inline rtk::AnyAction PatchPlayer(const FPatchPlayerPayload &P) {
  return PatchPlayerActionCreator()(P);
}
} // namespace PlayerActions

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

namespace GridActions {

/**
 * Returns the cached action creator for grid size changes.
 * User Story: As grid setup, I need a reusable action creator so scenarios can
 * resize the map through the store.
 */
inline rtk::ActionCreator<FSetGridSizePayload> SetGridSizeActionCreator() {
  static auto C =
      rtk::createAction<FSetGridSizePayload>(TEXT("testgame/grid/setGridSize"));
  return C;
}

/**
 * Returns the cached action creator for blocked-cell updates.
 * User Story: As map obstacle setup, I need a reusable action creator so
 * blocked tiles can be replaced consistently.
 */
inline rtk::ActionCreator<TArray<FPosition>> SetBlockedActionCreator() {
  static auto C = rtk::createAction<TArray<FPosition>>(
      TEXT("testgame/grid/setBlocked"));
  return C;
}

/**
 * Creates an action that updates the grid dimensions.
 * User Story: As map initialization, I need grid-size actions so scenarios can
 * define the playable area before commands run.
 */
inline rtk::AnyAction SetGridSize(const FSetGridSizePayload &P) {
  return SetGridSizeActionCreator()(P);
}
/**
 * Creates an action that replaces the blocked tile set.
 * User Story: As obstacle initialization, I need blocked-cell actions so map
 * hazards and walls can be configured per scenario.
 */
inline rtk::AnyAction SetBlocked(const TArray<FPosition> &B) {
  return SetBlockedActionCreator()(B);
}

} // namespace GridActions

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

namespace StealthActions {

/**
 * Returns the cached action creator for door visibility state.
 * User Story: As stealth scenario setup, I need a reusable action creator so
 * door-open state can be toggled consistently.
 */
inline rtk::ActionCreator<bool> SetDoorOpenActionCreator() {
  static auto C =
      rtk::createAction<bool>(TEXT("testgame/stealth/setDoorOpen"));
  return C;
}

/**
 * Returns the cached action creator for alert-level deltas.
 * User Story: As stealth escalation, I need a reusable action creator so alert
 * adjustments are applied through one reducer path.
 */
inline rtk::ActionCreator<int32> BumpAlertActionCreator() {
  static auto C =
      rtk::createAction<int32>(TEXT("testgame/stealth/bumpAlert"));
  return C;
}

/**
 * Creates an action that toggles whether the door is open.
 * User Story: As stealth state setup, I need door actions so scenarios can
 * reflect whether an entry point is compromised.
 */
inline rtk::AnyAction SetDoorOpen(bool V) {
  return SetDoorOpenActionCreator()(V);
}
/**
 * Creates an action that adjusts the alert level.
 * User Story: As stealth escalation, I need alert actions so suspicious events
 * can raise or lower the current alert score.
 */
inline rtk::AnyAction BumpAlert(int32 Delta) {
  return BumpAlertActionCreator()(Delta);
}

} // namespace StealthActions

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

namespace SocialActions {

/**
 * Returns the cached action creator for active dialogue text.
 * User Story: As social encounter setup, I need a reusable action creator so
 * dialogue prompts can be injected into state consistently.
 */
inline rtk::ActionCreator<FString> SetDialogueActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/social/setDialogue"));
  return C;
}

/**
 * Returns the cached action creator for trade-offer updates.
 * User Story: As social trading flows, I need a reusable action creator so the
 * current offer can be stored with one contract.
 */
inline rtk::ActionCreator<FTradeOffer> SetTradeOfferActionCreator() {
  static auto C =
      rtk::createAction<FTradeOffer>(TEXT("testgame/social/setTradeOffer"));
  return C;
}

/**
 * Returns the cached action creator for clearing social state.
 * User Story: As social cleanup, I need a reusable clear action creator so
 * dialogue and trade state reset together.
 */
inline rtk::ActionCreatorWithoutPayload ClearSocialStateActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/social/clearSocialState"));
  return C;
}

/**
 * Creates an action that sets the active dialogue line.
 * User Story: As social encounter setup, I need dialogue actions so the UI can
 * render the current NPC line.
 */
inline rtk::AnyAction SetDialogue(const FString &D) {
  return SetDialogueActionCreator()(D);
}
/**
 * Creates an action that stores the active trade offer.
 * User Story: As trading interactions, I need trade-offer actions so the UI
 * can present the current merchant offer.
 */
inline rtk::AnyAction SetTradeOffer(const FTradeOffer &T) {
  return SetTradeOfferActionCreator()(T);
}
/**
 * Creates an action that clears social state.
 * User Story: As scenario resets, I need social state cleared so the next
 * interaction starts without leftover dialogue or trade data.
 */
inline rtk::AnyAction ClearSocialState() {
  return ClearSocialStateActionCreator()();
}

} // namespace SocialActions

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

namespace GameBridgeActions {

struct FSetBridgeRulesPayload {
  int32 MaxJumpForce;
  bool bHasMaxJumpForce;
  int32 MaxMoveDistance;
  bool bHasMaxMoveDistance;
  FString ActivePreset;
  bool bHasActivePreset;
  FSetBridgeRulesPayload()
      : MaxJumpForce(0), bHasMaxJumpForce(false), MaxMoveDistance(0),
        bHasMaxMoveDistance(false), bHasActivePreset(false) {}
};

/**
 * Returns the cached action creator for bridge-rule updates.
 * User Story: As game-specific bridge rules, I need a reusable action creator
 * so the current bridge rules can be swapped consistently.
 */
inline rtk::ActionCreator<FSetBridgeRulesPayload> SetBridgeRulesActionCreator() {
  static auto C = rtk::createAction<FSetBridgeRulesPayload>(
      TEXT("testgame/bridge/setBridgeRules"));
  return C;
}

/**
 * Returns the cached action creator for bridge preset selection.
 * User Story: As bridge-preset setup, I need a reusable action creator so the
 * chosen preset name can drive local rule defaults.
 */
inline rtk::ActionCreator<FString> LoadBridgePresetActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/bridge/loadBridgePreset"));
  return C;
}

/**
 * Creates an action that replaces the active bridge rules.
 * User Story: As bridge-rule setup, I need set-rule actions so scenarios can
 * apply a complete local ruleset in one dispatch.
 */
inline rtk::AnyAction SetBridgeRules(const FSetBridgeRulesPayload &P) {
  return SetBridgeRulesActionCreator()(P);
}

inline rtk::AnyAction SetBridgeRules(const FBridgeRulesState &R) {
  FSetBridgeRulesPayload Payload;
  Payload.MaxJumpForce = R.MaxJumpForce;
  Payload.bHasMaxJumpForce = true;
  Payload.MaxMoveDistance = R.MaxMoveDistance;
  Payload.bHasMaxMoveDistance = true;
  Payload.ActivePreset = R.ActivePreset;
  Payload.bHasActivePreset = true;
  return SetBridgeRules(Payload);
}
/**
 * Creates an action that loads a named bridge preset.
 * User Story: As preset switching, I need preset actions so local rules can
 * follow the currently selected bridge profile.
 */
inline rtk::AnyAction LoadBridgePreset(const FString &P) {
  return LoadBridgePresetActionCreator()(P);
}

} // namespace GameBridgeActions

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

/**
 * Returns the entity adapter used for game-side memory records.
 * User Story: As test-game memory reducers, I need one shared adapter so local
 * memory records use a consistent normalized state structure.
 */
inline rtk::EntityAdapter<FMemoryRecord> &GetGameMemoryAdapter() {
  static rtk::EntityAdapter<FMemoryRecord> Adapter =
      rtk::createEntityAdapter<FMemoryRecord>(
          [](const FMemoryRecord &R) { return R.Id; });
  return Adapter;
}

struct FGameMemorySliceState {
  rtk::EntityState<FMemoryRecord> Entities;
  FGameMemorySliceState()
      : Entities(GetGameMemoryAdapter().getInitialState()) {}
  bool operator==(const FGameMemorySliceState &O) const {
    return Entities.ids == O.Entities.ids;
  }
};

namespace GameMemoryActions {

/**
 * Returns the cached action creator for storing game memory records.
 * User Story: As game-side memory capture, I need a reusable action creator so
 * observations can be persisted into the local memory slice.
 */
inline rtk::ActionCreator<FMemoryRecord> StoreMemoryActionCreator() {
  static auto C =
      rtk::createAction<FMemoryRecord>(TEXT("testgame/memory/storeMemory"));
  return C;
}

/**
 * Returns the cached action creator for clearing one NPC's memories.
 * User Story: As soul import and cleanup flows, I need a reusable action
 * creator so one NPC's local memories can be purged consistently.
 */
inline rtk::ActionCreator<FString> ClearMemoryForNpcActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/memory/clearMemoryForNpc"));
  return C;
}

/**
 * Creates an action that stores one game memory record.
 * User Story: As scenario initialization, I need memory-store actions so
 * NPC-specific observations can be seeded into local state.
 */
inline rtk::AnyAction StoreMemory(const FMemoryRecord &R) {
  return StoreMemoryActionCreator()(R);
}
/**
 * Creates an action that clears local memories for one NPC.
 * User Story: As cleanup flows, I need clear-memory actions so stale local
 * memories are removed when a scenario resets or imports a soul.
 */
inline rtk::AnyAction ClearMemoryForNpc(const FString &NpcId) {
  return ClearMemoryForNpcActionCreator()(NpcId);
}

} // namespace GameMemoryActions

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

namespace InventoryActions {

/**
 * Returns the cached action creator for owner-inventory replacement.
 * User Story: As inventory synchronization, I need a reusable action creator
 * so each owner's item list can be updated through one contract.
 */
inline rtk::ActionCreator<FSetOwnerInventoryPayload>
SetOwnerInventoryActionCreator() {
  static auto C = rtk::createAction<FSetOwnerInventoryPayload>(
      TEXT("testgame/inventory/setOwnerInventory"));
  return C;
}

/**
 * Creates an action that replaces one owner's inventory.
 * User Story: As inventory state setup, I need owner-inventory actions so test
 * scenarios can assign item lists to specific actors.
 */
inline rtk::AnyAction SetOwnerInventory(const FSetOwnerInventoryPayload &P) {
  return SetOwnerInventoryActionCreator()(P);
}

} // namespace InventoryActions

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

namespace GameSoulActions {

/**
 * Returns the cached action creator for soul export tracking.
 * User Story: As soul-tracking state, I need a reusable action creator so NPC
 * export records can be captured with one contract.
 */
inline rtk::ActionCreator<FMarkSoulExportedPayload>
MarkSoulExportedActionCreator() {
  static auto C = rtk::createAction<FMarkSoulExportedPayload>(
      TEXT("testgame/soul/markSoulExported"));
  return C;
}

/**
 * Returns the cached action creator for soul import tracking.
 * User Story: As soul-tracking state, I need a reusable action creator so
 * imported soul transaction ids can be recorded consistently.
 */
inline rtk::ActionCreator<FString> MarkSoulImportedActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/soul/markSoulImported"));
  return C;
}

/**
 * Creates an action that records a soul export for one NPC.
 * User Story: As persistence tracking, I need export actions so the test game
 * remembers which NPC soul was exported to which transaction id.
 */
inline rtk::AnyAction MarkSoulExported(const FMarkSoulExportedPayload &P) {
  return MarkSoulExportedActionCreator()(P);
}
/**
 * Creates an action that records an imported soul transaction id.
 * User Story: As persistence tracking, I need import actions so the game state
 * can remember which souls were brought back in.
 */
inline rtk::AnyAction MarkSoulImported(const FString &TxId) {
  return MarkSoulImportedActionCreator()(TxId);
}

} // namespace GameSoulActions

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

namespace UIActions {

/**
 * Returns the cached action creator for play-mode changes.
 * User Story: As terminal mode control, I need a reusable action creator so
 * manual and autoplay mode changes use one contract.
 */
inline rtk::ActionCreator<EPlayMode> SetModeActionCreator() {
  static auto C =
      rtk::createAction<EPlayMode>(TEXT("testgame/ui/setMode"));
  return C;
}

/**
 * Returns the cached action creator for UI messages.
 * User Story: As terminal feedback, I need a reusable action creator so
 * informational messages can be appended to the UI state consistently.
 */
inline rtk::ActionCreator<FString> AddMessageActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/ui/addMessage"));
  return C;
}

/**
 * Creates an action that sets the play mode.
 * User Story: As test-game control flows, I need mode actions so the UI can
 * switch between manual and autoplay execution.
 */
inline rtk::AnyAction SetMode(EPlayMode M) {
  return SetModeActionCreator()(M);
}
/**
 * Creates an action that appends a UI message.
 * User Story: As user feedback, I need message actions so the terminal can
 * surface warnings and status updates during a run.
 */
inline rtk::AnyAction AddMessage(const FString &Msg) {
  return AddMessageActionCreator()(Msg);
}

} // namespace UIActions

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

namespace TranscriptActions {

struct FRecordTranscriptPayload {
  FString ScenarioId;
  ECommandGroup CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  ETranscriptStatus Status;
  FString Output;
};

/**
 * Returns the cached action creator for transcript entries.
 * User Story: As transcript capture, I need a reusable action creator so each
 * executed command can be stored with its outcome.
 */
inline rtk::ActionCreator<FRecordTranscriptPayload>
RecordTranscriptActionCreator() {
  static auto C = rtk::createAction<FRecordTranscriptPayload>(
      TEXT("testgame/transcript/recordTranscript"));
  return C;
}

/**
 * Returns the cached action creator for transcript resets.
 * User Story: As transcript cleanup, I need a reusable clear action creator so
 * a new run can start with an empty transcript.
 */
inline rtk::ActionCreatorWithoutPayload ResetTranscriptActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/transcript/resetTranscript"));
  return C;
}

/**
 * Creates an action that records one transcript entry.
 * User Story: As command auditing, I need transcript actions so every command
 * and output pair is captured in the store.
 */
inline rtk::AnyAction RecordTranscript(const FRecordTranscriptPayload &P) {
  return RecordTranscriptActionCreator()(P);
}
/**
 * Creates an action that clears transcript history.
 * User Story: As run resets, I need transcript reset actions so old command
 * history does not leak into the next session.
 */
inline rtk::AnyAction ResetTranscript() {
  return ResetTranscriptActionCreator()();
}

} // namespace TranscriptActions

namespace TranscriptSelectors {
inline TArray<FTranscriptEntry>
SelectTranscriptEntries(const FTranscriptState &S) {
  return S.Entries;
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

namespace HarnessActions {

/**
 * Returns the cached action creator for coverage marks.
 * User Story: As CLI coverage tracking, I need a reusable action creator so
 * covered command groups are recorded consistently.
 */
inline rtk::ActionCreator<ECommandGroup> MarkCoveredActionCreator() {
  static auto C =
      rtk::createAction<ECommandGroup>(TEXT("testgame/harness/markCovered"));
  return C;
}

/**
 * Returns the cached action creator for coverage resets.
 * User Story: As coverage cleanup, I need a reusable reset action creator so
 * a new run starts with empty harness coverage.
 */
inline rtk::ActionCreatorWithoutPayload ResetCoverageActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/harness/resetCoverage"));
  return C;
}

/**
 * Creates an action that marks one command group as covered.
 * User Story: As coverage bookkeeping, I need mark-covered actions so the test
 * harness can prove which CLI groups were exercised.
 */
inline rtk::AnyAction MarkCovered(ECommandGroup G) {
  return MarkCoveredActionCreator()(G);
}
/**
 * Creates an action that resets CLI coverage.
 * User Story: As harness resets, I need coverage reset actions so a new run
 * measures coverage from a clean baseline.
 */
inline rtk::AnyAction ResetCoverage() {
  return ResetCoverageActionCreator()();
}

} // namespace HarnessActions

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
