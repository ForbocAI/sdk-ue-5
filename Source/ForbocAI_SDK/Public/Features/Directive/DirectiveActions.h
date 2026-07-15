#pragma once

#include "Core/rtk.hpp"
#include "Features/Directive/DirectiveTypes.h"

namespace DirectiveSlice {
namespace Actions {

inline const rtk::ActionCreator<FDirectiveRunStartedPayload> &
directiveRunStartedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRunStartedPayload> ActionCreator =
      rtk::createAction<FDirectiveRunStartedPayload>(
          TEXT("directive/directiveRunStarted"));
  return ActionCreator;
}

inline const rtk::ActionCreator<FDirectiveReceivedPayload> &
directiveReceivedActionCreator() {
  static const rtk::ActionCreator<FDirectiveReceivedPayload> ActionCreator =
      rtk::createAction<FDirectiveReceivedPayload>(
          TEXT("directive/directiveReceived"));
  return ActionCreator;
}

inline const rtk::ActionCreator<FVerdictValidatedPayload> &
verdictValidatedActionCreator() {
  static const rtk::ActionCreator<FVerdictValidatedPayload> ActionCreator =
      rtk::createAction<FVerdictValidatedPayload>(
          TEXT("directive/verdictValidated"));
  return ActionCreator;
}

inline const rtk::ActionCreator<FDirectiveRunFailedPayload> &
directiveRunFailedActionCreator() {
  static const rtk::ActionCreator<FDirectiveRunFailedPayload> ActionCreator =
      rtk::createAction<FDirectiveRunFailedPayload>(
          TEXT("directive/directiveRunFailed"));
  return ActionCreator;
}

inline const rtk::ActionCreator<FString> &
clearDirectivesForNpcActionCreator() {
  static const rtk::ActionCreator<FString> ActionCreator =
      rtk::createAction<FString>(TEXT("directive/clearDirectivesForNpc"));
  return ActionCreator;
}

inline rtk::AnyAction directiveRunStarted(const FString &Id,
                                          const FString &NpcId,
                                          const FString &Observation) {
  return directiveRunStartedActionCreator()(
      FDirectiveRunStartedPayload{Id, NpcId, Observation});
}

inline rtk::AnyAction directiveReceived(const FString &Id,
                                        const FDirectiveResponse &Response) {
  return directiveReceivedActionCreator()(FDirectiveReceivedPayload{Id, Response});
}

inline rtk::AnyAction verdictValidated(const FString &Id,
                                       const FVerdictResponse &Verdict) {
  return verdictValidatedActionCreator()(FVerdictValidatedPayload{Id, Verdict});
}

inline rtk::AnyAction directiveRunFailed(const FString &Id,
                                         const FString &Error) {
  return directiveRunFailedActionCreator()(FDirectiveRunFailedPayload{Id, Error});
}

inline rtk::AnyAction clearDirectivesForNpc(const FString &NpcId) {
  return clearDirectivesForNpcActionCreator()(NpcId);
}

} // namespace Actions
} // namespace DirectiveSlice
