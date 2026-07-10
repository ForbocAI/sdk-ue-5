#pragma once
/**
 * Test-game orchestrator — mirrors TS test-game-cli/src/game.ts
 * Executes scenario steps, records transcripts, enforces CLI coverage
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

#include "CoreMinimal.h"
#include "TestGame/TestGameCommandSurface.h"
#include "TestGame/TestGameContract.h"
#include "TestGame/Views/Terminal/TerminalView.h"
#include "TestGame/Features/Systems/Terminal/TerminalListeners.h"
#include "TestGame/TestGameRuntime.h"
#include "Core/ue_fp.hpp"
#include "HAL/PlatformProcess.h"

namespace TestGame {

/**
 * Attempts to parse a JSON verdict from CLI output.
 * User Story: As test-game transcript replay, I need a parsed verdict shape so
 * CLI output can be converted into structured follow-up actions.
 * Returns true if a verdict with an action type was found.
 */
struct FParsedVerdict {
  bool bValid;
  FString ActionType;
  FPosition TargetHex;
  int32 SuspicionDelta;
  FParsedVerdict() : bValid(false), SuspicionDelta(0) {}
};

/**
 * Attempts to parse a verdict payload from CLI output.
 * User Story: As transcript replay, I need verdict parsing so command output
 * can drive follow-up state updates inside the test-game store.
 */
inline FParsedVerdict TryParseVerdict(const FString &Output) {
  /**
   * Simple JSON extraction — look for {"action":{"type":"..."}}
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  int32 ActionIdx = Output.Find(TEXT("\"action\""));
  return ActionIdx == INDEX_NONE
             ? FParsedVerdict()
             : [&]() -> FParsedVerdict {
                 int32 TypeIdx =
                     Output.Find(TEXT("\"type\""), ESearchCase::IgnoreCase,
                                 ESearchDir::FromStart, ActionIdx);
                 return TypeIdx == INDEX_NONE
                            ? FParsedVerdict()
                            : [&]() -> FParsedVerdict {
                                /**
                                 * Extract type value between quotes after "type":
                                 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                                 */
                                int32 ColonIdx = Output.Find(
                                    TEXT(":"), ESearchCase::IgnoreCase,
                                    ESearchDir::FromStart, TypeIdx + 6);
                                return ColonIdx == INDEX_NONE
                                           ? FParsedVerdict()
                                           : [&]() -> FParsedVerdict {
                                               int32 QuoteStart = Output.Find(
                                                   TEXT("\""),
                                                   ESearchCase::IgnoreCase,
                                                   ESearchDir::FromStart,
                                                   ColonIdx + 1);
                                               return QuoteStart == INDEX_NONE
                                                          ? FParsedVerdict()
                                                          : [&]() -> FParsedVerdict {
                                                              int32 QuoteEnd =
                                                                  Output.Find(
                                                                      TEXT("\""),
                                                                      ESearchCase::IgnoreCase,
                                                                      ESearchDir::FromStart,
                                                                      QuoteStart + 1);
                                                              return QuoteEnd == INDEX_NONE
                                                                         ? FParsedVerdict()
                                                                         : [&]() -> FParsedVerdict {
                                                                             FParsedVerdict V;
                                                                             V.ActionType = Output.Mid(
                                                                                 QuoteStart + 1,
                                                                                 QuoteEnd - QuoteStart - 1);
                                                                             V.bValid = !V.ActionType.IsEmpty();
                                                                             return V;
                                                                           }();
                                                            }();
                                             }();
                              }();
               }();
}

/**
 * Applies initial state mutations for a scenario event type.
 * Mirrors TS applyScenarioInitialState().
 * User Story: As scenario setup, I need event-specific initialization so each
 * scenario starts with the right NPCs, dialogue, and memory state.
 */
