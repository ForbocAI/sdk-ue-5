#pragma once

#include "CoreMinimal.h"

namespace Testing::RTK::ToolkitSurface {

struct FToolkitSurfaceNames {
  FString ActionAndUtility;
  FString AutoBatch;
  FString Middleware;
};

struct FPreparedActionFixture {
  FString Format;
  FString MetaKey;
  FString MetaValue;
  FString ExpectedValue;
  int32 Input;
  bool bError;
};

struct FUtilityFixture {
  int32 InitialDispatchCount;
  int32 BoundPayload;
  TArray<int32> InitialValues;
  int32 Head;
  TArray<int32> Suffix;
  TArray<int32> ExpectedValues;
  int32 Original;
  int32 DraftDelta;
  int32 ExpectedNext;
  int32 NanoidSize;
  int32 ExpectedDispatchCount;
};

struct FAutoBatchFixture {
  int32 InitialState;
  int32 InitialSubscriberCalls;
  int32 FirstAmount;
  int32 SecondAmount;
  int32 ExpectedBatchedState;
  int32 ExpectedFirstQueueSize;
  int32 FirstNotificationIndex;
  int32 ExpectedFirstSubscriberCalls;
  int32 ThirdAmount;
  int32 NormalAmount;
  int32 ExpectedFinalState;
  int32 SecondNotificationIndex;
  int32 ExpectedFinalSubscriberCalls;
};

struct FMiddlewareFixture {
  int32 InitialState;
  int32 InitialDynamicCalls;
  int32 InitialImmutableChecks;
  int32 ExpectedDynamicCalls;
  int32 ExpectedImmutableChecks;
  int32 ExpectedFinalState;
};

struct FToolkitSurfaceLabels {
  FString PreparedPayload;
  FString PreparedTransform;
  FString PreparedMetadata;
  FString PreparedError;
  FString OptionalPayload;
  FString OptionalType;
  FString BoundAction;
  FString BoundDispatch;
  FString TupleOrder;
  FString NextOriginal;
  FString NextResult;
  FString NanoidSize;
  FString NanoidUnique;
  FString BatchState;
  FString BatchQueue;
  FString BatchWait;
  FString BatchNotify;
  FString NormalState;
  FString NormalNotify;
  FString StaleNotify;
  FString DynamicEmpty;
  FString DynamicLive;
  FString InvariantCount;
  FString ReducerState;
  FString EmptyActionPath;
  FString TypedAction;
};

struct FToolkitSurfaceFixtures {
  FToolkitSurfaceNames Names;
  FPreparedActionFixture Prepared;
  FUtilityFixture Utility;
  FAutoBatchFixture AutoBatch;
  FMiddlewareFixture Middleware;
  FToolkitSurfaceLabels Labels;
};

} // namespace Testing::RTK::ToolkitSurface
