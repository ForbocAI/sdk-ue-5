#include "Systems/Soul/Storage/Crypto/Wallet/WalletAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Crypto/Encoding/EncodingAdapters.h"

#define UI OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
THIRD_PARTY_INCLUDES_END
#undef UI

#include <memory>

namespace SoulStorage::Crypto::Wallet {
namespace {

using FKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using FKeyContext =
    std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using FSignContext =
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using FBigNumber = std::unique_ptr<BIGNUM, decltype(&BN_free)>;
using FRsaKey = std::unique_ptr<RSA, decltype(&RSA_free)>;

/**
 * User Story: As a cryptographic operator, I need OpenSSL failures converted
 * into actionable UE errors without exposing key material.
 * @fn FString walletOpenSslErrorAdapter(const FString &Prefix)
 */
FString walletOpenSslErrorAdapter(const FString &Prefix) {
  const unsigned long Code = ERR_get_error();
  ANSICHAR Buffer[FORBOCAI_SDK_AUTHORED_NUMBERV49CBEDE3562C] = {};
  ERR_error_string_n(Code, Buffer, sizeof(Buffer));
  return Prefix + TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB244656FFA69) + UTF8_TO_TCHAR(Buffer);
}

/**
 * User Story: As a JWK encoder, I need big integers serialized at their
 * unsigned minimal width before base64url encoding.
 * @fn FString walletBigNumberAdapter(const BIGNUM *Value)
 */
FString walletBigNumberAdapter(const BIGNUM *Value) {
  TArray<uint8> Bytes;
  Bytes.SetNumUninitialized(BN_num_bytes(Value));
  BN_bn2bin(Value, Bytes.GetData());
  return Encoding::soulBase64UrlEncodeAdapter(Bytes);
}

/**
 * User Story: As a persisted-wallet loader, I need one JWK integer decoded
 * into OpenSSL ownership without format coercion.
 * @fn BIGNUM *walletBigNumberFromJwkAdapter(const FString &Value)
 */
BIGNUM *walletBigNumberFromJwkAdapter(const FString &Value) {
  TArray<uint8> Bytes;
  return !Encoding::soulBase64UrlDecodeAdapter(Value, Bytes) ||
                 Bytes.IsEmpty()
             ? nullptr
             : BN_bin2bn(Bytes.GetData(), Bytes.Num(), nullptr);
}

/**
 * User Story: As a Soul signer, I need the complete persisted private JWK
 * reconstructed into an RSA key before signing.
 * @fn FKey walletPrivateKeyAdapter(const FSoulWallet &Wallet, FString &Error)
 */
FKey walletPrivateKeyAdapter(const FSoulWallet &Wallet, FString &Error) {
  FRsaKey Rsa(RSA_new(), RSA_free);
  FBigNumber N(walletBigNumberFromJwkAdapter(Wallet.N), BN_free);
  FBigNumber E(walletBigNumberFromJwkAdapter(Wallet.E), BN_free);
  FBigNumber D(walletBigNumberFromJwkAdapter(Wallet.D), BN_free);
  FBigNumber P(walletBigNumberFromJwkAdapter(Wallet.P), BN_free);
  FBigNumber Q(walletBigNumberFromJwkAdapter(Wallet.Q), BN_free);
  FBigNumber Dp(walletBigNumberFromJwkAdapter(Wallet.Dp), BN_free);
  FBigNumber Dq(walletBigNumberFromJwkAdapter(Wallet.Dq), BN_free);
  FBigNumber Qi(walletBigNumberFromJwkAdapter(Wallet.Qi), BN_free);
  const bool bComponents = Rsa && N && E && D && P && Q && Dp && Dq && Qi;
  const bool bKeyAssigned = bComponents &&
      RSA_set0_key(Rsa.get(), N.get(), E.get(), D.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  bKeyAssigned
      ? (N.release(), E.release(), D.release(), void())
      : void();
  const bool bFactorsAssigned = bKeyAssigned &&
      RSA_set0_factors(Rsa.get(), P.get(), Q.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  bFactorsAssigned ? (P.release(), Q.release(), void()) : void();
  const bool bCrtAssigned = bFactorsAssigned &&
      RSA_set0_crt_params(Rsa.get(), Dp.get(), Dq.get(), Qi.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  bCrtAssigned
      ? (Dp.release(), Dq.release(), Qi.release(), void())
      : void();
  FKey Key(bCrtAssigned ? EVP_PKEY_new() : nullptr, EVP_PKEY_free);
  const bool bAssigned =
      Key && EVP_PKEY_assign_RSA(Key.get(), Rsa.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  bAssigned ? (Rsa.release(), void()) : void();
  Error = bAssigned ? FString()
                    : Configuration::soulStorageData().Text.MissingPrivateKey;
  return bAssigned ? MoveTemp(Key) : FKey(nullptr, EVP_PKEY_free);
}

} // namespace

/** User Story: As a storage crypto wallet consumer, I need to invoke generate soul wallet adapter through a stable signature so the storage crypto wallet workflow remains explicit and composable. @fn bool generateSoulWalletAdapter(FSoulWallet &Wallet, FString &Error) */
bool generateSoulWalletAdapter(FSoulWallet &Wallet, FString &Error) {
  const Configuration::FSoulStorageConfigurationData &Data =
      Configuration::soulStorageData();
  FKeyContext Context(EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr),
                      EVP_PKEY_CTX_free);
  EVP_PKEY *RawKey = nullptr;
  const bool bGenerated = Context && EVP_PKEY_keygen_init(Context.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_PKEY_CTX_set_rsa_keygen_bits(Context.get(),
                                       Data.Wallet.ModulusLength) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_PKEY_keygen(Context.get(), &RawKey) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  FKey Key(RawKey, EVP_PKEY_free);
  const RSA *Rsa = bGenerated ? EVP_PKEY_get0_RSA(Key.get()) : nullptr;
  const BIGNUM *N = nullptr;
  const BIGNUM *E = nullptr;
  const BIGNUM *D = nullptr;
  const BIGNUM *P = nullptr;
  const BIGNUM *Q = nullptr;
  const BIGNUM *Dp = nullptr;
  const BIGNUM *Dq = nullptr;
  const BIGNUM *Qi = nullptr;
  Rsa ? (RSA_get0_key(Rsa, &N, &E, &D), RSA_get0_factors(Rsa, &P, &Q),
         RSA_get0_crt_params(Rsa, &Dp, &Dq, &Qi), void())
      : void();
  const bool bComplete = N && E && D && P && Q && Dp && Dq && Qi;
  Error = bComplete ? FString()
                    : walletOpenSslErrorAdapter(Data.Text.MissingPrivateKey);
  return !bComplete
             ? false
             : (Wallet.Kty = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD2DC267AC3C1),
                Wallet.N = walletBigNumberAdapter(N),
                Wallet.E = walletBigNumberAdapter(E),
                Wallet.D = walletBigNumberAdapter(D),
                Wallet.P = walletBigNumberAdapter(P),
                Wallet.Q = walletBigNumberAdapter(Q),
                Wallet.Dp = walletBigNumberAdapter(Dp),
                Wallet.Dq = walletBigNumberAdapter(Dq),
                Wallet.Qi = walletBigNumberAdapter(Qi), true);
}

/** User Story: As a storage crypto wallet consumer, I need to invoke sign soul bytes adapter through a stable signature so the storage crypto wallet workflow remains explicit and composable. @fn bool signSoulBytesAdapter(const FSoulWallet &Wallet, const TArray<uint8> &Bytes, TArray<uint8> &Signature, FString &Error) */
bool signSoulBytesAdapter(const FSoulWallet &Wallet,
                          const TArray<uint8> &Bytes,
                          TArray<uint8> &Signature, FString &Error) {
  FKey Key = walletPrivateKeyAdapter(Wallet, Error);
  FSignContext Context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
  EVP_PKEY_CTX *KeyContext = nullptr;
  const int32 SaltLength =
      Configuration::soulStorageData().Wallet.SaltLength;
  const bool bInitialized = Key && Context &&
      EVP_DigestSignInit(Context.get(), &KeyContext, EVP_sha256(), nullptr,
                         Key.get()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_PKEY_CTX_set_rsa_padding(KeyContext, RSA_PKCS1_PSS_PADDING) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_PKEY_CTX_set_rsa_pss_saltlen(KeyContext, SaltLength) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4 &&
      EVP_DigestSignUpdate(Context.get(), Bytes.GetData(), Bytes.Num()) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  size_t Length = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  const bool bSized = bInitialized &&
                      EVP_DigestSignFinal(Context.get(), nullptr, &Length) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  Signature.SetNumUninitialized(bSized ? static_cast<int32>(Length) : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  const bool bSigned = bSized &&
      EVP_DigestSignFinal(Context.get(), Signature.GetData(), &Length) == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4;
  Signature.SetNum(bSigned ? static_cast<int32>(Length) : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  Error = bSigned ? FString()
                  : walletOpenSslErrorAdapter(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB67542A7E254));
  return bSigned;
}

} // namespace SoulStorage::Crypto::Wallet
