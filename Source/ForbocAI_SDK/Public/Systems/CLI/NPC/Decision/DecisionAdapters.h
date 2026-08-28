#pragma once

#include "Components/CLI/NPC/CLINPCTypes.h"
#include "Components/Protocol/Process/ProtocolProcessTypes.h"
#include "Core/fp.hpp"

namespace ForbocAI::CLI::NPC {

/** User Story: As a thin decision CLI, I need one option description composed from authored vocabulary. @fn inline FCLIOptionSpec optionSpec(const FString &Option, const FCLINPCState &State) */
inline FCLIOptionSpec optionSpec(const FString &Option,
                                 const FCLINPCState &State) {
  return {Option, State.Limits.ValueOffset, State.Syntax.OptionPrefix};
}

/** User Story: As a thin decision CLI, I need one optional value removed without consuming a following option. @fn inline FCLIOptionExtraction extractOptionValue(const TArray<FString> &Args, const FCLIOptionSpec &Spec) */
inline FCLIOptionExtraction
extractOptionValue(const TArray<FString> &Args,
                   const FCLIOptionSpec &Spec) {
  const int32 OptionIndex = Args.IndexOfByKey(Spec.Option);
  const bool bHasOption = OptionIndex != INDEX_NONE;
  const int32 ValueIndex = OptionIndex + Spec.ValueOffset;
  const bool bHasValue =
      bHasOption && Args.IsValidIndex(ValueIndex) &&
      !Args[ValueIndex].StartsWith(Spec.OptionPrefix);
  FCLIOptionExtraction Result;
  Result.Value = bHasValue ? func::just(Args[ValueIndex])
                          : func::nothing<FString>();
  const int32 AfterIndex = bHasValue ? ValueIndex + Spec.ValueOffset
                                     : OptionIndex + Spec.ValueOffset;
  Result.Rest = !bHasOption
                    ? Args
                    : func::append_values<FString>(
                          func::slice_array<FString>(Args, int32{},
                                                     OptionIndex),
                          func::slice_array<FString>(
                              Args, AfterIndex, Args.Num() - AfterIndex));
  return Result;
}

/** User Story: As a constrained decision caller, I need a comma-delimited option value normalized into non-empty actions. @fn inline TArray<FString> parseActionOptions(const func::Maybe<FString> &Raw, const FCLINPCState &State) */
inline TArray<FString>
parseActionOptions(const func::Maybe<FString> &Raw,
                   const FCLINPCState &State) {
  return func::match(
      Raw,
      [&State](const FString &Value) {
        TArray<FString> Values;
        Value.ParseIntoArray(Values, *State.Syntax.LegalActionsSeparator,
                             true);
        return func::filter_array<FString>(
            func::map_array<FString, FString>(
                Values, [](const FString &Item) {
                  return Item.TrimStartAndEnd();
                }),
            [](const FString &Item) { return !Item.IsEmpty(); });
      },
      []() { return TArray<FString>(); });
}

/** User Story: As NPC and Ghost process commands, I need one profile decoder so both remain identical pass-throughs. @fn inline FCLINPCDecision decodeNpcProcessCommand(const TArray<FString> &Args, const FCLINPCState &State) */
inline FCLINPCDecision
decodeNpcProcessCommand(const TArray<FString> &Args,
                        const FCLINPCState &State) {
  const FCLIOptionExtraction Profile =
      extractOptionValue(Args, optionSpec(State.Messages.ProfileOption, State));
  FCLINPCDecision Decision;
  Decision.NpcId = Profile.Rest.IsValidIndex(State.Limits.FirstArgumentIndex)
                       ? Profile.Rest[State.Limits.FirstArgumentIndex]
                       : FString();
  Decision.Observation =
      Profile.Rest.IsValidIndex(State.Limits.FirstArgumentIndex +
                                State.Limits.ValueOffset)
          ? Profile.Rest[State.Limits.FirstArgumentIndex +
                         State.Limits.ValueOffset]
          : FString();
  Decision.bHasProfile = Profile.Value.hasValue;
  Decision.Profile = Profile.Value.hasValue ? Profile.Value.value : FString();
  return Decision;
}

/** User Story: As NPC and Ghost decide commands, I need one option decoder so both send identical process tapes. @fn inline FCLINPCDecision decodeNpcDecisionCommand(const TArray<FString> &Args, const FCLINPCState &State) */
inline FCLINPCDecision
decodeNpcDecisionCommand(const TArray<FString> &Args,
                         const FCLINPCState &State) {
  const FCLIOptionExtraction Profile =
      extractOptionValue(Args, optionSpec(State.Messages.ProfileOption, State));
  const FCLIOptionExtraction Legal = extractOptionValue(
      Profile.Rest, optionSpec(State.Syntax.LegalActionsOption, State));
  const FCLIOptionExtraction Visited = extractOptionValue(
      Legal.Rest, optionSpec(State.Syntax.VisitedActionsOption, State));
  const FCLIOptionExtraction Avoid = extractOptionValue(
      Visited.Rest, optionSpec(State.Syntax.AvoidActionsOption, State));
  FCLINPCDecision Decision = decodeNpcProcessCommand(Avoid.Rest, State);
  Decision.bHasProfile = Profile.Value.hasValue;
  Decision.Profile = Profile.Value.hasValue ? Profile.Value.value : FString();
  Decision.LegalActions = parseActionOptions(Legal.Value, State);
  Decision.bHasLegalActions = Legal.Value.hasValue;
  Decision.VisitedActions = parseActionOptions(Visited.Value, State);
  Decision.bHasVisitedActions = Visited.Value.hasValue;
  Decision.AvoidActions = parseActionOptions(Avoid.Value, State);
  Decision.bHasAvoidActions = Avoid.Value.hasValue;
  return Decision;
}

/** User Story: As the SDK protocol boundary, I need decoded CLI values projected without policy into one process input. @fn inline FProtocolProcessInput toProtocolProcessInput(const FCLINPCDecision &Decision) */
inline FProtocolProcessInput
toProtocolProcessInput(const FCLINPCDecision &Decision) {
  FProtocolProcessInput Input;
  Input.NpcId = Decision.NpcId;
  Input.Observation = Decision.Observation;
  Input.bHasThoughtProfile = Decision.bHasProfile;
  Input.ThoughtProfile = Decision.Profile;
  Input.bHasLegalActions = Decision.bHasLegalActions;
  Input.LegalActions = Decision.LegalActions;
  Input.bHasVisitedActions = Decision.bHasVisitedActions;
  Input.VisitedActions = Decision.VisitedActions;
  Input.bHasAvoidActions = Decision.bHasAvoidActions;
  Input.AvoidActions = Decision.AvoidActions;
  return Input;
}

} // namespace ForbocAI::CLI::NPC
