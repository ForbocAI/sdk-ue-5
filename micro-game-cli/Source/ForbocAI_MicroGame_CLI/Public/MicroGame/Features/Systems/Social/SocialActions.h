#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Social/SocialTypes.h"

namespace MicroGame {
namespace SocialActions {

/** User Story: As a features systems social consumer, I need to invoke set dialogue action creator through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> setDialogueActionCreator() */
inline rtk::ActionCreator<FString> setDialogueActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("microgame/social/setDialogue"));
  return C;
}

/** User Story: As a features systems social consumer, I need to invoke set trade offer action creator through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::ActionCreator<FTradeOffer> setTradeOfferActionCreator() */
inline rtk::ActionCreator<FTradeOffer> setTradeOfferActionCreator() {
  static auto C =
      rtk::createAction<FTradeOffer>(TEXT("microgame/social/setTradeOffer"));
  return C;
}

/** User Story: As a features systems social consumer, I need to invoke clear social state action creator through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload clearSocialStateActionCreator() */
inline rtk::ActionCreatorWithoutPayload clearSocialStateActionCreator() {
  static auto C =
      rtk::createAction(TEXT("microgame/social/clearSocialState"));
  return C;
}

/** User Story: As a features systems social consumer, I need to invoke set dialogue through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::AnyAction setDialogue(const FString &D) */
inline rtk::AnyAction setDialogue(const FString &D) {
  return setDialogueActionCreator()(D);
}

/** User Story: As a features systems social consumer, I need to invoke set trade offer through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::AnyAction setTradeOffer(const FTradeOffer &T) */
inline rtk::AnyAction setTradeOffer(const FTradeOffer &T) {
  return setTradeOfferActionCreator()(T);
}

/** User Story: As a features systems social consumer, I need to invoke clear social state through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::AnyAction clearSocialState() */
inline rtk::AnyAction clearSocialState() {
  return clearSocialStateActionCreator()();
}

} // namespace SocialActions
} // namespace MicroGame
