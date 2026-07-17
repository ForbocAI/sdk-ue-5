#pragma once
/**
 * Test-game store composition — mirrors TS test-game-cli/src/store.ts
 * Combines game slices into a single Redux store
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/ecs.hpp"
#include "Core/rtk.hpp"
#include "Features/Config/ConfigSlice.h"
#include "TestGame/Features/Entities/NPCs/NPCsSlice.h"
#include "TestGame/Features/Entities/Player/PlayerSlice.h"
#include "TestGame/Features/Systems/Bridge/BridgeSlice.h"
#include "TestGame/Features/Components/Spatial/Grid/GridSlice.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageSlice.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerSlice.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Components/Inventory/InventorySlice.h"
#include "TestGame/Features/Systems/Memory/MemorySlice.h"
#include "TestGame/Features/Systems/Quality/QualitySlice.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSlice.h"
#include "TestGame/Features/Systems/Social/SocialSlice.h"
#include "TestGame/Features/Systems/Soul/SoulSlice.h"
#include "TestGame/Features/Systems/Stealth/StealthSlice.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptSlice.h"
#include "TestGame/Features/Systems/Terminal/TerminalListeners.h"
#include "TestGame/Features/Systems/Terminal/UI/UISlice.h"

namespace TestGame {

typedef FTestGameState FRootState;
typedef ecs::FWorld FTestGameEcsWorld;

/** User Story: As a test game store consumer, I need to invoke create initial test game ecs world through a stable signature so the test game store workflow remains explicit and composable. @fn inline FTestGameEcsWorld CreateInitialTestGameEcsWorld() */
inline FTestGameEcsWorld CreateInitialTestGameEcsWorld() {
  return ecs::createWorld();
}

/**
 * Slice singletons
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

namespace GameSlices {

/**
 * Returns the NPC slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the NPC slice so
 * root state reduction reuses one canonical slice instance.
 * @fn inline const rtk::Slice<FNPCsSliceState> &NPCs()
 */
inline const rtk::Slice<FNPCsSliceState> &NPCs() {
  static const auto S = CreateNPCsSlice();
  return S;
}
/**
 * Returns the player slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the player slice
 * so player state reduction reuses one canonical slice instance.
 * @fn inline const rtk::Slice<FPlayerState> &Player()
 */
inline const rtk::Slice<FPlayerState> &Player() {
  static const auto S = CreatePlayerSlice();
  return S;
}
/**
 * Returns the grid slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the grid slice so
 * world layout state is reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FGridState> &Grid()
 */
inline const rtk::Slice<FGridState> &Grid() {
  static const auto S = CreateGridSlice();
  return S;
}
/**
 * Returns the stealth slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the stealth slice
 * so alert and door state are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FStealthState> &Stealth()
 */
inline const rtk::Slice<FStealthState> &Stealth() {
  static const auto S = CreateStealthSlice();
  return S;
}
/**
 * Returns the social slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the social slice
 * so dialogue and trade state are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FSocialState> &Social()
 */
inline const rtk::Slice<FSocialState> &Social() {
  static const auto S = CreateSocialSlice();
  return S;
}
/**
 * Returns the bridge slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the bridge slice
 * so local bridge rules are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FBridgeRulesState> &Bridge()
 */
inline const rtk::Slice<FBridgeRulesState> &Bridge() {
  static const auto S = CreateGameBridgeSlice();
  return S;
}
/**
 * Returns the memory slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the memory slice
 * so local memory records are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FGameMemorySliceState> &Memory()
 */
inline const rtk::Slice<FGameMemorySliceState> &Memory() {
  static const auto S = CreateGameMemorySlice();
  return S;
}
/**
 * Returns the inventory slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the inventory
 * slice so owner item lists are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FInventoryState> &Inventory()
 */
inline const rtk::Slice<FInventoryState> &Inventory() {
  static const auto S = CreateInventorySlice();
  return S;
}
/**
 * Returns the soul slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the soul slice so
 * export and import tracking are reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FSoulTrackingState> &Soul()
 */
inline const rtk::Slice<FSoulTrackingState> &Soul() {
  static const auto S = CreateGameSoulSlice();
  return S;
}
/**
 * Returns the UI slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the UI slice so
 * mode and message state are reduced through one shared slice instance.
 */
inline const rtk::Slice<FUIState> &UI() {
  static const auto S = CreateUISlice();
  return S;
}
/**
 * Returns the transcript slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the transcript
 * slice so command history is reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FTranscriptState> &Transcript()
 */
inline const rtk::Slice<FTranscriptState> &Transcript() {
  static const auto S = CreateTranscriptSlice();
  return S;
}
/**
 * Returns the scenario slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the scenario slice
 * so the default step list is reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FScenarioSliceState> &Scenario()
 */
inline const rtk::Slice<FScenarioSliceState> &Scenario() {
  static const auto S = CreateScenarioSlice();
  return S;
}
/**
 * Returns the harness slice singleton for the test game store.
 * User Story: As root reducer composition, I need access to the harness slice
 * so CLI coverage state is reduced through one shared slice instance.
 * @fn inline const rtk::Slice<FHarnessState> &Harness()
 */
