#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Entities/Player/PlayerTypes.h"

namespace TestGame {

inline FPlayerState CreatePlayerInitialState() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("entities/player.json"));
  const TSharedRef<FJsonObject> Initial =
      DataAdapters::ReadObjectField(Source, TEXT("initialState"));
  const TSharedRef<FJsonObject> Position =
      DataAdapters::ReadObjectField(Initial, TEXT("position"));
  FPlayerState State{};
  State.Name = DataAdapters::ReadStringField(Initial, TEXT("name"));
  State.Hp = DataAdapters::ReadNumberField(Initial, TEXT("hp"));
  State.bHidden = DataAdapters::ReadBooleanField(Initial, TEXT("hidden"));
  State.Position = FPosition(
      DataAdapters::ReadNumberField(Position, TEXT("x")),
      DataAdapters::ReadNumberField(Position, TEXT("y")));
  State.Inventory =
      DataAdapters::ReadStringArrayField(Initial, TEXT("inventory"));
  return State;
}

} // namespace TestGame
