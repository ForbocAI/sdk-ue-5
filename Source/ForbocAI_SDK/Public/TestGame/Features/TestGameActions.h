#pragma once
/**
 * Test-game action creators own test-game event construction.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/Features/TestGameTypes.h"

namespace TestGame {

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

} // namespace TestGame
