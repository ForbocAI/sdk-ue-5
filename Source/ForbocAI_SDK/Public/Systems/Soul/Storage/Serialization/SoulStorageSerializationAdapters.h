#pragma once

#include "CoreMinimal.h"
#include "Components/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Serialization {

/** User Story: As a Soul exporter, I need portable state serialized through the canonical schema before encryption. @fn FORBOCAI_SDK_API bool serializeSoulAdapter(const FSoul &Soul, TArray<uint8> &Bytes, FString &Error) */
FORBOCAI_SDK_API bool serializeSoulAdapter(const FSoul &Soul,
                                          TArray<uint8> &Bytes,
                                          FString &Error);

/** User Story: As a Soul importer, I need decrypted bytes normalized through the canonical schema before state hydration. @fn FORBOCAI_SDK_API bool normalizeSoulAdapter(const TArray<uint8> &Bytes, FSoul &Soul, FString &Error) */
FORBOCAI_SDK_API bool normalizeSoulAdapter(const TArray<uint8> &Bytes,
                                          FSoul &Soul, FString &Error);

/** User Story: As a Soul provider client, I need the authenticated envelope encoded as deterministic JSON bytes. @fn FORBOCAI_SDK_API TArray<uint8> encodeSoulEnvelopeAdapter(const FSoulEnvelope &Envelope) */
FORBOCAI_SDK_API TArray<uint8>
encodeSoulEnvelopeAdapter(const FSoulEnvelope &Envelope);

/** User Story: As a Soul importer, I need provider bytes validated as the exact supported envelope shape. @fn FORBOCAI_SDK_API bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes, FSoulEnvelope &Envelope, FString &Error) */
FORBOCAI_SDK_API bool decodeSoulEnvelopeAdapter(const TArray<uint8> &Bytes,
                                               FSoulEnvelope &Envelope,
                                               FString &Error);

/** User Story: As local Soul persistence, I need private JWK fields encoded without reflection or omitted key material. @fn FORBOCAI_SDK_API FString encodeSoulWalletAdapter(const FSoulWallet &Wallet) */
FORBOCAI_SDK_API FString encodeSoulWalletAdapter(const FSoulWallet &Wallet);

/** User Story: As local Soul persistence, I need every private JWK field validated before cryptographic use. @fn FORBOCAI_SDK_API bool normalizeSoulWalletAdapter(const FString &Json, FSoulWallet &Wallet, FString &Error) */
FORBOCAI_SDK_API bool normalizeSoulWalletAdapter(const FString &Json,
                                                FSoulWallet &Wallet,
                                                FString &Error);

/** User Story: As local Soul persistence, I need confirmed metadata encoded as one versioned durable catalog. @fn FORBOCAI_SDK_API FString encodeSoulCatalogAdapter(const FSoulCatalog &Catalog) */
FORBOCAI_SDK_API FString encodeSoulCatalogAdapter(const FSoulCatalog &Catalog);

/** User Story: As local Soul persistence, I need the complete durable catalog validated before provider retrieval. @fn FORBOCAI_SDK_API bool normalizeSoulCatalogAdapter(const FString &Json, FSoulCatalog &Catalog, FString &Error) */
FORBOCAI_SDK_API bool normalizeSoulCatalogAdapter(const FString &Json,
                                                 FSoulCatalog &Catalog,
                                                 FString &Error);

} // namespace SoulStorage::Serialization
