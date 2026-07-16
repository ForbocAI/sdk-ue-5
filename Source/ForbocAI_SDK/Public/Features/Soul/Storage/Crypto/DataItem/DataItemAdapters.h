#pragma once

#include "CoreMinimal.h"
#include "Features/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Crypto::DataItem {

/**
 * User Story: As a permanent-storage client, I need encrypted Soul bytes
 * wrapped in a signed ANS-104 data item accepted by the provider.
 * @fn FORBOCAI_SDK_API bool buildSoulDataItemAdapter( const TArray<uint8> &Payload, const FSoulWallet &Wallet, TArray<uint8> &DataItem, FString &Error)
 */
FORBOCAI_SDK_API bool buildSoulDataItemAdapter(
    const TArray<uint8> &Payload, const FSoulWallet &Wallet,
    TArray<uint8> &DataItem, FString &Error);

/**
 * User Story: As a Soul exporter, I need the signed ANS-104 identifier before
 * upload so API preparation binds the exact immutable transaction.
 * @fn FORBOCAI_SDK_API FString soulDataItemIdAdapter( const TArray<uint8> &DataItem, FString &Error)
 */
FORBOCAI_SDK_API FString soulDataItemIdAdapter(
    const TArray<uint8> &DataItem, FString &Error);

/**
 * User Story: As a Soul verifier, I need encrypted payload digests computed
 * identically before upload and after external retrieval.
 * @fn FORBOCAI_SDK_API FString soulPayloadDigestAdapter( const TArray<uint8> &Payload, FString &Error)
 */
FORBOCAI_SDK_API FString soulPayloadDigestAdapter(
    const TArray<uint8> &Payload, FString &Error);

} // namespace SoulStorage::Crypto::DataItem
