#pragma once

#include "Core/RTK/Types/Types.hpp"

#include <stdexcept>
#include <type_traits>

namespace rtk {

static const FString SHOULD_AUTOBATCH = TEXT("RTK_autoBatch");

struct __DO_NOT_USE__ActionTypes {
  /** User Story: As a core rtk utility consumer, I need to invoke init through a stable signature so the core rtk utility workflow remains explicit and composable. @fn static FString Init() */
  static FString Init() { return TEXT("@@redux/INIT"); }
  /** User Story: As a core rtk utility consumer, I need to invoke replace through a stable signature so the core rtk utility workflow remains explicit and composable. @fn static FString Replace() */
  static FString Replace() { return TEXT("@@redux/REPLACE"); }
  /** User Story: As a core rtk utility consumer, I need to invoke probe unknown action through a stable signature so the core rtk utility workflow remains explicit and composable. @fn static FString ProbeUnknownAction() */
  static FString ProbeUnknownAction() {
    return TEXT("@@redux/PROBE_UNKNOWN_ACTION_") + FGuid::NewGuid().ToString();
  }
};

class TaskAbortError : public std::runtime_error {
public:
  /** User Story: As a core rtk utility consumer, I need to invoke task abort error through a stable signature so the core rtk utility workflow remains explicit and composable. @fn explicit TaskAbortError(const std::string &Message) */
  explicit TaskAbortError(const std::string &Message)
      : std::runtime_error(Message) {}
};

namespace detail {
/** User Story: As a core rtk utility consumer, I need to invoke append nanoid characters through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline FString appendNanoidCharacters(int32 Remaining, FString Value) */
inline FString appendNanoidCharacters(int32 Remaining, FString Value) {
  static const FString Alphabet =
      TEXT("useandom-26T198340PX75pxJACKVERYMINDBLOWING");
  return Remaining <= 0
             ? Value
             : appendNanoidCharacters(
                   Remaining - 1,
                   Value + Alphabet.Mid(
                               FMath::RandRange(0, Alphabet.Len() - 1), 1));
}
} // namespace detail

/** User Story: As a core rtk utility consumer, I need to invoke nanoid through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline FString nanoid(int32 Size = 21) */
inline FString nanoid(int32 Size = 21) {
  return detail::appendNanoidCharacters(FMath::Max(Size, 0), FString());
}

/** User Story: As a core rtk utility consumer, I need to invoke format prod error message through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline FString formatProdErrorMessage(int32 Code) */
inline FString formatProdErrorMessage(int32 Code) {
  return FString::Printf(
      TEXT("Minified Redux Toolkit error #%d; visit https://redux-toolkit.js.org/Errors?code=%d for the full message."),
      Code, Code);
}

/** User Story: As a core rtk utility consumer, I need to invoke mini serialize error through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline SerializedError miniSerializeError(const FString &Message) */
inline SerializedError miniSerializeError(const FString &Message) {
  SerializedError Result;
  Result.Name = TEXT("Error");
  Result.Message = Message;
  return Result;
}

/** User Story: As a core rtk utility consumer, I need to invoke mini serialize error through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline SerializedError miniSerializeError(const std::exception &Error) */
inline SerializedError miniSerializeError(const std::exception &Error) {
  return miniSerializeError(FString(UTF8_TO_TCHAR(Error.what())));
}

/** User Story: As a core rtk utility consumer, I need to invoke is plain object through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline bool isPlainObject(const FString &) */
inline bool isPlainObject(const FString &) { return false; }
/** User Story: As a core rtk utility consumer, I need to invoke is plain object through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline bool isPlainObject(const AnyAction &) */
inline bool isPlainObject(const AnyAction &) { return true; }

/** User Story: As a core rtk utility consumer, I need to invoke is plain object through a stable signature so the core rtk utility workflow remains explicit and composable. @fn template <typename Value> bool isPlainObject(const Value &) */
template <typename Value> bool isPlainObject(const Value &) {
  return !std::is_pointer<Value>::value &&
         !std::is_arithmetic<Value>::value && !std::is_enum<Value>::value;
}

/** User Story: As a core rtk utility consumer, I need to invoke is plain through a stable signature so the core rtk utility workflow remains explicit and composable. @fn template <typename Value> bool isPlain(const Value &Input) */
template <typename Value> bool isPlain(const Value &Input) {
  return isPlainObject(Input);
}

/** User Story: As a core rtk utility consumer, I need to invoke is plain through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline bool isPlain(const FString &) */
inline bool isPlain(const FString &) { return true; }

/** User Story: As a core rtk utility consumer, I need to invoke is immutable default through a stable signature so the core rtk utility workflow remains explicit and composable. @fn template <typename Value> bool isImmutableDefault(const Value &) */
template <typename Value> bool isImmutableDefault(const Value &) {
  return std::is_arithmetic<Value>::value || std::is_enum<Value>::value;
}

/** User Story: As a core rtk utility consumer, I need to invoke is immutable default through a stable signature so the core rtk utility workflow remains explicit and composable. @fn inline bool isImmutableDefault(const FString &) */
inline bool isImmutableDefault(const FString &) { return true; }

} // namespace rtk
