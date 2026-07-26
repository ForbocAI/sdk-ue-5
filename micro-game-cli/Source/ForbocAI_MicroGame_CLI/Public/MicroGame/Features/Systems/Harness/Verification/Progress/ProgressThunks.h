#pragma once

#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"

namespace MicroGame {

using FGameProgressSink = TFunction<void(const FGameProgress &)>;

namespace VerificationThunksDetail {

/** User Story: As a game harness, I need progress emitted through one injected effect boundary. @fn inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) */
inline void Emit(const FGameProgressSink &Sink, FGameProgress Progress) {
  Sink ? (Sink(Progress), void()) : void();
}

} // namespace VerificationThunksDetail
} // namespace MicroGame
