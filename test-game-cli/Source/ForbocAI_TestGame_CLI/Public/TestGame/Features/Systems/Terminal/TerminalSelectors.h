#pragma once
/**
 * Test-game view-model selectors.
 */

#include "CoreMinimal.h"
#include "TestGame/TestGameStore.h"

namespace TestGame {

struct FTerminalRenderState {
  FString GridText;
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

} // namespace TestGame
