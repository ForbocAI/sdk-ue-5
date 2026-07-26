#pragma once

#include "CoreMinimal.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "TestGame/Features/Systems/Terminal/Chat/TerminalChatSelectors.h"
#include "TestGame/Features/Systems/Terminal/Conversation/ConversationSelectors.h"
#include "TestGame/Features/Systems/Terminal/Quality/TerminalQualitySelectors.h"

namespace TestGame {

namespace TerminalSelectorsDetail {

/** User Story: As a grid presenter, I need blocked positions searched recursively so selectors remain deterministic and side-effect free. @fn inline bool IsBlocked(const TArray<FPosition> &Blocked, const FPosition &Position, int32 Index) */
inline bool IsBlocked(const TArray<FPosition> &Blocked,
                      const FPosition &Position, int32 Index) {
  return Index >= Blocked.Num()
             ? false
             : Blocked[Index] == Position
                   ? true
                   : IsBlocked(Blocked, Position,
                               Index + GameAdapters::GameRuntimeData()
                                           .numbers.nextIndex);
}

/** User Story: As a grid presenter, I need authored NPC glyphs selected recursively so adding a marker never changes rendering logic. @fn inline FString SelectNpcGlyph(const FString &NpcId, const TArray<FTerminalNpcGlyph> &Glyphs, int32 Index) */
inline FString SelectNpcGlyph(const FString &NpcId,
                              const TArray<FTerminalNpcGlyph> &Glyphs,
                              int32 Index) {
  return Index >= Glyphs.Num()
             ? TerminalAdapters::TerminalData().unknownNpcGlyph
             : Glyphs[Index].Id == NpcId
                   ? Glyphs[Index].Glyph
                   : SelectNpcGlyph(
                         NpcId, Glyphs,
                         Index + GameAdapters::GameRuntimeData()
                                     .numbers.nextIndex);
}

/** User Story: As a grid presenter, I need entity cells derived from root state and authored glyph data so view code performs no state interpretation. @fn inline FString NpcCellAt(const TArray<FGameNPC> &Npcs, const FPosition &Position, int32 Index) */
inline FString NpcCellAt(const TArray<FGameNPC> &Npcs,
                         const FPosition &Position, int32 Index) {
  return Index >= Npcs.Num()
             ? TerminalAdapters::TerminalData().grid.open
             : Npcs[Index].Position == Position
                   ? SelectNpcGlyph(
                         Npcs[Index].Id,
                         TerminalAdapters::TerminalData().npcGlyphs,
                         GameAdapters::GameRuntimeData().numbers.emptyCount)
                   : NpcCellAt(
                         Npcs, Position,
                         Index + GameAdapters::GameRuntimeData()
                                     .numbers.nextIndex);
}

/** User Story: As a grid presenter, I need cell priority derived in one selector so blocked, player, and NPC state cannot disagree across views. @fn inline FString CellAt(const FPosition &Position, const FTestGameState &State) */
inline FString CellAt(const FPosition &Position,
                      const FTestGameState &State) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  return IsBlocked(State.Grid.Blocked, Position,
                   GameAdapters::GameRuntimeData().numbers.emptyCount)
             ? Data.grid.blocked
             : State.Player.Position == Position
                   ? Data.grid.player
                   : NpcCellAt(
                         NPCsSelectors::SelectAllNpcs(State.NPCs), Position,
                         GameAdapters::GameRuntimeData().numbers.emptyCount);
}

/** User Story: As a grid presenter, I need rows rendered recursively from selected cells so the fixed grid shape remains pure and testable. @fn inline FString RenderRowAt(const FTestGameState &State, int32 Y, int32 X, const FString &Acc) */
inline FString RenderRowAt(const FTestGameState &State, int32 Y, int32 X,
                           const FString &Acc) {
  const FGameRuntimeNumbers &Numbers =
      GameAdapters::GameRuntimeData().numbers;
  return X >= State.Grid.Width
             ? Acc
             : RenderRowAt(
                   State, Y, X + Numbers.nextIndex,
                   Acc + (X > Numbers.emptyCount
                              ? TerminalAdapters::TerminalData()
                                    .grid.cellSeparator
                              : FString()) +
                       CellAt(FPosition(X, Y), State));
}

/** User Story: As a grid presenter, I need all rows rendered recursively so line separation is authored and independent of terminal output. @fn inline FString RenderRows(const FTestGameState &State, int32 Y, const FString &Acc) */
inline FString RenderRows(const FTestGameState &State, int32 Y,
                          const FString &Acc) {
  const FGameRuntimeNumbers &Numbers =
      GameAdapters::GameRuntimeData().numbers;
  return Y >= State.Grid.Height
             ? Acc
             : RenderRows(
                   State, Y + Numbers.nextIndex,
                   Acc + (Y > Numbers.emptyCount
                              ? TerminalAdapters::TerminalData()
                                    .grid.rowSeparator
                              : FString()) +
                       RenderRowAt(State, Y, Numbers.emptyCount, FString()));
}

/** User Story: As a transcript presenter, I need entries mapped recursively through the authored line template so summary views contain no formatting policy. @fn inline void AppendTranscriptLines( const TArray<FTranscriptEntry> &Entries, int32 Index, TArray<FTerminalLineViewModel> &Lines) */
inline void AppendTranscriptLines(
    const TArray<FTranscriptEntry> &Entries, int32 Index,
    TArray<FTerminalLineViewModel> &Lines) {
  Index >= Entries.Num()
      ? void()
      : [&]() {
          const FTerminalData &Data = TerminalAdapters::TerminalData();
          const FTranscriptEntry &Entry = Entries[Index];
          TMap<FString, FString> Values;
          Values.Add(Data.tokens.timestamp, Entry.Timestamp);
          Values.Add(Data.tokens.status, Entry.Status);
          Values.Add(Data.tokens.command, Entry.Command);
          Lines.Add(FTerminalLineViewModel{
              Entry.Status != GameAdapters::GameRuntimeData().statuses.ok,
              GameAdapters::FormatGameTemplate(
                  Data.messages.transcriptEntry, Values)});
          AppendTranscriptLines(
              Entries,
              Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
              Lines);
        }();
}

/** User Story: As a CLI presenter, I need usage lines mapped recursively from authored data so command help stays portable without view logic. @fn inline void AppendUsageLines(const TArray<FString> &Usage, int32 Index, TArray<FTerminalLineViewModel> &Lines) */
inline void AppendUsageLines(const TArray<FString> &Usage, int32 Index,
                             TArray<FTerminalLineViewModel> &Lines) {
  Index >= Usage.Num()
      ? void()
      : (Lines.Add(FTerminalLineViewModel{
             TerminalAdapters::TerminalData().levels.display, Usage[Index]}),
         AppendUsageLines(
             Usage,
             Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
             Lines));
}

} // namespace TerminalSelectorsDetail

