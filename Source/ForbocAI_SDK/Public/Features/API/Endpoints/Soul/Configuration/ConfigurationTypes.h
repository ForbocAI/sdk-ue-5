#pragma once

#include "CoreMinimal.h"

namespace APISlice::Endpoints::SoulConfiguration {

struct FPreparationFieldData {
  FString NpcId;
  FString TransactionId;
  FString PayloadDigest;
  FString Provider;
};

struct FPreparationResponseFieldData : FPreparationFieldData {
  FString ConfirmationToken;
};

struct FConfirmationFieldData {
  FString StorageUrl;
  FString ProviderStatus;
};

struct FExportResponseFieldData {
  FString TransactionId;
  FString StorageUrl;
  FString Signature;
};

struct FVerificationFieldData {
  FString NpcId;
  FString PayloadDigest;
  FString Provider;
  FString Signature;
};

struct FVerifyResponseFieldData {
  FString Valid;
  FString Reason;
};

struct FSoulEndpointConfigurationData {
  FPreparationFieldData PreparationRequest;
  FPreparationResponseFieldData PreparationResponse;
  FConfirmationFieldData ConfirmationRequest;
  FExportResponseFieldData ExportResponse;
  FVerificationFieldData VerificationRequest;
  FVerifyResponseFieldData VerifyResponse;
};

} // namespace APISlice::Endpoints::SoulConfiguration
