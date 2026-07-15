#pragma once

#include "Features/Bridge/BridgeSlice.h"
#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Protocol/Logger/LoggerListeners.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/NPC/NPCListeners.h"
#include "Features/Dependencies/DependenciesSlice.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Vector/VectorSlice.h"

struct FRuntimeState {
  NPCSlice::FNPCSliceState NPCs;
  MemorySlice::FMemorySliceState Memory;
  DirectiveSlice::FDirectiveSliceState Directives;
  BridgeSlice::FBridgeSliceState Bridge;
  SoulSlice::FSoulSliceState Soul;
  GhostSlice::FGhostSliceState Ghost;
  FVectorState Vector;
  FDependenciesState Dependencies;

  /**
   * G8: Generic state bag for game-specific slices.
   * User Story: As game-specific runtime extensions, I need a shared extra bag
   * so custom slice state can live beside SDK-managed state.
   * Games can store serialized state keyed by slice name.
   * Extra reducers operate on this map alongside SDK reducers.
   */
  TMap<FString, FString> Extra;
};

namespace StoreInternal {

/**
 * Returns the singleton NPC slice definition.
 * User Story: As store assembly, I need one shared NPC slice instance so every
 * store uses the same reducer wiring.
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
 */
inline const rtk::Slice<GhostSlice::FGhostSliceState> &GetGhostSlice() {
  static const func::Lazy<rtk::Slice<GhostSlice::FGhostSliceState>> Slice =
      func::lazy([]() -> rtk::Slice<GhostSlice::FGhostSliceState> { return GhostSlice::createGhostSlice(); });
  return func::eval(Slice);
}

inline const rtk::Slice<FVectorState> &GetVectorSlice() {
  static const func::Lazy<rtk::Slice<FVectorState>> Slice = func::lazy(
      []() { return VectorSlice::createVectorSlice(); });
  return func::eval(Slice);
}

inline const rtk::Slice<FDependenciesState> &GetDependenciesSlice() {
  static const func::Lazy<rtk::Slice<FDependenciesState>> Slice =
      func::lazy([]() { return DependenciesSlice::createDependenciesSlice(); });
  return func::eval(Slice);
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
 *   addExtraReducer([](const FRuntimeState &S, const rtk::AnyAction &A) {
 *       FRuntimeState Next = S;
 *       if (A.Type == TEXT("game/setScore")) {
 *           Next.Extra.Add(TEXT("score"), A.Type);
 *       }
 *       return Next;
 *   });
 */
inline void addExtraReducer(const ExtraReducerFn &Reducer) {
  StoreInternal::ExtraReducers().push_back(Reducer);
}

/**
 * Creates a store with optional preloaded state and additional middleware.
 * User Story: As runtime bootstrap, I need a configurable store factory so
 * tests and games can start from custom state and middleware.
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
      PreloadedState.hasValue ? PreloadedState.value : FRuntimeState(),
      Middlewares);
}

/**
 * Returns the process-wide singleton runtime store.
 * User Story: As shared runtime access, I need a singleton store so Blueprint,
 * CLI, and subsystem helpers all dispatch through the same state container.
 */
inline rtk::EnhancedStore<FRuntimeState> store() {
  static rtk::EnhancedStore<FRuntimeState> GlobalStore = createRuntimeStore();
  return GlobalStore;
}
