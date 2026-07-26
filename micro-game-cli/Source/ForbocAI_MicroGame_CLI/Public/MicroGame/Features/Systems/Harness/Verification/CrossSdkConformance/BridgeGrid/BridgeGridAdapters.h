#pragma once

#include "Core/fp.hpp"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/Parsing/CrossSdkConformanceParsingAdapters.h"

namespace MicroGame::CrossSdkConformanceAdapters {

/** User Story: As a bridge/grid verifier, I need its authored case decoded atomically so input, stories, and expected outcomes cannot drift independently. @fn inline CrossSdkConformance::FBridgeGridVerificationData ReadBridgeGrid(const TSharedRef<FJsonObject> &Root) */
inline CrossSdkConformance::FBridgeGridVerificationData
ReadBridgeGrid(const TSharedRef<FJsonObject> &Root) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Root, TEXT("bridgeGrid"));
  const TSharedRef<FJsonObject> Stories =
      DataAdapters::ReadObjectField(Object, TEXT("stories"));
  const TSharedRef<FJsonObject> Rules =
      DataAdapters::ReadObjectField(Object, TEXT("rules"));
  const TSharedRef<FJsonObject> Jump =
      DataAdapters::ReadObjectField(Object, TEXT("jump"));
  const TSharedRef<FJsonObject> Move =
      DataAdapters::ReadObjectField(Object, TEXT("move"));
  const TSharedRef<FJsonObject> Grid =
      DataAdapters::ReadObjectField(Object, TEXT("grid"));
  const TSharedRef<FJsonObject> Coverage =
      DataAdapters::ReadObjectField(Object, TEXT("coverage"));
  CrossSdkConformance::FBridgeGridVerificationData Data;
  Data.suite = DataAdapters::ReadStringField(Object, TEXT("suite"));
  Data.automationName =
      DataAdapters::ReadStringField(Object, TEXT("automationName"));
#define FORBOCAI_READ_BRIDGE_GRID_STORY(Name)                              \
  Data.stories.Name = DataAdapters::ReadStringField(Stories, TEXT(#Name));
  FORBOCAI_READ_BRIDGE_GRID_STORY(jump)
  FORBOCAI_READ_BRIDGE_GRID_STORY(move)
  FORBOCAI_READ_BRIDGE_GRID_STORY(grid)
  FORBOCAI_READ_BRIDGE_GRID_STORY(coverage)
#undef FORBOCAI_READ_BRIDGE_GRID_STORY
  Data.rules.MaxJumpForce =
      DataAdapters::ReadNumberField(Rules, TEXT("maxJumpForce"));
  Data.rules.MaxMoveDistance =
      DataAdapters::ReadNumberField(Rules, TEXT("maxMoveDistance"));
  Data.rules.ActivePreset =
      DataAdapters::ReadStringField(Rules, TEXT("activePreset"));
  Data.jump.requestedForce =
      DataAdapters::ReadNumberField(Jump, TEXT("requestedForce"));
  Data.jump.reasonFragment =
      DataAdapters::ReadStringField(Jump, TEXT("reasonFragment"));
  Data.move.requestedDistance =
      DataAdapters::ReadNumberField(Move, TEXT("requestedDistance"));
  Data.move.expectedDistance =
      DataAdapters::ReadNumberField(Move, TEXT("expectedDistance"));
  Data.grid.state.Width = DataAdapters::ReadNumberField(Grid, TEXT("width"));
  Data.grid.state.Height =
      DataAdapters::ReadNumberField(Grid, TEXT("height"));
  Data.grid.state.Blocked = func::map_array<TSharedPtr<FJsonValue>, FPosition>(
      DataAdapters::ReadObjectArrayField(Grid, TEXT("blocked")),
      [](const TSharedPtr<FJsonValue> &Value) {
        return ReadPosition(Value->AsObject().ToSharedRef());
      });
  Data.grid.blockedPosition = ReadPosition(
      DataAdapters::ReadObjectField(Grid, TEXT("blockedPosition")));
  Data.grid.openPosition =
      ReadPosition(DataAdapters::ReadObjectField(Grid, TEXT("openPosition")));
  Data.grid.outsidePosition = ReadPosition(
      DataAdapters::ReadObjectField(Grid, TEXT("outsidePosition")));
  Data.coverage.covered = DataAdapters::ReadBooleanMap(
      DataAdapters::ReadObjectField(Coverage, TEXT("covered")));
  Data.coverage.minimumMissing =
      DataAdapters::ReadNumberField(Coverage, TEXT("minimumMissing"));
  Data.coverage.expectedMissing =
      DataAdapters::ReadStringField(Coverage, TEXT("expectedMissing"));
  return Data;
}

} // namespace MicroGame::CrossSdkConformanceAdapters
