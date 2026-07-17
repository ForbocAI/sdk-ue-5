#pragma once

#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"

namespace TestGame {

using FGameProgressSink = TFunction<void(const FGameProgress &)>;

namespace GameThunksDetail {

/** User Story: As a game harness, I need progress emitted through one injected effect boundary. @fn inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) */
inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) {
  Sink ? (Sink(Progress), void()) : void();
}

} // namespace GameThunksDetail
} // namespace TestGame
