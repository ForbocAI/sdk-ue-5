#pragma once

#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Stealth/StealthTypes.h"

namespace MicroGame {

struct FStealthAlertBounds {
  int32 Minimum{};
  int32 Maximum{};
};

/** User Story: As a features systems stealth consumer, I need to invoke stealth settings source through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline const DataAdapters::FSettingsSource &StealthSettingsSource() */
inline const DataAdapters::FSettingsSource &StealthSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("systems/stealth.json"));
  return Source;
}

/** User Story: As a features systems stealth consumer, I need to invoke create stealth initial state through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline FStealthState CreateStealthInitialState() */
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

/** User Story: As a features systems stealth consumer, I need to invoke read stealth alert bounds through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline FStealthAlertBounds ReadStealthAlertBounds() */
inline FStealthAlertBounds ReadStealthAlertBounds() {
  const TSharedRef<FJsonObject> Bounds = DataAdapters::ReadObjectField(
      StealthSettingsSource(), TEXT("alertBounds"));
  return {
      DataAdapters::ReadNumberField(Bounds, TEXT("minimum")),
      DataAdapters::ReadNumberField(Bounds, TEXT("maximum")),
  };
}

/** User Story: As a features systems stealth consumer, I need to invoke clamp stealth alert level through a stable signature so the features systems stealth workflow remains explicit and composable. @fn inline int32 ClampStealthAlertLevel(int32 AlertLevel) */
inline int32 ClampStealthAlertLevel(int32 AlertLevel) {
  static const FStealthAlertBounds Bounds = ReadStealthAlertBounds();
  return FMath::Clamp(AlertLevel, Bounds.Minimum, Bounds.Maximum);
}

} // namespace MicroGame
