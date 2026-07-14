#pragma once

#include "Core/rtk.hpp"

namespace TestGame {
struct FTestGameState;

namespace APISlice {

inline bool providesTags(const rtk::Api<FTestGameState> &ApiDefinition) {
  return ApiDefinition.TagTypes.Num() > 0;
}

inline rtk::Api<FTestGameState> api = []() {
  rtk::Api<FTestGameState> ApiDefinition = rtk::createApi<FTestGameState>(
      TEXT("testGameApi"),
      TArray<FString>{TEXT("TestGameContract")});
  check(providesTags(ApiDefinition));
  return ApiDefinition;
}();

} // namespace APISlice
} // namespace TestGame
