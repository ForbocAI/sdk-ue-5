#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

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
struct FArweaveUploadInstruction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString UploadUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString GatewayUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString PayloadJson;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ContentType;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString AuiAuthHeader;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TagsJson;
};
USTRUCT(BlueprintType)
struct FArweaveUploadResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Status;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  int32 StatusCode;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  bool bSuccess;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ArweaveUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ResponseJson;

  FArweaveUploadResult() : StatusCode(0), bSuccess(false) {}
};

USTRUCT(BlueprintType)
struct FArweaveDownloadInstruction {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ExpectedTxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString GatewayUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString DownloadUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString AdiAuthHeader;
};

USTRUCT(BlueprintType)
struct FArweaveDownloadResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString BodyJson;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Payload;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Status;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  int32 StatusCode;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  bool bSuccess;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Error;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ResponseJson;

  FArweaveDownloadResult() : StatusCode(0), bSuccess(false) {}
};

USTRUCT(BlueprintType)
struct FSoul {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Version;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Name;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Persona;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  TArray<FMemoryItem> Memories;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FAgentState State;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Signature;
};

USTRUCT(BlueprintType)
struct FSoulExportPhase1Request {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString NpcIdRef;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Persona;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FAgentState NpcState;
};

USTRUCT(BlueprintType)
struct FSoulExportPhase1Response {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FArweaveUploadInstruction se1Instruction;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString se1SignedPayload;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString se1Signature;
};

USTRUCT(BlueprintType)
struct FSoulExportConfirmRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FArweaveUploadResult secUploadResult;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString secSignedPayload;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString secSignature;
};

USTRUCT(BlueprintType)
struct FSoulExportResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString ArweaveUrl;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Signature;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FSoul Soul;
};

USTRUCT(BlueprintType)
struct FSoulImportPhase1Request {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString TxIdRef;
};

USTRUCT(BlueprintType)
struct FSoulImportPhase1Response {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FArweaveDownloadInstruction si1Instruction;
};

USTRUCT(BlueprintType)
struct FSoulImportConfirmRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString sicTxId;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FArweaveDownloadResult sicDownloadResult;
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
  FString ArweaveUrl;
};

USTRUCT(BlueprintType)
struct FSoulListResponse {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  TArray<FSoulListItem> Souls;
};

USTRUCT(BlueprintType)
struct FSoulVerifyResult {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  bool bValid;

  UPROPERTY(BlueprintReadOnly, Category = "Soul")
  FString Reason;

  FSoulVerifyResult() : bValid(false) {}
};
