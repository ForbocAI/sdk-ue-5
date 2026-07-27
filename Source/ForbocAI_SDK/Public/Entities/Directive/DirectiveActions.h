#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Components/Directive/DirectiveTypes.h"

namespace DirectiveSlice {
namespace Actions {

/** User Story: As a features directive consumer, I need to invoke directive run started action creator through a stable signature so the features directive workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FDirectiveRunStartedPayload> & directiveRunStartedActionCreator() */
inline const rtk::ActionCreator<FDirectiveRunStartedPayload> &
directiveRunStartedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRunStartedPayload> ActionCreator =
      rtk::createAction<FDirectiveRunStartedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV82A9F70CCE55));
  return ActionCreator;
}

/** User Story: As a features directive consumer, I need to invoke directive received action creator through a stable signature so the features directive workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FDirectiveReceivedPayload> & directiveReceivedActionCreator() */
inline const rtk::ActionCreator<FDirectiveReceivedPayload> &
directiveReceivedActionCreator() {
  static const rtk::ActionCreator<FDirectiveReceivedPayload> ActionCreator =
      rtk::createAction<FDirectiveReceivedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGV39465F589103));
  return ActionCreator;
}

/** User Story: As a features directive consumer, I need to invoke verdict validated action creator through a stable signature so the features directive workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FVerdictValidatedPayload> & verdictValidatedActionCreator() */
inline const rtk::ActionCreator<FVerdictValidatedPayload> &
verdictValidatedActionCreator() {
  static const rtk::ActionCreator<FVerdictValidatedPayload> ActionCreator =
      rtk::createAction<FVerdictValidatedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBAE376C63B71));
  return ActionCreator;
}

/** User Story: As a features directive consumer, I need to invoke directive run failed action creator through a stable signature so the features directive workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FDirectiveRunFailedPayload> & directiveRunFailedActionCreator() */
inline const rtk::ActionCreator<FDirectiveRunFailedPayload> &
directiveRunFailedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRunFailedPayload> ActionCreator =
      rtk::createAction<FDirectiveRunFailedPayload>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE16B0D2E856C));
  return ActionCreator;
}

/** User Story: As a features directive consumer, I need to invoke clear directives for npc action creator through a stable signature so the features directive workflow remains explicit and composable. @fn inline const rtk::ActionCreator<FString> & clearDirectivesForNpcActionCreator() */
inline const rtk::ActionCreator<FString> &
clearDirectivesForNpcActionCreator() {
  static const rtk::ActionCreator<FString> ActionCreator =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV644D4BD623B2));
  return ActionCreator;
}

/** User Story: As a features directive consumer, I need to invoke directive run started through a stable signature so the features directive workflow remains explicit and composable. @fn inline rtk::AnyAction directiveRunStarted(const FString &Id, const FString &NpcId, const FString &Observation) */
inline rtk::AnyAction directiveRunStarted(const FString &Id,
                                          const FString &NpcId,
                                          const FString &Observation) {
  return directiveRunStartedActionCreator()(
      FDirectiveRunStartedPayload{Id, NpcId, Observation});
}

/** User Story: As a features directive consumer, I need to invoke directive received through a stable signature so the features directive workflow remains explicit and composable. @fn inline rtk::AnyAction directiveReceived(const FString &Id, const FDirectiveResponse &Response) */
inline rtk::AnyAction directiveReceived(const FString &Id,
                                        const FDirectiveResponse &Response) {
  return directiveReceivedActionCreator()(FDirectiveReceivedPayload{Id, Response});
}

/** User Story: As a features directive consumer, I need to invoke verdict validated through a stable signature so the features directive workflow remains explicit and composable. @fn inline rtk::AnyAction verdictValidated(const FString &Id, const FVerdictResponse &Verdict) */
inline rtk::AnyAction verdictValidated(const FString &Id,
                                       const FVerdictResponse &Verdict) {
  return verdictValidatedActionCreator()(FVerdictValidatedPayload{Id, Verdict});
}

/** User Story: As a features directive consumer, I need to invoke directive run failed through a stable signature so the features directive workflow remains explicit and composable. @fn inline rtk::AnyAction directiveRunFailed(const FString &Id, const FString &Error) */
inline rtk::AnyAction directiveRunFailed(const FString &Id,
                                         const FString &Error) {
  return directiveRunFailedActionCreator()(FDirectiveRunFailedPayload{Id, Error});
}

/** User Story: As a features directive consumer, I need to invoke clear directives for npc through a stable signature so the features directive workflow remains explicit and composable. @fn inline rtk::AnyAction clearDirectivesForNpc(const FString &NpcId) */
inline rtk::AnyAction clearDirectivesForNpc(const FString &NpcId) {
  return clearDirectivesForNpcActionCreator()(NpcId);
}

} // namespace Actions
} // namespace DirectiveSlice
