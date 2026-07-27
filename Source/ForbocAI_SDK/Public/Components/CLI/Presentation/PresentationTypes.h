#pragma once

#include "CoreMinimal.h"

namespace ForbocAI::CLI::Presentation {

struct FCLIPresentationDefaults {
  int32 SuccessExitCode;
  int32 FailureExitCode;
};

struct FCLIPresentationCommonMessages {
  FString Blank;
  FString Version;
  FString Status;
  FString ApiUrl;
  FString ApiKey;
  FString ApiKeyMissing;
  FString MaskedValue;
  FString ApiStatus;
  FString ConfigSetUsage;
  FString ConfigGetUsage;
  FString ConfigEntry;
  FString ConfigListEntry;
};

struct FCLIPresentationRuntimeMessages {
  FString CommandStarted;
  FString CommandSucceeded;
  FString CommandFailed;
  FString DispatchFailed;
  FString EmptyCommand;
  FString InvalidCommand;
  FString UnknownCommand;
};

struct FCLIPresentationResultMessages {
  FString ConfigUpdated;
  FString ConfigPersistenceFailed;
  FString ConfigRetrieved;
  FString ConfigListed;
  FString VersionPrinted;
  FString StatusChecked;
  FString DoctorCompleted;
};

struct FCLIPresentationState {
  FString SliceName;
  FCLIPresentationDefaults Defaults;
  FCLIPresentationCommonMessages Common;
  FCLIPresentationRuntimeMessages Runtime;
  FCLIPresentationResultMessages Results;
};

} // namespace ForbocAI::CLI::Presentation
