#pragma once

#include "Features/Soul/Storage/StorageTypes.h"

namespace Testing::Soul::Storage::Serialization {

struct FStorageSerializationFixture {
  FSoulWallet Wallet;
  FSoulEnvelope Envelope;
  FSoulCatalog Catalog;
};

struct FStorageSerializationResult {
  bool bWalletRoundTrip{};
  bool bEnvelopeRoundTrip{};
  bool bCatalogRoundTrip{};
};

} // namespace Testing::Soul::Storage::Serialization
