#pragma once

#include "Systems/API/Endpoints/NPC/Generate/GenerateAdapters.h"
#include "Systems/CLI/Ghost/CLIGhostThunks.h"
#include "Systems/CLI/GhostCommands/Session/GhostSessionCommandAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace CLIOps::GhostCommands {

/** User Story: As Ghost persona composition, I need one attribute request delegated only to the Ghost SDK generation operation. @fn inline GhostResult GenerateGhostAttribute(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second, int32 Next) */
inline GhostResult GenerateGhostAttribute(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second, int32 Next) {
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        return CLIOps::CommandRouting::matchCondition<GhostResult>(
            Args.Num() > Second,
            [&]() {
              const int32 ContextIndex = Second + Next;
              const FString Context =
                  Args.Num() > ContextIndex
                      ? ForbocAI::CLI::NPC::joinNpcArguments(
                            Args, ContextIndex,
                            ActorState.Syntax.MessageSeparator)
                      : FString();
              const FNpcAttributeGenerateResponse Response =
                  Ops::generateGhostAttribute(
                      Store, FGhostAttributeGenerateInput{
                                 SessionId, Args[Second], Context});
              ForbocAI::CLI::Presentation::logCliMessage(
                  APISlice::Detail::EncodeNpcAttributeGenerateResponse(
                      Response));
              return CLIOps::CommandResult::Success(Response.Value);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  ActorState.Messages.GenerateUsage);
            });
      });
}

/** User Story: As Ghost actor setup, I need one persona delegated only to the Ghost SDK actor creation operation. @fn inline GhostResult CreateGhostActor(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second) */
inline GhostResult CreateGhostActor(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        return CLIOps::CommandRouting::matchCondition<GhostResult>(
            Args.Num() > Second && !Args[Second].TrimStartAndEnd().IsEmpty(),
            [&]() {
              ForbocAI::CLI::Presentation::logCliMessage(
                  formatCliMessage(ActorState.Messages.Creating,
                                   Args[Second]));
              const FNPCInternalState Actor = Ops::createGhostActor(
                  Store, FGhostActorCreateInput{SessionId, Args[Second]});
              ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
                  ActorState.Messages.Created,
                  Actor.Id.IsEmpty() ? ActorState.Messages.UnknownId
                                     : Actor.Id));
              ForbocAI::CLI::Presentation::logCliMessage(
                  formatCliMessage(ActorState.Messages.State,
                                   Actor.State.JsonData));
              return CLIOps::CommandResult::Success(Actor.Id);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  ActorState.Messages.CreateUsage);
            });
      });
}

/** User Story: As Ghost identity parsing, I need Ghost-facing flags translated into the shared actor vocabulary without changing the caller input. @fn inline TArray<FString> TranslateGhostUpdateArguments(const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, int32 Second) */
inline TArray<FString> TranslateGhostUpdateArguments(
    const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    int32 Second) {
  return func::map_array<FString, FString>(
      func::slice_array<FString>(Args, Second, Args.Num() - Second),
      [&GhostState](const FString &Argument) {
        return Argument == GhostState.NameOption
                   ? GhostState.StateNameOption
               : Argument == GhostState.RoleOption
                   ? GhostState.StateRoleOption
                   : Argument;
      });
}

/** User Story: As Ghost actor update presentation, I need one SDK result projected without reinterpreting its state transition. @fn inline GhostResult PresentGhostActorUpdate(const FActorUpdateResult &Updated, const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded, const ForbocAI::CLI::NPC::FCLINPCState &ActorState) */
inline GhostResult PresentGhostActorUpdate(
    const FActorUpdateResult &Updated,
    const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return func::match(
      Updated.TargetActorId,
      [&](const FString &TargetId) {
        return func::match(
            Updated.Actor,
            [&](const FNPCInternalState &) {
              ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
                  ActorState.Messages.Updated, TargetId,
                  Decoded.Delta.JsonData));
              return CLIOps::CommandResult::Success(
                  ActorState.Messages.UpdateDone);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(formatCliMessage(
                  ActorState.Messages.NotFound, TargetId));
            });
      },
      [&]() {
        return CLIOps::CommandResult::Failure(ActorState.Messages.MissingId);
      });
}

