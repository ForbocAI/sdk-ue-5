#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Soul/Storage/Configuration/ConfigurationTypes.h"

namespace SoulStorage::Configuration {

/**
 * User Story: As a Soul retrieval adapter, I need the gateway-cycle retry
 * policy decoded from authored data so dynamic gateway counts cannot shorten it.
 * @fn inline FRetrievalRetryData readRetrievalRetryData(const TSharedRef<FJsonObject> &Object)
 */
inline FRetrievalRetryData
readRetrievalRetryData(const TSharedRef<FJsonObject> &Object) {
  return {DataAdapters::ReadNumberField(Object, TEXT("maximumCycles")),
          DataAdapters::ReadNumberField(Object, TEXT("initialAttempt")),
          DataAdapters::ReadNumberField(Object, TEXT("attemptStep")),
          DataAdapters::ReadNumberField(Object, TEXT("delayMs"))};
}

/**
 * User Story: As a Soul upload adapter, I need the request retry policy decoded
 * separately from retrieval cycles so each transport owns precise semantics.
 * @fn inline FUploadRetryData readUploadRetryData(const TSharedRef<FJsonObject> &Object)
 */
inline FUploadRetryData
readUploadRetryData(const TSharedRef<FJsonObject> &Object) {
  return {DataAdapters::ReadNumberField(Object, TEXT("maximumAttempts")),
          DataAdapters::ReadNumberField(Object, TEXT("initialAttempt")),
          DataAdapters::ReadNumberField(Object, TEXT("attemptStep")),
          DataAdapters::ReadNumberField(Object, TEXT("delayMs"))};
}

/**
 * User Story: As an ANS-104 serializer, I need every authored tag translated
 * through one pure adapter so signatures are deterministic across SDKs.
 * @fn inline TArray<FDataItemTagData> readDataItemTags(const TSharedRef<FJsonObject> &Object)
 */
inline TArray<FDataItemTagData>
readDataItemTags(const TSharedRef<FJsonObject> &Object) {
  return func::map_array<TSharedPtr<FJsonObject>, FDataItemTagData>(
      DataAdapters::ReadObjectArrayField(Object, TEXT("tags")),
      [](const TSharedPtr<FJsonObject> &Tag) {
        const TSharedRef<FJsonObject> Value = Tag.ToSharedRef();
        return FDataItemTagData{
            DataAdapters::ReadStringField(Value, TEXT("name")),
            DataAdapters::ReadStringField(Value, TEXT("value"))};
      });
}

/**
 * User Story: As the UE Soul feature, I need the TS-authored storage contract
 * loaded without duplicated transport or cryptographic constants in source.
 * @fn inline FSoulStorageConfigurationData readSoulStorageConfigurationData()
 */
inline FSoulStorageConfigurationData readSoulStorageConfigurationData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/soul/storage.json"));
  const TSharedRef<FJsonObject> Registry =
      DataAdapters::ReadObjectField(Source, TEXT("registry"));
  const TSharedRef<FJsonObject> RuntimeTypes =
      DataAdapters::ReadObjectField(Source, TEXT("runtimeTypes"));
  const TSharedRef<FJsonObject> Numbers =
      DataAdapters::ReadObjectField(Source, TEXT("numbers"));
  const TSharedRef<FJsonObject> Encoding =
      DataAdapters::ReadObjectField(Source, TEXT("encoding"));
  const TSharedRef<FJsonObject> Catalog =
      DataAdapters::ReadObjectField(Source, TEXT("catalog"));
  const TSharedRef<FJsonObject> Soul =
      DataAdapters::ReadObjectField(Source, TEXT("soul"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Soul, TEXT("fields"));
  const TSharedRef<FJsonObject> Lifecycle =
      DataAdapters::ReadObjectField(Source, TEXT("lifecycle"));
  const TSharedRef<FJsonObject> Provider =
      DataAdapters::ReadObjectField(Source, TEXT("provider"));
  const TSharedRef<FJsonObject> ResponseFields =
      DataAdapters::ReadObjectField(Provider, TEXT("responseFields"));
  const TSharedRef<FJsonObject> Wallet =
      DataAdapters::ReadObjectField(Source, TEXT("wallet"));
  const TSharedRef<FJsonObject> Encryption =
      DataAdapters::ReadObjectField(Source, TEXT("encryption"));
  const TSharedRef<FJsonObject> DataItem =
      DataAdapters::ReadObjectField(Source, TEXT("dataItem"));
  const TSharedRef<FJsonObject> Digest =
      DataAdapters::ReadObjectField(Source, TEXT("digest"));
  const TSharedRef<FJsonObject> Persistence =
      DataAdapters::ReadObjectField(Source, TEXT("persistence"));
  const TSharedRef<FJsonObject> Text =
      DataAdapters::ReadObjectField(Source, TEXT("text"));

  return {
      {DataAdapters::ReadStringField(Registry, TEXT("namespace")),
       DataAdapters::ReadStringField(Registry, TEXT("client"))},
      {DataAdapters::ReadStringField(RuntimeTypes, TEXT("object")),
       DataAdapters::ReadStringField(RuntimeTypes, TEXT("string")),
       DataAdapters::ReadStringField(RuntimeTypes, TEXT("number")),
       DataAdapters::ReadStringField(RuntimeTypes, TEXT("undefined"))},
      {DataAdapters::ReadNumberField(Numbers, TEXT("zero")),
       DataAdapters::ReadNumberField(Numbers, TEXT("one")),
       DataAdapters::ReadNumberField(Numbers, TEXT("byteMask")),
       DataAdapters::ReadNumberField(Numbers, TEXT("byteShift")),
       DataAdapters::ReadNumberField(Numbers, TEXT("zigZagShift")),
       DataAdapters::ReadNumberField(Numbers, TEXT("continuationMask")),
       DataAdapters::ReadNumberField(Numbers, TEXT("continuationShift")),
       DataAdapters::ReadNumberField(Numbers, TEXT("continuationFlag")),
       DataAdapters::ReadNumberField(Numbers, TEXT("signatureTypeLength")),
       DataAdapters::ReadNumberField(Numbers, TEXT("tagNumberLength"))},
      {DataAdapters::ReadStringField(Encoding, TEXT("plus")),
       DataAdapters::ReadStringField(Encoding, TEXT("minus")),
       DataAdapters::ReadStringField(Encoding, TEXT("slash")),
       DataAdapters::ReadStringField(Encoding, TEXT("underscore")),
       DataAdapters::ReadStringField(Encoding, TEXT("padding")),
       DataAdapters::ReadNumberField(Encoding, TEXT("base64BlockSize")),
       DataAdapters::ReadStringField(Encoding, TEXT("paddingPatternSuffix")),
       DataAdapters::ReadStringField(Encoding,
                                     TEXT("caseInsensitiveRegexFlag"))},
      {DataAdapters::ReadStringArrayField(Catalog,
                                          TEXT("requiredStringFields"))},
      {DataAdapters::ReadStringField(Soul, TEXT("version")),
       DataAdapters::ReadStringField(Soul, TEXT("defaultName")),
       {DataAdapters::ReadStringField(Fields, TEXT("id")),
        DataAdapters::ReadStringField(Fields, TEXT("version")),
        DataAdapters::ReadStringField(Fields, TEXT("name")),
        DataAdapters::ReadStringField(Fields, TEXT("structuredPersona")),
        DataAdapters::ReadStringField(Fields, TEXT("memories")),
        DataAdapters::ReadStringField(Fields, TEXT("state"))}},
      {DataAdapters::ReadStringField(Lifecycle, TEXT("idle")),
       DataAdapters::ReadStringField(Lifecycle, TEXT("exporting")),
       DataAdapters::ReadStringField(Lifecycle, TEXT("importing")),
       DataAdapters::ReadStringField(Lifecycle, TEXT("succeeded")),
       DataAdapters::ReadStringField(Lifecycle, TEXT("failed"))},
      {DataAdapters::ReadStringField(Provider, TEXT("name")),
       DataAdapters::ReadStringField(Provider, TEXT("uploadUrl")),
       DataAdapters::ReadStringField(Provider, TEXT("uploadMethod")),
       DataAdapters::ReadStringField(Provider, TEXT("uploadContentType")),
       DataAdapters::ReadStringField(Provider, TEXT("accept")),
       DataAdapters::ReadStringField(Provider, TEXT("gatewayScheme")),
       DataAdapters::ReadStringField(Provider, TEXT("gatewayPathSeparator")),
       DataAdapters::ReadStringArrayField(Provider,
                                          TEXT("retrievalGateways")),
       DataAdapters::ReadNumberField(Provider, TEXT("requestTimeoutMs")),
       DataAdapters::ReadNumberField(Provider, TEXT("successMinimum")),
       DataAdapters::ReadNumberField(Provider,
                                     TEXT("successMaximumExclusive")),
       DataAdapters::ReadStringField(Provider, TEXT("contentTypeHeader")),
       DataAdapters::ReadStringField(Provider, TEXT("acceptHeader")),
       DataAdapters::ReadStringField(Provider, TEXT("gatewayTrailingPattern")),
       DataAdapters::ReadStringField(Provider, TEXT("absoluteUrlPattern")),
       {DataAdapters::ReadStringField(ResponseFields, TEXT("id")),
        DataAdapters::ReadStringField(ResponseFields, TEXT("owner")),
        DataAdapters::ReadStringField(ResponseFields, TEXT("dataCaches")),
        DataAdapters::ReadStringField(ResponseFields,
                                      TEXT("fastFinalityIndexes"))}},
      readRetrievalRetryData(
          DataAdapters::ReadObjectField(Source, TEXT("retrieval"))),
      readUploadRetryData(DataAdapters::ReadObjectField(Source, TEXT("upload"))),
      {DataAdapters::ReadStringField(Wallet, TEXT("algorithm")),
       DataAdapters::ReadStringField(Wallet, TEXT("hash")),
       DataAdapters::ReadNumberField(Wallet, TEXT("modulusLength")),
       DataAdapters::ReadNumberArrayField(Wallet, TEXT("publicExponent")),
       DataAdapters::ReadNumberField(Wallet, TEXT("saltLength")),
       DataAdapters::ReadStringField(Wallet, TEXT("keyType")),
       DataAdapters::ReadStringField(Wallet, TEXT("keyFormat")),
       DataAdapters::ReadStringArrayField(Wallet,
                                          TEXT("requiredPrivateFields")),
       DataAdapters::ReadStringArrayField(Wallet, TEXT("generateUsages")),
       DataAdapters::ReadStringArrayField(Wallet, TEXT("signUsages"))},
      {DataAdapters::ReadStringField(Encryption, TEXT("version")),
       DataAdapters::ReadStringField(Encryption, TEXT("algorithm")),
       DataAdapters::ReadStringField(Encryption, TEXT("algorithmLabel")),
       DataAdapters::ReadStringField(Encryption, TEXT("keyDerivation")),
       DataAdapters::ReadStringField(Encryption,
                                     TEXT("keyDerivationLabel")),
       DataAdapters::ReadStringField(Encryption, TEXT("hash")),
       DataAdapters::ReadNumberField(Encryption, TEXT("keyLength")),
       DataAdapters::ReadNumberField(Encryption, TEXT("saltLength")),
       DataAdapters::ReadNumberField(Encryption,
                                     TEXT("initializationVectorLength")),
       DataAdapters::ReadStringField(Encryption, TEXT("additionalData")),
       DataAdapters::ReadStringField(Encryption, TEXT("keyFormat")),
       DataAdapters::ReadStringArrayField(Encryption, TEXT("deriveUsages")),
       DataAdapters::ReadStringArrayField(Encryption, TEXT("cipherUsages")),
       DataAdapters::ReadStringArrayField(Encryption,
                                          TEXT("envelopeFields"))},
      {DataAdapters::ReadNumberField(DataItem, TEXT("signatureType")),
       DataAdapters::ReadNumberField(DataItem, TEXT("signatureLength")),
       DataAdapters::ReadNumberField(DataItem, TEXT("ownerLength")),
       DataAdapters::ReadNumberField(DataItem, TEXT("targetPresent")),
       DataAdapters::ReadNumberField(DataItem, TEXT("anchorPresent")),
       DataAdapters::ReadStringField(DataItem, TEXT("deepHashAlgorithm")),
       DataAdapters::ReadStringField(DataItem, TEXT("deepHashList")),
       DataAdapters::ReadStringField(DataItem, TEXT("deepHashBlob")),
       DataAdapters::ReadStringField(DataItem, TEXT("domain")),
       DataAdapters::ReadStringField(DataItem, TEXT("version")),
       readDataItemTags(DataItem)},
      {DataAdapters::ReadStringField(Digest, TEXT("algorithm"))},
      {DataAdapters::ReadStringField(Persistence, TEXT("directory")),
       DataAdapters::ReadStringField(Persistence, TEXT("walletFile")),
       DataAdapters::ReadStringField(Persistence, TEXT("catalogFile")),
       DataAdapters::ReadStringField(Persistence, TEXT("temporarySuffix")),
       DataAdapters::ReadNumberField(Persistence, TEXT("fileMode")),
       DataAdapters::ReadNumberField(Persistence, TEXT("directoryMode")),
       DataAdapters::ReadNumberField(Persistence, TEXT("jsonIndent")),
       DataAdapters::ReadStringField(Persistence, TEXT("browserWalletKey")),
       DataAdapters::ReadStringField(Persistence, TEXT("browserCatalogKey")),
       DataAdapters::ReadStringField(Persistence, TEXT("textEncoding")),
       DataAdapters::ReadStringField(Persistence, TEXT("errorCodeField")),
       DataAdapters::ReadStringField(Persistence, TEXT("missingFileCode"))},
      {DataAdapters::ReadStringField(Text, TEXT("missingClient")),
       DataAdapters::ReadStringField(Text, TEXT("missingPrivateKey")),
       DataAdapters::ReadStringField(Text, TEXT("invalidOwnerLength")),
       DataAdapters::ReadStringField(Text, TEXT("invalidSignatureLength")),
       DataAdapters::ReadStringField(Text, TEXT("uploadFailed")),
       DataAdapters::ReadStringField(Text, TEXT("uploadNetworkFailed")),
       DataAdapters::ReadStringField(Text, TEXT("uploadResponseInvalid")),
       DataAdapters::ReadStringField(Text, TEXT("transactionMismatch")),
       DataAdapters::ReadStringField(Text, TEXT("pendingUploadMissing")),
       DataAdapters::ReadStringField(Text, TEXT("retrievalFailed")),
       DataAdapters::ReadStringField(Text, TEXT("retrievalNetworkFailed")),
       DataAdapters::ReadStringField(Text, TEXT("retrievalUrlSeparator")),
       DataAdapters::ReadStringField(Text, TEXT("retrievalMismatch")),
       DataAdapters::ReadStringField(Text, TEXT("providerStatusMissing")),
       DataAdapters::ReadStringField(Text, TEXT("invalidEnvelope")),
       DataAdapters::ReadStringField(Text, TEXT("decryptFailed")),
       DataAdapters::ReadStringField(Text, TEXT("catalogEntryMissingPrefix")),
       DataAdapters::ReadStringField(Text, TEXT("catalogEntryMissingSuffix")),
       DataAdapters::ReadStringField(Text, TEXT("invalidCatalog")),
       DataAdapters::ReadStringField(Text, TEXT("invalidWallet")),
       DataAdapters::ReadStringField(Text, TEXT("invalidSoul")),
       DataAdapters::ReadStringField(Text, TEXT("unsupportedSoulVersion")),
       DataAdapters::ReadStringField(Text,
                                     TEXT("browserStorageUnavailable")),
       DataAdapters::ReadStringField(Text, TEXT("confirmationMismatch")),
       DataAdapters::ReadStringField(Text, TEXT("verificationFailed")),
       DataAdapters::ReadStringField(Text, TEXT("exportFailed")),
       DataAdapters::ReadStringField(Text, TEXT("importFailed")),
       DataAdapters::ReadStringField(Text, TEXT("listFailed")),
       DataAdapters::ReadStringField(Text, TEXT("verifyFailed")),
       DataAdapters::ReadStringField(Text, TEXT("invalidTextEncoding")),
       DataAdapters::ReadStringField(Text, TEXT("cleanupFailedSeparator")),
       DataAdapters::ReadStringField(Text, TEXT("errorSeparator")),
       DataAdapters::ReadStringField(Text, TEXT("causeSeparator")),
       DataAdapters::ReadStringField(Text, TEXT("statusSeparator")),
       DataAdapters::ReadStringField(Text, TEXT("catalogVersion")),
       DataAdapters::ReadStringField(Text, TEXT("empty"))}};
}

/**
 * User Story: As every Soul operation, I need one immutable authored storage
 * configuration per process so signatures and persistence cannot drift.
 * @fn inline const FSoulStorageConfigurationData &soulStorageData()
 */
inline const FSoulStorageConfigurationData &soulStorageData() {
  static const FSoulStorageConfigurationData Data =
      readSoulStorageConfigurationData();
  return Data;
}

} // namespace SoulStorage::Configuration
