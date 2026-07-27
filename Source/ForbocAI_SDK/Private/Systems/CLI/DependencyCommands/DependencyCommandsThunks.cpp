#include "Systems/CLI/DependencyCommands/DependencyCommandsThunks.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/CLI/Native/NativeThunks.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

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

/** User Story: As a CLI command-routing consumer, I need to invoke setup presentation through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn const FSetupPresentation &setupPresentation() */
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

/** User Story: As a CLI command-routing consumer, I need to invoke print line through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn void printLine(const FString &Line) */
void printLine(const FString &Line) {
  UE_LOG(LogTemp, Display, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV03A110C67C3C), *Line);
}

/** User Story: As a CLI command-routing consumer, I need to invoke print dependency detail through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn void printDependencyDetail(const FString &Detail) */
void printDependencyDetail(const FString &Detail) {
  printLine(FString::Format(*setupPresentation().DetailFormat, {Detail}));
}

/** User Story: As a CLI command-routing consumer, I need to invoke print dependency through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn void printDependency(const FNativeDependencyStatus &Dependency) */
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

/** User Story: As a CLI command-routing consumer, I need to invoke check dependencies through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result checkDependencies( rtk::EnhancedStore<FRuntimeState> &Store) */
CLIOps::CommandRouting::Result checkDependencies(
    rtk::EnhancedStore<FRuntimeState> &Store) {
  const FSetupPresentation &Presentation = setupPresentation();
  const FNativeDependenciesReport Report = Ops::checkNativeDependencies(Store);
  printLine(Presentation.CheckTitle);
  printDependency(Report.Vectorizer);
  printDependency(Report.VectorDb);
  return Report.Vectorizer.bAvailable && Report.VectorDb.bAvailable
             ? CLIOps::CommandRouting::Result::Success(
                   TCHAR_TO_UTF8(*Presentation.ReadyMessage))
             : CLIOps::CommandRouting::Result::Failure(
                   TCHAR_TO_UTF8(*Presentation.NotReadyMessage));
}

/** User Story: As a CLI command-routing consumer, I need to invoke run setup through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result runSetup(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Arguments) */
CLIOps::CommandRouting::Result
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
             ? CLIOps::CommandRouting::Result::Success(
                   TCHAR_TO_UTF8(*Presentation.CompletedMessage))
             : CLIOps::CommandRouting::Result::Failure(
                   TCHAR_TO_UTF8(*Presentation.FailedMessage));
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI command-routing consumer, I need to invoke handle setup through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn RouteResult RouteDependencyCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Arguments) */
RouteResult RouteDependencyCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Arguments) {
  using func::just;
  using func::nothing;
  return CommandKey == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV747A2A3D558C)
             ? just(runSetup(Store, Arguments))
         : CommandKey == TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEE56CC8C5043)
             ? just(checkDependencies(Store))
             : nothing<Result>();
}

} // namespace CommandRouting
} // namespace CLIOps