/** User Story: As a terminal view, I need grid and legend presentation selected from root state and authored data so rendering performs no calculations. @fn inline FTerminalRenderState SelectTerminalRenderState(const FTestGameState &State) */
inline FTerminalRenderState
SelectTerminalRenderState(const FTestGameState &State) {
  return FTerminalRenderState{
      TerminalSelectorsDetail::RenderRows(
          State, GameAdapters::GameRuntimeData().numbers.emptyCount,
          FString()),
      TerminalAdapters::TerminalData().messages.legend};
}

/** User Story: As a terminal view, I need lifecycle events mapped to authored line models so the view only emits selected presentation. @fn inline FTerminalProgressViewModel SelectTerminalProgressViewModel(const FGameProgress &Progress) */
inline FTerminalProgressViewModel
SelectTerminalProgressViewModel(const FGameProgress &Progress) {
  const FGameRuntimeData &Runtime = GameAdapters::GameRuntimeData();
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  FTerminalProgressViewModel ViewModel;
  Progress.Type == Runtime.lifecycleEvents.sessionStarted
      ? [&]() {
          TMap<FString, FString> Values;
          Values.Add(Data.tokens.mode, Progress.Mode);
          ViewModel.Lines.Add(FTerminalLineViewModel{
              Data.levels.display,
              GameAdapters::FormatGameTemplate(
                  Data.messages.sessionStarted, Values)});
        }()
      : void();
  Progress.Type == Runtime.lifecycleEvents.contractFailed
      ? (ViewModel.Lines.Add(
             FTerminalLineViewModel{Data.levels.error, Progress.Message}),
         void())
      : void();
  Progress.Type == Runtime.lifecycleEvents.stepStarted
      ? [&]() {
          TMap<FString, FString> Values;
          Values.Add(Data.tokens.title, Progress.Step.Title);
          Values.Add(Data.tokens.id, Progress.Step.Id);
          ViewModel.Lines.Add(FTerminalLineViewModel{
              Data.levels.display,
              GameAdapters::FormatGameTemplate(Data.messages.stepStarted,
                                                Values)});
          ViewModel.Lines.Add(FTerminalLineViewModel{
              Data.levels.display, Progress.Step.Description});
        }()
      : void();
  Progress.Type == Runtime.lifecycleEvents.commandCompleted
      ? [&]() {
          TMap<FString, FString> Values;
          Values.Add(Data.tokens.status, Progress.CommandResult.Status);
          Values.Add(Data.tokens.command, Progress.Command.Command);
          const bool bError =
              Progress.CommandResult.Status == Runtime.statuses.error;
          ViewModel.Lines.Add(FTerminalLineViewModel{
              bError,
              GameAdapters::FormatGameTemplate(Data.messages.commandResult,
                                                Values)});
          bError && !Progress.CommandResult.Output.IsEmpty()
              ? (ViewModel.Lines.Add(FTerminalLineViewModel{
                     Data.levels.error, Progress.CommandResult.Output}),
                 void())
              : void();
        }()
      : void();
  Progress.Type == Runtime.lifecycleEvents.sessionCompleted
      ? (ViewModel.Lines.Add(FTerminalLineViewModel{
             Data.levels.display, Data.messages.transcriptHeading}),
         TerminalSelectorsDetail::AppendTranscriptLines(
             Progress.RunResult.Transcript, Runtime.numbers.emptyCount,
             ViewModel.Lines),
         Progress.RunResult.QualityReport.hasValue
             ? ViewModel.Lines.Append(selectQualitySummaryViewModel(
                   Progress.RunResult.QualityReport.value,
                   Progress.RunResult.QualityReportPath))
             : void(),
         ViewModel.Lines.Add(FTerminalLineViewModel{
             !Progress.RunResult.bComplete, Progress.RunResult.Summary}),
         ViewModel.Lines.Append(
             TerminalChatSelectors::SelectChatTranscriptViewModel(
                 Progress.RunResult.QualityReport)),
         ViewModel.Lines.Append(
             ConversationSelectors::
                 SelectConversationTranscriptViewModel(
                     Progress.RunResult.Transcript)),
         void())
      : void();
  return ViewModel;
}

