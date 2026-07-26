#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Bridge/BridgeTypes.h"

namespace MicroGame {

/** User Story: As a features systems bridge consumer, I need to invoke bridge settings source through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline const DataAdapters::FSettingsSource &BridgeSettingsSource() */
inline const DataAdapters::FSettingsSource &BridgeSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/bridge.json"));
  return Source;
}

/** User Story: As a features systems bridge consumer, I need to invoke create bridge initial state through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline FBridgeState CreateBridgeInitialState() */
inline FBridgeState CreateBridgeInitialState() {
  const TSharedRef<FJsonObject> Initial = DataAdapters::ReadObjectField(
      BridgeSettingsSource(), TEXT("initialState"));
  FBridgeState State{};
  State.MaxJumpForce =
      DataAdapters::ReadNumberField(Initial, TEXT("maxJumpForce"));
  State.MaxMoveDistance =
      DataAdapters::ReadNumberField(Initial, TEXT("maxMoveDistance"));
  State.ActivePreset =
      DataAdapters::ReadStringField(Initial, TEXT("activePreset"));
  return State;
}

/** User Story: As a features systems bridge consumer, I need to invoke resolve bridge preset move distance through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline TOptional<int32> ResolveBridgePresetMoveDistance( const FString &Preset) */
inline TOptional<int32> ResolveBridgePresetMoveDistance(
    const FString &Preset) {
  const TSharedRef<FJsonObject> Presets = DataAdapters::ReadObjectField(
      BridgeSettingsSource(), TEXT("presetMoveDistances"));
  double Distance;
  return Presets->TryGetNumberField(Preset, Distance)
             ? TOptional<int32>(static_cast<int32>(Distance))
             : TOptional<int32>();
}

/** User Story: As a features systems bridge consumer, I need to invoke format jump force exceeded through a stable signature so the features systems bridge workflow remains explicit and composable. @fn inline FString FormatJumpForceExceeded(int32 Force, int32 Maximum) */
inline FString FormatJumpForceExceeded(int32 Force, int32 Maximum) {
  const TSharedRef<FJsonObject> Messages = DataAdapters::ReadObjectField(
      BridgeSettingsSource(), TEXT("messages"));
  return FString::Format(
      *DataAdapters::ReadStringField(Messages, TEXT("jumpForceExceeded")),
      {Force, Maximum});
}

} // namespace MicroGame