inline void ApplyScenarioInitialState(
    const FScenarioStep &Step,
    rtk::EnhancedStore<FTestGameState> &Store) {
  (Step.EventType == EEventType::Stealth)
      ? [&]() {
          Store.dispatch(StealthActions::setDoorOpen(true));
          Store.dispatch(StealthActions::bumpAlert(25));

          FGameNPC Doomguard;
          Doomguard.Id = TEXT("doomguard");
          Doomguard.Name = TEXT("Doomguard Patrol");
          Doomguard.Faction = TEXT("Doomguards");
          Doomguard.Hp = 100;
          Doomguard.Suspicion = 40;
          Doomguard.Position = FPosition(5, 10);
          Store.dispatch(NPCsActions::UpsertNPC(Doomguard));

          FMemoryRecord Mem;
          Mem.Id = TEXT("mem-door-001");
          Mem.NpcId = TEXT("doomguard");
          Mem.Text = TEXT("Armory door found open at x:5, y:12");
          Mem.Importance = 0.9f;
          Store.dispatch(GameMemoryActions::storeMemory(Mem));
        }()
      : (void)0;

  (Step.EventType == EEventType::Social)
      ? [&]() {
          FGameNPC Miller;
          Miller.Id = TEXT("miller");
          Miller.Name = TEXT("Miller");
          Miller.Faction = TEXT("Neutral");
          Miller.Hp = 100;
          Miller.Suspicion = 50;
          Miller.Inventory.Add(TEXT("medkit"));
          Miller.KnownSecrets.Add(TEXT("player_stole_rations"));
          Miller.Position = FPosition(5, 12);
          Store.dispatch(NPCsActions::UpsertNPC(Miller));

          Store.dispatch(SocialActions::setDialogue(
              TEXT("I know you took those rations...")));

          FTradeOffer Trade;
          Trade.NpcId = TEXT("miller");
          Trade.Item = TEXT("medkit");
          Trade.Price = 100;
          Store.dispatch(SocialActions::setTradeOffer(Trade));

          NPCsActions::FPatchNPCPayload Patch;
          Patch.Id = TEXT("miller");
          Patch.Patch.Suspicion = 75;
          Patch.Patch.bHasSuspicion = true;
          Store.dispatch(NPCsActions::PatchNPC(Patch));
        }()
      : (void)0;

  (Step.EventType == EEventType::Escape)
      ? (Store.dispatch(PlayerActions::setHidden(false)), void())
      : (void)0;

  (Step.EventType == EEventType::Persistence)
      ? [&]() {
          FMarkSoulExportedPayload ExportPayload;
          ExportPayload.NpcId = TEXT("doomguard");
          ExportPayload.TxId = TEXT("tx-runtime-001");
          Store.dispatch(GameSoulActions::markSoulExported(ExportPayload));
          Store.dispatch(
              GameSoulActions::markSoulImported(TEXT("tx-runtime-001")));
          Store.dispatch(
              GameMemoryActions::clearMemoryForNpc(TEXT("doomguard")));
        }()
      : (void)0;
}