/** User Story: As a CLI view, I need usage text selected from authored data so host presentation remains platform-neutral. @fn inline FTerminalProgressViewModel SelectUsageViewModel() */
inline FTerminalProgressViewModel SelectUsageViewModel() {
  FTerminalProgressViewModel ViewModel;
  TerminalSelectorsDetail::AppendUsageLines(
      TerminalAdapters::TerminalData().usage,
      GameAdapters::GameRuntimeData().numbers.emptyCount, ViewModel.Lines);
  return ViewModel;
}

/** User Story: As a contract CLI view, I need command output mapped to line severity so rendering does not know transport details. @fn inline FTerminalProgressViewModel SelectContractViewModel( const CommandRunner::FCommandOutput &Result) */
inline FTerminalProgressViewModel SelectContractViewModel(
    const CommandRunner::FCommandOutput &Result) {
  FTerminalProgressViewModel ViewModel;
  ViewModel.Lines.Add(FTerminalLineViewModel{
      Result.Status != GameAdapters::GameRuntimeData().statuses.ok,
      Result.Output});
  return ViewModel;
}

/** User Story: As a CLI view, I need invalid modes formatted through authored tokens so parsing remains separate from presentation. @fn inline FTerminalProgressViewModel SelectInvalidModeViewModel(const FString &Mode) */
inline FTerminalProgressViewModel
SelectInvalidModeViewModel(const FString &Mode) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  TMap<FString, FString> Values;
  Values.Add(Data.tokens.mode, Mode);
  FTerminalProgressViewModel ViewModel;
  ViewModel.Lines.Add(FTerminalLineViewModel{
      Data.levels.error,
      GameAdapters::FormatGameTemplate(Data.messages.invalidMode, Values)});
  return ViewModel;
}

} // namespace TestGame
