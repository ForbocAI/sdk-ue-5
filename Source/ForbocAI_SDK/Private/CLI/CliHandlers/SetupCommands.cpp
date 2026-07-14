#include "CLI/CliHandlers.h"
#include "Features/CLI/Native/NativeThunks.h"
#include "Store.h"

namespace {

void printDependencyDetail(const FString &Detail) {
  UE_LOG(LogTemp, Display, TEXT("       %s"), *Detail);
}

void printDependency(const FNativeDependencyStatus &Dependency) {
  UE_LOG(LogTemp, Display, TEXT("  [%s] %s%s%s"),
         Dependency.bAvailable ? TEXT("OK") : TEXT("--"), *Dependency.Name,
         Dependency.Version.IsEmpty() ? TEXT("") : TEXT(" ("),
         Dependency.Version.IsEmpty()
             ? TEXT("")
             : *FString::Printf(TEXT("%s)"), *Dependency.Version));
  Dependency.Detail.IsEmpty()
      ? void()
      : printDependencyDetail(Dependency.Detail);
}

CLIOps::Handlers::Result checkDependencies(
    rtk::EnhancedStore<FRuntimeState> &Store) {
  const FNativeDependenciesReport Report = Ops::checkNativeDependencies(Store);
  UE_LOG(LogTemp, Display, TEXT("ForbocAI Native Dependency Check"));
  printDependency(Report.Vectorizer);
  printDependency(Report.VectorDb);
  return Report.Vectorizer.bAvailable && Report.VectorDb.bAvailable
             ? CLIOps::Handlers::Result::Success(
                   "Native dependencies are ready")
             : CLIOps::Handlers::Result::Failure(
                   "Native dependencies are not ready");
}

CLIOps::Handlers::Result
runSetup(rtk::EnhancedStore<FRuntimeState> &Store,
         const TArray<FString> &Arguments) {
  FDependenciesOptions Options;
  Options.bForce = Arguments.Contains(TEXT("--force"));
  Options.bVerbose = Arguments.Contains(TEXT("--verbose"));
  const FDependenciesResult Result =
      Options.bForce ? Ops::refreshNativeDependencies(Store, Options)
                     : Ops::setupNativeDependencies(Store, Options);

  UE_LOG(LogTemp, Display, TEXT("ForbocAI Native Setup%s"),
         Options.bForce ? TEXT(" (force)") : TEXT(""));
  UE_LOG(LogTemp, Display, TEXT("  [%s] Vector: %s"),
         Result.Vector.bOk ? TEXT("OK") : TEXT("--"),
         *Result.Vector.Detail);
  UE_LOG(LogTemp, Display, TEXT("  [%s] Memory: %s"),
         Result.Memory.bOk ? TEXT("OK") : TEXT("--"),
         *Result.Memory.Detail);
  return Result.Vector.bOk && Result.Memory.bOk
             ? CLIOps::Handlers::Result::Success(
                   "Native dependency setup completed")
             : CLIOps::Handlers::Result::Failure(
                   "Native dependency setup did not complete");
}

} // namespace

namespace CLIOps {
namespace Handlers {

HandlerResult HandleSetup(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Arguments) {
  using func::just;
  using func::nothing;
  return CommandKey == TEXT("dependencies")
             ? just(runSetup(Store, Arguments))
         : CommandKey == TEXT("setup_check")
             ? just(checkDependencies(Store))
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
