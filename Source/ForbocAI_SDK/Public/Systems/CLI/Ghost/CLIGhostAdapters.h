#pragma once

#include "Components/CLI/Ghost/CLIGhostTypes.h"
#include "Systems/Data/DataAdapters.h"

namespace ForbocAI::CLI::Ghost {

/** User Story: As the package root CLI slice, I need Ghost defaults, limits, and presentation decoded from authored data. @fn inline FCLIGhostState readCliGhostState() */
inline FCLIGhostState readCliGhostState() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/cli/presentation.json"));
  const TSharedRef<FJsonObject> Ghost =
      DataAdapters::ReadObjectField(Source, TEXT("ghost"));
  return {
      DataAdapters::ReadStringField(Ghost, TEXT("defaultSuite")),
      DataAdapters::ReadNumberField(Ghost, TEXT("defaultDuration")),
      DataAdapters::ReadStringField(Ghost, TEXT("durationOption")),
      DataAdapters::ReadNumberField(Ghost, TEXT("historyLimit")),
      DataAdapters::ReadNumberField(Ghost, TEXT("emptyCount")),
      DataAdapters::ReadStringField(Ghost, TEXT("requiredSession")),
      DataAdapters::ReadStringField(Ghost, TEXT("starting")),
      DataAdapters::ReadStringField(Ghost, TEXT("started")),
      DataAdapters::ReadStringField(Ghost, TEXT("failed")),
      DataAdapters::ReadStringField(Ghost, TEXT("error")),
      DataAdapters::ReadStringField(Ghost, TEXT("results")),
      DataAdapters::ReadStringField(Ghost, TEXT("result")),
      DataAdapters::ReadStringField(Ghost, TEXT("passed")),
      DataAdapters::ReadStringField(Ghost, TEXT("failedTest")),
      DataAdapters::ReadStringField(Ghost, TEXT("session")),
      DataAdapters::ReadStringField(Ghost, TEXT("status")),
      DataAdapters::ReadStringField(Ghost, TEXT("completedStatus")),
      DataAdapters::ReadStringField(Ghost, TEXT("completedColor")),
      DataAdapters::ReadStringField(Ghost, TEXT("activeColor")),
      DataAdapters::ReadStringField(Ghost, TEXT("progress")),
      DataAdapters::ReadStringField(Ghost, TEXT("errors")),
      DataAdapters::ReadStringField(Ghost, TEXT("duration")),
      DataAdapters::ReadStringField(Ghost, TEXT("stopping")),
      DataAdapters::ReadStringField(Ghost, TEXT("stopped")),
      DataAdapters::ReadStringField(Ghost, TEXT("stopFailed")),
      DataAdapters::ReadStringField(Ghost, TEXT("historyTitle")),
      DataAdapters::ReadStringField(Ghost, TEXT("historyEmpty")),
      DataAdapters::ReadStringField(Ghost, TEXT("history")),
      DataAdapters::ReadStringField(Ghost, TEXT("nameOption")),
      DataAdapters::ReadStringField(Ghost, TEXT("roleOption")),
      DataAdapters::ReadStringField(Ghost, TEXT("stateNameOption")),
      DataAdapters::ReadStringField(Ghost, TEXT("stateRoleOption"))};
}

} // namespace ForbocAI::CLI::Ghost
