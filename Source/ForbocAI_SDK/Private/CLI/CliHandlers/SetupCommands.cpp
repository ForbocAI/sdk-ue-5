#include "CLI/CliHandlers.h"
#include "Features/CLI/Native/NativeThunks.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Store.h"

namespace {

struct FSetupPresentation {
  FString DetailFormat;
  FString DependencyFormat;
  FString AvailableMarker;
  FString UnavailableMarker;
  FString VersionPrefix;
  FString VersionSuffixFormat;
  FString CheckTitle;
  FString ReadyMessage;
  FString NotReadyMessage;
  FString ForceFlag;
  FString VerboseFlag;
  FString SetupTitleFormat;
  FString ForceSuffix;
  FString VectorFormat;
  FString MemoryFormat;
  FString CompletedMessage;
  FString FailedMessage;
};

/** User Story: As a cli cli handlers consumer, I need to invoke setup presentation through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn const FSetupPresentation &setupPresentation() */
const FSetupPresentation &setupPresentation() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/cli/setup.json"));
  static const FSetupPresentation Presentation = {
      DataAdapters::ReadStringField(Source.Root, TEXT("detailFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("dependencyFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("availableMarker")),
      DataAdapters::ReadStringField(Source.Root, TEXT("unavailableMarker")),
      DataAdapters::ReadStringField(Source.Root, TEXT("versionPrefix")),
      DataAdapters::ReadStringField(Source.Root, TEXT("versionSuffixFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("checkTitle")),
      DataAdapters::ReadStringField(Source.Root, TEXT("readyMessage")),
      DataAdapters::ReadStringField(Source.Root, TEXT("notReadyMessage")),
      DataAdapters::ReadStringField(Source.Root, TEXT("forceFlag")),
      DataAdapters::ReadStringField(Source.Root, TEXT("verboseFlag")),
      DataAdapters::ReadStringField(Source.Root, TEXT("setupTitleFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("forceSuffix")),
      DataAdapters::ReadStringField(Source.Root, TEXT("vectorFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("memoryFormat")),
      DataAdapters::ReadStringField(Source.Root, TEXT("completedMessage")),
      DataAdapters::ReadStringField(Source.Root, TEXT("failedMessage")),
  };
  return Presentation;
}

/** User Story: As a cli cli handlers consumer, I need to invoke print line through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn void printLine(const FString &Line) */
void printLine(const FString &Line) {
  UE_LOG(LogTemp, Display, TEXT("%s"), *Line);
}

/** User Story: As a cli cli handlers consumer, I need to invoke print dependency detail through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn void printDependencyDetail(const FString &Detail) */
void printDependencyDetail(const FString &Detail) {
  printLine(FString::Format(*setupPresentation().DetailFormat, {Detail}));
}

/** User Story: As a cli cli handlers consumer, I need to invoke print dependency through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn void printDependency(const FNativeDependencyStatus &Dependency) */
void printDependency(const FNativeDependencyStatus &Dependency) {
  const FSetupPresentation &Presentation = setupPresentation();
  const FString VersionSuffix = Dependency.Version.IsEmpty()
                                    ? FString()
                                    : FString::Format(
                                          *Presentation.VersionSuffixFormat,
                                          {Dependency.Version});
  printLine(FString::Format(
      *Presentation.DependencyFormat,
      {Dependency.bAvailable ? Presentation.AvailableMarker
                             : Presentation.UnavailableMarker,
       Dependency.Name,
       Dependency.Version.IsEmpty() ? FString() : Presentation.VersionPrefix,
       VersionSuffix}));
  Dependency.Detail.IsEmpty()
      ? void()
      : printDependencyDetail(Dependency.Detail);
}

/** User Story: As a cli cli handlers consumer, I need to invoke check dependencies through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result checkDependencies( rtk::EnhancedStore<FRuntimeState> &Store) */
CLIOps::Handlers::Result checkDependencies(
    rtk::EnhancedStore<FRuntimeState> &Store) {
  const FSetupPresentation &Presentation = setupPresentation();
  const FNativeDependenciesReport Report = Ops::checkNativeDependencies(Store);
  printLine(Presentation.CheckTitle);
  printDependency(Report.Vectorizer);
  printDependency(Report.VectorDb);
  return Report.Vectorizer.bAvailable && Report.VectorDb.bAvailable
             ? CLIOps::Handlers::Result::Success(
                   TCHAR_TO_UTF8(*Presentation.ReadyMessage))
             : CLIOps::Handlers::Result::Failure(
                   TCHAR_TO_UTF8(*Presentation.NotReadyMessage));
}

/** User Story: As a cli cli handlers consumer, I need to invoke run setup through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result runSetup(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Arguments) */
CLIOps::Handlers::Result
runSetup(rtk::EnhancedStore<FRuntimeState> &Store,
         const TArray<FString> &Arguments) {
  const FSetupPresentation &Presentation = setupPresentation();
  FDependenciesOptions Options;
  Options.bForce = Arguments.Contains(Presentation.ForceFlag);
  Options.bVerbose = Arguments.Contains(Presentation.VerboseFlag);
  const FDependenciesResult Result =
      Options.bForce ? Ops::refreshNativeDependencies(Store, Options)
                     : Ops::setupNativeDependencies(Store, Options);

  printLine(FString::Format(
      *Presentation.SetupTitleFormat,
      {Options.bForce ? Presentation.ForceSuffix : FString()}));
  printLine(FString::Format(
      *Presentation.VectorFormat,
      {Result.Vector.bOk ? Presentation.AvailableMarker
                         : Presentation.UnavailableMarker,
       Result.Vector.Detail}));
  printLine(FString::Format(
      *Presentation.MemoryFormat,
      {Result.Memory.bOk ? Presentation.AvailableMarker
                         : Presentation.UnavailableMarker,
       Result.Memory.Detail}));
  return Result.Vector.bOk && Result.Memory.bOk
             ? CLIOps::Handlers::Result::Success(
                   TCHAR_TO_UTF8(*Presentation.CompletedMessage))
             : CLIOps::Handlers::Result::Failure(
                   TCHAR_TO_UTF8(*Presentation.FailedMessage));
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle setup through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleSetup(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Arguments) */
HandlerResult HandleSetup(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Arguments) {
  using func::just;
  using func::nothing;
  return CommandKey == TEXT("setup")
             ? just(runSetup(Store, Arguments))
         : CommandKey == TEXT("setup_check")
             ? just(checkDependencies(Store))
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
