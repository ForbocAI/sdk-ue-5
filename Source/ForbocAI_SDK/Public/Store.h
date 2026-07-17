#pragma once

#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Features/Bridge/BridgeSlice.h"
#include "Features/CLI/CLISlice.h"
#include "Features/CLI/Presentation/PresentationSlice.h"
#include "Features/Config/ConfigSlice.h"
#include "Features/Dependencies/DependenciesSlice.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/Logger/LoggerListeners.h"
#include "Features/NPC/NPCListeners.h"
#include "Features/State/StateTypes.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Vector/VectorSlice.h"

namespace StoreInternal {

/**
 * Returns the singleton NPC slice definition.
 * User Story: As store assembly, I need one shared NPC slice instance so every
 * store uses the same reducer wiring.
 * @fn inline const rtk::Slice<NPCSlice::FNPCSliceState> &GetNPCSlice()
 */
inline const rtk::Slice<NPCSlice::FNPCSliceState> &GetNPCSlice() {
  static const func::Lazy<rtk::Slice<NPCSlice::FNPCSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<NPCSlice::FNPCSliceState> { return NPCSlice::createNPCSlice(); });
  return func::eval(Slice);
}

/**
 * Returns the singleton memory slice definition.
 * User Story: As store assembly, I need one shared memory slice instance so
 * memory reducers stay consistent across stores.
 * @fn inline const rtk::Slice<MemorySlice::FMemorySliceState> &GetMemorySlice()
 */
inline const rtk::Slice<MemorySlice::FMemorySliceState> &GetMemorySlice() {
  static const func::Lazy<rtk::Slice<MemorySlice::FMemorySliceState>> Slice =
      func::lazy([]() -> rtk::Slice<MemorySlice::FMemorySliceState> { return MemorySlice::createMemorySlice(); });
  return func::eval(Slice);
}

/**
 * Returns the singleton directive slice definition.
 * User Story: As store assembly, I need one shared directive slice instance so
 * directive lifecycle updates are wired uniformly.
 * @fn inline const rtk::Slice<DirectiveSlice::FDirectiveSliceState> & GetDirectiveSlice()
 */
inline const rtk::Slice<DirectiveSlice::FDirectiveSliceState> &
GetDirectiveSlice() {
  static const func::Lazy<rtk::Slice<DirectiveSlice::FDirectiveSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<DirectiveSlice::FDirectiveSliceState> { return DirectiveSlice::createDirectiveSlice(); });
  return func::eval(Slice);
}

/**
 * Returns the singleton bridge slice definition.
 * User Story: As store assembly, I need one shared bridge slice instance so
 * validation state uses a single reducer definition.
 * @fn inline const rtk::Slice<BridgeSlice::FBridgeSliceState> &GetBridgeSlice()
 */
inline const rtk::Slice<BridgeSlice::FBridgeSliceState> &GetBridgeSlice() {
  static const func::Lazy<rtk::Slice<BridgeSlice::FBridgeSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<BridgeSlice::FBridgeSliceState> { return BridgeSlice::createBridgeSlice(); });
  return func::eval(Slice);
}

/**
 * Returns the singleton soul slice definition.
 * User Story: As store assembly, I need one shared soul slice instance so
 * export and import state uses the same reducer wiring.
 * @fn inline const rtk::Slice<SoulSlice::FSoulSliceState> &GetSoulSlice()
 */
inline const rtk::Slice<SoulSlice::FSoulSliceState> &GetSoulSlice() {
  static const func::Lazy<rtk::Slice<SoulSlice::FSoulSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<SoulSlice::FSoulSliceState> { return SoulSlice::createSoulSlice(); });
  return func::eval(Slice);
}

/**
 * Returns the singleton ghost slice definition.
 * User Story: As store assembly, I need one shared ghost slice instance so QA
 * state transitions are defined once for the runtime.
 * @fn inline const rtk::Slice<GhostSlice::FGhostSliceState> &GetGhostSlice()
 */
inline const rtk::Slice<GhostSlice::FGhostSliceState> &GetGhostSlice() {
  static const func::Lazy<rtk::Slice<GhostSlice::FGhostSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<GhostSlice::FGhostSliceState> { return GhostSlice::createGhostSlice(); });
  return func::eval(Slice);
}

/** User Story: As a store consumer, I need to invoke get vector slice through a stable signature so the store workflow remains explicit and composable. @fn inline const rtk::Slice<FVectorState> &GetVectorSlice() */
inline const rtk::Slice<FVectorState> &GetVectorSlice() {
  static const func::Lazy<rtk::Slice<FVectorState>> Slice = func::lazy(
      []() { return VectorSlice::createVectorSlice(); });
  return func::eval(Slice);
}

