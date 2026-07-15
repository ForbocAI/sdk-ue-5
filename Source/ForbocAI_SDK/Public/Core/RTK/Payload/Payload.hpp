#pragma once

#include "Core/RTK/Prelude.hpp"

namespace rtk {

struct FEmptyPayload;

namespace payload_debug {

/**
 * @brief Converts an FEmptyPayload to a debug string.
 * @signature FString DebugPayloadString(const FEmptyPayload &)
 * @param Payload The empty payload (unused).
 * @return FString A string representation indicating no payload.
 *
 * User Story: As a debugger, I need this function to visually identify when an action carries no payload.
 */
inline FString DebugPayloadString(const FEmptyPayload &) { return TEXT("<none>"); }

/**
 * @brief Returns the FString payload as a debug string.
 * @signature FString DebugPayloadString(const FString &Value)
 * @param Value The FString payload.
 * @return FString The FString payload itself.
 *
 * User Story: As a debugger, I need this function to easily read FString payloads directly in the debug output.
 */
inline FString DebugPayloadString(const FString &Value) { return Value; }

/**
 * @brief Converts a boolean payload to a debug string.
 * @signature FString DebugPayloadString(const bool &Value)
 * @param Value The boolean payload.
 * @return FString "true" if the value is true, "false" otherwise.
 *
 * User Story: As a debugger, I need this function to clearly see the true/false state of boolean payloads.
 */
inline FString DebugPayloadString(const bool &Value) {
  return Value ? TEXT("true") : TEXT("false");
}

/**
 * @brief Converts an integral payload (non-boolean) to a debug string.
 * @signature template <typename T> typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The integral payload.
 * @return FString The integral value formatted as a string.
 *
 * User Story: As a debugger, I need this function to reliably inspect integer payloads of various sizes in a uniform string format.
 */
template <typename T>
typename std::enable_if<std::is_integral<T>::value &&
                            !std::is_same<T, bool>::value,
                        FString>::type
DebugPayloadString(const T &Value) {
  return FString::Printf(TEXT("%lld"), static_cast<long long>(Value));
}

/**
 * @brief Converts a floating-point payload to a debug string.
 * @signature template <typename T> typename std::enable_if<std::is_floating_point<T>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The floating-point payload.
 * @return FString The floating-point value safely converted to a string.
 *
 * User Story: As a debugger, I need this function to verify decimal precision or floating-point states without truncation.
 */
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, FString>::type
DebugPayloadString(const T &Value) {
  return FString::SanitizeFloat(static_cast<double>(Value));
}

template <typename T> class HasToString {
  template <typename U>
  static auto Test(int) -> decltype(std::declval<const U &>().ToString(),
                                    std::true_type());

  template <typename> static std::false_type Test(...);

public:
  static const bool value = decltype(Test<T>(0))::value;
};

/**
 * @brief Converts a payload with a ToString() method to a debug string.
 * @signature template <typename T> typename std::enable_if<HasToString<T>::value, FString>::type DebugPayloadString(const T &Value)
 * @param Value The payload object.
 * @return FString The string representation obtained from the object's ToString() method.
 *
 * User Story: As a debugger, I need this function so that complex objects can define their own string representations automatically.
 */
template <typename T>
typename std::enable_if<HasToString<T>::value, FString>::type
DebugPayloadString(const T &Value) {
  return Value.ToString();
}

/**
 * @brief Converts a TArray payload to a debug string indicating its length.
 * @signature template <typename T> FString DebugPayloadString(const TArray<T> &Values)
 * @param Values The TArray payload.
 * @return FString A string showing the length of the array.
 *
 * User Story: As a debugger, I need this function to quickly gauge the size of array payloads without overwhelming the log output.
 */
template <typename T>
FString DebugPayloadString(const TArray<T> &Values) {
  return FString::Printf(TEXT("TArray(len=%d)"), Values.Num());
}

/**
 * @brief Converts a TMap payload to a debug string indicating its length.
 * @signature template <typename K, typename V> FString DebugPayloadString(const TMap<K, V> &Map)
 * @param Map The TMap payload.
 * @return FString A string showing the number of entries in the map.
 *
 * User Story: As a debugger, I need this function to quickly assess the size of map payloads in store actions.
 */
template <typename K, typename V> FString DebugPayloadString(const TMap<K, V> &Map) {
  return FString::Printf(TEXT("TMap(len=%d)"), Map.Num());
}

/**
 * @brief Default conversion for payloads that cannot be easily stringified.
 * @signature template <typename T> typename std::enable_if<!HasToString<T>::value && !std::is_integral<T>::value && !std::is_floating_point<T>::value && !std::is_same<T, bool>::value, FString>::type DebugPayloadString(const T &)
 * @param unused The opaque payload.
 * @return FString A placeholder string "<opaque>".
 *
 * User Story: As a debugger, I need this function to gracefully handle unknown types without causing compilation errors when actions are dispatched.
 */
template <typename T>
typename std::enable_if<!HasToString<T>::value &&
                            !std::is_integral<T>::value &&
                            !std::is_floating_point<T>::value &&
                            !std::is_same<T, bool>::value,
                        FString>::type
DebugPayloadString(const T &) {
  return TEXT("<opaque>");
}

} // namespace payload_debug
} // namespace rtk
