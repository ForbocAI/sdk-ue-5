#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Terminal/TerminalTypes.h"

namespace TestGame {
namespace TranscriptActions {

struct FRecordTranscriptPayload {
  FString ScenarioId;
  ECommandGroup CommandGroup;
  FString Command;
  TArray<FString> ExpectedRoutes;
  ETranscriptStatus Status;
  FString Output;
};

inline rtk::ActionCreator<FRecordTranscriptPayload>
recordTranscriptActionCreator() {
  static auto C = rtk::createAction<FRecordTranscriptPayload>(
      TEXT("testgame/transcript/recordTranscript"));
  return C;
}

inline rtk::ActionCreatorWithoutPayload resetTranscriptActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/transcript/resetTranscript"));
  return C;
}

inline rtk::AnyAction recordTranscript(const FRecordTranscriptPayload &P) {
  return recordTranscriptActionCreator()(P);
}

inline rtk::AnyAction resetTranscript() {
  return resetTranscriptActionCreator()();
}

} // namespace TranscriptActions
} // namespace TestGame
