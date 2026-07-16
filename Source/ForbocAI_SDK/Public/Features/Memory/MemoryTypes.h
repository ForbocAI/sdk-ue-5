#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"

// clang-format off
#include "CoreMinimal.h"
#include "MemoryTypes.generated.h"
// clang-format on

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicMemoryMemoryTypesHOptionalDomainId;
} } }


/**
 * Memory Item — Immutable data.
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 */
USTRUCT(BlueprintType)
struct FMemoryItem {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  FString Id;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  FString Text;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  TArray<float> Embedding;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  int64 Timestamp;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  FString Type;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  float Importance;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  float Similarity;

  /** User Story: As a features memory consumer, I need to invoke fmemory item through a stable signature so the features memory workflow remains explicit and composable. @fn FMemoryItem() */
  FMemoryItem() : Timestamp(), Importance(), Similarity() {}
};

/**
 * Memory Recall Request
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 */
USTRUCT(BlueprintType)
struct FMemoryRecallRequest {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  FString Query;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  int32 Limit;

  UPROPERTY(BlueprintReadOnly, Category = "Memory")
  float Threshold;

  /** User Story: As a features memory consumer, I need to invoke fmemory recall request through a stable signature so the features memory workflow remains explicit and composable. @fn FMemoryRecallRequest() */
  FMemoryRecallRequest() : Limit(), Threshold() {}
};

namespace TypeFactory {

/** User Story: As a features memory consumer, I need to invoke memory item through a stable signature so the features memory workflow remains explicit and composable. @fn inline FMemoryItem MemoryItem(FString Id, FString Text, FString Type, float Importance, int64 Timestamp) */
inline FMemoryItem MemoryItem(FString Id, FString Text, FString Type,
                              float Importance, int64 Timestamp) {
  FMemoryItem M;
  M.Id = MoveTemp(Id);
  M.Text = MoveTemp(Text);
  M.Type = MoveTemp(Type);
  M.Importance = Importance;
  M.Timestamp = Timestamp;
  return M;
}

} // namespace TypeFactory

namespace MemorySlice {

struct FMemorySliceState {
  rtk::EntityState<FMemoryItem> Entities;
  FString StorageStatus;
  FString RecallStatus;
  FString Error;
  TArray<FString> RecalledIds;
};

} // namespace MemorySlice
