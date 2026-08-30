#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "MicroGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Terminal/Chat/TerminalChatSelectors.h"
#include "MicroGame/Features/Systems/Terminal/Conversation/ConversationSelectors.h"
#include "MicroGame/Features/Systems/Terminal/Quality/TerminalQualitySelectors.h"

namespace MicroGame {

namespace TerminalSelectorsDetail {

/** User Story: As a grid presenter, I need blocked positions selected through the shared collection algebra so rendering stays deterministic and side-effect free. @fn inline bool IsBlocked(const TArray<FPosition> &Blocked, const FPosition &Position) */
inline bool IsBlocked(const TArray<FPosition> &Blocked,
                      const FPosition &Position) {
  return func::contains_value<FPosition>(Blocked, Position);
}

/** User Story: As a grid presenter, I need authored NPC glyphs selected through a reusable lookup so adding a marker never changes rendering logic. @fn inline FString SelectNpcGlyph(const FString &NpcId, const TArray<FTerminalNpcGlyph> &Glyphs) */
inline FString SelectNpcGlyph(const FString &NpcId,
                              const TArray<FTerminalNpcGlyph> &Glyphs) {
  return func::or_else(
      func::fmap(
          func::find_array<FTerminalNpcGlyph>(
              Glyphs, [&NpcId](const FTerminalNpcGlyph &Glyph) {
                return Glyph.Id == NpcId;
              }),
          [](const FTerminalNpcGlyph &Glyph) { return Glyph.Glyph; }),
      TerminalAdapters::TerminalData().unknownNpcGlyph);
}

/** User Story: As a grid presenter, I need entity cells derived from root state and authored glyph data so view code performs no state interpretation. @fn inline FString NpcCellAt(const TArray<FGameNPC> &Npcs, const FPosition &Position) */
inline FString NpcCellAt(const TArray<FGameNPC> &Npcs,
                         const FPosition &Position) {
  return func::or_else(
      func::fmap(
          func::find_array<FGameNPC>(
              Npcs, [&Position](const FGameNPC &Npc) {
                return Npc.Position == Position;
              }),
          [](const FGameNPC &Npc) {
            return SelectNpcGlyph(
                Npc.Id, TerminalAdapters::TerminalData().npcGlyphs);
          }),
      TerminalAdapters::TerminalData().grid.open);
}

/** User Story: As a grid presenter, I need cell priority derived in one selector so blocked, player, and NPC state cannot disagree across views. @fn inline FString CellAt(const FPosition &Position, const FMicroGameState &State) */
inline FString CellAt(const FPosition &Position,
                      const FMicroGameState &State) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  return IsBlocked(State.Grid.Blocked, Position)
             ? Data.grid.blocked
             : State.Player.Position == Position
                   ? Data.grid.player
                   : NpcCellAt(NPCsSelectors::SelectAllNpcs(State.NPCs),
                               Position);
}

/** User Story: As a grid presenter, I need each row projected from its coordinate range so the fixed grid shape remains pure and testable. @fn inline FString RenderRowAt(const FMicroGameState &State, int32 Y) */
inline FString RenderRowAt(const FMicroGameState &State, int32 Y) {
  return FString::Join(
      func::map_index_range(State.Grid.Width, [&State, Y](int32 X) {
        return CellAt(FPosition(X, Y), State);
      }),
      *TerminalAdapters::TerminalData().grid.cellSeparator);
}

/** User Story: As a grid presenter, I need all rows projected from the grid range so line separation is authored and independent of terminal output. @fn inline FString RenderRows(const FMicroGameState &State) */
inline FString RenderRows(const FMicroGameState &State) {
  return FString::Join(
      func::map_index_range(State.Grid.Height, [&State](int32 Y) {
        return RenderRowAt(State, Y);
      }),
      *TerminalAdapters::TerminalData().grid.rowSeparator);
}

/** User Story: As a transcript presenter, I need entries mapped through the authored line template so summary views contain no formatting policy. @fn inline TArray<FTerminalLineViewModel> SelectTranscriptLines(const TArray<FTranscriptEntry> &Entries) */
inline TArray<FTerminalLineViewModel>
SelectTranscriptLines(const TArray<FTranscriptEntry> &Entries) {
  return func::map_array<FTranscriptEntry, FTerminalLineViewModel>(
      Entries, [](const FTranscriptEntry &Entry) {
        const FTerminalData &Data = TerminalAdapters::TerminalData();
        const TMap<FString, FString> Values{
            {Data.tokens.timestamp, Entry.Timestamp},
            {Data.tokens.status, Entry.Status},
            {Data.tokens.command, Entry.Command}};
        return FTerminalLineViewModel{
            Entry.Status != VerificationVocabularyAdapters::GameRuntimeData()
                                .statuses.ok,
            VerificationAdapters::FormatGameTemplate(
                Data.messages.transcriptEntry, Values)};
      });
}

/** User Story: As a terminal view, I need a session-start event projected from authored text so the presenter performs no lifecycle branching. @fn inline TArray<FTerminalLineViewModel> SelectSessionStartedLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectSessionStartedLines(const FGameProgress &Progress) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const TMap<FString, FString> Values{{Data.tokens.mode, Progress.Mode}};
  return {FTerminalLineViewModel{
      Data.levels.display,
      VerificationAdapters::FormatGameTemplate(Data.messages.sessionStarted,
                                                Values)}};
}

/** User Story: As a terminal view, I need a contract failure projected as one authored error line so failure presentation stays declarative. @fn inline TArray<FTerminalLineViewModel> SelectContractFailedLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectContractFailedLines(const FGameProgress &Progress) {
  return {FTerminalLineViewModel{
      TerminalAdapters::TerminalData().levels.error, Progress.Message}};
}

/** User Story: As a terminal view, I need a step-start event projected as authored heading and description lines so the presenter performs no lifecycle mutation. @fn inline TArray<FTerminalLineViewModel> SelectStepStartedLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectStepStartedLines(const FGameProgress &Progress) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const TMap<FString, FString> Values{{Data.tokens.title,
                                      Progress.Step.Title},
                                     {Data.tokens.id, Progress.Step.Id}};
  return {FTerminalLineViewModel{
              Data.levels.display,
              VerificationAdapters::FormatGameTemplate(
                  Data.messages.stepStarted, Values)},
          FTerminalLineViewModel{Data.levels.display,
                                 Progress.Step.Description}};
}

/** User Story: As a terminal view, I need a completed command projected as one result line plus optional real error output so diagnostics remain attributable. @fn inline TArray<FTerminalLineViewModel> SelectCommandCompletedLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectCommandCompletedLines(const FGameProgress &Progress) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const FGameRuntimeData &Runtime =
      VerificationVocabularyAdapters::GameRuntimeData();
  const bool bError =
      Progress.CommandResult.Status == Runtime.statuses.error;
  const TMap<FString, FString> Values{
      {Data.tokens.status, Progress.CommandResult.Status},
      {Data.tokens.command, Progress.Command.Command}};
  const TArray<FTerminalLineViewModel> ErrorOutput =
      bError && !Progress.CommandResult.Output.IsEmpty()
          ? TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
                Data.levels.error, Progress.CommandResult.Output}}
          : TArray<FTerminalLineViewModel>();
  return func::concat_arrays<FTerminalLineViewModel>({
      TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
          bError, VerificationAdapters::FormatGameTemplate(
                      Data.messages.commandResult, Values)}},
      ErrorOutput});
}

/** User Story: As a terminal view, I need a completed run projected from transcript, quality, chat, and conversation selectors so one pure composition owns final ordering. @fn inline TArray<FTerminalLineViewModel> SelectSessionCompletedLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectSessionCompletedLines(const FGameProgress &Progress) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const TArray<FTerminalLineViewModel> QualityLines = func::match(
      Progress.RunResult.QualityReport,
      [&Progress](const FQualityReport &Report) {
        return selectQualitySummaryViewModel(
            Report, Progress.RunResult.QualityReportPath);
      },
      []() { return TArray<FTerminalLineViewModel>(); });
  return func::concat_arrays<FTerminalLineViewModel>({
      TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
          Data.levels.display, Data.messages.transcriptHeading}},
      SelectTranscriptLines(Progress.RunResult.Transcript), QualityLines,
      TArray<FTerminalLineViewModel>{FTerminalLineViewModel{
          !Progress.RunResult.bComplete, Progress.RunResult.Summary}},
      TerminalChatSelectors::SelectChatTranscriptViewModel(
          Progress.RunResult.QualityReport),
      ConversationSelectors::SelectConversationTranscriptViewModel(
          Progress.RunResult.Transcript)});
}

/** User Story: As a terminal view, I need lifecycle events dispatched to pure line selectors so rendering receives one immutable projection. @fn inline TArray<FTerminalLineViewModel> SelectProgressLines(const FGameProgress &Progress) */
inline TArray<FTerminalLineViewModel>
SelectProgressLines(const FGameProgress &Progress) {
  const FGameLifecycleEvents &Events =
      VerificationVocabularyAdapters::GameRuntimeData().lifecycleEvents;
  return Progress.Type == Events.sessionStarted
             ? SelectSessionStartedLines(Progress)
         : Progress.Type == Events.contractFailed
             ? SelectContractFailedLines(Progress)
         : Progress.Type == Events.stepStarted
             ? SelectStepStartedLines(Progress)
         : Progress.Type == Events.commandCompleted
             ? SelectCommandCompletedLines(Progress)
         : Progress.Type == Events.sessionCompleted
             ? SelectSessionCompletedLines(Progress)
             : TArray<FTerminalLineViewModel>();
}

} // namespace TerminalSelectorsDetail

