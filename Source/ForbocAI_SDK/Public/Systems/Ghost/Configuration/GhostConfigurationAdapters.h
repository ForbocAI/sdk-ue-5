#pragma once

#include "Components/Ghost/Configuration/GhostConfigurationTypes.h"
#include "Systems/Data/DataAdapters.h"

namespace GhostConfiguration {

/** User Story: As the Ghost slice, I need authored lifecycle policy loaded once from the feature-owned JSON contract. @fn inline FGhostConfiguration readGhostConfiguration() */
inline FGhostConfiguration readGhostConfiguration() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/ghost/runtime.json"));
  const TSharedRef<FJsonObject> Defaults =
      DataAdapters::ReadObjectField(Source, TEXT("defaults"));
  const TSharedRef<FJsonObject> Statuses =
      DataAdapters::ReadObjectField(Source, TEXT("statuses"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {
      DataAdapters::ReadNumberField(Defaults, TEXT("duration")),
      DataAdapters::ReadNumberField(Defaults, TEXT("historyLimit")),
      DataAdapters::ReadFloatField(Defaults, TEXT("emptyProgress")),
      DataAdapters::ReadFloatField(Defaults, TEXT("completeProgress")),
      DataAdapters::ReadStringField(Statuses, TEXT("idle")),
      DataAdapters::ReadStringField(Statuses, TEXT("completed")),
      DataAdapters::ReadStringField(Statuses, TEXT("failed")),
      DataAdapters::ReadStringField(Messages, TEXT("empty")),
      DataAdapters::ReadStringField(Messages, TEXT("noActiveSession")),
      DataAdapters::ReadStringField(Messages, TEXT("startFailed")),
      DataAdapters::ReadStringField(Messages, TEXT("statusFailed")),
      DataAdapters::ReadStringField(Messages, TEXT("resultsFailed")),
      DataAdapters::ReadStringField(Messages, TEXT("stopFailed")),
      DataAdapters::ReadStringField(Messages, TEXT("historyFailed")),
      DataAdapters::ReadStringField(Messages, TEXT("stopNotCompleted")),
  };
}

/** User Story: As Ghost feature consumers, I need one immutable lifecycle policy shared by state and thunk defaults. @fn inline const FGhostConfiguration &ghostConfiguration() */
inline const FGhostConfiguration &ghostConfiguration() {
  static const FGhostConfiguration Configuration = readGhostConfiguration();
  return Configuration;
}

} // namespace GhostConfiguration
