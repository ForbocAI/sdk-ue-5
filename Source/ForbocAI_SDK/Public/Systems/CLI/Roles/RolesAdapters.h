#pragma once

#include "Components/CLI/CLITypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ForbocAI::CLI::Roles {

/** User Story: As the CLI dispatcher, I need semantic command roles decoded independently from command records so both SDK bindings share stable capability identifiers. @fn inline FCLICommandRoles ReadCommandRoles( const TSharedRef<FJsonObject> &Roles) */
inline FCLICommandRoles ReadCommandRoles(
    const TSharedRef<FJsonObject> &Roles) {
  return {
      DataAdapters::ReadStringField(Roles, TEXT("version")),
      DataAdapters::ReadStringField(Roles, TEXT("doctor")),
      DataAdapters::ReadStringField(Roles, TEXT("status")),
      DataAdapters::ReadStringField(Roles, TEXT("contract")),
      DataAdapters::ReadStringField(Roles, TEXT("setup")),
      DataAdapters::ReadStringField(Roles, TEXT("setupCheck")),
      DataAdapters::ReadStringField(Roles, TEXT("configSet")),
      DataAdapters::ReadStringField(Roles, TEXT("configGet")),
      DataAdapters::ReadStringField(Roles, TEXT("configList")),
      DataAdapters::ReadStringField(Roles, TEXT("npcCreate")),
      DataAdapters::ReadStringField(Roles, TEXT("npcState")),
      DataAdapters::ReadStringField(Roles, TEXT("npcUpdate")),
      DataAdapters::ReadStringField(Roles, TEXT("npcProcess")),
      DataAdapters::ReadStringField(Roles, TEXT("npcGenerate")),
      DataAdapters::ReadStringField(Roles, TEXT("npcRecall")),
      DataAdapters::ReadStringField(Roles, TEXT("npcDecide")),
      DataAdapters::ReadStringField(Roles, TEXT("npcChat")),
      DataAdapters::ReadStringField(Roles, TEXT("npcImport")),
      DataAdapters::ReadStringField(Roles, TEXT("soulExport")),
      DataAdapters::ReadStringField(Roles, TEXT("soulImport")),
      DataAdapters::ReadStringField(Roles, TEXT("soulList")),
      DataAdapters::ReadStringField(Roles, TEXT("soulChat")),
      DataAdapters::ReadStringField(Roles, TEXT("soulVerify")),
      DataAdapters::ReadStringField(Roles, TEXT("rulesList")),
      DataAdapters::ReadStringField(Roles, TEXT("rulesPresets")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostRun")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostGenerate")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostCreate")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostUpdate")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostRecall")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostDecide")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostMemoryStore")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostStatus")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostResults")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostStop")),
      DataAdapters::ReadStringField(Roles, TEXT("ghostHistory")),
      DataAdapters::ReadStringField(Roles, TEXT("memoryList")),
      DataAdapters::ReadStringField(Roles, TEXT("memoryRecall")),
      DataAdapters::ReadStringField(Roles, TEXT("memoryStore")),
      DataAdapters::ReadStringField(Roles, TEXT("memoryClear")),
      DataAdapters::ReadStringField(Roles, TEXT("memoryExport")),
      DataAdapters::ReadStringField(Roles, TEXT("bridgeValidate")),
      DataAdapters::ReadStringField(Roles, TEXT("bridgeRules")),
      DataAdapters::ReadStringField(Roles, TEXT("bridgePreset")),
  };
}

} // namespace ForbocAI::CLI::Roles
