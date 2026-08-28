#pragma once

#include "Core/rtk.hpp"
#include "Components/Ghost/Operations/GhostOperationsTypes.h"
#include "Systems/Actor/Local/ActorLocalThunks.h"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/API/Endpoints/Ghost/GhostApi.h"
#include "Systems/Ghost/GhostThunks.h"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Local/MemoryLocalThunks.h"
#include "Systems/Protocol/Handlers/HandlersThunks.h"
#include "Systems/Protocol/Process/ProcessThunks.h"

struct FRuntimeState;

namespace Ops {

/** User Story: As a session-bound Ghost SDK operation, I need one C++14-compatible callable that validates the API-owned Ghost session before SDK-owned effects execute. */
template <typename RuntimeState>
struct FGhostSessionBinding {
  rtk::EnhancedStore<RuntimeState> &Store;
  FString SessionId;

  /** User Story: As a bound Ghost operation, I need validation composed immediately before the requested effect. @fn template <typename Operation> auto operator()(Operation Execute) const -> decltype(Execute()) */
  template <typename Operation>
  auto operator()(Operation Execute) const -> decltype(Execute()) {
    AsyncAdapters::waitForResult(
        Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
    return Execute();
  }
};

/** User Story: As every session-bound Ghost SDK operation, I need the API-owned Ghost session bound once before SDK-owned actor or vector effects execute. @fn template <typename RuntimeState = FRuntimeState> inline FGhostSessionBinding<RuntimeState> withGhostSession(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostSessionBinding<RuntimeState>
withGhostSession(rtk::EnhancedStore<RuntimeState> &Store,
                 const FString &SessionId) {
  return FGhostSessionBinding<RuntimeState>{Store, SessionId};
}

/** User Story: As a Ghost CLI consumer, I need session creation dispatched through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &TestSuite, int32 Duration) */
template <typename RuntimeState = FRuntimeState>
inline FGhostRunResponse startGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &TestSuite,
                                    int32 Duration) {
  FGhostConfig Config;
  Config.TestSuite = TestSuite;
  Config.Duration = Duration;
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::startGhostThunk(Config)));
}

/** User Story: As a Ghost CLI consumer, I need session status selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStatus getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStatus
getGhostStatus(rtk::EnhancedStore<RuntimeState> &Store,
               const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
}

/**
 * User Story: As Ghost attribute generation, I need one SDK call whose only generation transport is the Ghost generation endpoint.
 * @fn template <typename RuntimeState = FRuntimeState> inline FNpcAttributeGenerateResponse generateGhostAttribute( rtk::EnhancedStore<RuntimeState> &Store, const FGhostAttributeGenerateInput &Input)
 */
template <typename RuntimeState = FRuntimeState>
inline FNpcAttributeGenerateResponse generateGhostAttribute(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostAttributeGenerateInput &Input) {
  return withGhostSession(Store, Input.SessionId)([&Store, &Input]() {
    FNpcAttributeGenerateRequest Request;
    Request.Context = Input.Context;
    return AsyncAdapters::waitForResult(Store.dispatch(
        APISlice::Endpoints::postGhostNpcGenerateAttribute(
            Input.SessionId, Input.Attribute, Request)));
  });
}

/**
 * User Story: As Ghost actor setup, I need one Ghost SDK operation that validates through the Ghost API before composing neutral SDK-owned actor storage.
 * @fn template <typename RuntimeState = FRuntimeState> inline FNPCInternalState createGhostActor( rtk::EnhancedStore<RuntimeState> &Store, const FGhostActorCreateInput &Input)
 */
template <typename RuntimeState = FRuntimeState>
inline FNPCInternalState createGhostActor(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostActorCreateInput &Input) {
  return withGhostSession(Store, Input.SessionId)(
      [&Store, &Input]() {
        return LocalActor::createActor(Store, Input.Persona);
      });
}

/**
 * User Story: As Ghost actor state, I need one Ghost SDK operation that validates through the Ghost API before composing neutral ECS target resolution and reduction.
 * @fn template <typename RuntimeState = FRuntimeState> inline FActorUpdateResult updateGhostActor( rtk::EnhancedStore<RuntimeState> &Store, const FGhostActorUpdateInput &Input)
 */
