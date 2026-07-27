#pragma once

#include "CoreMinimal.h"
#include "Components/CLI/Presentation/PresentationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Misc/OutputDevice.h"

namespace ForbocAI {
namespace CLI {
namespace Presentation {

/** User Story: As CLI presentation bootstrap, I need authored common messages decoded into typed package state. @fn inline FCLIPresentationCommonMessages readCliPresentationCommonMessages( const TSharedRef<FJsonObject> &Common) */
inline FCLIPresentationCommonMessages readCliPresentationCommonMessages(
    const TSharedRef<FJsonObject> &Common) {
  return {
      DataAdapters::ReadStringField(Common, TEXT("blank")),
      DataAdapters::ReadStringField(Common, TEXT("version")),
      DataAdapters::ReadStringField(Common, TEXT("status")),
      DataAdapters::ReadStringField(Common, TEXT("apiUrl")),
      DataAdapters::ReadStringField(Common, TEXT("apiKey")),
      DataAdapters::ReadStringField(Common, TEXT("apiKeyMissing")),
      DataAdapters::ReadStringField(Common, TEXT("maskedValue")),
      DataAdapters::ReadStringField(Common, TEXT("apiStatus")),
      DataAdapters::ReadStringField(Common, TEXT("configSetUsage")),
      DataAdapters::ReadStringField(Common, TEXT("configGetUsage")),
      DataAdapters::ReadStringField(Common, TEXT("configEntry")),
      DataAdapters::ReadStringField(Common, TEXT("configListEntry")),
  };
}

/** User Story: As CLI execution, I need authored runtime messages decoded into typed package state. @fn inline FCLIPresentationRuntimeMessages readCliPresentationRuntimeMessages( const TSharedRef<FJsonObject> &Runtime) */
inline FCLIPresentationRuntimeMessages readCliPresentationRuntimeMessages(
    const TSharedRef<FJsonObject> &Runtime) {
  return {
      DataAdapters::ReadStringField(Runtime, TEXT("commandStarted")),
      DataAdapters::ReadStringField(Runtime, TEXT("commandSucceeded")),
      DataAdapters::ReadStringField(Runtime, TEXT("commandFailed")),
      DataAdapters::ReadStringField(Runtime, TEXT("dispatchFailed")),
      DataAdapters::ReadStringField(Runtime, TEXT("emptyCommand")),
      DataAdapters::ReadStringField(Runtime, TEXT("invalidCommand")),
      DataAdapters::ReadStringField(Runtime, TEXT("unknownCommand")),
  };
}

/** User Story: As CLI handlers, I need authored semantic results decoded into typed package state. @fn inline FCLIPresentationResultMessages readCliPresentationResultMessages( const TSharedRef<FJsonObject> &Results) */
inline FCLIPresentationResultMessages readCliPresentationResultMessages(
    const TSharedRef<FJsonObject> &Results) {
  return {
      DataAdapters::ReadStringField(Results, TEXT("configUpdated")),
      DataAdapters::ReadStringField(Results, TEXT("configPersistenceFailed")),
      DataAdapters::ReadStringField(Results, TEXT("configRetrieved")),
      DataAdapters::ReadStringField(Results, TEXT("configListed")),
      DataAdapters::ReadStringField(Results, TEXT("versionPrinted")),
      DataAdapters::ReadStringField(Results, TEXT("statusChecked")),
      DataAdapters::ReadStringField(Results, TEXT("doctorCompleted")),
  };
}

/** User Story: As the UE package root store, I need CLI presentation authored data loaded once into immutable slice state. @fn inline FCLIPresentationState readCliPresentation() */
inline FCLIPresentationState readCliPresentation() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/cli/presentation.json"));
  const TSharedRef<FJsonObject> Slice =
      DataAdapters::ReadObjectField(Source, TEXT("slice"));
  const TSharedRef<FJsonObject> Defaults =
      DataAdapters::ReadObjectField(Source, TEXT("defaults"));
  return {
      DataAdapters::ReadStringField(Slice, TEXT("name")),
      {DataAdapters::ReadNumberField(Defaults, TEXT("successExitCode")),
       DataAdapters::ReadNumberField(Defaults, TEXT("failureExitCode"))},
      readCliPresentationCommonMessages(
          DataAdapters::ReadObjectField(Source, TEXT("common"))),
      readCliPresentationRuntimeMessages(
          DataAdapters::ReadObjectField(Source, TEXT("runtime"))),
      readCliPresentationResultMessages(
          DataAdapters::ReadObjectField(Source, TEXT("results"))),
  };
}

/** User Story: As a features cli presentation consumer, I need to invoke format cli message through a stable signature so the features cli presentation workflow remains explicit and composable. @fn template <typename... Values> inline FString formatCliMessage(const FString &Template, Values &&...Arguments) */
template <typename... Values>
inline FString formatCliMessage(const FString &Template,
                                Values &&...Arguments) {
  return FString::Format(*Template,
                         {Forward<Values>(Arguments)...});
}

/** User Story: As a features cli presentation consumer, I need to invoke log cli message through a stable signature so the features cli presentation workflow remains explicit and composable. @fn inline void logCliMessage(const FString &Message) */
inline void logCliMessage(const FString &Message) {
  GLog != nullptr ? GLog->Log(*Message) : void();
}

/** User Story: As CLI failure presentation, I need errors emitted with Unreal error severity through a stable presentation boundary. @fn inline void logCliError(const FString &Message) */
inline void logCliError(const FString &Message) {
  GLog != nullptr
      ? GLog->Serialize(*Message, ELogVerbosity::Error, NAME_None)
      : void();
}

/** User Story: As a features cli presentation consumer, I need to invoke log cli message when through a stable signature so the features cli presentation workflow remains explicit and composable. @fn inline void logCliMessageWhen(bool Predicate, const FString &Message) */
inline void logCliMessageWhen(bool Predicate, const FString &Message) {
  Predicate ? logCliMessage(Message) : void();
}

} // namespace Presentation
} // namespace CLI
} // namespace ForbocAI
