#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Chat/ChatAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Parsing/CommandRunnerParsingAdapters.h"
#include "MicroGame/Features/Systems/Quality/QualityTypes.h"

namespace MicroGame::ChatSelectors {

namespace detail {

/** User Story: As a chat selector, I need quality samples searched by authored identifier without introducing duplicate transcript state. @fn inline const FQualitySample *FindSample(const TArray<FQualitySample> &Samples, const FString &Id, int32 Index) */
inline const FQualitySample *FindSample(const TArray<FQualitySample> &Samples,
                                       const FString &Id, int32 Index) {
  return Index >= Samples.Num()
             ? nullptr
         : Samples[Index].Id == Id
             ? &Samples[Index]
             : FindSample(Samples, Id,
                          Index + ChatAdapters::ChatData().Numbers.NextIndex);
}

/** User Story: As a chat selector, I need each turn derived from SDK-CLI quality evidence with explicit availability. @fn inline FChatTranscriptTurn SelectTurn(const func::Maybe<FQualityReport> &Evidence, const FString &Id) */
inline FChatTranscriptTurn
SelectTurn(const func::Maybe<FQualityReport> &Evidence, const FString &Id) {
  const FQualitySample *Sample =
      Evidence.hasValue
          ? FindSample(Evidence.value.Samples, Id,
                       ChatAdapters::ChatData().Numbers.InitialIndex)
          : nullptr;
  const func::Maybe<FString> Prompt =
      Sample == nullptr
          ? func::nothing<FString>()
          : CommandRunner::Parsing::ReadCommandOption(
                Sample->Command, ChatAdapters::ChatData().Command.TextOption);
  return FChatTranscriptTurn{
      Id, Prompt.hasValue ? Prompt.value : ChatAdapters::ChatData().Output.Empty,
      Sample == nullptr ? ChatAdapters::ChatData().Output.Empty
                        : Sample->Response,
      Sample == nullptr ? ChatAdapters::ChatData().Numbers.EmptyCount
                        : Sample->DurationMs,
      Sample != nullptr && Prompt.hasValue && !Prompt.value.IsEmpty() &&
          !Sample->Response.IsEmpty()};
}

/** User Story: As a chat selector, I need authored turns selected recursively so ordering stays data-driven. @fn inline TArray<FChatTranscriptTurn> SelectTurns(const func::Maybe<FQualityReport> &Evidence, const TArray<FString> &Ids, int32 Index, TArray<FChatTranscriptTurn> Acc) */
inline TArray<FChatTranscriptTurn>
SelectTurns(const func::Maybe<FQualityReport> &Evidence,
            const TArray<FString> &Ids, int32 Index,
            TArray<FChatTranscriptTurn> Acc) {
  return Index >= Ids.Num()
             ? Acc
             : SelectTurns(
                   Evidence, Ids,
                   Index + ChatAdapters::ChatData().Numbers.NextIndex,
                   func::append_value<FChatTranscriptTurn>(
                       MoveTemp(Acc), SelectTurn(Evidence, Ids[Index])));
}

} // namespace detail

/** User Story: As a micro-game operator, I need one ordered conversation projected from root-owned live quality evidence. @fn inline FChatTranscript SelectChatTranscript(const func::Maybe<FQualityReport> &Evidence) */
inline FChatTranscript
SelectChatTranscript(const func::Maybe<FQualityReport> &Evidence) {
  const FChatData &Data = ChatAdapters::ChatData();
  return FChatTranscript{
      Data.SessionId,
      detail::SelectTurns(Evidence, Data.ProbeIds, Data.Numbers.InitialIndex,
                          {})};
}

} // namespace MicroGame::ChatSelectors