/** User Story: As a terminal view, I need grid and legend presentation selected from root state and authored data so rendering performs no calculations. @fn inline FTerminalRenderState SelectTerminalRenderState(const FMicroGameState &State) */
inline FTerminalRenderState
SelectTerminalRenderState(const FMicroGameState &State) {
  return FTerminalRenderState{
      TerminalSelectorsDetail::RenderRows(State),
      TerminalAdapters::TerminalData().messages.legend};
}

/** User Story: As a terminal view, I need lifecycle events mapped to authored line models so the view only emits selected presentation. @fn inline FTerminalProgressViewModel SelectTerminalProgressViewModel(const FGameProgress &Progress) */
inline FTerminalProgressViewModel
SelectTerminalProgressViewModel(const FGameProgress &Progress) {
  return FTerminalProgressViewModel{
      TerminalSelectorsDetail::SelectProgressLines(Progress)};
}

/** User Story: As a CLI view, I need usage text selected from authored data so host presentation remains platform-neutral. @fn inline FTerminalProgressViewModel SelectUsageViewModel() */
inline FTerminalProgressViewModel SelectUsageViewModel() {
  return FTerminalProgressViewModel{
      func::map_array<FString, FTerminalLineViewModel>(
          TerminalAdapters::TerminalData().usage,
          [](const FString &Usage) {
            return FTerminalLineViewModel{
                TerminalAdapters::TerminalData().levels.display, Usage};
          })};
}

/** User Story: As a contract CLI view, I need command output mapped to line severity so rendering does not know transport details. @fn inline FTerminalProgressViewModel SelectContractViewModel( const CommandRunner::FCommandOutput &Result) */
inline FTerminalProgressViewModel SelectContractViewModel(
    const CommandRunner::FCommandOutput &Result) {
  return FTerminalProgressViewModel{{FTerminalLineViewModel{
      Result.Status != VerificationVocabularyAdapters::GameRuntimeData()
                           .statuses.ok,
      Result.Output}}};
}

/** User Story: As a CLI view, I need invalid modes formatted through authored tokens so parsing remains separate from presentation. @fn inline FTerminalProgressViewModel SelectInvalidModeViewModel(const FString &Mode) */
inline FTerminalProgressViewModel
SelectInvalidModeViewModel(const FString &Mode) {
  const FTerminalData &Data = TerminalAdapters::TerminalData();
  const TMap<FString, FString> Values{{Data.tokens.mode, Mode}};
  return FTerminalProgressViewModel{{FTerminalLineViewModel{
      Data.levels.error,
      VerificationAdapters::FormatGameTemplate(Data.messages.invalidMode,
                                                Values)}}};
}

} // namespace MicroGame
