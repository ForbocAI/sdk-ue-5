#pragma once

#include "MicroGame/Features/Entities/NPCs/NPCsTypes.h"
#include "MicroGame/Features/Systems/Memory/MemoryTypes.h"
#include "MicroGame/Features/Systems/Social/SocialTypes.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

#define FORBOCAI_GAME_RUNTIME_TYPE_FIELDS(X)                              \
  X(object)                                                               \
  X(string)                                                               \
  X(boolean)

#define FORBOCAI_GAME_RUNTIME_NUMBER_FIELDS(X)                            \
  X(int32, emptyCount)                                                     \
  X(int32, singularCount)                                                  \
  X(int32, captureIndex)                                                   \
  X(int32, nextIndex)

#define FORBOCAI_GAME_MODE_FIELDS(X)                                      \
  X(manual)                                                               \
  X(autoplay)                                                             \
  X(chat)

#define FORBOCAI_GAME_SCENARIO_EVENT_TYPE_FIELDS(X)                       \
  X(stealth)                                                              \
  X(social)                                                               \
  X(escape)                                                               \
  X(persistence)

#define FORBOCAI_GAME_COMMAND_GROUP_FIELDS(X)                             \
  X(status)                                                               \
  X(npc_lifecycle)                                                        \
  X(npc_process_chat)                                                     \
  X(npc_conversation)                                                     \
  X(memory_list)                                                          \
  X(memory_recall)                                                        \
  X(memory_store)                                                         \
  X(memory_clear)                                                         \
  X(memory_export)                                                        \
  X(bridge_rules)                                                         \
  X(bridge_validate)                                                      \
  X(bridge_preset)                                                        \
  X(soul_export)                                                          \
  X(soul_import)                                                          \
  X(soul_list)                                                            \
  X(soul_chat)                                                            \
  X(ghost_lifecycle)                                                      \
  X(contract)

#define FORBOCAI_GAME_STATUS_FIELDS(X)                                    \
  X(ok)                                                                   \
  X(error)

#define FORBOCAI_GAME_OUTPUT_ASSERTION_KIND_FIELDS(X)                     \
  X(includesAlias)                                                        \
  X(includesText)                                                         \
  X(excludesText)

#define FORBOCAI_GAME_LIFECYCLE_EVENT_FIELDS(X)                           \
  X(sessionStarted)                                                       \
  X(contractFailed)                                                       \
  X(sessionReady)                                                         \
  X(stepStarted)                                                          \
  X(commandCompleted)                                                     \
  X(autoplayTick)                                                         \
  X(sessionCompleted)

#define FORBOCAI_GAME_RUNTIME_MESSAGE_FIELDS(X)                           \
  X(FString, bridgeValidationFailed)                                      \
  X(FString, bridgeActionBlocked)                                         \
  X(FString, unknownReason)                                               \
  X(FString, coverageComplete)                                            \
  X(FString, coverageErrors)                                              \
  X(FString, singularSuffix)                                              \
  X(FString, pluralSuffix)                                                \
  X(FString, missingGroups)                                               \
  X(FString, noMissingGroups)                                             \
  X(FString, coverageIncomplete)                                          \
  X(FString, contractFailed)                                              \
  X(FString, finalCommandsMissing)                                        \
  X(FString, outputAssertionAliasMissing)                                 \
  X(FString, outputAssertionValueMissing)                                 \
  X(FString, outputAssertionValuePresent)                                 \
  X(FString, outputAssertionKindUnsupported)                              \
  X(FString, outputAssertionFailure)                                      \
  X(FString, commandAliasUnavailable)

#define FORBOCAI_GAME_MESSAGE_FIELDS(X)                                   \
  X(FString, contractGroupInvalid)                                        \
  X(FString, gamePresenterRequired)                                       \
  X(FString, runtimeUrlMissing)                                           \
  X(FString, apiContractUnavailable)                                      \
  X(FString, contractEndpointUnavailable)

#define FORBOCAI_GAME_NUMBER_FIELDS(X)                                    \
  X(int32, initialIndex)                                                   \
  X(int32, minimumDelayMs)                                                 \
  X(int32, nextIndex)                                                      \
  X(int32, radix)                                                          \
  X(int32, millisecondsPerSecond)                                         \
  X(int32, contractTimeoutSeconds)

