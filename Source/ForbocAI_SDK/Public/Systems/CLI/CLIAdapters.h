#pragma once

#include "Components/CLI/CLITypes.h"
#include "Systems/CLI/Bridge/CLIBridgeAdapters.h"
#include "Systems/CLI/Catalog/CatalogAdapters.h"
#include "Systems/CLI/Ghost/CLIGhostAdapters.h"
#include "Systems/CLI/Memory/CLIMemoryAdapters.h"
#include "Systems/CLI/NPC/CLINPCAdapters.h"
#include "Systems/CLI/Roles/RolesAdapters.h"
#include "Systems/CLI/Soul/CLISoulAdapters.h"
#include "Systems/CLI/Vocabulary/VocabularyAdapters.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ForbocAI {
namespace CLI {

/** User Story: As a features cli consumer, I need to invoke read cli state through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCLIState readCliState() */
inline FCLIState readCliState() {
  const DataAdapters::FSettingsSource CatalogSource =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/cli/catalog.json"));
  const TSharedRef<FJsonObject> CatalogRoot = CatalogSource.Root;
  const DataAdapters::FSettingsSource VocabularySource =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"),
          DataAdapters::ReadStringField(CatalogRoot, TEXT("vocabulary")));
  const DataAdapters::FSettingsSource RolesSource =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"),
          DataAdapters::ReadStringField(CatalogRoot, TEXT("roles")));
  const TSharedRef<FJsonObject> SurfaceRoles =
      DataAdapters::ReadObjectField(VocabularySource, TEXT("surfaceRoles"));
  FCLIState State;
  State.Commands = Catalog::ReadCommandCatalogs(
      DataAdapters::ReadStringArrayField(CatalogRoot, TEXT("commands")));
  State.NodeSurface =
      DataAdapters::ReadStringField(SurfaceRoles, TEXT("node"));
  State.BrowserSurface =
      DataAdapters::ReadStringField(SurfaceRoles, TEXT("browser"));
  State.Parsing = Vocabulary::ReadParsingSettings(
      DataAdapters::ReadObjectField(VocabularySource, TEXT("parsing")));
  State.Invocation = Vocabulary::ReadInvocationSettings(
      DataAdapters::ReadObjectField(VocabularySource, TEXT("invocation")));
  State.CommandRoles = Roles::ReadCommandRoles(RolesSource.Root);
  State.Bridge = Bridge::readCliBridgeState();
  State.Ghost = Ghost::readCliGhostState();
  State.Memory = Memory::readCliMemoryState();
  State.NPC = NPC::readCliNpcState();
  State.Soul = Soul::readCliSoulState();
  return State;
}

} // namespace CLI
} // namespace ForbocAI
