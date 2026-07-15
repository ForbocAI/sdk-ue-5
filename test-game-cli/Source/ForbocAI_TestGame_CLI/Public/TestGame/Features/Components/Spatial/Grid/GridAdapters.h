#pragma once

#include "Core/fp.hpp"
#include "TestGame/Features/Components/Spatial/Grid/GridTypes.h"
#include "TestGame/Features/Data/DataAdapters.h"

namespace TestGame {

struct FGridBounds {
  int32 MinimumCoordinate{};
};

inline FPosition ReadGridPosition(const TSharedRef<FJsonObject> &Object) {
  return FPosition(
      DataAdapters::ReadNumberField(Object, TEXT("x")),
      DataAdapters::ReadNumberField(Object, TEXT("y")));
}

inline FGridState CreateGridInitialState() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("components/spatial/grid.json"));
  const TSharedRef<FJsonObject> Initial =
      DataAdapters::ReadObjectField(Source, TEXT("initialState"));
  FGridState State{};
  State.Width = DataAdapters::ReadNumberField(Initial, TEXT("width"));
  State.Height = DataAdapters::ReadNumberField(Initial, TEXT("height"));
  State.Blocked = func::map_array<TSharedPtr<FJsonValue>, FPosition>(
      DataAdapters::ReadObjectArrayField(Initial, TEXT("blocked")),
      [](const TSharedPtr<FJsonValue> &Value) {
        return ReadGridPosition(Value->AsObject().ToSharedRef());
      });
  return State;
}

inline FGridBounds ReadGridBounds() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("components/spatial/grid.json"));
  const TSharedRef<FJsonObject> Bounds =
      DataAdapters::ReadObjectField(Source, TEXT("bounds"));
  FGridBounds Result{};
  Result.MinimumCoordinate =
      DataAdapters::ReadNumberField(Bounds, TEXT("minimumCoordinate"));
  return Result;
}

} // namespace TestGame
