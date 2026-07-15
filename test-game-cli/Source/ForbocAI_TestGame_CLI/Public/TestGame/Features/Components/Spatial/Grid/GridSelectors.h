#pragma once

#include "TestGame/Features/Components/Spatial/Grid/GridAdapters.h"

namespace TestGame {

inline bool IsPassable(const FGridState &Grid, const FPosition &Position) {
  static const FGridBounds Bounds = ReadGridBounds();
  return Position.X >= Bounds.MinimumCoordinate &&
         Position.Y >= Bounds.MinimumCoordinate && Position.X < Grid.Width &&
         Position.Y < Grid.Height &&
         !Grid.Blocked.Contains(Position);
}

} // namespace TestGame
