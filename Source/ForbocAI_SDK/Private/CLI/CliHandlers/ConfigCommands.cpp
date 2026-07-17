// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/Config/ConfigThunks.h"
#include "Features/CLI/Presentation/PresentationSelectors.h"
#include "Store.h"

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle config through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleConfig(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleConfig(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::FCLIParsingSettings &Parsing = State.CLI.Parsing;
  const ForbocAI::CLI::FCLICommandRoles &Roles = State.CLI.CommandRoles;
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);

  return CommandKey == Roles.ConfigSet
             ? (!Args.IsValidIndex(Parsing.SecondTokenIndex)
                    ? just(Result::Failure(TCHAR_TO_UTF8(
                          *PresentationState.Common.ConfigSetUsage)))
                    : (Ops::setConfigValue(
                           Store, Args[Parsing.FirstTokenIndex],
                           Args[Parsing.SecondTokenIndex])
                           ? just(Result::Success(TCHAR_TO_UTF8(
                                 *PresentationState.Results.ConfigUpdated)))
                           : just(Result::Failure(TCHAR_TO_UTF8(
                                 *PresentationState.Results
                                      .ConfigPersistenceFailed)))))
         : CommandKey == Roles.ConfigGet
             ? (!Args.IsValidIndex(Parsing.FirstTokenIndex)
                    ? just(Result::Failure(TCHAR_TO_UTF8(
                          *PresentationState.Common.ConfigGetUsage)))
                    : [&]() -> HandlerResult {
                        const FString &Key = Args[Parsing.FirstTokenIndex];
                        const FString Value = Ops::getConfigValue(State, Key);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            ForbocAI::CLI::Presentation::
                                selectCliConfigEntryLine(
                                    PresentationState, Key, Value,
                                    Key == ConfigSlice::configRuntimeData()
                                               .Fields.ApiKey,
                                    false));
                        return just(Result::Success(TCHAR_TO_UTF8(
                            *PresentationState.Results.ConfigRetrieved)));
                      }())
         : CommandKey == Roles.ConfigList
             ? [&]() -> HandlerResult {
                 const ConfigSlice::FConfigRuntimeData &Data =
                     ConfigSlice::configRuntimeData();
                 const TArray<FString> ConfigKeys = {
                     Data.Fields.SdkVersion, Data.Fields.ApiUrl,
                     Data.Fields.ApiKey,
                     Data.Fields.DatabasePath, Data.Fields.VectorDimension,
                     Data.Fields.MaxRecallResults};
                 struct LogKeys {
                   static void apply(const TArray<FString> &Keys, int32 Idx,
                                     const FRuntimeState &State,
                                     const FString &ApiKeyField,
                                     const ForbocAI::CLI::FCLIParsingSettings
                                         &Parsing,
                                     const ForbocAI::CLI::Presentation::
                                         FCLIPresentationState
                                             &PresentationState) {
                     Idx >= Keys.Num()
                         ? void()
                         : ([&]() {
                              const FString Val =
                                  Ops::getConfigValue(State, Keys[Idx]);
                              const bool bMask = Keys[Idx] == ApiKeyField;
                              ForbocAI::CLI::Presentation::logCliMessage(
                                  ForbocAI::CLI::Presentation::
                                      selectCliConfigEntryLine(
                                          PresentationState, Keys[Idx], Val,
                                          bMask, true));
                            }(),
                            apply(Keys, Idx + Parsing.NextIndexOffset, State,
                                  ApiKeyField, Parsing, PresentationState),
                            void());
                   }
                 };
                 LogKeys::apply(ConfigKeys, Parsing.FirstTokenIndex, State,
                                Data.Fields.ApiKey, Parsing,
                                PresentationState);
                 return just(Result::Success(TCHAR_TO_UTF8(
                     *PresentationState.Results.ConfigListed)));
               }()
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
