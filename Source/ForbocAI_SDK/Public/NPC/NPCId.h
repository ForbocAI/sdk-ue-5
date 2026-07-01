#pragma once

#include "CoreMinimal.h"

namespace NPCId {

/**
 * Encodes an integer value as base36 text.
 * User Story: As id generation, I need compact base36 encoding so NPC ids stay
 * short while preserving timestamp uniqueness.
 */
namespace detail {
inline FString ToBase36Recursive(uint64 Value, const TCHAR *Digits,
                                 const FString &Acc) {
  return Value == 0
             ? Acc
             : ToBase36Recursive(
                   Value / 36, Digits,
                   FString::Chr(Digits[static_cast<int32>(Value % 36)]) + Acc);
}
} // namespace detail

inline FString ToBase36(uint64 Value) {
  const TCHAR Digits[] = TEXT("0123456789abcdefghijklmnopqrstuvwxyz");
  return Value == 0 ? FString(TEXT("0"))
                    : detail::ToBase36Recursive(Value, Digits, FString());
}

/**
 * Generates an NPC id matching the TS SDK shape: ag_<base36 timestamp>.
 * UE uses Unix 100ns ticks so rapid same-frame creates do not collide while
 * preserving the timestamp-only id contract.
 * User Story: As cross-SDK id generation, I need UE NPC ids to match the TS
 * format so imported and synchronized agents share one identifier shape.
 */
inline FString GenerateNPCId() {
  const int64 UnixTicks =
      (FDateTime::UtcNow() - FDateTime(1970, 1, 1)).GetTicks();
  return TEXT("ag_") + ToBase36(static_cast<uint64>(UnixTicks));
}

} // namespace NPCId
