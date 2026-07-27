#include "Systems/Soul/Storage/Crypto/DataItem/DataItemAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Crypto/Encoding/EncodingAdapters.h"
#include "Systems/Soul/Storage/Crypto/Wallet/WalletAdapters.h"

#define UI OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/err.h>
#include <openssl/evp.h>
THIRD_PARTY_INCLUDES_END
#undef UI

#include <memory>

namespace SoulStorage::Crypto::DataItem {
namespace {

using FDigestContext =
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

/**
 * User Story: As a Soul storage operator, I need digest failures surfaced at
 * the cryptographic boundary with no ambiguous empty identifiers.
 * @fn FString dataItemOpenSslErrorAdapter(const FString &Prefix)
 */
FString dataItemOpenSslErrorAdapter(const FString &Prefix) {
  const unsigned long Code = ERR_get_error();
  ANSICHAR Buffer[FORBOCAI_SDK_AUTHORED_NUMBERV49CBEDE3562C] = {};
  ERR_error_string_n(Code, Buffer, sizeof(Buffer));
  return Prefix + TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB244656FFA69) + UTF8_TO_TCHAR(Buffer);
}

/**
 * User Story: As a Soul digest caller, I need one OpenSSL adapter for SHA-256
 * and SHA-384 so every hash uses exact binary lengths.
 * @fn bool digestSoulBytesAdapter(const EVP_MD *Algorithm, const TArray<uint8> &Bytes, TArray<uint8> &Digest)
 */
bool digestSoulBytesAdapter(const EVP_MD *Algorithm,
                            const TArray<uint8> &Bytes,
                            TArray<uint8> &Digest) {
  FDigestContext Context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
  unsigned int Length = EVP_MAX_MD_SIZE;
  Digest.SetNumUninitialized(EVP_MAX_MD_SIZE);
  const bool bDigested = Context &&
      EVP_DigestInit_ex(Context.get(), Algorithm, nullptr) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_DigestUpdate(Context.get(), Bytes.GetData(), Bytes.Num()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_DigestFinal_ex(Context.get(), Digest.GetData(), &Length) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  Digest.SetNum(bDigested ? static_cast<int32>(Length) : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  return bDigested;
}

/**
 * User Story: As an ANS-104 deep-hash caller, I need blob length and content
 * hashed using the exact SHA-384 composition required by the protocol.
 * @fn TArray<uint8> deepHashSoulBlobAdapter(const TArray<uint8> &Bytes, FString &Error)
 */
TArray<uint8> deepHashSoulBlobAdapter(const TArray<uint8> &Bytes,
                                      FString &Error) {
  const FString Tag = Configuration::soulStorageData().DataItem.DeepHashBlob +
                      FString::FromInt(Bytes.Num());
  TArray<uint8> TagHash;
  TArray<uint8> ValueHash;
  TArray<uint8> Result;
  const bool bDigested = digestSoulBytesAdapter(
                             EVP_sha384(), Encoding::soulUtf8BytesAdapter(Tag),
                             TagHash) &&
                         digestSoulBytesAdapter(EVP_sha384(), Bytes,
                                                ValueHash) &&
                         digestSoulBytesAdapter(
                             EVP_sha384(),
                             func::concat_arrays<uint8>({TagHash, ValueHash}),
                             Result);
  Error = bDigested
              ? FString()
              : dataItemOpenSslErrorAdapter(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9C15551AA45A));
  return Result;
}

/**
 * User Story: As an ANS-104 signer, I need list members folded recursively into
 * the protocol accumulator in authored order.
 * @fn bool deepHashSoulListStepAdapter(const TArray<TArray<uint8>> &Values, int32 Index, TArray<uint8> &Accumulator, FString &Error)
 */
bool deepHashSoulListStepAdapter(const TArray<TArray<uint8>> &Values,
                                 int32 Index,
                                 TArray<uint8> &Accumulator,
                                 FString &Error) {
  return Index >= Values.Num()
             ? true
             : [&]() {
                 const TArray<uint8> Item =
                     deepHashSoulBlobAdapter(Values[Index], Error);
                 TArray<uint8> Next;
                 const bool bNext =
                     Error.IsEmpty() &&
                     digestSoulBytesAdapter(
                         EVP_sha384(),
                         func::concat_arrays<uint8>({Accumulator, Item}),
                         Next);
                 Accumulator = MoveTemp(Next);
                 return bNext && deepHashSoulListStepAdapter(
                                     Values, Index + FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4, Accumulator, Error);
               }();
}

/**
 * User Story: As an ANS-104 signer, I need the root list tag initialized and
 * every member deep-hashed before RSA-PSS signing.
 * @fn TArray<uint8> deepHashSoulListAdapter(const TArray<TArray<uint8>> &Values, FString &Error)
 */
TArray<uint8> deepHashSoulListAdapter(const TArray<TArray<uint8>> &Values,
                                      FString &Error) {
  const FString Tag = Configuration::soulStorageData().DataItem.DeepHashList +
                      FString::FromInt(Values.Num());
  TArray<uint8> Accumulator;
  const bool bInitial = digestSoulBytesAdapter(
      EVP_sha384(), Encoding::soulUtf8BytesAdapter(Tag), Accumulator);
  const bool bFolded = bInitial && deepHashSoulListStepAdapter(
                                     Values, FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA, Accumulator, Error);
  Error = bFolded
              ? FString()
              : dataItemOpenSslErrorAdapter(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9C15551AA45A));
  return bFolded ? Accumulator : TArray<uint8>();
}

} // namespace

