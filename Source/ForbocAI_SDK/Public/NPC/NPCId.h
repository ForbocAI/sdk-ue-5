#pragma once

#include "CoreMinimal.h"
#include <atomic>

namespace NPCId {

/**
 * Encodes an integer value as base36 text.
 * User Story: As id generation, I need compact base36 encoding so NPC ids stay
 * short while preserving timestamp uniqueness.
 */
namespace detail {
inline std::atomic<uint64> &SequenceStorage() {
  static std::atomic<uint64> Value{0};
  return Value;
}

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
 * Generates an NPC id matching the TS SDK shape: ag_<base36 numeric id>.
 * UE combines Unix milliseconds with a process-local sequence so rapid
 * same-frame creates do not collide.
 * User Story: As cross-SDK id generation, I need UE NPC ids to match the TS
 * format so imported and synchronized agents share one identifier shape.
 */
inline FString GenerateNPCId() {
  const int64 UnixTicks =
      (FDateTime::UtcNow() - FDateTime(1970, 1, 1)).GetTicks();
  const uint64 UnixMs = static_cast<uint64>(UnixTicks / 10000);
  const uint64 Sequence =
      detail::SequenceStorage().fetch_add(1, std::memory_order_relaxed) %
      1000000;
  return TEXT("ag_") + ToBase36((UnixMs * 1000000) + Sequence);
}

} // namespace NPCId
