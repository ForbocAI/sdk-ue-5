#pragma once

#include "CoreMinimal.h"

struct FNativeDependencyStatus {
  FString Name;
  bool bAvailable = false;
  FString Version;
  FString Detail;
};

struct FNativeDependenciesReport {
  FNativeDependencyStatus Vectorizer;
  FNativeDependencyStatus VectorDb;
};

struct FDependenciesSubsystemResult {
  bool bOk = false;
  FString Detail;
};

struct FDependenciesResult {
  FDependenciesSubsystemResult Vector;
  FDependenciesSubsystemResult Memory;
};

struct FDependenciesOptions {
  bool bVerbose = false;
  bool bForce = false;
};

struct FDependenciesState {
  FString Status = TEXT("idle");
  bool bHasReport = false;
  FNativeDependenciesReport Report;
  bool bHasResult = false;
  FDependenciesResult Result;
  FString Error;
};
