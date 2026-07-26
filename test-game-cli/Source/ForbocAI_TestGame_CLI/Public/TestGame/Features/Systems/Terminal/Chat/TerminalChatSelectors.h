#pragma once

#include "TestGame/Features/Systems/Chat/SystemsChatSelectors.h"
#include "TestGame/Features/Systems/Terminal/TerminalAdapters.h"

namespace TestGame::TerminalChatSelectors {

namespace detail {

/** User Story: As a terminal selector, I need chat turns mapped recursively into authored lines with unavailable evidence kept visible. @fn inline void AppendTurnLines(const TArray<FChatTranscriptTurn> &Turns, int32 Index, TArray<FTerminalLineViewModel> &Lines) */
inline void AppendTurnLines(const TArray<FChatTranscriptTurn> &Turns,
                            int32 Index,
                            TArray<FTerminalLineViewModel> &Lines) {
  Index >= Turns.Num()
      ? void()
      : [&]() {
          const FChatData &Data = ChatAdapters::ChatData();
          const FChatTranscriptTurn &Turn = Turns[Index];
          !Turn.bAvailable
              ? [&]() {
                  TMap<FString, FString> Values;
                  Values.Add(Data.Tokens.Id, Turn.Id);
                  Lines.Add(FTerminalLineViewModel{
                      TerminalAdapters::TerminalData().levels.error,
                      ChatAdapters::FormatChatTemplate(Data.Output.Missing,
                                                       Values)});
                }()
              : [&]() {
                  TMap<FString, FString> UserValues;
                  UserValues.Add(Data.Tokens.Prompt, Turn.Prompt);
                  TMap<FString, FString> NpcValues;
                  NpcValues.Add(Data.Tokens.Response, Turn.Response);
                  TMap<FString, FString> LatencyValues;
                  LatencyValues.Add(
                      Data.Tokens.DurationMs,
                      FString::FromInt(FMath::RoundToInt(Turn.DurationMs)));
                  Lines.Add(FTerminalLineViewModel{
                      TerminalAdapters::TerminalData().levels.display,
                      ChatAdapters::FormatChatTemplate(Data.Output.User,
                                                       UserValues)});
                  Lines.Add(FTerminalLineViewModel{
                      TerminalAdapters::TerminalData().levels.display,
                      ChatAdapters::FormatChatTemplate(Data.Output.Npc,
                                                       NpcValues)});
                  Lines.Add(FTerminalLineViewModel{
                      TerminalAdapters::TerminalData().levels.display,
                      ChatAdapters::FormatChatTemplate(Data.Output.Latency,
                                                       LatencyValues)});
                }();
          AppendTurnLines(Turns, Index + Data.Numbers.NextIndex, Lines);
        }();
}

} // namespace detail

/** User Story: As a test-game operator, I need the SDK-CLI-backed generic NPC conversation rendered as one final log block. @fn inline TArray<FTerminalLineViewModel> SelectChatTranscriptViewModel(const func::Maybe<FQualityReport> &Evidence) */
inline TArray<FTerminalLineViewModel>
SelectChatTranscriptViewModel(const func::Maybe<FQualityReport> &Evidence) {
  const FChatData &Data = ChatAdapters::ChatData();
  const FChatTranscript Transcript =
      ChatSelectors::SelectChatTranscript(Evidence);
  TArray<FTerminalLineViewModel> Lines;
  Lines.Add(FTerminalLineViewModel{
      TerminalAdapters::TerminalData().levels.display, Data.Output.Heading});
  detail::AppendTurnLines(Transcript.Turns, Data.Numbers.InitialIndex, Lines);
  Lines.Add(FTerminalLineViewModel{
      TerminalAdapters::TerminalData().levels.display, Data.Output.Footer});
  return Lines;
}

} // namespace TestGame::TerminalChatSelectors
