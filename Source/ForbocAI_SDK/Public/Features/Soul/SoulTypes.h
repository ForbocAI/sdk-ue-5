#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "Features/Memory/MemoryTypes.h"
#include "NPC/NPCBaseTypes.h"
#include "SoulTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicSoulSoulTypesHOptionalDomainId;
} } }

USTRUCT(BlueprintType)
struct FSoul {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Version;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Name;

  /** Canonical structured-persona JSON. */
  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString StructuredPersona;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  TArray<FMemoryItem> Memories;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FAgentState State;
};

USTRUCT(BlueprintType)
struct FSoulExportPreparationRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcIdRef;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TransactionId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString PayloadDigest;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Provider;
};

USTRUCT(BlueprintType)
struct FSoulExportPreparation {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcIdRef;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TransactionId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString PayloadDigest;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Provider;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ConfirmationToken;
};

USTRUCT(BlueprintType)
struct FSoulExportConfirmationRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcIdRef;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TransactionId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString PayloadDigest;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Provider;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ConfirmationToken;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString StorageUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  int32 ProviderStatus{};
};

USTRUCT(BlueprintType)
struct FSoulVerificationRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcIdRef;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString PayloadDigest;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Provider;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Signature;
};

USTRUCT(BlueprintType)
struct FSoulExportResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString StorageUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Signature;
};

USTRUCT(BlueprintType)
struct FSoulExportResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Url;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FSoul Soul;
};

USTRUCT(BlueprintType)
struct FSoulListItem {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ExportedAt;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString StorageUrl;
};

USTRUCT(BlueprintType)
struct FSoulVerifyResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  bool bValid = false;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Reason;
};

namespace SoulSlice {

struct FSoulSliceState {
  FString ExportStatus;
  FString ImportStatus;
  bool bListing{};
  bool bVerifying{};
  FSoulExportResult ExportResult;
  bool bHasExportResult{};
  FSoul ImportedSoul;
  bool bHasImportedSoul{};
  TArray<FSoulListItem> AvailableSouls;
  FString Error;
};

} // namespace SoulSlice
