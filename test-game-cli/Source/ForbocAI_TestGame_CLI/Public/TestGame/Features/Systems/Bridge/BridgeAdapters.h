#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Bridge/BridgeTypes.h"

namespace TestGame {

inline const DataAdapters::FSettingsSource &BridgeSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/bridge.json"));
  return Source;
}

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

inline TOptional<int32> ResolveBridgePresetMoveDistance(
    const FString &Preset) {
  const TSharedRef<FJsonObject> Presets = DataAdapters::ReadObjectField(
      BridgeSettingsSource(), TEXT("presetMoveDistances"));
  double Distance;
  return Presets->TryGetNumberField(Preset, Distance)
             ? TOptional<int32>(static_cast<int32>(Distance))
             : TOptional<int32>();
}

inline FString FormatJumpForceExceeded(int32 Force, int32 Maximum) {
  const TSharedRef<FJsonObject> Messages = DataAdapters::ReadObjectField(
      BridgeSettingsSource(), TEXT("messages"));
  return FString::Format(
      *DataAdapters::ReadStringField(Messages, TEXT("jumpForceExceeded")),
      {Force, Maximum});
}

} // namespace TestGame
