#include "Features/Soul/Storage/Crypto/Cipher/CipherAdapters.h"

#include "Core/fp.hpp"
#include "Features/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Features/Soul/Storage/Crypto/Encoding/EncodingAdapters.h"

#define UI OPENSSL_UI
THIRD_PARTY_INCLUDES_START
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
THIRD_PARTY_INCLUDES_END
#undef UI

#include <memory>

namespace SoulStorage::Crypto::Cipher {
namespace {

using FCipherContext =
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

/**
 * User Story: As a Soul cryptographic operator, I need OpenSSL errors retained
 * at the cipher boundary without leaking private bytes.
 * @fn FString cipherOpenSslErrorAdapter(const FString &Prefix)
 */
FString cipherOpenSslErrorAdapter(const FString &Prefix) {
  const unsigned long Code = ERR_get_error();
  ANSICHAR Buffer[256] = {};
  ERR_error_string_n(Code, Buffer, sizeof(Buffer));
  return Prefix + TEXT(": ") + UTF8_TO_TCHAR(Buffer);
}

/**
 * User Story: As a Soul envelope writer, I need cryptographically secure
 * authored-length random bytes for salt and nonce values.
 * @fn bool randomSoulBytesAdapter(int32 Length, TArray<uint8> &Bytes)
 */
bool randomSoulBytesAdapter(int32 Length, TArray<uint8> &Bytes) {
  Bytes.SetNumUninitialized(Length);
  const bool bGenerated = RAND_bytes(Bytes.GetData(), Length) == 1;
  !bGenerated ? Bytes.Reset() : void();
  return bGenerated;
}

/**
 * User Story: As a Soul cipher, I need HKDF-SHA-256 derived exactly from the
 * persisted private exponent, random salt, and authored envelope context.
 * @fn bool deriveSoulKeyAdapter(const FSoulWallet &Wallet, const TArray<uint8> &Salt, TArray<uint8> &Key)
 */
bool deriveSoulKeyAdapter(const FSoulWallet &Wallet,
                          const TArray<uint8> &Salt,
                          TArray<uint8> &Key) {
  TArray<uint8> PrivateMaterial;
  const bool bDecoded =
      Encoding::soulBase64UrlDecodeAdapter(Wallet.D, PrivateMaterial);
  unsigned int PrkLength = EVP_MAX_MD_SIZE;
  TArray<uint8> Prk;
  Prk.SetNumUninitialized(EVP_MAX_MD_SIZE);
  const unsigned char *Extracted =
      bDecoded ? HMAC(EVP_sha256(), Salt.GetData(), Salt.Num(),
                      PrivateMaterial.GetData(), PrivateMaterial.Num(),
                      Prk.GetData(), &PrkLength)
               : nullptr;
  Prk.SetNum(Extracted ? static_cast<int32>(PrkLength) : 0);
  const TArray<uint8> Info = func::append_value<uint8>(
      Encoding::soulUtf8BytesAdapter(
          Configuration::soulStorageData().Encryption.AdditionalData),
      1);
  unsigned int KeyLength = EVP_MAX_MD_SIZE;
  Key.SetNumUninitialized(EVP_MAX_MD_SIZE);
  const unsigned char *Expanded = Extracted
      ? HMAC(EVP_sha256(), Prk.GetData(), Prk.Num(), Info.GetData(),
             Info.Num(), Key.GetData(), &KeyLength)
      : nullptr;
  Key.SetNum(Expanded ? static_cast<int32>(KeyLength) : 0);
  return Expanded && Key.Num() * 8 ==
                         Configuration::soulStorageData().Encryption.KeyLength;
}

/**
 * User Story: As a Soul envelope writer, I need AES-GCM ciphertext and its
 * authentication tag emitted in WebCrypto-compatible byte order.
 * @fn bool encryptSoulBytesAdapter(const TArray<uint8> &Plaintext, const TArray<uint8> &Key, const TArray<uint8> &Iv, TArray<uint8> &Ciphertext)
 */
bool encryptSoulBytesAdapter(const TArray<uint8> &Plaintext,
                             const TArray<uint8> &Key,
                             const TArray<uint8> &Iv,
                             TArray<uint8> &Ciphertext) {
  FCipherContext Context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
  const TArray<uint8> Aad = Encoding::soulUtf8BytesAdapter(
      Configuration::soulStorageData().Encryption.AdditionalData);
  int32 AadLength = 0;
  int32 Written = 0;
  int32 FinalLength = 0;
  TArray<uint8> Encrypted;
  Encrypted.SetNumUninitialized(Plaintext.Num() + EVP_MAX_BLOCK_LENGTH);
  const bool bEncrypted = Context &&
      EVP_EncryptInit_ex(Context.get(), EVP_aes_256_gcm(), nullptr, nullptr,
                         nullptr) == 1 &&
      EVP_CIPHER_CTX_ctrl(Context.get(), EVP_CTRL_GCM_SET_IVLEN, Iv.Num(),
                          nullptr) == 1 &&
      EVP_EncryptInit_ex(Context.get(), nullptr, nullptr, Key.GetData(),
                         Iv.GetData()) == 1 &&
      EVP_EncryptUpdate(Context.get(), nullptr, &AadLength, Aad.GetData(),
                        Aad.Num()) == 1 &&
      EVP_EncryptUpdate(Context.get(), Encrypted.GetData(), &Written,
                        Plaintext.GetData(), Plaintext.Num()) == 1 &&
      EVP_EncryptFinal_ex(Context.get(), Encrypted.GetData() + Written,
                          &FinalLength) == 1;
  Encrypted.SetNum(bEncrypted ? Written + FinalLength : 0);
  TArray<uint8> Tag;
  Tag.SetNumUninitialized(16);
  const bool bTagged = bEncrypted &&
      EVP_CIPHER_CTX_ctrl(Context.get(), EVP_CTRL_GCM_GET_TAG, Tag.Num(),
                          Tag.GetData()) == 1;
  Ciphertext = bTagged
                   ? func::append_values<uint8>(MoveTemp(Encrypted), Tag)
                   : TArray<uint8>();
  return bTagged;
}

/**
 * User Story: As a Soul importer, I need AES-GCM authentication completed
 * before any plaintext bytes are returned.
 * @fn bool decryptSoulBytesAdapter(const TArray<uint8> &Ciphertext, const TArray<uint8> &Key, const TArray<uint8> &Iv, TArray<uint8> &Plaintext)
 */
bool decryptSoulBytesAdapter(const TArray<uint8> &Ciphertext,
                             const TArray<uint8> &Key,
                             const TArray<uint8> &Iv,
                             TArray<uint8> &Plaintext) {
  const int32 TagLength = 16;
  return Ciphertext.Num() <= TagLength
             ? false
             : [&]() {
                 const int32 BodyLength = Ciphertext.Num() - TagLength;
                 const TArray<uint8> Body(Ciphertext.GetData(), BodyLength);
                 const TArray<uint8> Tag(Ciphertext.GetData() + BodyLength,
                                         TagLength);
                 const TArray<uint8> Aad = Encoding::soulUtf8BytesAdapter(
                     Configuration::soulStorageData()
                         .Encryption.AdditionalData);
                 FCipherContext Context(EVP_CIPHER_CTX_new(),
                                        EVP_CIPHER_CTX_free);
                 int32 AadLength = 0;
                 int32 Written = 0;
                 int32 FinalLength = 0;
                 Plaintext.SetNumUninitialized(BodyLength +
                                               EVP_MAX_BLOCK_LENGTH);
                 const bool bDecrypted = Context &&
                     EVP_DecryptInit_ex(Context.get(), EVP_aes_256_gcm(),
                                        nullptr, nullptr, nullptr) == 1 &&
                     EVP_CIPHER_CTX_ctrl(Context.get(),
                                         EVP_CTRL_GCM_SET_IVLEN, Iv.Num(),
                                         nullptr) == 1 &&
                     EVP_DecryptInit_ex(Context.get(), nullptr, nullptr,
                                        Key.GetData(), Iv.GetData()) == 1 &&
                     EVP_DecryptUpdate(Context.get(), nullptr, &AadLength,
                                       Aad.GetData(), Aad.Num()) == 1 &&
                     EVP_DecryptUpdate(Context.get(), Plaintext.GetData(),
                                       &Written, Body.GetData(),
                                       Body.Num()) == 1 &&
                     EVP_CIPHER_CTX_ctrl(
                         Context.get(), EVP_CTRL_GCM_SET_TAG, Tag.Num(),
                         const_cast<uint8 *>(Tag.GetData())) == 1 &&
                     EVP_DecryptFinal_ex(Context.get(),
                                         Plaintext.GetData() + Written,
                                         &FinalLength) == 1;
                 Plaintext.SetNum(bDecrypted ? Written + FinalLength : 0);
                 return bDecrypted;
               }();
}

} // namespace

/** User Story: As a storage crypto cipher consumer, I need to invoke encrypt soul adapter through a stable signature so the storage crypto cipher workflow remains explicit and composable. @fn bool encryptSoulAdapter(const TArray<uint8> &Plaintext, const FSoulWallet &Wallet, FSoulEnvelope &Envelope, FString &Error) */
bool encryptSoulAdapter(const TArray<uint8> &Plaintext,
                        const FSoulWallet &Wallet, FSoulEnvelope &Envelope,
                        FString &Error) {
  const Configuration::FEncryptionData &Data =
      Configuration::soulStorageData().Encryption;
  TArray<uint8> Salt;
  TArray<uint8> Iv;
  TArray<uint8> Key;
  TArray<uint8> Ciphertext;
  const bool bEncrypted = randomSoulBytesAdapter(Data.SaltLength, Salt) &&
      randomSoulBytesAdapter(Data.InitializationVectorLength, Iv) &&
      deriveSoulKeyAdapter(Wallet, Salt, Key) &&
      encryptSoulBytesAdapter(Plaintext, Key, Iv, Ciphertext);
  Error = bEncrypted ? FString()
                     : cipherOpenSslErrorAdapter(TEXT("Soul encryption failed"));
  return !bEncrypted
             ? false
             : (Envelope.Version = Data.Version,
                Envelope.Algorithm = Data.AlgorithmLabel,
                Envelope.KeyDerivation = Data.KeyDerivationLabel,
                Envelope.Salt = Encoding::soulBase64UrlEncodeAdapter(Salt),
                Envelope.InitializationVector =
                    Encoding::soulBase64UrlEncodeAdapter(Iv),
                Envelope.Ciphertext =
                    Encoding::soulBase64UrlEncodeAdapter(Ciphertext),
                true);
}

/** User Story: As a storage crypto cipher consumer, I need to invoke decrypt soul adapter through a stable signature so the storage crypto cipher workflow remains explicit and composable. @fn bool decryptSoulAdapter(const FSoulEnvelope &Envelope, const FSoulWallet &Wallet, TArray<uint8> &Plaintext, FString &Error) */
bool decryptSoulAdapter(const FSoulEnvelope &Envelope,
                        const FSoulWallet &Wallet, TArray<uint8> &Plaintext,
                        FString &Error) {
  const Configuration::FEncryptionData &Data =
      Configuration::soulStorageData().Encryption;
  TArray<uint8> Salt;
  TArray<uint8> Iv;
  TArray<uint8> Ciphertext;
  TArray<uint8> Key;
  const bool bEnvelope = Envelope.Version == Data.Version &&
      Envelope.Algorithm == Data.AlgorithmLabel &&
      Envelope.KeyDerivation == Data.KeyDerivationLabel;
  const bool bDecrypted = bEnvelope &&
      Encoding::soulBase64UrlDecodeAdapter(Envelope.Salt, Salt) &&
      Encoding::soulBase64UrlDecodeAdapter(Envelope.InitializationVector, Iv) &&
      Encoding::soulBase64UrlDecodeAdapter(Envelope.Ciphertext, Ciphertext) &&
      deriveSoulKeyAdapter(Wallet, Salt, Key) &&
      decryptSoulBytesAdapter(Ciphertext, Key, Iv, Plaintext);
  Error = bDecrypted
              ? FString()
              : bEnvelope
                    ? Configuration::soulStorageData().Text.DecryptFailed
                    : Configuration::soulStorageData().Text.InvalidEnvelope;
  return bDecrypted;
}

} // namespace SoulStorage::Crypto::Cipher