/** User Story: As a store consumer, I need to invoke get dependencies slice through a stable signature so the store workflow remains explicit and composable. @fn inline const rtk::Slice<FDependenciesState> &GetDependenciesSlice() */
inline const rtk::Slice<FDependenciesState> &GetDependenciesSlice() {
  static const func::Lazy<rtk::Slice<FDependenciesState>> Slice =
      func::lazy([]() { return DependenciesSlice::createDependenciesSlice(); });
  return func::eval(Slice);
}

/** User Story: As a store consumer, I need to invoke get clislice through a stable signature so the store workflow remains explicit and composable. @fn inline const rtk::Slice<ForbocAI::CLI::FCLIState> &GetCLISlice() */
inline const rtk::Slice<ForbocAI::CLI::FCLIState> &GetCLISlice() {
  static const func::Lazy<rtk::Slice<ForbocAI::CLI::FCLIState>> Slice =
      func::lazy([]() { return CLISlice::createCLISlice(); });
  return func::eval(Slice);
}

/** User Story: As store assembly, I need one shared CLI presentation slice so every command edge selects the same authored contract. @fn inline const rtk::Slice<ForbocAI::CLI::Presentation::FCLIPresentationState> & GetCLIPresentationSlice() */
inline const rtk::Slice<ForbocAI::CLI::Presentation::FCLIPresentationState> &
GetCLIPresentationSlice() {
  static const func::Lazy<
      rtk::Slice<ForbocAI::CLI::Presentation::FCLIPresentationState>> Slice =
      func::lazy([]() {
        return ForbocAI::CLI::Presentation::createCliPresentationSlice();
      });
  return func::eval(Slice);
}

/** User Story: As store assembly, I need one shared Config slice instance so every runtime consumer observes one package-owned configuration snapshot. @fn inline const rtk::Slice<ConfigSlice::FConfigState> &GetConfigSlice() */
inline const rtk::Slice<ConfigSlice::FConfigState> &GetConfigSlice() {
  static const func::Lazy<rtk::Slice<ConfigSlice::FConfigState>> Slice =
      func::lazy([]() { return ConfigSlice::createConfigSlice(); });
  return func::eval(Slice);
}

/**
 * User Story: As root store initialization, I need every feature slice's
 * canonical initial state composed once so direct struct defaults cannot drift
 * from createSlice.
 * @fn inline FRuntimeState createRuntimeInitialState()
 */
inline FRuntimeState createRuntimeInitialState() {
  FRuntimeState InitialState;
  InitialState.NPCs = GetNPCSlice().InitialState;
  InitialState.Memory = GetMemorySlice().InitialState;
  InitialState.Directives = GetDirectiveSlice().InitialState;
  InitialState.Bridge = GetBridgeSlice().InitialState;
  InitialState.Soul = GetSoulSlice().InitialState;
  InitialState.Ghost = GetGhostSlice().InitialState;
  InitialState.Vector = GetVectorSlice().InitialState;
  InitialState.Dependencies = GetDependenciesSlice().InitialState;
  InitialState.CLI = GetCLISlice().InitialState;
  InitialState.CLIPresentation = GetCLIPresentationSlice().InitialState;
  InitialState.Config = GetConfigSlice().InitialState;
  return InitialState;
}

} // namespace StoreInternal

/**
 * G8: Extra reducer type for game slices.
 * User Story: As game extension points, I need a reducer hook type so custom
 * game reducers can participate without replacing SDK reducers.
 * Receives current state + action, returns updated state.
 * Only the Extra map should be modified; SDK slice state is managed
 * by SDK reducers and must not be overwritten.
 */
using ExtraReducerFn =
    std::function<FRuntimeState(const FRuntimeState &, const rtk::AnyAction &)>;

namespace StoreInternal {

/**
 * Returns the extra reducers registered by game-specific extensions.
 * User Story: As store extensibility, I need a shared reducer registry so
 * game-specific reducers can be mounted before store creation.
 * @fn inline std::vector<ExtraReducerFn> &ExtraReducers()
 */
inline std::vector<ExtraReducerFn> &ExtraReducers() {
  static std::vector<ExtraReducerFn> Reducers;
  return Reducers;
}

} // namespace StoreInternal (extension)

/**
 * Runs the SDK reducers, then applies any registered extra reducers.
 * User Story: As root store reduction, I need SDK and game reducers composed
 * together so one dispatch updates all registered state.
 * @fn inline FRuntimeState StoreReducer(const FRuntimeState &State, const rtk::AnyAction &Action)
 */
