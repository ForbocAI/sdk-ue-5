#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalTypes.h"

namespace MicroGame {
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

/** User Story: As a systems terminal transcript consumer, I need recorded entries carried by one typed action so reducers remain deterministic. @fn inline rtk::ActionCreator<FTranscriptEntry> recordTranscriptActionCreator() */
inline rtk::ActionCreator<FTranscriptEntry>
recordTranscriptActionCreator() {
  static auto C = rtk::createAction<FTranscriptEntry>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV93C0187A3C76));
  return C;
}

/** User Story: As a systems terminal transcript consumer, I need to invoke reset transcript action creator through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload resetTranscriptActionCreator() */
inline rtk::ActionCreatorWithoutPayload resetTranscriptActionCreator() {
  static auto C =
      rtk::createAction(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV12AC8BEC942C));
  return C;
}

/** User Story: As a systems terminal transcript consumer, I need time and identity prepared before dispatch so replaying the reducer stays deterministic. @fn inline rtk::AnyAction recordTranscript(const FRecordTranscriptPayload &P) */
inline rtk::AnyAction recordTranscript(const FRecordTranscriptPayload &P) {
  return recordTranscriptActionCreator()(FTranscriptEntry{
      FString::Format(
          *TerminalAdapters::TerminalData().transcript.idFormat,
          {FDateTime::Now().GetTicks(), FMath::Rand()}),
      P.ScenarioId,
      P.CommandGroup,
      P.Command,
      P.ExpectedRoutes,
      P.Status,
      P.Output,
      P.DurationMs,
      FDateTime::Now().ToIso8601()});
}

/** User Story: As a systems terminal transcript consumer, I need to invoke reset transcript through a stable signature so the systems terminal transcript workflow remains explicit and composable. @fn inline rtk::AnyAction resetTranscript() */
inline rtk::AnyAction resetTranscript() {
  return resetTranscriptActionCreator()();
}

} // namespace TranscriptActions
} // namespace MicroGame