/** User Story: As a storage crypto data item consumer, I need to invoke build soul data item adapter through a stable signature so the storage crypto data item workflow remains explicit and composable. @fn bool buildSoulDataItemAdapter(const TArray<uint8> &Payload, const FSoulWallet &Wallet, TArray<uint8> &DataItem, FString &Error) */
bool buildSoulDataItemAdapter(const TArray<uint8> &Payload,
                              const FSoulWallet &Wallet,
                              TArray<uint8> &DataItem, FString &Error) {
  const Configuration::FDataItemData &Data =
      Configuration::soulStorageData().DataItem;
  TArray<uint8> Owner;
  const bool bOwner =
      Encoding::soulBase64UrlDecodeAdapter(Wallet.N, Owner) &&
      Owner.Num() == Data.OwnerLength;
  Error = bOwner ? FString()
                 : Configuration::soulStorageData().Text.InvalidOwnerLength;
  return !bOwner
             ? false
             : [&]() {
                 const TArray<uint8> Tags =
                     Encoding::soulTagsAdapter(Data.Tags);
                 const TArray<TArray<uint8>> DeepHashValues{
                     Encoding::soulUtf8BytesAdapter(Data.Domain),
                     Encoding::soulUtf8BytesAdapter(Data.Version),
                     Encoding::soulUtf8BytesAdapter(
                         FString::FromInt(Data.SignatureType)),
                     Owner,
                     TArray<uint8>(),
                     TArray<uint8>(),
                     Tags,
                     Payload};
                 const TArray<uint8> SignatureData =
                     deepHashSoulListAdapter(DeepHashValues, Error);
                 TArray<uint8> Signature;
                 const bool bSigned =
                     Error.IsEmpty() &&
                     Wallet::signSoulBytesAdapter(
                         Wallet, SignatureData, Signature, Error) &&
                     Signature.Num() == Data.SignatureLength;
                 Error = bSigned
                             ? FString()
                             : Error.IsEmpty()
                                   ? Configuration::soulStorageData()
                                         .Text.InvalidSignatureLength
                                   : Error;
                 return !bSigned
                            ? false
                            : [&]() {
                                const TArray<uint8> SignatureType =
                                    Encoding::soulLittleEndianAdapter(
                                        Data.SignatureType, FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);
                                TArray<uint8> SignaturePlaceholder;
                                SignaturePlaceholder.SetNumZeroed(
                                    Data.SignatureLength);
                                DataItem = func::concat_arrays<uint8>({
                                    SignatureType, SignaturePlaceholder, Owner,
                                    TArray<uint8>{static_cast<uint8>(
                                        Data.TargetPresent)},
                                    TArray<uint8>{static_cast<uint8>(
                                        Data.AnchorPresent)},
                                    Encoding::soulLittleEndianAdapter(
                                        Data.Tags.Num(), FORBOCAI_SDK_AUTHORED_NUMBERVAC6FC173B700),
                                    Encoding::soulLittleEndianAdapter(
                                        Tags.Num(), FORBOCAI_SDK_AUTHORED_NUMBERVAC6FC173B700),
                                    Tags, Payload});
                                FMemory::Memcpy(
                                    DataItem.GetData() + SignatureType.Num(),
                                    Signature.GetData(), Signature.Num());
                                return true;
                              }();
               }();
}

/** User Story: As a storage crypto data item consumer, I need to invoke soul data item id adapter through a stable signature so the storage crypto data item workflow remains explicit and composable. @fn FString soulDataItemIdAdapter(const TArray<uint8> &DataItem, FString &Error) */
FString soulDataItemIdAdapter(const TArray<uint8> &DataItem,
                              FString &Error) {
  const int32 SignatureLength =
      Configuration::soulStorageData().DataItem.SignatureLength;
  const bool bSized = DataItem.Num() >= SignatureLength + FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561;
  Error = bSized ? FString()
                 : Configuration::soulStorageData().Text.InvalidSignatureLength;
  return !bSized
             ? FString()
             : [&]() {
                 const TArray<uint8> Signature(DataItem.GetData() + FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561,
                                               SignatureLength);
                 TArray<uint8> Digest;
                 const bool bDigested = digestSoulBytesAdapter(
                     EVP_sha256(), Signature, Digest);
                 Error = bDigested
                             ? FString()
                             : dataItemOpenSslErrorAdapter(
                                   TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC078B222F7D8));
                 return bDigested
                            ? Encoding::soulBase64UrlEncodeAdapter(Digest)
                            : FString();
               }();
}

/** User Story: As a storage crypto data item consumer, I need to invoke soul payload digest adapter through a stable signature so the storage crypto data item workflow remains explicit and composable. @fn FString soulPayloadDigestAdapter(const TArray<uint8> &Payload, FString &Error) */
FString soulPayloadDigestAdapter(const TArray<uint8> &Payload,
                                 FString &Error) {
  TArray<uint8> Digest;
  const bool bDigested = digestSoulBytesAdapter(EVP_sha256(), Payload, Digest);
  Error = bDigested ? FString()
                    : dataItemOpenSslErrorAdapter(
                          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF8FAB4D15D70));
  return bDigested ? Encoding::soulBase64UrlEncodeAdapter(Digest) : FString();
}

} // namespace SoulStorage::Crypto::DataItem
