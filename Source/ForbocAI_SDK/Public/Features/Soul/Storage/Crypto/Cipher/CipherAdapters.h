#pragma once

#include "CoreMinimal.h"
#include "Features/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Crypto::Cipher {

/**
 * User Story: As a Soul exporter, I need plaintext encrypted with authenticated
 * SDK-owned key material before any provider receives bytes.
 * @fn FORBOCAI_SDK_API bool encryptSoulAdapter(const TArray<uint8> &Plaintext, const FSoulWallet &Wallet, FSoulEnvelope &Envelope, FString &Error)
 */
FORBOCAI_SDK_API bool encryptSoulAdapter(const TArray<uint8> &Plaintext,
                                        const FSoulWallet &Wallet,
                                        FSoulEnvelope &Envelope,
                                        FString &Error);

/**
 * User Story: As a Soul importer, I need provider bytes authenticated and
 * decrypted locally before character state can enter Redux.
 * @fn FORBOCAI_SDK_API bool decryptSoulAdapter(const FSoulEnvelope &Envelope, const FSoulWallet &Wallet, TArray<uint8> &Plaintext, FString &Error)
 */
FORBOCAI_SDK_API bool decryptSoulAdapter(const FSoulEnvelope &Envelope,
                                        const FSoulWallet &Wallet,
                                        TArray<uint8> &Plaintext,
                                        FString &Error);

} // namespace SoulStorage::Crypto::Cipher
