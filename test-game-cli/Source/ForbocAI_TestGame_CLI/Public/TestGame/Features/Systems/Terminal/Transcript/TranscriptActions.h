#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace TranscriptActions {

struct FRecordTranscriptPayload {
  FString ScenarioId;
  FString CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  FString Status;
  FString Output;
  double DurationMs{};
};

/** User Story: As a systems terminal transcript consumer, I need to invoke record transcript action creator through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::ActionCreator<FRecordTranscriptPayload> recordTranscriptActionCreator() */
inline rtk::ActionCreator<FRecordTranscriptPayload>
recordTranscriptActionCreator() {
  static auto C = rtk::createAction<FRecordTranscriptPayload>(
      TEXT("testgame/transcript/recordTranscript"));
  return C;
}

/** User Story: As a systems terminal transcript consumer, I need to invoke reset transcript action creator through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload resetTranscriptActionCreator() */
inline rtk::ActionCreatorWithoutPayload resetTranscriptActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/transcript/resetTranscript"));
  return C;
}

/** User Story: As a systems terminal transcript consumer, I need to invoke record transcript through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::AnyAction recordTranscript(const FRecordTranscriptPayload &P) */
inline rtk::AnyAction recordTranscript(const FRecordTranscriptPayload &P) {
  return recordTranscriptActionCreator()(P);
}

/** User Story: As a systems terminal transcript consumer, I need to invoke reset transcript through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::AnyAction resetTranscript() */
inline rtk::AnyAction resetTranscript() {
  return resetTranscriptActionCreator()();
}

} // namespace TranscriptActions
} // namespace TestGame
