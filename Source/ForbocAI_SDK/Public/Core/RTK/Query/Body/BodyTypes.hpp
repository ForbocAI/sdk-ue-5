#pragma once

#include "Core/FP/Maybe/Maybe.hpp"

namespace rtk {

struct FetchBody {
  func::Maybe<FString> text;
  func::Maybe<TArray<uint8>> bytes;

  /** User Story: As an RTK Query caller, I need an absent request body represented explicitly so read requests do not synthesize payloads. @fn FetchBody() */
  FetchBody()
      : text(func::nothing<FString>()),
        bytes(func::nothing<TArray<uint8>>()) {}

  /** User Story: As an RTK Query caller, I need text request bodies accepted through the canonical body property. @fn FetchBody(const FString &Value) */
  FetchBody(const FString &Value)
      : text(func::just(Value)), bytes(func::nothing<TArray<uint8>>()) {}

  /** User Story: As an RTK Query caller, I need binary request bodies accepted through the canonical body property without an HTTP-specific escape hatch. @fn FetchBody(const TArray<uint8> &Value) */
  FetchBody(const TArray<uint8> &Value)
      : text(func::nothing<FString>()), bytes(func::just(Value)) {}

  /** User Story: As an RTK Query caller, I need text body assignment to replace any prior binary representation atomically. @fn FetchBody &operator=(const FString &Value) */
  FetchBody &operator=(const FString &Value) {
    text = func::just(Value);
    bytes = func::nothing<TArray<uint8>>();
    return *this;
  }

  /** User Story: As an RTK Query caller, I need binary body assignment to replace any prior text representation atomically. @fn FetchBody &operator=(const TArray<uint8> &Value) */
  FetchBody &operator=(const TArray<uint8> &Value) {
    text = func::nothing<FString>();
    bytes = func::just(Value);
    return *this;
  }
};

} // namespace rtk
