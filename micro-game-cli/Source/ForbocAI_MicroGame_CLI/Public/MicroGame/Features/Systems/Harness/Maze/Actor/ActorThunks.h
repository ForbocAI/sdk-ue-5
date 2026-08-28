#pragma once

#include "MicroGame/Features/Systems/Harness/Maze/Actor/ActorAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Command/MazeCommandAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Response/MazeResponseAdapters.h"
#include "MicroGame/Features/Systems/Harness/Maze/Run/Command/RunCommandThunks.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"

namespace MicroGame::Maze {

/**
 * User Story: As Maze persona construction, I need attributes generated recursively with each successful value threaded into the next request.
 * @fn inline FMazeAttributesResult GenerateMazeAttributes( FMicroGameStore &Store, const FMazeRunOptions &Options, const FMazeCommands &Commands, int32 Index, const FString &Context, TArray<FString> Values)
 */
inline FMazeAttributesResult GenerateMazeAttributes(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FMazeCommands &Commands, int32 Index, const FString &Context,
    TArray<FString> Values) {
  return Index >= MazeConfig().PersonaAttributes.Num()
             ? FMazeAttributesResult{true, Values,
                                     MazeConfig().EmptyContext}
             : [&]() {
                 const FMazeCommandResult Result = RunMazeCommand(
                     Store, Options,
                     FormatMazeGenerate(
                         Commands, MazeConfig().PersonaAttributes[Index],
                         Context));
                 const FString Value = Result.bSuccess
                                           ? ReadMazeGeneratedValue(
                                                 Result.Output.Output)
                                           : MazeConfig().EmptyContext;
                 return !Result.bSuccess
                            ? FMazeAttributesResult{false, Values,
                                                    Result.Failure}
                        : Value.IsEmpty()
                            ? FMazeAttributesResult{
                                  false, Values,
                                  MazeConfig().GeneratedValueMissing}
                            : (Values.Add(Value),
                               GenerateMazeAttributes(
                                   Store, Options, Commands,
                                   Index + MazeConfig().Numbers.One,
                                   ExtendMazeContext(Context, Value),
                                   MoveTemp(Values)));
               }();
}

/**
 * User Story: As Maze actor setup, I need one generated actor created, named, assigned a role, and grounded with scene memory through granular CLI operations.
 * @fn inline FMazeNpcResult BuildMazeNpc( FMicroGameStore &Store, const FMazeRunOptions &Options, const FMazeCommands &Commands)
 */
inline FMazeNpcResult BuildMazeNpc(
    FMicroGameStore &Store, const FMazeRunOptions &Options,
    const FMazeCommands &Commands) {
  const FMazeCommandResult NameResult = RunMazeCommand(
      Store, Options,
      FormatMazeGenerate(Commands, MazeConfig().NameAttribute,
                         MazeConfig().SceneText));
  const FString Name = NameResult.bSuccess
                           ? ReadMazeGeneratedValue(
                                 NameResult.Output.Output)
                           : MazeConfig().EmptyContext;
  return CLIOps::CommandRouting::matchCondition<FMazeNpcResult>(
      NameResult.bSuccess && !Name.IsEmpty(),
      [&]() {
        const FMazeAttributesResult Attributes = GenerateMazeAttributes(
            Store, Options, Commands, MazeConfig().Numbers.Zero,
            MazeConfig().SceneText, {});
        return CLIOps::CommandRouting::matchCondition<FMazeNpcResult>(
            Attributes.bSuccess,
            [&]() {
              const FMazeCommandResult Created = RunMazeCommand(
                  Store, Options,
                  FormatMazeCreate(
                      Commands, JoinMazePersona(Attributes.Values)));
              const FString Id =
                  Created.bSuccess
                      ? ReadMazeActorId(Created.Output.Output)
                      : MazeConfig().EmptyContext;
              return CLIOps::CommandRouting::matchCondition<FMazeNpcResult>(
                  Created.bSuccess && !Id.IsEmpty(),
                  [&]() {
                    const FString Role =
                        SelectMazeRole(Attributes.Values);
                    const FMazeCommandResult Updated = RunMazeCommand(
                        Store, Options,
                        FormatMazeIdentityUpdate(Commands, Id, Name, Role));
                    return CLIOps::CommandRouting::matchCondition<
                        FMazeNpcResult>(
                        Updated.bSuccess,
                        [&]() {
                          const FMazeCommandResult Grounded = RunMazeCommand(
                              Store, Options,
                              FormatMazeTypedMemoryStore(
                                  Commands, Id, MazeConfig().SceneText,
                                  MazeConfig().SceneMemoryType));
                          return Grounded.bSuccess
                                     ? FMazeNpcResult{
                                           true,
                                           {Id, FormatMazeDisplayName(Name,
                                                                      Role)},
                                           MazeConfig().EmptyContext}
                                     : FMazeNpcResult{false, {},
                                                      Grounded.Failure};
                        },
                        [&]() {
                          return FMazeNpcResult{false, {}, Updated.Failure};
                        });
                  },
                  [&]() {
                    return FMazeNpcResult{
                        false, {},
                        Created.bSuccess ? MazeConfig().CreatedIdMissing
                                         : Created.Failure};
                  });
            },
            [&]() {
              return FMazeNpcResult{false, {}, Attributes.Failure};
            });
      },
      [&]() {
        return FMazeNpcResult{
            false, {}, NameResult.bSuccess
                           ? MazeConfig().GeneratedValueMissing
                           : NameResult.Failure};
      });
}

} // namespace MicroGame::Maze
