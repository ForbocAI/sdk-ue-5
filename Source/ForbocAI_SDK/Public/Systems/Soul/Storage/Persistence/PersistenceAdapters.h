#pragma once

#include "CoreMinimal.h"
#include "Components/Soul/Storage/StorageTypes.h"

namespace SoulStorage::Persistence {

/** User Story: As a returning UE SDK user, I need the same validated private wallet loaded across processes. @fn FORBOCAI_SDK_API bool readSoulWalletAdapter(FSoulWallet &Wallet, bool &bExists, FString &Error) */
FORBOCAI_SDK_API bool readSoulWalletAdapter(FSoulWallet &Wallet,
                                           bool &bExists, FString &Error);

/** User Story: As a first-run UE SDK user, I need generated wallet material persisted atomically under the user profile. @fn FORBOCAI_SDK_API bool writeSoulWalletAdapter(const FSoulWallet &Wallet, FString &Error) */
FORBOCAI_SDK_API bool writeSoulWalletAdapter(const FSoulWallet &Wallet,
                                            FString &Error);

/** User Story: As a UE SDK user, I need an absent catalog initialized explicitly while malformed metadata fails closed. @fn FORBOCAI_SDK_API bool readSoulCatalogAdapter(FSoulCatalog &Catalog, FString &Error) */
FORBOCAI_SDK_API bool readSoulCatalogAdapter(FSoulCatalog &Catalog,
                                            FString &Error);

/** User Story: As a UE SDK user, I need confirmed provider records published atomically for later processes. @fn FORBOCAI_SDK_API bool writeSoulCatalogAdapter(const FSoulCatalog &Catalog, FString &Error) */
FORBOCAI_SDK_API bool writeSoulCatalogAdapter(const FSoulCatalog &Catalog,
                                             FString &Error);

} // namespace SoulStorage::Persistence