/** User Story: As Ghost identity setup, I need decoded state delegated to one Ghost SDK actor update operation. @fn inline GhostResult ApplyGhostActorUpdate(rtk::EnhancedStore<FRuntimeState> &Store, const FString &SessionId, const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded, const ForbocAI::CLI::NPC::FCLINPCState &ActorState) */
inline GhostResult ApplyGhostActorUpdate(
    rtk::EnhancedStore<FRuntimeState> &Store, const FString &SessionId,
    const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState) {
  return PresentGhostActorUpdate(
      Ops::updateGhostActor(
          Store, FGhostActorUpdateInput{
                     SessionId,
                     FActorUpdateInput{Decoded.RequestedNpcId, Decoded.Delta}}),
      Decoded, ActorState);
}

/** User Story: As Ghost identity setup, I need Ghost-facing identity flags translated before one Ghost SDK actor update operation. @fn inline GhostResult UpdateGhostActor(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second) */
inline GhostResult UpdateGhostActor(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second) {
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        return func::match(
            ForbocAI::CLI::NPC::decodeNpcUpdate(
                TranslateGhostUpdateArguments(Args, GhostState, Second),
                ActorState),
            [&](const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded) {
              return ApplyGhostActorUpdate(Store, SessionId, Decoded,
                                           ActorState);
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  ActorState.Messages.UpdateUsage);
            });
      });
}

/** User Story: As Ghost actor hydration, I need one actor identifier delegated only to the Ghost SDK recall operation. @fn inline GhostResult RecallGhostActor(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second) */
inline GhostResult RecallGhostActor(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return WithRequiredSession(
      Args, GhostState, First,
      [&](const FString &SessionId) {
        return CLIOps::CommandRouting::matchCondition<GhostResult>(
            Args.Num() > Second,
            [&]() {
              return func::match(
                  Ops::recallGhostActor(
                      Store,
                      FGhostActorRecallInput{SessionId, Args[Second]}),
                  [&](const FNPCInternalState &Actor) {
                    ForbocAI::CLI::Presentation::logCliMessage(
                        formatCliMessage(ActorState.Messages.Recalled,
                                         Actor.Id));
                    ForbocAI::CLI::Presentation::logCliMessage(
                        formatCliMessage(ActorState.Messages.Persona,
                                         Actor.Persona));
                    return CLIOps::CommandResult::Success(Actor.Id);
                  },
                  [&]() {
                    return CLIOps::CommandResult::Failure(formatCliMessage(
                        ActorState.Messages.NotFound, Args[Second]));
                  });
            },
            [&]() {
              return CLIOps::CommandResult::Failure(
                  ActorState.Messages.RecallUsage);
            });
      });
}

/** User Story: As Ghost actor routing, I need actor command roles composed as a focused immutable dispatcher. @fn inline func::Dispatcher<FString, GhostResult> CreateGhostActorDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState, const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First, int32 Second, int32 Next) */
inline func::Dispatcher<FString, GhostResult> CreateGhostActorDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const TArray<FString> &Args,
    const ForbocAI::CLI::Ghost::FCLIGhostState &GhostState,
    const ForbocAI::CLI::NPC::FCLINPCState &ActorState, int32 First,
    int32 Second, int32 Next) {
  return func::createDispatcher<FString, GhostResult>({
      {Roles.GhostGenerate,
       [&]() { return GenerateGhostAttribute(Store, Args, GhostState, ActorState,
                                              First, Second, Next); }},
      {Roles.GhostCreate,
       [&]() { return CreateGhostActor(Store, Args, GhostState, ActorState,
                                       First, Second); }},
      {Roles.GhostUpdate,
       [&]() { return UpdateGhostActor(Store, Args, GhostState, ActorState,
                                       First, Second); }},
      {Roles.GhostRecall,
       [&]() { return RecallGhostActor(Store, Args, GhostState, ActorState,
                                       First, Second); }},
  });
}

} // namespace CLIOps::GhostCommands
