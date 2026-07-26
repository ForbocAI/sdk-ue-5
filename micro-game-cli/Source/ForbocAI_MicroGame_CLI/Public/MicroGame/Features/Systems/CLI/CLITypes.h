#pragma once

#include "CoreMinimal.h"

#define FORBOCAI_CLI_ARGV_FIELDS(X)                                      \
  X(FString, cliArgTemplate)                                              \
  X(int32, commandIndex)                                                  \
  X(int32, firstIndex)                                                    \
  X(int32, nextIndex)                                                     \
  X(int32, valueOffset)                                                   \
  X(int32, pairOffset)

#define FORBOCAI_CLI_EXIT_CODE_FIELDS(X)                                 \
  X(int32, success)                                                       \
  X(int32, failure)                                                       \
  X(int32, incomplete)

namespace MicroGame::CLI {

#define FORBOCAI_DECLARE_CLI_FIELD(Type, Name) Type Name{};

struct FCliArgvData {
  FORBOCAI_CLI_ARGV_FIELDS(FORBOCAI_DECLARE_CLI_FIELD)
};

struct FCliExitCodeData {
  FORBOCAI_CLI_EXIT_CODE_FIELDS(FORBOCAI_DECLARE_CLI_FIELD)
};

#undef FORBOCAI_DECLARE_CLI_FIELD

struct FCliFlagData {
  TArray<FString> help;
  FString mode;
  TArray<FString> apiUrl;
  TArray<FString> apiKey;
  FString assignmentSeparator;
};

struct FCliRuntimeData {
  FCliArgvData argv;
  FCliExitCodeData exitCodes;
  FCliFlagData flags;
};

struct FCliInvocation {
  TArray<FString> Tokens;
  FString ApiUrl;
  FString ApiKey;
};

} // namespace MicroGame::CLI
