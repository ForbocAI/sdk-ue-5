#pragma once

#include "HAL/PlatformProcess.h"
#include "Features/Config/ConfigAdapters.h"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/Player/PlayerActions.h"
#include "TestGame/Features/Systems/Contract/ContractThunks.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameSelectors.h"
#include "TestGame/Features/Systems/Harness/HarnessThunks.h"
#include "TestGame/Features/Systems/Memory/MemoryActions.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "TestGame/Features/Systems/Social/SocialActions.h"
#include "TestGame/Features/Systems/Soul/SoulActions.h"
#include "TestGame/Features/Systems/Stealth/StealthActions.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptActions.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

using FGameProgressSink = TFunction<void(const FGameProgress &)>;

namespace GameThunksDetail {

inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) {
  Sink ? (Sink(Progress), void()) : void();
}

inline int32 CommandDelayMs() {
  const FString Raw = FPlatformMisc::GetEnvironmentVariable(
      TEXT("FORBOCAI_TEST_GAME_COMMAND_DELAY_MS"));
  const int32 Parsed = Raw.IsEmpty() ? 1000 : FCString::Atoi(*Raw);
  return Parsed >= 0 ? Parsed : 1000;
}

inline void DelayAfterCommand() {
  const float Seconds = static_cast<float>(CommandDelayMs()) / 1000.0f;
  Seconds > 0.0f ? FPlatformProcess::Sleep(Seconds) : void();
}

inline void ApplyScenarioInitialState(
    const FScenarioStep &Step, FTestGameStore &Store) {
  (Step.EventType == EEventType::Stealth)
      ? [&]() {
          Store.dispatch(StealthActions::setDoorOpen(true));
          Store.dispatch(StealthActions::bumpAlert(25));

          FGameNPC Npc;
          Npc.Id = TEXT("doomguard");
          Npc.Name = TEXT("Doomguard Patrol");
          Npc.Faction = TEXT("Doomguards");
          Npc.Hp = 100;
          Npc.Suspicion = 40;
          Npc.Position = FPosition(5, 10);
          Store.dispatch(NPCsActions::UpsertNPC(Npc));

          FMemoryRecord Memory;
          Memory.Id = TEXT("mem-door-001");
          Memory.NpcId = TEXT("doomguard");
          Memory.Text = TEXT("Armory door found open at x:5, y:12");
          Memory.Importance = 0.9f;
          Store.dispatch(GameMemoryActions::storeMemory(Memory));
        }()
      : void();

  (Step.EventType == EEventType::Social)
      ? [&]() {
          FGameNPC Npc;
          Npc.Id = TEXT("miller");
          Npc.Name = TEXT("Miller");
          Npc.Faction = TEXT("Neutral");
          Npc.Hp = 100;
          Npc.Suspicion = 50;
          Npc.Inventory.Add(TEXT("medkit"));
          Npc.KnownSecrets.Add(TEXT("player_stole_rations"));
          Npc.Position = FPosition(5, 12);
          Store.dispatch(NPCsActions::UpsertNPC(Npc));
          Store.dispatch(SocialActions::setDialogue(
              TEXT("I know you took those rations...")));

          FTradeOffer Offer;
          Offer.NpcId = TEXT("miller");
          Offer.Item = TEXT("medkit");
          Offer.Price = 100;
          Store.dispatch(SocialActions::setTradeOffer(Offer));

          NPCsActions::FPatchNPCPayload Patch;
          Patch.Id = TEXT("miller");
          Patch.Patch.Suspicion = 75;
          Patch.Patch.bHasSuspicion = true;
          Store.dispatch(NPCsActions::PatchNPC(Patch));
        }()
      : void();

  Step.EventType == EEventType::Escape
      ? (Store.dispatch(PlayerActions::setHidden(false)), void())
      : void();

  (Step.EventType == EEventType::Persistence)
      ? [&]() {
          FMarkSoulExportedPayload Export;
          Export.NpcId = TEXT("doomguard");
          Export.TxId = TEXT("tx-runtime-001");
          Store.dispatch(GameSoulActions::markSoulExported(Export));
          Store.dispatch(
              GameSoulActions::markSoulImported(TEXT("tx-runtime-001")));
          Store.dispatch(
              GameMemoryActions::clearMemoryForNpc(TEXT("doomguard")));
        }()
      : void();
}

inline void ApplyCommandResult(
    const FCommandSpec &Command,
    const CommandRunner::FCommandOutput &CommandResult,
    FTestGameStore &Store) {
  Command.Group == ECommandGroup::NpcProcessChat
      ? [&]() {
          const FParsedVerdict Verdict =
              GameAdapters::ParseVerdict(CommandResult.Output);
          const FString NpcId = GameAdapters::ExtractNpcId(Command.Command);
          (Verdict.bValid && !NpcId.IsEmpty())
              ? [&]() {
                  NPCsActions::FApplyNpcVerdictPayload Payload;
                  Payload.Id = NpcId;
                  Payload.Action.Type = Verdict.ActionType;
                  Payload.Action.TargetHex = Verdict.TargetHex;
                  Payload.Action.bHasTargetHex = true;
                  Payload.StateDelta.Suspicion = Verdict.SuspicionDelta;
                  Payload.StateDelta.bHasSuspicion =
                      Verdict.SuspicionDelta != 0;
                  Store.dispatch(NPCsActions::ApplyNpcVerdict(Payload));
                }()
              : void();
        }()
      : void();

  (Command.Group == ECommandGroup::BridgeValidate &&
   CommandResult.Status == ETranscriptStatus::Error)
      ? (Store.dispatch(UIActions::addMessage(
             FString(TEXT("Bridge validation failed: ")) +
             CommandResult.Output)),
         void())
      : void();
}

