#pragma once

#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Quality/QualityTypes.h"

namespace MicroGame::QualityActions {

/** User Story: As a quality evaluator, I need one run initialized with an explicit host and baseline. @fn inline rtk::ActionCreator<FQualityEvaluationStartedPayload> evaluationStartedActionCreator() */
inline rtk::ActionCreator<FQualityEvaluationStartedPayload>
evaluationStartedActionCreator() {
  static auto Creator = rtk::createAction<FQualityEvaluationStartedPayload>(
      TEXT("microgame/quality/evaluationStarted"));
  return Creator;
}

/** User Story: As a quality evaluator, I need tested model identity retained before samples are scored. @fn inline rtk::ActionCreator<FQualityModelMetadata> metadataCapturedActionCreator() */
inline rtk::ActionCreator<FQualityModelMetadata>
metadataCapturedActionCreator() {
  static auto Creator = rtk::createAction<FQualityModelMetadata>(
      TEXT("microgame/quality/metadataCaptured"));
  return Creator;
}

/** User Story: As a quality evaluator, I need normalized evidence upserted by deterministic probe identity. @fn inline rtk::ActionCreator<FQualitySample> sampleRecordedActionCreator() */
inline rtk::ActionCreator<FQualitySample> sampleRecordedActionCreator() {
  static auto Creator = rtk::createAction<FQualitySample>(
      TEXT("microgame/quality/sampleRecorded"));
  return Creator;
}

/** User Story: As a quality evaluator, I need completion time retained for reproducible reports. @fn inline rtk::ActionCreator<FQualityEvaluationCompletedPayload> evaluationCompletedActionCreator() */
inline rtk::ActionCreator<FQualityEvaluationCompletedPayload>
evaluationCompletedActionCreator() {
  static auto Creator = rtk::createAction<FQualityEvaluationCompletedPayload>(
      TEXT("microgame/quality/evaluationCompleted"));
  return Creator;
}

/** User Story: As a release reviewer, I need setup and metadata failures represented explicitly. @fn inline rtk::ActionCreator<FQualityEvaluationFailedPayload> evaluationFailedActionCreator() */
inline rtk::ActionCreator<FQualityEvaluationFailedPayload>
evaluationFailedActionCreator() {
  static auto Creator = rtk::createAction<FQualityEvaluationFailedPayload>(
      TEXT("microgame/quality/evaluationFailed"));
  return Creator;
}

/** User Story: As a features systems quality consumer, I need to invoke evaluation started through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline rtk::AnyAction evaluationStarted( const FQualityEvaluationStartedPayload &Payload) */
inline rtk::AnyAction evaluationStarted(
    const FQualityEvaluationStartedPayload &Payload) {
  return evaluationStartedActionCreator()(Payload);
}
/** User Story: As a features systems quality consumer, I need to invoke metadata captured through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline rtk::AnyAction metadataCaptured( const FQualityModelMetadata &Metadata) */
inline rtk::AnyAction metadataCaptured(
    const FQualityModelMetadata &Metadata) {
  return metadataCapturedActionCreator()(Metadata);
}
/** User Story: As a features systems quality consumer, I need to invoke sample recorded through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline rtk::AnyAction sampleRecorded(const FQualitySample &Sample) */
inline rtk::AnyAction sampleRecorded(const FQualitySample &Sample) {
  return sampleRecordedActionCreator()(Sample);
}
/** User Story: As a features systems quality consumer, I need to invoke evaluation completed through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline rtk::AnyAction evaluationCompleted( const FQualityEvaluationCompletedPayload &Payload) */
inline rtk::AnyAction evaluationCompleted(
    const FQualityEvaluationCompletedPayload &Payload) {
  return evaluationCompletedActionCreator()(Payload);
}
/** User Story: As a features systems quality consumer, I need to invoke evaluation failed through a stable signature so the features systems quality workflow remains explicit and composable. @fn inline rtk::AnyAction evaluationFailed( const FQualityEvaluationFailedPayload &Payload) */
inline rtk::AnyAction evaluationFailed(
    const FQualityEvaluationFailedPayload &Payload) {
  return evaluationFailedActionCreator()(Payload);
}

} // namespace MicroGame::QualityActions