/**
 * Extracts an NPC id from a command string containing "process" or "chat".
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline FString ExtractNpcIdFromCommand(const FString &Command) {
  int32 ProcessIdx = Command.Find(TEXT("process "));
  int32 ChatIdx = Command.Find(TEXT("chat "));
  int32 StartIdx = ProcessIdx != INDEX_NONE
                       ? ProcessIdx + 8
                       : (ChatIdx != INDEX_NONE ? ChatIdx + 5 : -1);
  return StartIdx < 0
             ? FString()
             : [&]() -> FString {
                 int32 EndIdx = Command.Find(TEXT(" "), ESearchCase::IgnoreCase,
                                             ESearchDir::FromStart, StartIdx);
                 return EndIdx != INDEX_NONE
                            ? Command.Mid(StartIdx, EndIdx - StartIdx)
                            : Command.Mid(StartIdx);
               }();
}

/**
 * Applies a parsed verdict to the store for a given command.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline void ApplyVerdictIfValid(
    const FCommandSpec &Cmd, const CommandSurface::FCommandOutput &CmdResult,
    rtk::EnhancedStore<FTestGameState> &Store) {
  (Cmd.Group == ECommandGroup::NpcProcessChat)
      ? [&]() {
          FParsedVerdict Verdict = TryParseVerdict(CmdResult.Output);
          Verdict.bValid
              ? [&]() {
                  /**
                   * Extract NPC id from command string
                   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
                   */
                  FString NpcId = ExtractNpcIdFromCommand(Cmd.Command);
                  !NpcId.IsEmpty()
                      ? [&]() {
                          NPCsActions::FApplyNpcVerdictPayload VP;
                          VP.Id = NpcId;
                          VP.Action.Type = Verdict.ActionType;
                          VP.Action.TargetHex = Verdict.TargetHex;
                          VP.Action.bHasTargetHex = true;
                          VP.StateDelta.Suspicion = Verdict.SuspicionDelta;
                          VP.StateDelta.bHasSuspicion =
                              Verdict.SuspicionDelta != 0;
                          Store.dispatch(NPCsActions::ApplyNpcVerdict(VP));
                        }()
                      : (void)0;
                }()
              : (void)0;
        }()
      : (void)0;

  (Cmd.Group == ECommandGroup::BridgeValidate)
      ? [&]() {
          FParsedVerdict Validation = TryParseVerdict(CmdResult.Output);
          (Validation.bValid && Validation.ActionType == TEXT("BLOCKED"))
              ? (Store.dispatch(
                     UIActions::addMessage(TEXT("System Bridge Blocked Action"))),
                 void())
              : (void)0;
        }()
      : (void)0;
}

