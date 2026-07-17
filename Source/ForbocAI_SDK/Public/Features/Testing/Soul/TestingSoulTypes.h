#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Testing/Soul/Storage/Serialization/SerializationTypes.h"

namespace Testing::Soul {

enum class ESoulTestActionKind : uint8 {
  ExportPending,
  ExportSuccess,
  ExportFailed,
  ImportPending,
  ImportSuccess,
  ImportFailed,
  SetList,
  Clear,
  Inspect,
  Count
};

struct FSoulTestAction {
  ESoulTestActionKind Kind;
  func::Maybe<FString> TxId;
  func::Maybe<FString> SoulId;
  func::Maybe<FString> Persona;
  func::Maybe<FString> Error;
  TArray<FString> TxIds;
};

struct FSoulTestExpected {
  func::Maybe<FString> ExportStatus;
  func::Maybe<FString> ImportStatus;
  func::Maybe<bool> HasExport;
  func::Maybe<FString> ExportTxId;
  func::Maybe<bool> HasImport;
  func::Maybe<FString> ImportId;
  func::Maybe<FString> ImportPersona;
  func::Maybe<int32> AvailableSoulCount;
  func::Maybe<FString> AvailableSoulFirstTxId;
  func::Maybe<FString> Error;
};

struct FSoulTestStep {
  FSoulTestAction Action;
  FSoulTestExpected Expected;
};

struct FSoulTestScenario {
  FString Name;
  TArray<FSoulTestStep> Steps;
};

struct FSoulTestLabels {
  FString Suite;
  FString CaseName;
  FString RequiredField;
  FString ScenarioPresent;
  FString ExportStatus;
  FString ImportStatus;
  FString HasExport;
  FString ExportTxId;
  FString HasImport;
  FString ImportId;
  FString ImportPersona;
  FString AvailableSoulCount;
  FString AvailableSoulFirstTxId;
  FString Error;
  FString ProviderRetryWithinCycle;
  FString ProviderRetryCycleBoundary;
  FString ProviderRetryBeforeMaximumCycle;
  FString ProviderRetryAtMaximumCycle;
  FString ProviderRetryWithoutGateways;
  FString StorageWalletRoundTrip;
  FString StorageEnvelopeRoundTrip;
  FString StorageCatalogRoundTrip;
};

struct FSoulProviderRetryTestData {
  int32 WithinCycleAttempt;
  int32 CycleBoundaryAttempt;
  int32 UrlCount;
};

struct FSoulTestFixtures {
  FSoulTestLabels Labels;
  FSoulProviderRetryTestData ProviderRetry;
  Storage::Serialization::FStorageSerializationFixture StorageSerialization;
  TArray<FSoulTestScenario> Scenarios;
};

} // namespace Testing::Soul
