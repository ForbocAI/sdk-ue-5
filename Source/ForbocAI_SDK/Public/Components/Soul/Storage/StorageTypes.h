#pragma once

#include "CoreMinimal.h"
#include "Components/Soul/SoulTypes.h"

struct FSoulWallet {
  FString Kty;
  FString E;
  FString N;
  FString D;
  FString P;
  FString Q;
  FString Dp;
  FString Dq;
  FString Qi;
};

struct FSoulEnvelope {
  FString Version;
  FString Algorithm;
  FString KeyDerivation;
  FString Salt;
  FString InitializationVector;
  FString Ciphertext;
};

struct FSoulStorageReceipt {
  FString TxId;
  FString Url;
  FString Digest;
  int32 Status{};
  FString Provider;
};

struct FSoulStoragePreparation {
  FString TxId;
  FString Digest;
  FString Provider;
};

struct FSoulPreparedUpload {
  TArray<uint8> DataItem;
  FString Digest;
};

struct FSoulCatalogEntry : FSoulListItem {
  FString Provider;
  FString Digest;
  FString Signature;
};

struct FSoulCatalog {
  FString Version;
  TArray<FSoulCatalogEntry> Souls;
};

struct FSoulStorageCommit {
  FSoul Soul;
  FSoulStorageReceipt Receipt;
  FString Signature;
};

struct FSoulProviderUploadResponse {
  FString Id;
  FString Owner;
  TArray<FString> DataCaches;
  TArray<FString> FastFinalityIndexes;
};

struct FSoulStorageTransactionRequest {
  FString TxId;
};

struct FSoulStorageListRequest {
  int32 Limit{};
};