/**
 * Logs command result status and error details.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline void LogCommandResult(const FCommandSpec &Cmd,
                              const CommandSurface::FCommandOutput &CmdResult) {
  FString StatusStr = CmdResult.Status == ETranscriptStatus::Ok
                          ? FString(TEXT("[ok]"))
                          : FString(TEXT("[error]"));
  UE_LOG(LogTemp, Display, TEXT("%s %s"), *StatusStr, *Cmd.Command);

  if (CmdResult.Status == ETranscriptStatus::Error) {
    UE_LOG(LogTemp, Error, TEXT("LOG_ERR_CRITICAL // BIT_ROT_DETECTED"));
    if (!CmdResult.Output.IsEmpty()) {
      UE_LOG(LogTemp, Error, TEXT("  | %s"), *CmdResult.Output);
    }
  }
}

/**
 * Processes a single command within a scenario step.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline void ProcessCommand(const FScenarioStep &Step, const FCommandSpec &Cmd,
                           CommandSurface::FAliasState &Aliases,
                           rtk::EnhancedStore<FTestGameState> &Store) {
  const FString ScenarioId = Step.Id;
  const FCommandSpec StableCmd = Cmd;
  const CommandSurface::FCommandOutput CmdResult =
      CommandSurface::Execute(StableCmd.Command, Aliases);

  /**
   * Update coverage
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  (CmdResult.Status == ETranscriptStatus::Ok)
      ? (Store.dispatch(HarnessActions::markCovered(StableCmd.Group)), void())
      : (void)0;

  /**
   * Record transcript
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TranscriptActions::FRecordTranscriptPayload TxPayload;
  TxPayload.ScenarioId = ScenarioId;
  TxPayload.CommandGroup = StableCmd.Group;
  TxPayload.Command = StableCmd.Command;
  TxPayload.ExpectedRoutes = StableCmd.ExpectedRoutes;
  TxPayload.Status = CmdResult.Status;
  TxPayload.Output = CmdResult.Output;
  Store.dispatch(TranscriptActions::recordTranscript(TxPayload));

  /**
   * Drive state from CLI output
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  (CmdResult.Status == ETranscriptStatus::Ok)
      ? (ApplyVerdictIfValid(StableCmd, CmdResult, Store), void())
      : (void)0;

  LogCommandResult(StableCmd, CmdResult);
}

/**
 * Recursive helper — iterates commands in a step.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
namespace detail {
inline int32 CommandDelayMs() {
  const FString Raw =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_TEST_GAME_COMMAND_DELAY_MS"));
  const int32 Parsed = Raw.IsEmpty() ? 1000 : FCString::Atoi(*Raw);
  return Parsed >= 0 ? Parsed : 1000;
}

inline void DelayAfterCommand() {
  const float Seconds = static_cast<float>(CommandDelayMs()) / 1000.0f;
  Seconds > 0.0f ? FPlatformProcess::Sleep(Seconds) : void();
}

inline void ProcessCommands(const FScenarioStep &Step,
                            const TArray<FCommandSpec> &Commands, int32 Index,
                            CommandSurface::FAliasState &Aliases,
                            rtk::EnhancedStore<FTestGameState> &Store) {
  return Index >= Commands.Num()
              ? (void)0
              : (ProcessCommand(Step, Commands[Index], Aliases, Store),
                DelayAfterCommand(),
                ProcessCommands(Step, Commands, Index + 1, Aliases, Store));
}

/**
 * Recursive helper — iterates scenario steps.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index,
                         CommandSurface::FAliasState &Aliases,
                         rtk::EnhancedStore<FTestGameState> &Store) {
  if (Index >= Steps.Num()) {
    return;
  }

  UE_LOG(LogTemp, Display, TEXT("\n:: %s [%s]"), *Steps[Index].Title,
         *Steps[Index].Id);
  UE_LOG(LogTemp, Display, TEXT("%s"), *Steps[Index].Description);
  ApplyScenarioInitialState(Steps[Index], Store);
  ProcessCommands(Steps[Index], Steps[Index].Commands, 0, Aliases, Store);
  ProcessSteps(Steps, Index + 1, Aliases, Store);
}

/**
 * Recursive helper — joins missing group ids into a comma-separated string.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline FString JoinMissingGroups(const TArray<ECommandGroup> &Missing,
                                  int32 Index, const FString &Acc) {
  return Index >= Missing.Num()
             ? Acc
             : JoinMissingGroups(
                   Missing, Index + 1,
                   Acc + (Index > 0 ? FString(TEXT(", ")) : FString()) +
                       FString::FromInt(static_cast<int32>(Missing[Index])));
}

/**
 * Recursive helper — logs transcript entries.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline void LogTranscriptEntries(const TArray<FTranscriptEntry> &Entries,
                                  int32 Index) {
  if (Index >= Entries.Num()) {
    return;
  }

  const FString Status =
      Entries[Index].Status == ETranscriptStatus::Ok ? TEXT("ok   ")
                                                     : TEXT("error");
  UE_LOG(LogTemp, Display, TEXT("%s | %s | %s"), *Entries[Index].Timestamp,
         *Status, *Entries[Index].Command);
  LogTranscriptEntries(Entries, Index + 1);
}

inline int32 CountTranscriptErrors(const TArray<FTranscriptEntry> &Entries,
                                   int32 Index) {
  return Index >= Entries.Num()
             ? 0
             : ((Entries[Index].Status == ETranscriptStatus::Error ? 1 : 0) +
                CountTranscriptErrors(Entries, Index + 1));
}
} // namespace detail

/**
 * Runs a full game session in the given mode.
 * Returns a GameRunResult with coverage status and transcript.
 * Mirrors TS runGame().
 * User Story: As end-to-end test execution, I need one orchestrator entrypoint
 * so a full scenario suite can run and report transcript plus coverage state.
 */
