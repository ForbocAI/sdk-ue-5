#pragma once

#include "Features/CLI/CLITypes.h"
#include "Core/fp.hpp"
#include "Features/CLI/Bridge/CLIBridgeAdapters.h"
#include "Features/CLI/Ghost/CLIGhostAdapters.h"
#include "Features/CLI/Memory/CLIMemoryAdapters.h"
#include "Features/CLI/NPC/CLINPCAdapters.h"
#include "Features/CLI/Soul/CLISoulAdapters.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"

namespace ForbocAI {
namespace CLI {

/** User Story: As a features cli consumer, I need to invoke read optional text through a stable signature so the features cli workflow remains explicit and composable. @fn inline FString ReadOptionalText(const func::Maybe<FString> &Value) */
inline FString ReadOptionalText(const func::Maybe<FString> &Value) {
  return Value.hasValue ? Value.value : FString();
}

/** User Story: As a features cli consumer, I need to invoke read command spec through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCommandSpec ReadCommandSpec( const FString &Key, const TSharedRef<FJsonObject> &Command) */
inline FCommandSpec ReadCommandSpec(
    const FString &Key, const TSharedRef<FJsonObject> &Command) {
  FCommandSpec Spec;
  Spec.Key = Key;
  Spec.Group = DataAdapters::ReadStringField(Command, TEXT("group"));
  Spec.Subcommand = ReadOptionalText(
      DataAdapters::ReadOptionalStringField(Command, TEXT("subcommand")));
  Spec.Surfaces =
      DataAdapters::ReadStringArrayField(Command, TEXT("surfaces"));
  Spec.BrowserBehavior = ReadOptionalText(
      DataAdapters::ReadOptionalStringField(Command, TEXT("browserBehavior")));
  return Spec;
}

/** User Story: As a features cli consumer, I need to invoke read command data through a stable signature so the features cli workflow remains explicit and composable. @fn inline TArray<FCommandSpec> ReadCommandData( const TSharedRef<FJsonObject> &Commands) */
inline TArray<FCommandSpec> ReadCommandData(
    const TSharedRef<FJsonObject> &Commands) {
  const TArray<FJsonObject::FStringType> StoredKeys =
      func::map_keys(Commands->Values);
  const TArray<FString> Keys = func::map_array(
      StoredKeys, [](const FJsonObject::FStringType &Key) {
        return FString(Key.ToView());
      });
  return func::map_array<FString, FCommandSpec>(
      Keys, [Commands](const FString &Key) {
        return ReadCommandSpec(
            Key, DataAdapters::ReadObjectField(Commands, Key));
      });
}

/** User Story: As a features cli consumer, I need to invoke read parsing settings through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCLIParsingSettings ReadParsingSettings( const TSharedRef<FJsonObject> &Parsing) */
inline FCLIParsingSettings ReadParsingSettings(
    const TSharedRef<FJsonObject> &Parsing) {
  FCLIParsingSettings Settings;
  Settings.FirstTokenIndex =
      DataAdapters::ReadNumberField(Parsing, TEXT("firstTokenIndex"));
  Settings.SecondTokenIndex =
      DataAdapters::ReadNumberField(Parsing, TEXT("secondTokenIndex"));
  Settings.NextIndexOffset =
      DataAdapters::ReadNumberField(Parsing, TEXT("nextIndexOffset"));
  Settings.DirectDropCount =
      DataAdapters::ReadNumberField(Parsing, TEXT("directDropCount"));
  Settings.SubcommandDropCount =
      DataAdapters::ReadNumberField(Parsing, TEXT("subcommandDropCount"));
  Settings.MinimumTokenCount =
      DataAdapters::ReadNumberField(Parsing, TEXT("minimumTokenCount"));
  return Settings;
}

/** User Story: As a features cli consumer, I need to invoke read invocation settings through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCLIInvocationSettings ReadInvocationSettings( const TSharedRef<FJsonObject> &Invocation) */
inline FCLIInvocationSettings ReadInvocationSettings(
    const TSharedRef<FJsonObject> &Invocation) {
  FCLIInvocationSettings Settings;
  Settings.ApiUrlOptions = DataAdapters::ReadStringArrayField(
      Invocation, TEXT("apiUrlOptions"));
  Settings.ApiKeyOptions = DataAdapters::ReadStringArrayField(
      Invocation, TEXT("apiKeyOptions"));
  Settings.AssignmentSeparator = DataAdapters::ReadStringField(
      Invocation, TEXT("assignmentSeparator"));
  Settings.OptionValueOffset = DataAdapters::ReadNumberField(
      Invocation, TEXT("optionValueOffset"));
  return Settings;
}

/** User Story: As a features cli consumer, I need to invoke read command roles through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCLICommandRoles ReadCommandRoles( const TSharedRef<FJsonObject> &Roles) */
inline FCLICommandRoles ReadCommandRoles(
    const TSharedRef<FJsonObject> &Roles) {
  return {
      DataAdapters::ReadStringField(Roles, TEXT("version")),
      DataAdapters::ReadStringField(Roles, TEXT("doctor")),
      DataAdapters::ReadStringField(Roles, TEXT("status")),
      DataAdapters::ReadStringField(Roles, TEXT("setup")),
      DataAdapters::ReadStringField(Roles, TEXT("setupCheck")),
      DataAdapters::ReadStringField(Roles, TEXT("configSet")),
      DataAdapters::ReadStringField(Roles, TEXT("configGet")),
      DataAdapters::ReadStringField(Roles, TEXT("configList")),
      DataAdapters::ReadStringField(Roles, TEXT("npcCreate")),
      DataAdapters::ReadStringField(Roles, TEXT("npcState")),
      DataAdapters::ReadStringField(Roles, TEXT("npcUpdate")),
      DataAdapters::ReadStringField(Roles, TEXT("npcProcess")),
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

/** User Story: As a features cli consumer, I need to invoke read cli state through a stable signature so the features cli workflow remains explicit and composable. @fn inline FCLIState readCliState() */
inline FCLIState readCliState() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/cli/commands.json"));
  const TSharedRef<FJsonObject> SurfaceRoles =
      DataAdapters::ReadObjectField(Source, TEXT("surfaceRoles"));
  FCLIState State;
  State.Commands = ReadCommandData(
      DataAdapters::ReadObjectField(Source, TEXT("commands")));
  State.NodeSurface =
      DataAdapters::ReadStringField(SurfaceRoles, TEXT("node"));
  State.BrowserSurface =
      DataAdapters::ReadStringField(SurfaceRoles, TEXT("browser"));
  State.Parsing = ReadParsingSettings(
      DataAdapters::ReadObjectField(Source, TEXT("parsing")));
  State.Invocation = ReadInvocationSettings(
      DataAdapters::ReadObjectField(Source, TEXT("invocation")));
  State.CommandRoles = ReadCommandRoles(
      DataAdapters::ReadObjectField(Source, TEXT("commandRoles")));
  State.Bridge = Bridge::readCliBridgeState();
  State.Ghost = Ghost::readCliGhostState();
  State.Memory = Memory::readCliMemoryState();
  State.NPC = NPC::readCliNpcState();
  State.Soul = Soul::readCliSoulState();
  return State;
}

} // namespace CLI
} // namespace ForbocAI
