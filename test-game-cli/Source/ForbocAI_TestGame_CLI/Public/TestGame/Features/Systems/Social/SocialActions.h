#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Social/SocialTypes.h"

namespace TestGame {
namespace SocialActions {

inline rtk::ActionCreator<FString> setDialogueActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/social/setDialogue"));
  return C;
}

inline rtk::ActionCreator<FTradeOffer> setTradeOfferActionCreator() {
  static auto C =
      rtk::createAction<FTradeOffer>(TEXT("testgame/social/setTradeOffer"));
  return C;
}

inline rtk::ActionCreatorWithoutPayload clearSocialStateActionCreator() {
  static auto C =
      rtk::createAction(TEXT("testgame/social/clearSocialState"));
  return C;
}

inline rtk::AnyAction setDialogue(const FString &D) {
  return setDialogueActionCreator()(D);
}

inline rtk::AnyAction setTradeOffer(const FTradeOffer &T) {
  return setTradeOfferActionCreator()(T);
}

inline rtk::AnyAction clearSocialState() {
  return clearSocialStateActionCreator()();
}

} // namespace SocialActions
} // namespace TestGame