inline void ProcessCommand(const FScenarioStep &Step,
                           const FCommandSpec &Command,
                           CommandRunner::FAliasState &Aliases,
                           FTestGameStore &Store,
                           const FGameProgressSink &Sink) {
  const CommandRunner::FCommandOutput Result =
      CommandRunner::Execute(Command.Command, Aliases);
  Result.Status == ETranscriptStatus::Ok
      ? (Store.dispatch(HarnessActions::markCovered(Command.Group)), void())
      : void();

  TranscriptActions::FRecordTranscriptPayload Transcript;
  Transcript.ScenarioId = Step.Id;
  Transcript.CommandGroup = Command.Group;
  Transcript.Command = Command.Command;
  Transcript.ExpectedRoutes = Command.ExpectedRoutes;
  Transcript.Status = Result.Status;
  Transcript.Output = Result.Output;
  Store.dispatch(TranscriptActions::recordTranscript(Transcript));

  Result.Status == ETranscriptStatus::Ok
      ? (ApplyCommandResult(Command, Result, Store), void())
      : void();

  FGameProgress Progress;
  Progress.Type = EGameProgressType::CommandCompleted;
  Progress.Command = Command;
  Progress.CommandResult = Result;
  Emit(Sink, MoveTemp(Progress));
}

inline void ProcessCommands(const FScenarioStep &Step, int32 Index,
                            CommandRunner::FAliasState &Aliases,
                            FTestGameStore &Store,
                            const FGameProgressSink &Sink) {
  Index >= Step.Commands.Num()
      ? void()
      : (ProcessCommand(Step, Step.Commands[Index], Aliases, Store, Sink),
         DelayAfterCommand(),
         ProcessCommands(Step, Index + 1, Aliases, Store, Sink));
}

inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index,
                         CommandRunner::FAliasState &Aliases,
                         FTestGameStore &Store,
                         const FGameProgressSink &Sink) {
  Index >= Steps.Num()
      ? void()
      : [&]() {
          FGameProgress Progress;
          Progress.Type = EGameProgressType::StepStarted;
          Progress.Step = Steps[Index];
          Emit(Sink, MoveTemp(Progress));
          ApplyScenarioInitialState(Steps[Index], Store);
          ProcessCommands(Steps[Index], 0, Aliases, Store, Sink);
          ProcessSteps(Steps, Index + 1, Aliases, Store, Sink);
        }();
}

inline FGameRunResult FailRun(const FString &Message,
                              const FGameProgressSink &Sink) {
  FGameRunResult Failure;
  Failure.Summary = Message;
  FGameProgress Progress;
  Progress.Type = EGameProgressType::ContractFailed;
  Progress.Message = Message;
  Emit(Sink, MoveTemp(Progress));
  return Failure;
}

} // namespace GameThunksDetail

inline FGameRunResult RunGame(FTestGameStore &Store, EPlayMode Mode,
                              const FString &ApiUrlOverride,
                              const FGameProgressSink &ProgressSink = {}) {
  SDKConfig::InitializeConfig();
  Store.dispatch(UIActions::setMode(Mode));

  FGameProgress Started;
  Started.Type = EGameProgressType::SessionStarted;
  Started.Mode = Mode;
  GameThunksDetail::Emit(ProgressSink, MoveTemp(Started));

  FGameNPC PlayerNpc;
  PlayerNpc.Id = TEXT("scout");
  PlayerNpc.Name = TEXT("Scout");
  PlayerNpc.Faction = TEXT("Player");
  PlayerNpc.Hp = 100;
  PlayerNpc.Suspicion = 0;
  PlayerNpc.Inventory.Add(TEXT("coin-pouch"));
  PlayerNpc.Position = FPosition(1, 1);
  Store.dispatch(NPCsActions::UpsertNPC(PlayerNpc));

  const FString ApiUrl = !ApiUrlOverride.IsEmpty()
                             ? ApiUrlOverride
                             : ResolveRuntimeUrl();
  return ApiUrl.IsEmpty()
             ? GameThunksDetail::FailRun(TEXT("Runtime URL not configured"),
                                         ProgressSink)
             : [&]() {
                 const Contract::FContractResponse Contract =
                     Contract::GetContract(Store, ApiUrl);
                 return !Contract.bValid
                            ? GameThunksDetail::FailRun(
                                  TEXT("API contract unavailable"),
                                  ProgressSink)
                            : [&]() {
                                Store.dispatch(
                                    ScenarioActions::contractReceived(
                                        Contract::ToScenarioSteps(
                                            Contract.Scenarios, 0, {})));
                                const TArray<FScenarioStep> Steps =
                                    ScenarioSelectors::SelectScenarioSteps(
                                        Store.getState().Scenario);
                                CommandRunner::FAliasState Aliases =
                                    CommandRunner::CreateAliasState(Contract);
                                GameThunksDetail::ProcessSteps(
                                    Steps, 0, Aliases, Store, ProgressSink);

                                FGameRunResult Result =
                                    GameSelectors::SelectGameRunResult(
                                        Store.getState(),
                                        Contract::ToCommandGroups(
                                            Contract.RequiredCommandGroups, 0,
                                            {}));
                                FGameProgress Completed;
                                Completed.Type =
                                    EGameProgressType::SessionCompleted;
                                Completed.RunResult = Result;
                                GameThunksDetail::Emit(
                                    ProgressSink, MoveTemp(Completed));
                                return Result;
                              }();
               }();
}

} // namespace TestGame