inline FGameRunResult RunGame(EPlayMode Mode,
                              const FString &ApiUrlOverride = TEXT("")) {
  SDKConfig::InitializeConfig();
  auto Store = createTestGameStoreWithListeners();
  Store.dispatch(UIActions::setMode(Mode));

  /**
   * Seed initial player NPC
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FGameNPC Scout;
  Scout.Id = TEXT("scout");
  Scout.Name = TEXT("Scout");
  Scout.Faction = TEXT("Player");
  Scout.Hp = 100;
  Scout.Suspicion = 0;
  Scout.Inventory.Add(TEXT("coin-pouch"));
  Scout.Position = FPosition(1, 1);
  Store.dispatch(NPCsActions::UpsertNPC(Scout));

  UE_LOG(LogTemp, Display, TEXT("SYSTEM_OVERRIDE // NEURAL_LINK_ESTABLISHED"));
  UE_LOG(LogTemp, Display, TEXT("[VOID::WATCHER] Echoes session booting at "
                                "mode=%s"),
         Mode == EPlayMode::Autoplay ? TEXT("autoplay") : TEXT("manual"));
  UE_LOG(LogTemp, Display, TEXT("%s"), *RenderLegend());

  const FString ApiUrl =
      !ApiUrlOverride.IsEmpty() ? ApiUrlOverride : ResolveRuntimeUrl();
  if (ApiUrl.IsEmpty()) {
    UE_LOG(LogTemp, Error,
           TEXT("TestGameContract: explicit runtime URL required. Set "
                "FORBOCAI_API_URL before running."));
    FGameRunResult Result;
    Result.bComplete = false;
    Result.Summary = TEXT("Runtime URL not configured");
    return Result;
  }
  const Contract::FContractResponse ContractResp = Contract::FetchContract(ApiUrl);
  if (!ContractResp.bValid) {
    UE_LOG(LogTemp, Error, TEXT("TestGameContract: API contract unavailable. Aborting RunGame."));
    FGameRunResult Result;
    Result.bComplete = false;
    Result.Summary = TEXT("API contract unavailable");
    return Result;
  }

  CommandSurface::FAliasState Aliases = CommandSurface::CreateAliasState(ContractResp);
  const TArray<FScenarioStep> Steps = Contract::ConvertScenariosRecursive(ContractResp.Scenarios, 0, {});
  
  detail::ProcessSteps(Steps, 0, Aliases, Store);

  /**
   * Build result
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  const auto &State = Store.getState();
    TArray<ECommandGroup> RequiredGroups;
  struct CollectGroups {
    static void apply(const TArray<FString>& In, TArray<ECommandGroup>& Out, int32 Idx) {
      if (Idx >= In.Num()) return;
      Out.Add(Contract::detail::ParseCommandGroup(In[Idx]));
      apply(In, Out, Idx + 1);
    }
  };
  CollectGroups::apply(ContractResp.RequiredCommandGroups, RequiredGroups, 0);
  TArray<ECommandGroup> Missing = SelectMissingGroups(State.Harness.Covered, RequiredGroups);
  const int32 ErrorCount =
      TranscriptSelectors::SelectTranscriptErrorCount(State.Transcript);
  bool bComplete = Missing.Num() == 0 && ErrorCount == 0;

  FString Summary = bComplete
                        ? FString(TEXT("ALL_BINDINGS_COMPLETE :: Coverage achieved."))
                        : FString::Printf(TEXT("VOID_GAPS_DETECTED :: %d transcript error%s"),
                                          ErrorCount,
                                          ErrorCount == 1 ? TEXT("")
                                                          : TEXT("s"));
  Missing.Num() > 0
      ? (Summary += FString(TEXT(" | Missing groups -> ")) +
                    detail::JoinMissingGroups(Missing, 0, FString()),
         void())
      : (void)0;

  /**
   * Log transcript summary
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  UE_LOG(LogTemp, Display, TEXT("\n=== Transcript Summary ==="));
  detail::LogTranscriptEntries(State.Transcript.Entries, 0);
  UE_LOG(LogTemp, Display, TEXT("%s"), *Summary);

  FGameRunResult Result;
  Result.bComplete = bComplete;
  Result.MissingGroups = Missing;
  Result.Transcript = State.Transcript.Entries;
  Result.Summary = Summary;
  return Result;
}

} // namespace TestGame
