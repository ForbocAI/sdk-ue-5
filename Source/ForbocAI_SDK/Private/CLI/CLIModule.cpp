#include "CLI/CLIModule.h"
#include "CLI/CliHandlers.h"
#include "Store.h"
#include <exception>

namespace CLIOps {

namespace {

/** User Story: As a cli consumer, I need to invoke get store through a stable signature so the cli workflow remains explicit and composable. @fn rtk::EnhancedStore<FRuntimeState> &GetStore() */
rtk::EnhancedStore<FRuntimeState> &GetStore() {
  return store();
}

} // namespace

/** User Story: As a cli consumer, I need to invoke dispatch command through a stable signature so the cli workflow remains explicit and composable. @fn func::TestResult<void> DispatchCommand(const FString &CommandKey, const TArray<FString> &Args) */
func::TestResult<void> DispatchCommand(const FString &CommandKey,
                                       const TArray<FString> &Args) {
  using Result = func::TestResult<void>;
  using namespace Handlers;
  using Handler = std::function<HandlerResult(rtk::EnhancedStore<FRuntimeState> &,
                                              const FString &,
                                              const TArray<FString> &)>;

  rtk::EnhancedStore<FRuntimeState> &Store = GetStore();

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
          const TArray<FString> &Args) {
      return Index >= Hs.size()
                 ? Result::Failure(TCHAR_TO_UTF8(
                       *FString::Printf(TEXT("Unknown command: %s"), *Key)))
                 : [&]() -> func::TestResult<void> {
                     HandlerResult R = Hs[Index](Store, Key, Args);
                     return R.hasValue ? R.value
                                       : apply(Hs, Index + 1, Store, Key, Args);
                   }();
    }
  };

  try {
    return DispatchRecursive::apply(Handlers, 0, Store, CommandKey, Args);
  } catch (const std::exception &Error) {
    return Result::Failure(std::string(Error.what()));
  }
}

} // namespace CLIOps
