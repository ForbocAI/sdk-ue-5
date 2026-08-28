#pragma once

#include "MicroGame/Features/Components/Harness/Maze/Topology/TopologyTypes.h"
#include "MicroGame/Features/Systems/Harness/Maze/Definition/MazeDefinitionAdapters.h"

namespace MicroGame::Maze {

namespace RandomDetail {

struct FRankedValue {
  FString Value;
  double Rank{};
};

struct FRankedValues {
  uint32 State{};
  TArray<FRankedValue> Values;
};

/** User Story: As deterministic Maze generation, I need each pseudo-random value derived from one explicit state transition. @fn inline FRandomStep NextRandom(uint32 State) */
inline FRandomStep NextRandom(uint32 State) {
  const FMazeNumbers &Numbers = MazeConfig().Numbers;
  const uint32 NextState = State + Numbers.PrngIncrement;
  uint32 Mixed =
      (NextState ^ (NextState >> Numbers.PrngFirstShift)) *
      (static_cast<uint32>(Numbers.One) | NextState);
  Mixed =
      (Mixed +
       (Mixed ^ (Mixed >> Numbers.PrngSecondShift)) *
           (Numbers.PrngMultiplier | Mixed)) ^
      Mixed;
  return {NextState,
          static_cast<double>(
              Mixed ^ (Mixed >> Numbers.PrngThirdShift)) /
              Numbers.Uint32Divisor};
}

/** User Story: As deterministic shuffling, I need random ranks accumulated recursively with the updated generator state. @fn inline FRankedValues RankValues(const TArray<FString> &Values, int32 Index, uint32 State, TArray<FRankedValue> Ranked) */
inline FRankedValues RankValues(const TArray<FString> &Values, int32 Index,
                                uint32 State,
                                TArray<FRankedValue> Ranked) {
  return Index >= Values.Num()
             ? FRankedValues{State, Ranked}
             : [&]() {
                 const FRandomStep Step = NextRandom(State);
                 Ranked.Add({Values[Index], Step.Value});
                 return RankValues(Values, Index + MazeConfig().Numbers.One,
                                   Step.State, MoveTemp(Ranked));
               }();
}

} // namespace RandomDetail

/** User Story: As Maze dimension selection, I need an inclusive integer pick paired with the next random state. @fn inline FIntegerPick PickInteger(uint32 State, int32 Minimum, int32 Maximum) */
inline FIntegerPick PickInteger(uint32 State, int32 Minimum, int32 Maximum) {
  const FRandomStep Step = RandomDetail::NextRandom(State);
  return {Step.State,
          Minimum + static_cast<int32>(FMath::FloorToInt(
                        Step.Value * static_cast<double>(
                                         Maximum - Minimum +
                                         MazeConfig().Numbers.One)))};
}

/** User Story: As deterministic topology generation, I need a shuffled copy paired with the next random state. @fn inline FShuffleResult ShuffleValues(const TArray<FString> &Values, uint32 State) */
inline FShuffleResult ShuffleValues(const TArray<FString> &Values,
                                    uint32 State) {
  RandomDetail::FRankedValues Ranked = RandomDetail::RankValues(
      Values, MazeConfig().Numbers.Zero, State, {});
  Ranked.Values.Sort(
      [](const RandomDetail::FRankedValue &Left,
         const RandomDetail::FRankedValue &Right) {
        return Left.Rank < Right.Rank;
      });
  return {Ranked.State,
          func::map_array<RandomDetail::FRankedValue, FString>(
              Ranked.Values,
              [](const RandomDetail::FRankedValue &Value) {
                return Value.Value;
              })};
}

} // namespace MicroGame::Maze
