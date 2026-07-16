#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Soul/Storage/Configuration/StorageConfigurationTypes.h"
#include "Misc/Base64.h"

namespace SoulStorage::Crypto::Encoding {

/**
 * User Story: As a cryptographic adapter, I need UE strings converted to exact
 * UTF-8 bytes before hashing and encryption.
 * @fn inline TArray<uint8> soulUtf8BytesAdapter(const FString &Value)
 */
inline TArray<uint8> soulUtf8BytesAdapter(const FString &Value) {
  const FTCHARToUTF8 Encoded(*Value);
  return TArray<uint8>(reinterpret_cast<const uint8 *>(Encoded.Get()),
                       Encoded.Length());
}

/**
 * User Story: As a Soul decoder, I need authenticated UTF-8 bytes converted
 * back to UE strings without null-termination assumptions.
 * @fn inline FString soulUtf8StringAdapter(const TArray<uint8> &Value)
 */
inline FString soulUtf8StringAdapter(const TArray<uint8> &Value) {
  const FUTF8ToTCHAR Decoded(
      reinterpret_cast<const ANSICHAR *>(Value.GetData()), Value.Num());
  return FString(Decoded.Length(), Decoded.Get());
}

/**
 * User Story: As a base64url encoder, I need padding removed recursively so no
 * mutable string trimming leaks into cryptographic call sites.
 * @fn inline FString soulBase64UrlWithoutPaddingAdapter(const FString &Value)
 */
inline FString soulBase64UrlWithoutPaddingAdapter(const FString &Value) {
  return Value.EndsWith(TEXT("="))
             ? soulBase64UrlWithoutPaddingAdapter(Value.LeftChop(1))
             : Value;
}

/**
 * User Story: As a JWK and ANS-104 codec, I need unpadded URL-safe base64 so
 * identifiers are byte-compatible with the TS SDK.
 * @fn inline FString soulBase64UrlEncodeAdapter(const TArray<uint8> &Value)
 */
inline FString soulBase64UrlEncodeAdapter(const TArray<uint8> &Value) {
  const FString Encoded = FBase64::Encode(Value)
                              .Replace(TEXT("+"), TEXT("-"))
                              .Replace(TEXT("/"), TEXT("_"));
  return soulBase64UrlWithoutPaddingAdapter(Encoded);
}

/**
 * User Story: As a base64url decoder, I need canonical padding restored before
 * UE's standard decoder processes private key material.
 * @fn inline FString soulBase64UrlPaddingAdapter(const FString &Value)
 */
inline FString soulBase64UrlPaddingAdapter(const FString &Value) {
  return Value.Len() % 4 == 0
             ? Value
             : soulBase64UrlPaddingAdapter(Value + TEXT("="));
}

/**
 * User Story: As a Soul decryptor, I need URL-safe values decoded through one
 * strict binary adapter.
 * @fn inline bool soulBase64UrlDecodeAdapter(const FString &Value, TArray<uint8> &Bytes)
 */
inline bool soulBase64UrlDecodeAdapter(const FString &Value,
                                       TArray<uint8> &Bytes) {
  const FString Normalized = soulBase64UrlPaddingAdapter(
      Value.Replace(TEXT("-"), TEXT("+")).Replace(TEXT("_"), TEXT("/")));
  return FBase64::Decode(Normalized, Bytes);
}

/**
 * User Story: As an ANS-104 serializer, I need fixed-width little-endian
 * integers built without platform-endian assumptions.
 * @fn inline TArray<uint8> soulLittleEndianAdapter( uint64 Value, int32 Length, TArray<uint8> Bytes = {})
 */
inline TArray<uint8> soulLittleEndianAdapter(
    uint64 Value, int32 Length, TArray<uint8> Bytes = {}) {
  return Bytes.Num() >= Length
             ? Bytes
             : soulLittleEndianAdapter(
                   Value >> 8, Length,
                   func::append_value<uint8>(MoveTemp(Bytes),
                                             static_cast<uint8>(Value & 255)));
}

/**
 * User Story: As an Avro tag serializer, I need nonnegative long values encoded
 * with the exact zig-zag variable-length representation used by ANS-104.
 * @fn inline TArray<uint8> soulAvroLongAdapter(uint64 Value, TArray<uint8> Bytes = {})
 */
inline TArray<uint8> soulAvroLongAdapter(uint64 Value,
                                        TArray<uint8> Bytes = {}) {
  const uint8 Byte = static_cast<uint8>(Value & 127);
  const uint64 Remaining = Value >> 7;
  TArray<uint8> Next = func::append_value<uint8>(
      MoveTemp(Bytes), Remaining == 0 ? Byte : static_cast<uint8>(Byte | 128));
  return Remaining == 0 ? Next
                        : soulAvroLongAdapter(Remaining, MoveTemp(Next));
}

/**
 * User Story: As an Avro tag serializer, I need UTF-8 strings length-prefixed
 * identically to the TS SDK.
 * @fn inline TArray<uint8> soulAvroStringAdapter(const FString &Value)
 */
inline TArray<uint8> soulAvroStringAdapter(const FString &Value) {
  const TArray<uint8> Bytes = soulUtf8BytesAdapter(Value);
  return func::concat_arrays<uint8>({
      soulAvroLongAdapter(static_cast<uint64>(Bytes.Num()) << 1), Bytes});
}

/**
 * User Story: As an ANS-104 signer, I need authored tags serialized in stable
 * order so the computed transaction ID matches provider validation.
 * @fn inline TArray<uint8> soulTagsAdapter( const TArray<Configuration::FDataItemTagData> &Tags)
 */
inline TArray<uint8> soulTagsAdapter(
    const TArray<Configuration::FDataItemTagData> &Tags) {
  const TArray<TArray<uint8>> Records =
      func::map_array<Configuration::FDataItemTagData, TArray<uint8>>(
          Tags, [](const Configuration::FDataItemTagData &Tag) {
            return func::concat_arrays<uint8>({
                soulAvroStringAdapter(Tag.Name),
                soulAvroStringAdapter(Tag.Value)});
          });
  return func::concat_arrays<uint8>({
      soulAvroLongAdapter(static_cast<uint64>(Tags.Num()) << 1),
      func::concat_arrays<uint8>(Records), TArray<uint8>{0}});
}

} // namespace SoulStorage::Crypto::Encoding