template <typename RuntimeState = FRuntimeState>
inline FActorUpdateResult updateGhostActor(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostActorUpdateInput &Input) {
  return withGhostSession(Store, Input.SessionId)([&Store, &Input]() {
    return LocalActor::updateActor(Store, Input.Update);
  });
}

/**
 * User Story: As Ghost actor hydration, I need one Ghost SDK operation that validates through the Ghost API before executing neutral vector recall.
 * @fn template <typename RuntimeState = FRuntimeState> inline func::Maybe<FNPCInternalState> recallGhostActor( rtk::EnhancedStore<RuntimeState> &Store, const FGhostActorRecallInput &Input)
 */
template <typename RuntimeState = FRuntimeState>
inline func::Maybe<FNPCInternalState> recallGhostActor(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostActorRecallInput &Input) {
  return withGhostSession(Store, Input.SessionId)(
      [&Store, &Input]() {
        return LocalActor::recallActor(Store, Input.ActorId);
      });
}

/**
 * User Story: As Ghost vector evidence, I need one Ghost SDK operation that validates through the Ghost API before the SDK executes vector storage.
 * @fn template <typename RuntimeState = FRuntimeState> inline FMemoryItem storeGhostMemory( rtk::EnhancedStore<RuntimeState> &Store, const FGhostMemoryStoreInput &Input)
 */
template <typename RuntimeState = FRuntimeState>
inline FMemoryItem storeGhostMemory(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostMemoryStoreInput &Input) {
  return withGhostSession(Store, Input.SessionId)([&Store, &Input]() {
    const auto &Defaults = MemoryConfiguration::memoryData().Defaults;
    return AsyncAdapters::waitForResult(Store.dispatch(
        rtk::storeNodeMemoryThunk(
            Input.Observation,
            Input.Type.IsEmpty() ? Defaults.MemoryType : Input.Type,
            Defaults.Importance, Input.ActorId)));
  });
}

/**
 * User Story: As Ghost decision making, I need the shared tape interpreter bound to the Ghost process endpoint with actor-scoped vector execution available for API instructions.
 * @fn template <typename RuntimeState = FRuntimeState> inline FAgentResponse decideGhost( rtk::EnhancedStore<RuntimeState> &Store, const FGhostProcessInput &RequestedInput)
 */
template <typename RuntimeState = FRuntimeState>
inline FAgentResponse decideGhost(
    rtk::EnhancedStore<RuntimeState> &Store,
    const FGhostProcessInput &RequestedInput) {
  return withGhostSession(Store, RequestedInput.SessionId)(
      [&Store, &RequestedInput]() {
        FGhostProcessInput Input = RequestedInput;
        const FNPCInternalState Actor = func::requireJust(
            LocalActor::ensureActor(Store, Input.Process.NpcId),
            std::string(TCHAR_TO_UTF8(
                *ForbocAI::CLI::NPC::readCliNpcState().Messages.NotFound)));
        Input.Process.Persona = Actor.Persona;
        return AsyncAdapters::waitForResult(Store.dispatch(
            rtk::processGhost(Input,
                              rtk::LocalProtocolHandlerContext(
                                  Input.Process.NpcId))));
      });
}

/** User Story: As a Ghost CLI consumer, I need session results selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostResults getGhostResults(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostResults
getGhostResults(rtk::EnhancedStore<RuntimeState> &Store,
                const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostResultsThunk(SessionId)));
}

/** User Story: As a Ghost CLI consumer, I need a session stop request dispatched through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store, const FString &SessionId) */
template <typename RuntimeState = FRuntimeState>
inline FGhostStopResponse stopGhost(rtk::EnhancedStore<RuntimeState> &Store,
                                    const FString &SessionId) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::stopGhostThunk(SessionId)));
}

/** User Story: As a Ghost CLI consumer, I need bounded session history selected through the package root store. @fn template <typename RuntimeState = FRuntimeState> inline TArray<FGhostHistoryEntry> getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit) */
template <typename RuntimeState = FRuntimeState>
inline TArray<FGhostHistoryEntry>
getGhostHistory(rtk::EnhancedStore<RuntimeState> &Store, int32 Limit) {
  return AsyncAdapters::waitForResult(
      Store.dispatch(rtk::getGhostHistoryThunk(Limit)));
}

} // namespace Ops
