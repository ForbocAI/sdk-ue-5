#pragma once

#include "Features/CLI/Soul/CLISoulTypes.h"
#include "Features/Data/DataAdapters.h"

namespace ForbocAI {
namespace CLI {
namespace Soul {

/** User Story: As a CLI Soul consumer, I need authored limits and messages loaded through a stable signature so hosts share deterministic command behavior without source literals. @fn inline FCLISoulState readCliSoulState() */
inline FCLISoulState readCliSoulState() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/cli/soul.json"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Syntax =
      DataAdapters::ReadObjectField(Source, TEXT("syntax"));
  const TSharedRef<FJsonObject> Timeouts =
      DataAdapters::ReadObjectField(Source, TEXT("timeouts"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {
      {DataAdapters::ReadNumberField(Limits, TEXT("emptyItemCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("singleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("doubleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("firstArgumentIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("secondArgumentIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("defaultListLimit"))},
      {DataAdapters::ReadStringField(Syntax, TEXT("listSeparator"))},
      {DataAdapters::ReadFloatField(Timeouts,
                                    TEXT("providerOperationSeconds"))},
      {DataAdapters::ReadStringField(Messages, TEXT("exportUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("exported")),
       DataAdapters::ReadStringField(Messages, TEXT("importUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("imported")),
       DataAdapters::ReadStringField(Messages, TEXT("listEmpty")),
       DataAdapters::ReadStringField(Messages, TEXT("chatUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("chatUser")),
       DataAdapters::ReadStringField(Messages, TEXT("chatNpc")),
       DataAdapters::ReadStringField(Messages, TEXT("verifyUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("verificationValid")),
       DataAdapters::ReadStringField(Messages, TEXT("verificationInvalid"))}};
}

/** User Story: As a CLI Soul operation, I need the authored permanent-storage budget resolved through one immutable adapter so export, import, and verification cannot drift. @fn inline double soulProviderOperationTimeoutSeconds() */
inline double soulProviderOperationTimeoutSeconds() {
  static const double TimeoutSeconds =
      readCliSoulState().Timeouts.ProviderOperationSeconds;
  return TimeoutSeconds;
}

/** User Story: As a CLI Soul consumer, I need list limits decoded through a stable signature so optional input has one authored default. @fn inline int32 decodeSoulListLimit(const TArray<FString> &Args, const FCLISoulState &State) */
inline int32 decodeSoulListLimit(const TArray<FString> &Args,
                                 const FCLISoulState &State) {
  const int32 Parsed = Args.Num() >= State.Limits.SingleArgumentCount
                           ? FCString::Atoi(
                                 *Args[State.Limits.FirstArgumentIndex])
                           : State.Limits.DefaultListLimit;
  return Parsed > State.Limits.EmptyItemCount
             ? Parsed
             : State.Limits.DefaultListLimit;
}

} // namespace Soul
} // namespace CLI
} // namespace ForbocAI
