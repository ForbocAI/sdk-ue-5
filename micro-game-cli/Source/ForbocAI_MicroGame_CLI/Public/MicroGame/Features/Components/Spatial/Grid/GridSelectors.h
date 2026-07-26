#pragma once

#include "MicroGame/Features/Components/Spatial/Grid/GridAdapters.h"

namespace MicroGame {

/** User Story: As a components spatial grid consumer, I need to invoke is passable through a stable signature so the components spatial grid workflow remains explicit and composable. @fn inline bool IsPassable(const FGridState &Grid, const FPosition &Position) */
inline bool IsPassable(const FGridState &Grid, const FPosition &Position) {
  static const FGridBounds Bounds = ReadGridBounds();
  return Position.X >= Bounds.MinimumCoordinate &&
         Position.Y >= Bounds.MinimumCoordinate && Position.X < Grid.Width &&
         Position.Y < Grid.Height &&
         !Grid.Blocked.Contains(Position);
}

} // namespace MicroGame
