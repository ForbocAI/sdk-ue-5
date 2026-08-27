// User Story: As a developer, I need this module to function.
#include "Systems/CLI/ConfigurationCommands/ConfigurationCommandsThunks.h"
#include "Systems/Config/ConfigThunks.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Entities/CLI/Presentation/PresentationSelectors.h"

namespace {

using ConfigResult = CLIOps::CommandRouting::Result;

/** User Story: As a CLI command-routing config consumer, I need to invoke set config value through a stable signature so the CLI command-routing config workflow remains explicit and composable. @fn ConfigResult SetConfigValue( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
ConfigResult SetConfigValue(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  return CLIOps::CommandRouting::matchCondition<ConfigResult>(
      Args.IsValidIndex(Parsing.SecondTokenIndex),
      [&]() {
        return Ops::setConfigValue(Store, Args[Parsing.FirstTokenIndex],
                                   Args[Parsing.SecondTokenIndex])
                   ? ConfigResult::Success(TCHAR_TO_UTF8(
                         *PresentationState.Results.ConfigUpdated))
                   : ConfigResult::Failure(TCHAR_TO_UTF8(
                         *PresentationState.Results.ConfigPersistenceFailed));
      },
      [&]() {
        return ConfigResult::Failure(
            TCHAR_TO_UTF8(*PresentationState.Common.ConfigSetUsage));
      });
}

/** User Story: As a CLI command-routing config consumer, I need to invoke get config value through a stable signature so the CLI command-routing config workflow remains explicit and composable. @fn ConfigResult GetConfigValue( const FRuntimeState &State, const TArray<FString> &Args, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
ConfigResult GetConfigValue(
    const FRuntimeState &State, const TArray<FString> &Args,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  return CLIOps::CommandRouting::matchCondition<ConfigResult>(
      Args.IsValidIndex(Parsing.FirstTokenIndex),
      [&]() {
        const FString &Key = Args[Parsing.FirstTokenIndex];
        const FString Value = Ops::getConfigValue(State, Key);
        ForbocAI::CLI::Presentation::logCliMessage(
            ForbocAI::CLI::Presentation::selectCliConfigEntryLine(
                PresentationState, Key, Value,
                Key == ConfigSlice::configRuntimeData().Fields.ApiKey, false));
        return ConfigResult::Success(
            TCHAR_TO_UTF8(*PresentationState.Results.ConfigRetrieved));
      },
      [&]() {
        return ConfigResult::Failure(
            TCHAR_TO_UTF8(*PresentationState.Common.ConfigGetUsage));
      });
}

/** User Story: As a CLI command-routing config consumer, I need to invoke log config keys through a stable signature so the CLI command-routing config workflow remains explicit and composable. @fn void LogConfigKeys( const TArray<FString> &Keys, int32 Index, const FRuntimeState &State, const ConfigSlice::FConfigRuntimeData &Data, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
void LogConfigKeys(
    const TArray<FString> &Keys, int32 Index, const FRuntimeState &State,
    const ConfigSlice::FConfigRuntimeData &Data,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  CLIOps::CommandRouting::matchCondition<void>(
      Index < Keys.Num(),
      [&]() {
        const FString Value = Ops::getConfigValue(State, Keys[Index]);
        ForbocAI::CLI::Presentation::logCliMessage(
            ForbocAI::CLI::Presentation::selectCliConfigEntryLine(
                PresentationState, Keys[Index], Value,
                Keys[Index] == Data.Fields.ApiKey, true));
        LogConfigKeys(Keys, Index + Parsing.NextIndexOffset, State, Data,
                      Parsing, PresentationState);
      },
      []() {});
}

/** User Story: As a CLI command-routing config consumer, I need to invoke list config values through a stable signature so the CLI command-routing config workflow remains explicit and composable. @fn ConfigResult ListConfigValues( const FRuntimeState &State, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
ConfigResult ListConfigValues(
    const FRuntimeState &State,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  const ConfigSlice::FConfigRuntimeData &Data =
      ConfigSlice::configRuntimeData();
  const TArray<FString> ConfigKeys = {
      Data.Fields.SdkVersion,       Data.Fields.ApiUrl,
      Data.Fields.ApiKey,           Data.Fields.DatabasePath,
      Data.Fields.VectorDimension,  Data.Fields.MaxRecallResults,
  };
  LogConfigKeys(ConfigKeys, Parsing.FirstTokenIndex, State, Data, Parsing,
                PresentationState);
  return ConfigResult::Success(
      TCHAR_TO_UTF8(*PresentationState.Results.ConfigListed));
}

/** User Story: As a CLI command-routing config consumer, I need to invoke create config dispatcher through a stable signature so the CLI command-routing config workflow remains explicit and composable. @fn func::Dispatcher<FString, ConfigResult> CreateConfigDispatcher( rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
func::Dispatcher<FString, ConfigResult>
CreateConfigDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State,
    const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  return func::createDispatcher<FString, ConfigResult>({
      {Roles.ConfigSet,
       [&]() { return SetConfigValue(Store, Args, Parsing, PresentationState); }},
      {Roles.ConfigGet,
       [&]() { return GetConfigValue(State, Args, Parsing, PresentationState); }},
      {Roles.ConfigList,
       [&]() { return ListConfigValues(State, Parsing, PresentationState); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI command-routing consumer, I need to invoke handle config through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn RouteResult RouteConfigurationCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteConfigurationCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args) {
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::FCLIParsingSettings &Parsing = State.CLI.Parsing;
  const ForbocAI::CLI::FCLICommandRoles &Roles = State.CLI.CommandRoles;
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);
  return func::dispatch(CreateConfigDispatcher(Store, State, Roles, Args,
                                                Parsing, PresentationState),
                        CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
