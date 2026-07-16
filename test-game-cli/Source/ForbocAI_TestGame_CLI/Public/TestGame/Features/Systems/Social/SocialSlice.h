#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Social/SocialActions.h"

namespace TestGame {

namespace SocialSelectors {
/** User Story: As a features systems social consumer, I need to invoke select social active dialogue through a stable signature so the features systems social workflow remains explicit and composable. @fn inline FString SelectSocialActiveDialogue(const FSocialState &S) */
inline FString SelectSocialActiveDialogue(const FSocialState &S) {
  return S.ActiveDialogue;
}
/** User Story: As a features systems social consumer, I need to invoke select social active trade through a stable signature so the features systems social workflow remains explicit and composable. @fn inline func::Maybe<FTradeOffer> SelectSocialActiveTrade(const FSocialState &S) */
inline func::Maybe<FTradeOffer> SelectSocialActiveTrade(const FSocialState &S) {
  return S.bHasActiveTrade ? func::just<FTradeOffer>(S.ActiveTrade)
                           : func::nothing<FTradeOffer>();
}
} // namespace SocialSelectors

/** User Story: As a features systems social consumer, I need to invoke create social slice through a stable signature so the features systems social workflow remains explicit and composable. @fn inline rtk::Slice<FSocialState> CreateSocialSlice() */
inline rtk::Slice<FSocialState> CreateSocialSlice() {
  return rtk::createSlice<FSocialState>(
      TEXT("testgame/social"), FSocialState(),
      [](rtk::ActionReducerMapBuilder<FSocialState> &Builder) {
        Builder.addCase(
            SocialActions::setDialogueActionCreator(),
            [](const FSocialState &S,
               const rtk::Action<FString> &A) -> FSocialState {
              FSocialState Next = S;
              Next.ActiveDialogue = A.PayloadValue;
              return Next;
            });
        Builder.addCase(
            SocialActions::setTradeOfferActionCreator(),
            [](const FSocialState &S,
               const rtk::Action<FTradeOffer> &A) -> FSocialState {
              FSocialState Next = S;
              Next.ActiveTrade = A.PayloadValue;
              Next.bHasActiveTrade = true;
              return Next;
            });
        Builder.addCase(
            SocialActions::clearSocialStateActionCreator(),
            [](const FSocialState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FSocialState {
              return FSocialState();
            });
      });
}

} // namespace TestGame