inline const rtk::Slice<FHarnessState> &Harness() {
  static const auto S = CreateHarnessSlice();
  return S;
}
/**
 * Returns the command-runner slice singleton for runtime alias ownership.
 * User Story: As a test game store consumer, I need to invoke command runner through a stable signature so the test game store workflow remains explicit and composable.
 * @fn inline const rtk::Slice<CommandRunner::FCommandAliasState> &CommandRunner()
 */
inline const rtk::Slice<CommandRunner::FCommandAliasState> &CommandRunner() {
  static const auto S = CreateCommandRunnerSlice();
  return S;
}

/** User Story: As test-game configuration, I need one Config slice mounted in this package root store so direct contract verification has no process-global state. @fn inline const rtk::Slice<ConfigSlice::FConfigState> &Config() */
inline const rtk::Slice<ConfigSlice::FConfigState> &Config() {
  static const auto S = ConfigSlice::createConfigSlice();
  return S;
}

/** User Story: As a test-game quality evaluator, I need one Quality slice mounted in the package root store so lifecycle, evidence, and baselines have one authority. @fn inline const rtk::Slice<FQualityState> &Quality() */
inline const rtk::Slice<FQualityState> &Quality() {
  static const auto S = CreateQualitySlice();
  return S;
}

} // namespace GameSlices

/**
 * Reduces one action across all test-game slices.
 * User Story: As test-game store updates, I need a root reducer so every slice
 * receives the same action and the combined state stays in sync.
 * @fn inline FTestGameState TestGameReducer(const FTestGameState &State, const rtk::AnyAction &Action)
 */
inline FTestGameState TestGameReducer(const FTestGameState &State,
                                      const rtk::AnyAction &Action) {
  FTestGameState Next;
  Next.NPCs = GameSlices::NPCs().Reducer(State.NPCs, Action);
  Next.Player = GameSlices::Player().Reducer(State.Player, Action);
  Next.Grid = GameSlices::Grid().Reducer(State.Grid, Action);
  Next.Stealth = GameSlices::Stealth().Reducer(State.Stealth, Action);
  Next.Social = GameSlices::Social().Reducer(State.Social, Action);
  Next.Bridge = GameSlices::Bridge().Reducer(State.Bridge, Action);
  Next.Memory = GameSlices::Memory().Reducer(State.Memory, Action);
  Next.Inventory = GameSlices::Inventory().Reducer(State.Inventory, Action);
  Next.Soul = GameSlices::Soul().Reducer(State.Soul, Action);
  Next.UI = GameSlices::UI().Reducer(State.UI, Action);
  Next.Transcript =
      GameSlices::Transcript().Reducer(State.Transcript, Action);
  Next.Scenario = GameSlices::Scenario().Reducer(State.Scenario, Action);
  Next.Harness = GameSlices::Harness().Reducer(State.Harness, Action);
  Next.CommandRunner =
      GameSlices::CommandRunner().Reducer(State.CommandRunner, Action);
  Next.Config = GameSlices::Config().Reducer(State.Config, Action);
  Next.Quality = GameSlices::Quality().Reducer(State.Quality, Action);
  return Next;
}

/** User Story: As a test game store consumer, I need to invoke create initial test game state through a stable signature so the test game store workflow remains explicit and composable. @fn inline FTestGameState CreateInitialTestGameState() */
inline FTestGameState CreateInitialTestGameState() {
  FTestGameState Initial;
  Initial.NPCs = GameSlices::NPCs().InitialState;
  Initial.Player = GameSlices::Player().InitialState;
  Initial.Grid = GameSlices::Grid().InitialState;
  Initial.Stealth = GameSlices::Stealth().InitialState;
  Initial.Social = GameSlices::Social().InitialState;
  Initial.Bridge = GameSlices::Bridge().InitialState;
  Initial.Memory = GameSlices::Memory().InitialState;
  Initial.Inventory = GameSlices::Inventory().InitialState;
  Initial.Soul = GameSlices::Soul().InitialState;
  Initial.UI = GameSlices::UI().InitialState;
  Initial.Transcript = GameSlices::Transcript().InitialState;
  Initial.Scenario = GameSlices::Scenario().InitialState;
  Initial.Harness = GameSlices::Harness().InitialState;
  Initial.CommandRunner = GameSlices::CommandRunner().InitialState;
  Initial.Config = GameSlices::Config().InitialState;
  Initial.Quality = GameSlices::Quality().InitialState;
  return Initial;
}

/**
 * Creates a fresh test-game store instance.
 * User Story: As deterministic game runs, I need a new store per session so
 * each run starts from a clean, reproducible state baseline.
 * @fn inline FTestGameStore createTestGameStore()
 */
inline FTestGameStore createTestGameStore() {
  std::vector<rtk::Middleware<FTestGameState>> Middlewares;
  Middlewares.push_back(createGameListenerMiddleware());
  return rtk::configureStore<FTestGameState>(&TestGameReducer,
                                             CreateInitialTestGameState(),
                                             Middlewares);
}

} // namespace TestGame
