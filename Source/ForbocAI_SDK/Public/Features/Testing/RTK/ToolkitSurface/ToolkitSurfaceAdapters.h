#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/RTK/ToolkitSurface/ToolkitSurfaceTypes.h"

namespace Testing::RTK::ToolkitSurface {

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read names through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FToolkitSurfaceNames ReadNames(const DataAdapters::FSettingsSource &Source) */
inline FToolkitSurfaceNames
ReadNames(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("names"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("actionAndUtility")),
      DataAdapters::ReadStringField(Object, TEXT("autoBatch")),
      DataAdapters::ReadStringField(Object, TEXT("middleware")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read prepared through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FPreparedActionFixture ReadPrepared(const DataAdapters::FSettingsSource &Source) */
inline FPreparedActionFixture
ReadPrepared(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("prepared"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("format")),
      DataAdapters::ReadStringField(Object, TEXT("metaKey")),
      DataAdapters::ReadStringField(Object, TEXT("metaValue")),
      DataAdapters::ReadStringField(Object, TEXT("expectedValue")),
      DataAdapters::ReadNumberField(Object, TEXT("input")),
      DataAdapters::ReadBooleanField(Object, TEXT("error")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read utility through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FUtilityFixture ReadUtility(const DataAdapters::FSettingsSource &Source) */
inline FUtilityFixture
ReadUtility(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("utility"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("initialDispatchCount")),
      DataAdapters::ReadNumberField(Object, TEXT("boundPayload")),
      DataAdapters::ReadNumberArrayField(Object, TEXT("initialValues")),
      DataAdapters::ReadNumberField(Object, TEXT("head")),
      DataAdapters::ReadNumberArrayField(Object, TEXT("suffix")),
      DataAdapters::ReadNumberArrayField(Object, TEXT("expectedValues")),
      DataAdapters::ReadNumberField(Object, TEXT("original")),
      DataAdapters::ReadNumberField(Object, TEXT("draftDelta")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedNext")),
      DataAdapters::ReadNumberField(Object, TEXT("nanoidSize")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedDispatchCount")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read auto batch through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FAutoBatchFixture ReadAutoBatch(const DataAdapters::FSettingsSource &Source) */
inline FAutoBatchFixture
ReadAutoBatch(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("autoBatch"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("initialState")),
      DataAdapters::ReadNumberField(Object, TEXT("initialSubscriberCalls")),
      DataAdapters::ReadNumberField(Object, TEXT("firstAmount")),
      DataAdapters::ReadNumberField(Object, TEXT("secondAmount")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedBatchedState")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedFirstQueueSize")),
      DataAdapters::ReadNumberField(Object, TEXT("firstNotificationIndex")),
      DataAdapters::ReadNumberField(Object,
                                    TEXT("expectedFirstSubscriberCalls")),
      DataAdapters::ReadNumberField(Object, TEXT("thirdAmount")),
      DataAdapters::ReadNumberField(Object, TEXT("normalAmount")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedFinalState")),
      DataAdapters::ReadNumberField(Object, TEXT("secondNotificationIndex")),
      DataAdapters::ReadNumberField(Object,
                                    TEXT("expectedFinalSubscriberCalls")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read middleware through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FMiddlewareFixture ReadMiddleware(const DataAdapters::FSettingsSource &Source) */
inline FMiddlewareFixture
ReadMiddleware(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("middleware"));
  return {
      DataAdapters::ReadNumberField(Object, TEXT("initialState")),
      DataAdapters::ReadNumberField(Object, TEXT("initialDynamicCalls")),
      DataAdapters::ReadNumberField(Object, TEXT("initialImmutableChecks")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedDynamicCalls")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedImmutableChecks")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedFinalState")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke read labels through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline FToolkitSurfaceLabels ReadLabels(const DataAdapters::FSettingsSource &Source) */
inline FToolkitSurfaceLabels
ReadLabels(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("preparedPayload")),
      DataAdapters::ReadStringField(Object, TEXT("preparedTransform")),
      DataAdapters::ReadStringField(Object, TEXT("preparedMetadata")),
      DataAdapters::ReadStringField(Object, TEXT("preparedError")),
      DataAdapters::ReadStringField(Object, TEXT("optionalPayload")),
      DataAdapters::ReadStringField(Object, TEXT("optionalType")),
      DataAdapters::ReadStringField(Object, TEXT("boundAction")),
      DataAdapters::ReadStringField(Object, TEXT("boundDispatch")),
      DataAdapters::ReadStringField(Object, TEXT("tupleOrder")),
      DataAdapters::ReadStringField(Object, TEXT("nextOriginal")),
      DataAdapters::ReadStringField(Object, TEXT("nextResult")),
      DataAdapters::ReadStringField(Object, TEXT("nanoidSize")),
      DataAdapters::ReadStringField(Object, TEXT("nanoidUnique")),
      DataAdapters::ReadStringField(Object, TEXT("batchState")),
      DataAdapters::ReadStringField(Object, TEXT("batchQueue")),
      DataAdapters::ReadStringField(Object, TEXT("batchWait")),
      DataAdapters::ReadStringField(Object, TEXT("batchNotify")),
      DataAdapters::ReadStringField(Object, TEXT("normalState")),
      DataAdapters::ReadStringField(Object, TEXT("normalNotify")),
      DataAdapters::ReadStringField(Object, TEXT("staleNotify")),
      DataAdapters::ReadStringField(Object, TEXT("dynamicEmpty")),
      DataAdapters::ReadStringField(Object, TEXT("dynamicLive")),
      DataAdapters::ReadStringField(Object, TEXT("invariantCount")),
      DataAdapters::ReadStringField(Object, TEXT("reducerState")),
      DataAdapters::ReadStringField(Object, TEXT("emptyActionPath")),
      DataAdapters::ReadStringField(Object, TEXT("typedAction")),
  };
}

/** User Story: As a testing rtk toolkit surface consumer, I need to invoke toolkit surface fixtures through a stable signature so the testing rtk toolkit surface workflow remains explicit and composable. @fn inline const FToolkitSurfaceFixtures &ToolkitSurfaceFixtures() */
inline const FToolkitSurfaceFixtures &ToolkitSurfaceFixtures() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/tests/rtk/toolkit-surface.json"));
  static const FToolkitSurfaceFixtures Fixtures = {
      ReadNames(Source), ReadPrepared(Source), ReadUtility(Source),
      ReadAutoBatch(Source), ReadMiddleware(Source), ReadLabels(Source),
  };
  return Fixtures;
}

} // namespace Testing::RTK::ToolkitSurface