namespace MicroGame {

#define FORBOCAI_DECLARE_GAME_NAME_FIELD(Name) FString Name{};
#define FORBOCAI_DECLARE_GAME_VALUE_FIELD(Type, Name) Type Name{};

struct FGameRuntimeTypes {
  FORBOCAI_GAME_RUNTIME_TYPE_FIELDS(FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameRuntimeNumbers {
  FORBOCAI_GAME_RUNTIME_NUMBER_FIELDS(FORBOCAI_DECLARE_GAME_VALUE_FIELD)
};

struct FGameModes {
  FORBOCAI_GAME_MODE_FIELDS(FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameScenarioEventTypes {
  FORBOCAI_GAME_SCENARIO_EVENT_TYPE_FIELDS(
      FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameCommandGroups {
  FORBOCAI_GAME_COMMAND_GROUP_FIELDS(FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameStatuses {
  FORBOCAI_GAME_STATUS_FIELDS(FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameOutputAssertionKinds {
  FORBOCAI_GAME_OUTPUT_ASSERTION_KIND_FIELDS(
      FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameLifecycleEvents {
  FORBOCAI_GAME_LIFECYCLE_EVENT_FIELDS(FORBOCAI_DECLARE_GAME_NAME_FIELD)
  TSet<FString> all;
};

struct FGameRuntimePatterns {
  FString npcCommand;
};

struct FGameRuntimeTemplate {
  FString tokenPrefix;
  FString tokenSuffix;
};

struct FGameRuntimeWireFields {
  FString action;
  FString type;
  FString targetHex;
  FString x;
  FString y;
  FString stateDelta;
  FString suspicion;
};

struct FGameRuntimeSeparators {
  FString list;
};

struct FGameRuntimeTokens {
  FString alias;
  FString command;
  FString count;
  FString errors;
  FString groups;
  FString kind;
  FString missing;
  FString output;
  FString reason;
  FString suffix;
  FString value;
};

struct FGameRuntimeMessages {
  FORBOCAI_GAME_RUNTIME_MESSAGE_FIELDS(FORBOCAI_DECLARE_GAME_VALUE_FIELD)
};

struct FGameStealthInitialState {
  bool doorOpen{};
  int32 alertDelta{};
  FGameNPC npc;
  FMemoryRecord memory;
};

struct FGameSocialInitialState {
  FGameNPC npc;
  FString dialogue;
  FTradeOffer tradeOffer;
  int32 suspicion{};
};

struct FGameEscapeInitialState {
  bool hidden{};
};

struct FGamePersistenceInitialState {
  FString npcId;
  FString txId;
};

struct FGameInitialState {
  FGameStealthInitialState stealth;
  FGameSocialInitialState social;
  FGameEscapeInitialState escape;
  FGamePersistenceInitialState persistence;
  FGameNPC sessionNpc;
};

struct FGameRuntimeData {
  FGameRuntimeTypes runtimeTypes;
  FGameRuntimeNumbers numbers;
  FGameModes modes;
  FGameScenarioEventTypes scenarioEventTypes;
  FGameCommandGroups commandGroups;
  FGameStatuses statuses;
  FGameOutputAssertionKinds outputAssertionKinds;
  FGameLifecycleEvents lifecycleEvents;
  FGameRuntimePatterns patterns;
  FGameRuntimeTemplate templateData;
  FGameRuntimeWireFields wireFields;
  FGameRuntimeSeparators separators;
  FGameRuntimeTokens tokens;
  FGameRuntimeMessages messages;
  FGameInitialState initialState;
};

struct FGameAutoplayData {
  TArray<FString> dots;
  int32 dotDelayMs{};
};

struct FGameEnvironmentData {
  FString runtimeUrlKey;
  FString commandDelayKey;
  int32 defaultCommandDelayMs{};
};

struct FGameMessages {
  FORBOCAI_GAME_MESSAGE_FIELDS(FORBOCAI_DECLARE_GAME_VALUE_FIELD)
};

struct FGameNumbers {
  FORBOCAI_GAME_NUMBER_FIELDS(FORBOCAI_DECLARE_GAME_VALUE_FIELD)
};

struct FGameOutputData {
  TArray<FString> emptyLines;
  FString emptyText;
  FString lineBreak;
};

struct FGameData {
  FGameAutoplayData autoplay;
  FCommandSpec contractCommand;
  TArray<FCommandSpec> finalCommands;
  FGameEnvironmentData environment;
  FGameMessages messages;
  FGameNumbers numbers;
  FGameOutputData output;
};

#undef FORBOCAI_DECLARE_GAME_NAME_FIELD
#undef FORBOCAI_DECLARE_GAME_VALUE_FIELD

} // namespace MicroGame
