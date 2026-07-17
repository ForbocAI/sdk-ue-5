#pragma once

#include "Features/CLI/Presentation/PresentationAdapters.h"

namespace ForbocAI::CLI::Presentation {

/** User Story: As a CLI consumer, I need presentation selected from the package root store. @fn template <typename RootState> inline const FCLIPresentationState &selectCliPresentation( const RootState &State) */
template <typename RootState>
inline const FCLIPresentationState &selectCliPresentation(
    const RootState &State) {
  return State.CLIPresentation;
}

/** User Story: As CLI diagnostics, I need the authored version line derived from typed state. @fn inline FString selectCliVersionLine(const FCLIPresentationState &State, const FString &Version) */
inline FString selectCliVersionLine(const FCLIPresentationState &State,
                                    const FString &Version) {
  return formatCliMessage(State.Common.Version, Version);
}

/** User Story: As CLI health checks, I need the API status line derived from typed state. @fn inline FString selectCliStatusLine(const FCLIPresentationState &State, const FString &Status) */
inline FString selectCliStatusLine(const FCLIPresentationState &State,
                                   const FString &Status) {
  return formatCliMessage(State.Common.Status, Status);
}

/** User Story: As CLI diagnostics, I need API location output derived from typed state. @fn inline FString selectCliApiUrlLine(const FCLIPresentationState &State, const FString &ApiUrl) */
inline FString selectCliApiUrlLine(const FCLIPresentationState &State,
                                   const FString &ApiUrl) {
  return formatCliMessage(State.Common.ApiUrl, ApiUrl);
}

/** User Story: As CLI diagnostics, I need credentials represented without exposing their value. @fn inline FString selectCliApiKeyLine(const FCLIPresentationState &State, bool bApiKeySet) */
inline FString selectCliApiKeyLine(const FCLIPresentationState &State,
                                   bool bApiKeySet) {
  return formatCliMessage(
      State.Common.ApiKey,
      bApiKeySet ? State.Common.MaskedValue : State.Common.ApiKeyMissing);
}

/** User Story: As CLI diagnostics, I need API version status derived from typed service data. @fn inline FString selectCliApiStatusLine(const FCLIPresentationState &State, const FString &Status, const FString &Version) */
inline FString selectCliApiStatusLine(const FCLIPresentationState &State,
                                      const FString &Status,
                                      const FString &Version) {
  return formatCliMessage(State.Common.ApiStatus, Status, Version);
}

/** User Story: As CLI configuration output, I need one entry formatted consistently with optional credential masking. @fn inline FString selectCliConfigEntryLine(const FCLIPresentationState &State, const FString &Key, const FString &Value, bool bMasked, bool bListEntry) */
inline FString selectCliConfigEntryLine(const FCLIPresentationState &State,
                                        const FString &Key,
                                        const FString &Value, bool bMasked,
                                        bool bListEntry) {
  return formatCliMessage(
      bListEntry ? State.Common.ConfigListEntry : State.Common.ConfigEntry, Key,
      bMasked && !Value.IsEmpty() ? State.Common.MaskedValue : Value);
}

/** User Story: As CLI startup, I need the active command represented through authored presentation state. @fn inline FString selectCliCommandStartedLine(const FCLIPresentationState &State, const FString &Command) */
inline FString selectCliCommandStartedLine(const FCLIPresentationState &State,
                                           const FString &Command) {
  return formatCliMessage(State.Runtime.CommandStarted, Command);
}

/** User Story: As CLI failures, I need rejected command output derived from authored presentation state. @fn inline FString selectCliCommandFailedLine(const FCLIPresentationState &State, const FString &Message) */
inline FString selectCliCommandFailedLine(const FCLIPresentationState &State,
                                          const FString &Message) {
  return formatCliMessage(State.Runtime.CommandFailed, Message);
}

/** User Story: As CLI dispatch, I need fallback failure text derived from authored presentation state. @fn inline FString selectCliDispatchFailedMessage( const FCLIPresentationState &State, const FString &Command) */
inline FString selectCliDispatchFailedMessage(
    const FCLIPresentationState &State, const FString &Command) {
  return formatCliMessage(State.Runtime.DispatchFailed, Command);
}

/** User Story: As CLI validation, I need invalid-command text derived from authored presentation state. @fn inline FString selectCliInvalidCommandMessage( const FCLIPresentationState &State, const FString &Command) */
inline FString selectCliInvalidCommandMessage(
    const FCLIPresentationState &State, const FString &Command) {
  return formatCliMessage(State.Runtime.InvalidCommand, Command);
}

/** User Story: As CLI routing, I need unknown-command text derived from authored presentation state. @fn inline FString selectCliUnknownCommandMessage( const FCLIPresentationState &State, const FString &Command) */
inline FString selectCliUnknownCommandMessage(
    const FCLIPresentationState &State, const FString &Command) {
  return formatCliMessage(State.Runtime.UnknownCommand, Command);
}

} // namespace ForbocAI::CLI::Presentation
