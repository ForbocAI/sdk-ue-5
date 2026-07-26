#include "CLI/CLIModule.h"
#include "CLI/CliHandlers.h"
#include "Features/CLI/Presentation/PresentationSelectors.h"
#include "Features/CLI/Config/ConfigThunks.h"
#include "Store.h"
#include <exception>

namespace CLIOps {

namespace {

/** User Story: As a cli consumer, I need to invoke get store through a stable signature so the cli workflow remains explicit and composable. @fn rtk::EnhancedStore<FRuntimeState> &GetStore() */
rtk::EnhancedStore<FRuntimeState> &GetStore() {
  return store();
}

} // namespace

/** User Story: As an embedded CLI host, I need invocation overrides delegated to the SDK config thunk and root store. @fn void ConfigureInvocation(const FString &ApiUrl, const FString &ApiKey) */
void ConfigureInvocation(const FString &ApiUrl, const FString &ApiKey) {
  Ops::hydrateRuntimeConfig(GetStore(), {ApiUrl, ApiKey});
}

/** User Story: As a cli consumer, I need to invoke dispatch command through a stable signature so the cli workflow remains explicit and composable. @fn func::TestResult<void> DispatchCommand(const FString &CommandKey, const TArray<FString> &Args) */
func::TestResult<void> DispatchCommand(const FString &CommandKey,
                                       const TArray<FString> &Args) {
  using Result = func::TestResult<void>;
  using namespace Handlers;
  using Handler = std::function<HandlerResult(rtk::EnhancedStore<FRuntimeState> &,
                                              const FString &,
                                              const TArray<FString> &)>;

  rtk::EnhancedStore<FRuntimeState> &Store = GetStore();
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);

  /**
   * Phase 3.4: Handler chain — first match wins
   * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
   */
  static const std::vector<Handler> Handlers = {
      HandleSystem, HandleNpc,    HandleMemory, HandleGhost,
      HandleBridge, HandleSoul,   HandleConfig, HandleSetup,
  };

  struct DispatchRecursive {
    static func::TestResult<void>
    apply(const std::vector<Handler> &Hs, size_t Index,
          rtk::EnhancedStore<FRuntimeState> &Store, const FString &Key,
          const TArray<FString> &Args,
          const ForbocAI::CLI::FCLIParsingSettings &Parsing,
          const ForbocAI::CLI::Presentation::FCLIPresentationState
              &PresentationState) {
      return Index >= Hs.size()
                 ? Result::Failure(TCHAR_TO_UTF8(
                       *ForbocAI::CLI::Presentation::
                           selectCliUnknownCommandMessage(PresentationState,
                                                          Key)))
                 : [&]() -> func::TestResult<void> {
                     HandlerResult R = Hs[Index](Store, Key, Args);
                     return R.hasValue ? R.value
                                       : apply(
                                             Hs,
                                             Index + Parsing.NextIndexOffset,
                                             Store, Key, Args, Parsing,
                                             PresentationState);
                   }();
    }
  };

  try {
    return DispatchRecursive::apply(
        Handlers, State.CLI.Parsing.FirstTokenIndex, Store, CommandKey, Args,
        State.CLI.Parsing, PresentationState);
  } catch (const std::exception &Error) {
    return Result::Failure(std::string(Error.what()));
  }
}

} // namespace CLIOps
