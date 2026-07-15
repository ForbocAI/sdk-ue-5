#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Bridge/BridgeActions.h"
#include "TestGame/Features/Systems/Bridge/BridgeAdapters.h"

namespace TestGame {

namespace GameBridgeSelectors {
inline int32 SelectBridgeMaxJumpForce(const FBridgeRulesState &S) {
  return S.MaxJumpForce;
}
inline int32 SelectBridgeMaxMoveDistance(const FBridgeRulesState &S) {
  return S.MaxMoveDistance;
}
inline FString SelectBridgeActivePreset(const FBridgeRulesState &S) {
  return S.ActivePreset;
}
} // namespace GameBridgeSelectors

inline rtk::Slice<FBridgeRulesState> CreateGameBridgeSlice() {
  return rtk::createSlice<FBridgeRulesState>(
      TEXT("testgame/bridge"), CreateBridgeInitialState(),
      [](rtk::ActionReducerMapBuilder<FBridgeRulesState> &Builder) {
        Builder.addCase(
            GameBridgeActions::setBridgeRulesActionCreator(),
            [](const FBridgeRulesState &S,
               const rtk::Action<GameBridgeActions::FSetBridgeRulesPayload> &A)
                -> FBridgeRulesState {
              FBridgeRulesState Next = S;
              Next.MaxJumpForce = A.PayloadValue.bHasMaxJumpForce
                                      ? A.PayloadValue.MaxJumpForce
                                      : Next.MaxJumpForce;
              Next.MaxMoveDistance = A.PayloadValue.bHasMaxMoveDistance
                                         ? A.PayloadValue.MaxMoveDistance
                                         : Next.MaxMoveDistance;
              Next.ActivePreset = A.PayloadValue.bHasActivePreset
                                      ? A.PayloadValue.ActivePreset
                                      : Next.ActivePreset;
              return Next;
            });
        Builder.addCase(
            GameBridgeActions::loadBridgePresetActionCreator(),
            [](const FBridgeRulesState &S,
               const rtk::Action<FString> &A) -> FBridgeRulesState {
              FBridgeRulesState Next = S;
              Next.ActivePreset = A.PayloadValue;
              const TOptional<int32> Distance =
                  ResolveBridgePresetMoveDistance(A.PayloadValue);
              Next.MaxMoveDistance =
                  Distance.IsSet() ? Distance.GetValue() : Next.MaxMoveDistance;
              return Next;
            });
      });
}

} // namespace TestGame
