#pragma once

#include "Components/CLI/CLITypes.h"
#include "Core/fp.hpp"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace ForbocAI::CLI::Catalog {

/** User Story: As a CLI catalog decoder, I need nullable JSON strings lifted into one value convention so command records stay typed. @fn inline FString ReadOptionalText(const func::Maybe<FString> &Value) */
inline FString ReadOptionalText(const func::Maybe<FString> &Value) {
  return Value.hasValue ? Value.value : FString();
}

/** User Story: As a CLI catalog decoder, I need one command record decoded from its domain-owned JSON object. @fn inline FCLICommandSpec ReadCommandSpec( const FString &Key, const TSharedRef<FJsonObject> &Command) */
inline FCLICommandSpec ReadCommandSpec(
    const FString &Key, const TSharedRef<FJsonObject> &Command) {
  FCLICommandSpec Spec;
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

/** User Story: As a CLI catalog decoder, I need one command subdomain traversed without hard-coded command keys. @fn inline TArray<FCLICommandSpec> ReadCommandData( const TSharedRef<FJsonObject> &Commands) */
inline TArray<FCLICommandSpec> ReadCommandData(
    const TSharedRef<FJsonObject> &Commands) {
  const TArray<FString> Keys = func::map_array(
      func::map_keys(Commands->Values),
      [](const FJsonObject::FStringType &Key) { return FString(Key.ToView()); });
  return func::map_array<FString, FCLICommandSpec>(
      Keys, [Commands](const FString &Key) {
        return ReadCommandSpec(
            Key, DataAdapters::ReadObjectField(Commands, Key));
      });
}

/** User Story: As the package CLI root, I need ordered command subdomains composed from a JSON-owned catalog so adding a command never recreates a monolith. @fn inline TArray<FCLICommandSpec> ReadCommandCatalogs( const TArray<FString> &Paths) */
inline TArray<FCLICommandSpec> ReadCommandCatalogs(
    const TArray<FString> &Paths) {
  return func::concat_arrays(func::map_array<FString, TArray<FCLICommandSpec>>(
      Paths, [](const FString &Path) {
        return ReadCommandData(
            DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"), Path).Root);
      }));
}

} // namespace ForbocAI::CLI::Catalog
