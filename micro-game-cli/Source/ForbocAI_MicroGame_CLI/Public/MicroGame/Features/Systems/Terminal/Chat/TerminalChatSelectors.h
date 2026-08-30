#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Chat/SystemsChatSelectors.h"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"

namespace MicroGame::TerminalChatSelectors {

namespace detail {

/** User Story: As a terminal selector, I need each chat turn projected into authored lines with unavailable evidence kept visible. @fn inline TArray<FTerminalLineViewModel> SelectTurnLines(const FChatTranscriptTurn &Turn) */
inline TArray<FTerminalLineViewModel>
SelectTurnLines(const FChatTranscriptTurn &Turn) {
  const FChatData &Data = ChatAdapters::ChatData();
  const FTerminalData &Terminal = TerminalAdapters::TerminalData();
  const TMap<FString, FString> MissingValues{{Data.Tokens.Id, Turn.Id}};
  const TMap<FString, FString> UserValues{{Data.Tokens.Prompt, Turn.Prompt}};
  const TMap<FString, FString> NpcValues{{Data.Tokens.Response,
                                          Turn.Response}};
  const TMap<FString, FString> LatencyValues{
      {Data.Tokens.DurationMs,
       FString::FromInt(FMath::RoundToInt(Turn.DurationMs))}};
  return !Turn.bAvailable
             ? TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
                   Terminal.levels.error,
                   ChatAdapters::FormatChatTemplate(Data.Output.Missing,
                                                    MissingValues)}}
             : TArray<FTerminalLineViewModel>{
                   FTerminalLineViewModel{
                       Terminal.levels.display,
                       ChatAdapters::FormatChatTemplate(Data.Output.User,
                                                        UserValues)},
                   FTerminalLineViewModel{
                       Terminal.levels.display,
                       ChatAdapters::FormatChatTemplate(Data.Output.Npc,
                                                        NpcValues)},
                   FTerminalLineViewModel{
                       Terminal.levels.display,
                       ChatAdapters::FormatChatTemplate(Data.Output.Latency,
                                                        LatencyValues)}};
}

} // namespace detail

/** User Story: As a micro-game operator, I need the SDK-CLI-backed generic NPC conversation rendered as one final log block. @fn inline TArray<FTerminalLineViewModel> SelectChatTranscriptViewModel(const func::Maybe<FQualityReport> &Evidence) */
inline TArray<FTerminalLineViewModel>
SelectChatTranscriptViewModel(const func::Maybe<FQualityReport> &Evidence) {
  const FChatData &Data = ChatAdapters::ChatData();
  const FChatTranscript Transcript =
      ChatSelectors::SelectChatTranscript(Evidence);
  const bool bDisplay = TerminalAdapters::TerminalData().levels.display;
  return func::concat_arrays<FTerminalLineViewModel>({
      TArray<FTerminalLineViewModel>{
          FTerminalLineViewModel{bDisplay, Data.Output.Heading}},
      func::concat_arrays<FTerminalLineViewModel>(
          func::map_array<FChatTranscriptTurn,
                          TArray<FTerminalLineViewModel>>(
              Transcript.Turns, detail::SelectTurnLines)),
      TArray<FTerminalLineViewModel>{
          FTerminalLineViewModel{bDisplay, Data.Output.Footer}}});
}

} // namespace MicroGame::TerminalChatSelectors