inline FRuntimeState StoreReducer(const FRuntimeState &State,
                                const rtk::AnyAction &Action) {
  FRuntimeState Next = State;
  Next.NPCs = StoreInternal::GetNPCSlice().Reducer(State.NPCs, Action);
  Next.Memory = StoreInternal::GetMemorySlice().Reducer(State.Memory, Action);
  Next.Directives =
      StoreInternal::GetDirectiveSlice().Reducer(State.Directives, Action);
  Next.Bridge = StoreInternal::GetBridgeSlice().Reducer(State.Bridge, Action);
  Next.Soul = StoreInternal::GetSoulSlice().Reducer(State.Soul, Action);
  Next.Ghost = StoreInternal::GetGhostSlice().Reducer(State.Ghost, Action);
  Next.Vector = StoreInternal::GetVectorSlice().Reducer(State.Vector, Action);
  Next.Dependencies = StoreInternal::GetDependenciesSlice().Reducer(State.Dependencies, Action);
  Next.CLI = StoreInternal::GetCLISlice().Reducer(State.CLI, Action);
  Next.CLIPresentation = StoreInternal::GetCLIPresentationSlice().Reducer(
      State.CLIPresentation, Action);
  Next.Config = StoreInternal::GetConfigSlice().Reducer(State.Config, Action);

  /**
   * G8: Run extra reducers (game slices) — recursive application.
   * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
   */
  return [&]() -> FRuntimeState {
    struct ApplyReducers {
      static FRuntimeState apply(FRuntimeState S, const rtk::AnyAction &A,
                               const std::vector<ExtraReducerFn> &Rs,
                               size_t Index) {
        return Index >= Rs.size() ? S
                                 : apply(Rs[Index](S, A), A, Rs, Index + 1);
      }
    };
    return ApplyReducers::apply(Next, Action, StoreInternal::ExtraReducers(),
                                0);
  }();
}

/**
 * Builds middleware that clears dependent state when an NPC is removed.
 * User Story: As NPC teardown, I need dependent slices cleaned up
 * automatically so removed NPCs do not leave stale state behind.
 */
/**
 * G8: Register an extra reducer before store creation.
 * User Story: As game integration, I need a registration hook so custom game
 * reducers can extend the store without replacing SDK behavior.
 * Game slices call this to mount their reducers alongside SDK slices.
 *
 * Example:
 * addExtraReducer([](const FRuntimeState &S, const rtk::AnyAction &A) {
 * FRuntimeState Next = S;
 * if (A.Type == TEXT("game/setScore")) {
 * Next.Extra.Add(TEXT("score"), A.Type);
 * }
 * return Next;
 * });
 * @fn inline void addExtraReducer(const ExtraReducerFn &Reducer)
 */
inline void addExtraReducer(const ExtraReducerFn &Reducer) {
  StoreInternal::ExtraReducers().push_back(Reducer);
}

/**
 * Creates a store with optional preloaded state and additional middleware.
 * User Story: As runtime bootstrap, I need a configurable store factory so
 * tests and games can start from custom state and middleware.
 * @fn inline rtk::EnhancedStore<FRuntimeState> createRuntimeStore(func::Maybe<FRuntimeState> PreloadedState = func::nothing<FRuntimeState>(), std::vector<rtk::Middleware<FRuntimeState>> ExtraMiddlewares = {})
 */
inline rtk::EnhancedStore<FRuntimeState>
createRuntimeStore(func::Maybe<FRuntimeState> PreloadedState =
                       func::nothing<FRuntimeState>(),
                   std::vector<rtk::Middleware<FRuntimeState>> ExtraMiddlewares =
                       {}) {
  std::vector<rtk::Middleware<FRuntimeState>> Middlewares;
  Middlewares.push_back(
      LoggerListeners::createProtocolLoggerMiddleware<FRuntimeState>());
  Middlewares.push_back(NPCListeners::createNpcRemovalListener<FRuntimeState>());

  /**
   * G8: Append game-provided middleware — recursive merge.
   * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
   */
  struct AppendMiddlewares {
    static void apply(std::vector<rtk::Middleware<FRuntimeState>> &Target,
                      const std::vector<rtk::Middleware<FRuntimeState>> &Source,
                      size_t Index) {
      Index < Source.size()
          ? (Target.push_back(Source[Index]),
             apply(Target, Source, Index + 1), void())
          : void();
    }
  };
  AppendMiddlewares::apply(Middlewares, ExtraMiddlewares, 0);

  return rtk::configureStore<FRuntimeState>(
      &StoreReducer,
      PreloadedState.hasValue ? PreloadedState.value
                              : StoreInternal::createRuntimeInitialState(),
      Middlewares);
}

/**
 * Returns the process-wide singleton runtime store.
 * User Story: As shared runtime access, I need a singleton store so Blueprint,
 * CLI, and subsystem helpers all dispatch through the same state container.
 * @fn inline rtk::EnhancedStore<FRuntimeState> &store()
 */
inline rtk::EnhancedStore<FRuntimeState> &store() {
  static rtk::EnhancedStore<FRuntimeState> GlobalStore = createRuntimeStore();
  return GlobalStore;
}
