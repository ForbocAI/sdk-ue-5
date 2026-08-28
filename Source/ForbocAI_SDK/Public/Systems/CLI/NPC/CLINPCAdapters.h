#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Components/CLI/NPC/CLINPCTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ForbocAI {
namespace CLI {
namespace NPC {

/** User Story: As a features cli npc consumer, I need to invoke read cli npc state through a stable signature so the features cli npc workflow remains explicit and composable. @fn inline FCLINPCState readCliNpcState() */
inline FCLINPCState readCliNpcState() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/cli/npc.json"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Syntax =
      DataAdapters::ReadObjectField(Source, TEXT("syntax"));
  const TSharedRef<FJsonObject> Analysis =
      DataAdapters::ReadObjectField(Source, TEXT("analysis"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {
      {DataAdapters::ReadNumberField(Limits, TEXT("emptyArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("singleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("doubleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("updateArgumentCount")),
       DataAdapters::ReadNumberField(Limits,
                                     TEXT("activeUpdateArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("firstArgumentIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("firstUpdateIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("activeUpdateIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("pairStride")),
       DataAdapters::ReadNumberField(Limits, TEXT("keyOffset")),
       DataAdapters::ReadNumberField(Limits, TEXT("valueOffset"))},
      {DataAdapters::ReadStringField(Syntax, TEXT("optionPrefix")),
       DataAdapters::ReadStringField(Syntax, TEXT("textOption")),
       DataAdapters::ReadStringField(Syntax, TEXT("legalActionsOption")),
       DataAdapters::ReadStringField(Syntax, TEXT("visitedActionsOption")),
       DataAdapters::ReadStringField(Syntax, TEXT("avoidActionsOption")),
       DataAdapters::ReadStringField(Syntax, TEXT("personaMemoryType")),
       DataAdapters::ReadStringField(Syntax, TEXT("legalActionsSeparator")),
       DataAdapters::ReadStringField(Syntax, TEXT("messageSeparator")),
       DataAdapters::ReadStringField(Syntax, TEXT("personaPattern")),
       DataAdapters::ReadStringField(Syntax, TEXT("formatOpenToken")),
       DataAdapters::ReadStringField(Syntax, TEXT("formatCloseToken"))},
      {DataAdapters::ReadStringField(Analysis, TEXT("diagnosisContainer")),
       DataAdapters::ReadStringField(Analysis,
                                     TEXT("diagnosticResultField"))},
      {DataAdapters::ReadStringField(Messages, TEXT("createUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("creating")),
       DataAdapters::ReadStringField(Messages, TEXT("unknownId")),
       DataAdapters::ReadStringField(Messages, TEXT("created")),
       DataAdapters::ReadStringField(Messages, TEXT("id")),
       DataAdapters::ReadStringField(Messages, TEXT("state")),
       DataAdapters::ReadStringField(Messages, TEXT("noActive")),
       DataAdapters::ReadStringField(Messages, TEXT("missingId")),
       DataAdapters::ReadStringField(Messages, TEXT("notFound")),
       DataAdapters::ReadStringField(Messages, TEXT("updateUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("updated")),
       DataAdapters::ReadStringField(Messages, TEXT("processUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("generateUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("decideUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("recallUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("recalled")),
       DataAdapters::ReadStringField(Messages, TEXT("dialogueFallback")),
       DataAdapters::ReadStringField(Messages, TEXT("dialogue")),
       DataAdapters::ReadStringField(Messages, TEXT("action")),
       DataAdapters::ReadStringField(Messages, TEXT("thoughtResultPrefix")),
       DataAdapters::ReadStringField(Messages,
                                     TEXT("reasoningResultPrefix")),
       DataAdapters::ReadStringField(Messages,
                                     TEXT("diagnosticResultPrefix")),
       DataAdapters::ReadStringField(Messages, TEXT("promptTraceEnvKey")),
       DataAdapters::ReadStringField(Messages, TEXT("promptTraceHeader")),
       DataAdapters::ReadStringField(Messages, TEXT("promptTraceFooter")),
       DataAdapters::ReadStringField(Messages, TEXT("profileOption")),
       DataAdapters::ReadStringField(Messages,
                                     TEXT("thoughtProfileMemoryType")),
       DataAdapters::ReadStringField(Messages,
                                     TEXT("thoughtProfileStateKey")),
       DataAdapters::ReadStringField(Messages, TEXT("chatUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("chatMissingId")),
       DataAdapters::ReadStringField(Messages, TEXT("chatHeader")),
       DataAdapters::ReadStringField(Messages, TEXT("chatUser")),
       DataAdapters::ReadStringField(Messages, TEXT("chatNpc")),
       DataAdapters::ReadStringField(Messages, TEXT("chatAction")),
       DataAdapters::ReadStringArrayField(Messages, TEXT("exitCommands")),
       DataAdapters::ReadStringField(Messages, TEXT("importUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("imported")),
       DataAdapters::ReadStringField(Messages, TEXT("persona")),
       DataAdapters::ReadStringField(Messages, TEXT("importDone")),
       DataAdapters::ReadStringField(Messages, TEXT("statePrinted")),
       DataAdapters::ReadStringField(Messages, TEXT("updateDone")),
       DataAdapters::ReadStringField(Messages, TEXT("emptyPersona")),
       DataAdapters::ReadStringField(Messages, TEXT("apiFailure")),
       DataAdapters::ReadStringArrayField(Messages, TEXT("networkErrors"))}};
}

/** User Story: As a features cli npc consumer, I need to invoke join npc arguments through a stable signature so the features cli npc workflow remains explicit and composable. @fn inline FString joinNpcArguments(const TArray<FString> &Arguments, int32 Index, const FString &Separator) */
inline FString joinNpcArguments(const TArray<FString> &Arguments, int32 Index,
                                const FString &Separator) {
  return Index >= Arguments.Num()
             ? FString()
             : Arguments[Index] +
                   (Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 >= Arguments.Num()
                        ? FString()
                        : Separator + joinNpcArguments(Arguments, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4,
                                                       Separator));
}

/** User Story: As a features cli npc consumer, I need to invoke add update pair through a stable signature so the features cli npc workflow remains explicit and composable. @fn inline bool addUpdatePair(const TArray<FString> &Args, int32 Index, const FCLINPCState &State, const TSharedRef<FJsonObject> &Delta, TArray<FString> &Fields) */
inline bool addUpdatePair(const TArray<FString> &Args, int32 Index,
                          const FCLINPCState &State,
                          const TSharedRef<FJsonObject> &Delta,
                          TArray<FString> &Fields) {
  return Index >= Args.Num()
             ? true
             : [&]() {
                 const FString &Option =
                     Args[Index + State.Limits.KeyOffset];
                 const FString Field =
                     Option.RightChop(State.Syntax.OptionPrefix.Len());
                 return !Option.StartsWith(State.Syntax.OptionPrefix) ||
                                Field.IsEmpty()
                            ? false
                            : (Delta->SetStringField(
                                   Field,
                                   Args[Index + State.Limits.ValueOffset]),
                               Fields.Add(Field),
                               addUpdatePair(
                                   Args, Index + State.Limits.PairStride, State,
                                   Delta, Fields));
               }();
}

/** User Story: As NPC and Ghost update commands, I need supplied identity distinguished from active-actor selection without reading runtime state in the CLI decoder. @fn inline func::Maybe<FCLINPCUpdate> decodeNpcUpdate(const TArray<FString> &Args, const FCLINPCState &State) */
inline func::Maybe<FCLINPCUpdate> decodeNpcUpdate(
    const TArray<FString> &Args, const FCLINPCState &State) {
  return Args.Num() < State.Limits.ActiveUpdateArgumentCount
             ? func::nothing<FCLINPCUpdate>()
             : [&]() -> func::Maybe<FCLINPCUpdate> {
                 const bool bUsesActive =
                     Args[State.Limits.FirstArgumentIndex].StartsWith(
                         State.Syntax.OptionPrefix);
                 const int32 UpdateIndex =
                     bUsesActive ? State.Limits.ActiveUpdateIndex
                                 : State.Limits.FirstUpdateIndex;
                 const int32 RequiredCount =
                     bUsesActive ? State.Limits.ActiveUpdateArgumentCount
                                 : State.Limits.UpdateArgumentCount;
                 return Args.Num() < RequiredCount ||
                                (Args.Num() - UpdateIndex) %
                                        State.Limits.PairStride !=
                                    State.Limits.EmptyArgumentCount
                            ? func::nothing<FCLINPCUpdate>()
                            : [&]() -> func::Maybe<FCLINPCUpdate> {
                                FCLINPCUpdate Update;
                                Update.RequestedNpcId =
                                    bUsesActive
                                        ? func::nothing<FString>()
                                        : func::just(
                                              Args[State.Limits
                                                       .FirstArgumentIndex]);
                                const TSharedRef<FJsonObject> Delta =
                                    MakeShared<FJsonObject>();
                                return !addUpdatePair(
                                           Args, UpdateIndex, State, Delta,
                                           Update.Fields)
                                           ? func::nothing<FCLINPCUpdate>()
                                           : (Update.Delta.JsonData =
                                                  DataAdapters::SerializeObject(
                                                      Delta),
                                              func::just(Update));
                              }();
               }();
}

} // namespace NPC
} // namespace CLI
} // namespace ForbocAI
