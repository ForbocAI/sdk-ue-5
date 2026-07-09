#pragma once

#include "CoreMinimal.h"
#include "Core/ue_fp.hpp"

#include <initializer_list>

namespace frmt {

/**
 * @brief Adapts an FString into the runtime formatter argument list.
 * @signature inline FString Arg(const FString &Value)
 * @return The input string unchanged.
 *
 * User Story: As RTK reducer and ECS projection code, I need authored text
 * values to enter UI and logging formatters without leaving the functional
 * value pipeline.
 */
inline FString Arg(const FString &Value) { return Value; }

/**
 * @brief Adapts a TCHAR string literal into the runtime formatter argument list.
 * @signature inline FString Arg(const TCHAR *Value)
 * @return An FString copy of the literal pointer value.
 *
 * User Story: As feature code, I need literal labels to share the same small
 * formatter boundary as reducer-owned runtime values.
 */
inline FString Arg(const TCHAR *Value) { return FString(Value); }

/**
 * @brief Adapts one TCHAR into the runtime formatter argument list.
 * @signature inline FString Arg(TCHAR Value)
 * @return An FString containing the single character.
 *
 * User Story: As speech and UI formatting code, I need character values to
 * remain explicit data before they are rendered or logged.
 */
inline FString Arg(TCHAR Value) { return FString::Printf(TEXT("%c"), Value); }

/**
 * @brief Adapts an int32 into the runtime formatter argument list.
 * @signature inline FString Arg(int32 Value)
 * @return The decimal FString representation of the integer.
 *
 * User Story: As selector-derived UI and logger text, numeric RTK/ECS state
 * should format through one value adapter instead of ad hoc conversions.
 */
inline FString Arg(int32 Value) { return FString::FromInt(Value); }

/**
 * @brief Adapts an int64 into the runtime formatter argument list.
 * @signature inline FString Arg(int64 Value)
 * @return The decimal FString representation of the integer.
 *
 * User Story: As runtime stats UI, larger aggregate counters should format
 * through the same authored-string path as smaller reducer-owned values.
 */
inline FString Arg(int64 Value) { return LexToString(Value); }

/**
 * @brief Adapts a float into the runtime formatter argument list.
 * @signature inline FString Arg(float Value)
 * @return The sanitized FString representation of the float.
 *
 * User Story: As rendering and speech thunks, numeric effect payloads should
 * log through a deterministic formatter boundary.
 */
inline FString Arg(float Value) { return FString::SanitizeFloat(Value); }

/**
 * @brief Adapts a double into the runtime formatter argument list.
 * @signature inline FString Arg(double Value)
 * @return The sanitized FString representation of the double.
 *
 * User Story: As runtime adapters, wider numeric values should collapse into
 * the same FString argument shape used by UI and logger reducers.
 */
inline FString Arg(double Value) { return FString::SanitizeFloat(Value); }

/**
 * @brief Builds the ordered runtime formatter argument array.
 * @signature inline TArray<FString> Args(std::initializer_list<FString> Values)
 * @return A UE array preserving the initializer-list argument order.
 *
 * User Story: As FP-style feature code, I need grouped declaration data for
 * formatter arguments so call sites remain data-first and side-effect free.
 */
inline TArray<FString> Args(std::initializer_list<FString> Values) {
  return TArray<FString>(Values);
}

/**
 * @brief Selects the earlier valid runtime format token index.
 * @signature inline int32 EarlierTokenIndex(int32 Left, int32 Right)
 * @return The lower valid index, or INDEX_NONE when neither index is valid.
 *
 * User Story: As formatter internals, token selection should stay as a pure
 * value transform that can support RTK logs and ECS-derived UI text equally.
 */
inline int32 EarlierTokenIndex(int32 Left, int32 Right) {
  return Left == INDEX_NONE
             ? Right
             : (Right == INDEX_NONE ? Left : FMath::Min(Left, Right));
}

/**
 * @brief Finds the first supported runtime format token in a pattern.
 * @signature inline int32 FirstRuntimeFormatTokenIndex(const FString &Pattern)
 * @return The first %s, %d, %f, or %c token index, or INDEX_NONE when absent.
 *
 * User Story: As feature text formatting, supported token lookup should be a
 * reusable pure helper instead of repeated string scanning at reducer call
 * sites.
 */
inline int32 FirstRuntimeFormatTokenIndex(const FString &Pattern) {
  return EarlierTokenIndex(
      EarlierTokenIndex(Pattern.Find(TEXT("%s")), Pattern.Find(TEXT("%d"))),
      EarlierTokenIndex(Pattern.Find(TEXT("%f")), Pattern.Find(TEXT("%c"))));
}

inline int32 RuntimeFormatTokenWidth() { return FString(TEXT("%s")).Len(); }

/**
 * @brief Replaces the first supported runtime format token with one value.
 * @signature inline FString ReplaceRuntimeFormatToken(const FString &Pattern, const FString &Value)
 * @return The pattern with its first token replaced, or the pattern unchanged.
 *
 * User Story: As UI and logger reducers, each replacement should be an
 * immutable FString transform so formatting remains outside UE side effects.
 */
inline FString ReplaceRuntimeFormatToken(const FString &Pattern,
                                         const FString &Value) {
  const int32 TokenIndex = FirstRuntimeFormatTokenIndex(Pattern);
  return TokenIndex == INDEX_NONE
             ? Pattern
             : Pattern.Left(TokenIndex) + Value +
                   Pattern.Mid(TokenIndex + RuntimeFormatTokenWidth());
}

/**
 * @brief Formats a runtime string by replacing supported tokens in order.
 * @signature inline FString RuntimeString(const FString &Pattern, const TArray<FString> &Values)
 * @return The pattern formatted with the supplied argument values.
 *
 * User Story: As gameplay UI, RTK logger, and ECS projection code, I need one
 * small formatter boundary so user-visible text is derived from state without
 * scattering FString conversion logic.
 */
inline FString RuntimeString(const FString &Pattern,
                             const TArray<FString> &Values) {
  return func::fold_array<FString, FString>(
      Values, Pattern,
      [](const FString &Current, const FString &Value) {
        return ReplaceRuntimeFormatToken(Current, Value);
      });
}

} // namespace frmt
