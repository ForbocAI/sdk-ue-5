#pragma once

#include "CoreMinimal.h"
#include "Components/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Crypto::Wallet {

/**
 * User Story: As a first-run SDK user, I need a private RSA-PSS JWK generated
 * locally so permanent Soul ownership never moves to the API.
 * @fn FORBOCAI_SDK_API bool generateSoulWalletAdapter(FSoulWallet &Wallet, FString &Error)
 */
FORBOCAI_SDK_API bool generateSoulWalletAdapter(FSoulWallet &Wallet,
                                                FString &Error);

/**
 * User Story: As an ANS-104 serializer, I need deep-hash bytes signed by the
 * persisted local wallet using the same RSA-PSS policy as the TS SDK.
 * @fn FORBOCAI_SDK_API bool signSoulBytesAdapter(const FSoulWallet &Wallet, const TArray<uint8> &Bytes, TArray<uint8> &Signature, FString &Error)
 */
FORBOCAI_SDK_API bool signSoulBytesAdapter(const FSoulWallet &Wallet,
                                          const TArray<uint8> &Bytes,
                                          TArray<uint8> &Signature,
                                          FString &Error);

} // namespace SoulStorage::Crypto::Wallet
