#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Stealth/StealthTypes.h"

namespace TestGame {

struct FStealthAlertBounds {
  int32 Minimum{};
  int32 Maximum{};
};

inline const DataAdapters::FSettingsSource &StealthSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/stealth.json"));
  return Source;
}

inline FStealthState CreateStealthInitialState() {
  const TSharedRef<FJsonObject> Initial = DataAdapters::ReadObjectField(
      StealthSettingsSource(), TEXT("initialState"));
  FStealthState State{};
  State.bDoorOpen =
      DataAdapters::ReadBooleanField(Initial, TEXT("doorOpen"));
  State.AlertLevel =
      DataAdapters::ReadNumberField(Initial, TEXT("alertLevel"));
  return State;
}

inline FStealthAlertBounds ReadStealthAlertBounds() {
  const TSharedRef<FJsonObject> Bounds = DataAdapters::ReadObjectField(
      StealthSettingsSource(), TEXT("alertBounds"));
  return {
      DataAdapters::ReadNumberField(Bounds, TEXT("minimum")),
      DataAdapters::ReadNumberField(Bounds, TEXT("maximum")),
  };
}

inline int32 ClampStealthAlertLevel(int32 AlertLevel) {
  static const FStealthAlertBounds Bounds = ReadStealthAlertBounds();
  return FMath::Clamp(AlertLevel, Bounds.Minimum, Bounds.Maximum);
}

} // namespace TestGame
