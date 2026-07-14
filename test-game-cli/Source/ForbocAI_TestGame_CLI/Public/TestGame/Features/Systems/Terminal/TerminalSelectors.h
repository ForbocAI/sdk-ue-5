#pragma once
/**
 * Test-game view-model selectors.
 */

#include "CoreMinimal.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/Features/Systems/Contract/ContractTypes.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"

namespace TestGame {

struct FTerminalRenderState {
  FString GridText;
};

enum class ETerminalLineLevel : uint8 { Display, Error };

struct FTerminalLineViewModel {
  ETerminalLineLevel Level;
  FString Text;

  FTerminalLineViewModel()
      : Level(ETerminalLineLevel::Display) {}
  FTerminalLineViewModel(ETerminalLineLevel InLevel, FString InText)
      : Level(InLevel), Text(MoveTemp(InText)) {}
};

struct FTerminalProgressViewModel {
  TArray<FTerminalLineViewModel> Lines;
};

namespace TerminalSelectorsDetail {
inline bool IsBlocked(const TArray<FPosition> &Blocked, const FPosition &Pos,
                      int32 Index) {
  return Index >= Blocked.Num()
             ? false
             : (Blocked[Index] == Pos ? true
                                      : IsBlocked(Blocked, Pos, Index + 1));
}

inline TCHAR NpcCellAt(const TArray<FGameNPC> &Npcs, const FPosition &Pos,
                       int32 Index) {
  return Index >= Npcs.Num()
             ? TEXT('.')
             : (Npcs[Index].Position == Pos
                    ? (Npcs[Index].Id == TEXT("miller")
                           ? TEXT('M')
                           : (Npcs[Index].Id == TEXT("doomguard") ? TEXT('D')
                                                                  : TEXT('N')))
                    : NpcCellAt(Npcs, Pos, Index + 1));
}

inline TCHAR CellAt(const FPosition &Pos, const FTestGameState &State) {
  return IsBlocked(State.Grid.Blocked, Pos, 0)
             ? TEXT('#')
             : (State.Player.Position == Pos
                    ? TEXT('P')
                    : NpcCellAt(NPCsSelectors::SelectAllNpcs(State.NPCs), Pos,
                                0));
}

inline FString RenderRowAt(const FTestGameState &State, int32 Y, int32 X,
                            int32 Width, const FString &Acc) {
  return X >= Width
             ? Acc
             : RenderRowAt(State, Y, X + 1, Width,
                           Acc + (X > 0 ? FString(TEXT(" ")) : FString()) +
                               CellAt(FPosition(X, Y), State));
}

inline FString RenderRows(const FTestGameState &State, int32 Y, int32 Height,
                           const FString &Acc) {
  return Y >= Height
             ? Acc
             : RenderRows(State, Y + 1, Height,
                          Acc + (Y > 0 ? FString(TEXT("\n")) : FString()) +
                              RenderRowAt(State, Y, 0, State.Grid.Width,
                                          FString()));
}
} // namespace TerminalSelectorsDetail

inline FTerminalRenderState SelectTerminalRenderState(
    const FTestGameState &State) {
  FTerminalRenderState ViewModel;
  ViewModel.GridText =
      TerminalSelectorsDetail::RenderRows(State, 0, State.Grid.Height,
                                          FString());
  return ViewModel;
}

namespace TerminalProgressSelectorsDetail {

inline void AppendTranscriptLines(
    const TArray<FTranscriptEntry> &Entries, int32 Index,
    TArray<FTerminalLineViewModel> &Lines) {
  Index >= Entries.Num()
      ? void()
      : [&]() {
          const FTranscriptEntry &Entry = Entries[Index];
          Lines.Add(FTerminalLineViewModel{
              Entry.Status == ETranscriptStatus::Ok
                  ? ETerminalLineLevel::Display
                  : ETerminalLineLevel::Error,
              FString::Printf(
                  TEXT("%s | %s | %s"), *Entry.Timestamp,
                  Entry.Status == ETranscriptStatus::Ok ? TEXT("ok")
                                                        : TEXT("error"),
                  *Entry.Command)});
          AppendTranscriptLines(Entries, Index + 1, Lines);
        }();
}

} // namespace TerminalProgressSelectorsDetail

inline FTerminalProgressViewModel
SelectTerminalProgressViewModel(const FGameProgress &Progress) {
  FTerminalProgressViewModel ViewModel;
  Progress.Type == EGameProgressType::SessionStarted
      ? (ViewModel.Lines.Add(FTerminalLineViewModel{
             ETerminalLineLevel::Display,
             FString::Printf(
                 TEXT("ForbocAI test-game session started: mode=%s"),
                 Progress.Mode == EPlayMode::Autoplay ? TEXT("autoplay")
                                                     : TEXT("manual"))}),
         void())
      : void();
  Progress.Type == EGameProgressType::ContractFailed
      ? (ViewModel.Lines.Add(FTerminalLineViewModel{
             ETerminalLineLevel::Error, Progress.Message}),
         void())
      : void();
  Progress.Type == EGameProgressType::StepStarted
      ? (ViewModel.Lines.Add(FTerminalLineViewModel{
             ETerminalLineLevel::Display,
             FString::Printf(TEXT(":: %s [%s]"), *Progress.Step.Title,
                             *Progress.Step.Id)}),
         ViewModel.Lines.Add(FTerminalLineViewModel{
             ETerminalLineLevel::Display, Progress.Step.Description}),
         void())
      : void();
  Progress.Type == EGameProgressType::CommandCompleted
      ? [&]() {
          ViewModel.Lines.Add(FTerminalLineViewModel{
              Progress.CommandResult.Status == ETranscriptStatus::Ok
                  ? ETerminalLineLevel::Display
                  : ETerminalLineLevel::Error,
              FString::Printf(
                  TEXT("%s %s"),
                  Progress.CommandResult.Status == ETranscriptStatus::Ok
                      ? TEXT("[ok]")
                      : TEXT("[error]"),
                  *Progress.Command.Command)});
          (Progress.CommandResult.Status == ETranscriptStatus::Error &&
           !Progress.CommandResult.Output.IsEmpty())
              ? (ViewModel.Lines.Add(FTerminalLineViewModel{
                     ETerminalLineLevel::Error,
                     Progress.CommandResult.Output}),
                 void())
              : void();
        }()
      : void();
  Progress.Type == EGameProgressType::SessionCompleted
      ? (ViewModel.Lines.Add(FTerminalLineViewModel{
             ETerminalLineLevel::Display,
             TEXT("=== Transcript Summary ===")}),
         TerminalProgressSelectorsDetail::AppendTranscriptLines(
             Progress.RunResult.Transcript, 0, ViewModel.Lines),
         ViewModel.Lines.Add(FTerminalLineViewModel{
             Progress.RunResult.bComplete ? ETerminalLineLevel::Display
                                          : ETerminalLineLevel::Error,
             Progress.RunResult.Summary}),
         void())
      : void();
  return ViewModel;
}

inline FTerminalProgressViewModel SelectUsageViewModel() {
  FTerminalProgressViewModel ViewModel;
  ViewModel.Lines.Add(FTerminalLineViewModel{
      ETerminalLineLevel::Display,
      TEXT("Usage: forbocai-ue-test-game [contract|--mode autoplay|manual]")});
  ViewModel.Lines.Add(FTerminalLineViewModel{
      ETerminalLineLevel::Display,
      TEXT("       forbocai-ue-test-game --api-url https://api.forboc.ai "
           "--mode autoplay")});
  return ViewModel;
}

inline FTerminalProgressViewModel SelectContractViewModel(
    const Contract::FRawContractResponse &Response) {
  FTerminalProgressViewModel ViewModel;
  ViewModel.Lines.Add(FTerminalLineViewModel{
      Response.bSuccess ? ETerminalLineLevel::Display
                        : ETerminalLineLevel::Error,
      Response.bSuccess ? Response.Body : Response.Error});
  return ViewModel;
}

inline FTerminalProgressViewModel
SelectInvalidModeViewModel(const FString &Mode) {
  FTerminalProgressViewModel ViewModel;
  ViewModel.Lines.Add(FTerminalLineViewModel{
      ETerminalLineLevel::Error,
      FString::Printf(TEXT("Invalid mode: %s"), *Mode)});
  return ViewModel;
}

} // namespace TestGame
