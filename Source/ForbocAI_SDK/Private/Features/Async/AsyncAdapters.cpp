#include "Features/Async/AsyncAdapters.h"

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"

namespace AsyncAdapters {
namespace {

/** User Story: As an Async settings owner, I need wait timing and messages loaded inside the SDK module so host modules remain decoupled from plugin data infrastructure. @fn FAsyncWaitSettings readAsyncWaitSettings() */
FAsyncWaitSettings readAsyncWaitSettings() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/async/wait.json"));
  const TSharedRef<FJsonObject> Timing =
      DataAdapters::ReadObjectField(Source, TEXT("timing"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {{DataAdapters::ReadFloatField(Timing,
                                        TEXT("unboundedTimeoutSeconds")),
           DataAdapters::ReadFloatField(Timing, TEXT("pollIntervalSeconds"))},
          {DataAdapters::ReadStringField(Messages, TEXT("timeout"))}};
}

} // namespace

/** User Story: As an Async adapter consumer, I need one immutable wait policy shared by every blocking adaptation without linking host modules to the SDK data loader. @fn const FAsyncWaitSettings &asyncWaitSettings() */
const FAsyncWaitSettings &asyncWaitSettings() {
  static const FAsyncWaitSettings Settings = readAsyncWaitSettings();
  return Settings;
}

} // namespace AsyncAdapters
