#pragma once

#include "CoreMinimal.h"

namespace SoulStorage::Configuration {

struct FRegistryData {
  FString NamespaceValue;
  FString Client;
};

struct FRuntimeTypeData {
  FString Object;
  FString String;
  FString Number;
  FString Undefined;
};

struct FNumberData {
  int32 Zero{};
  int32 One{};
  int32 ByteMask{};
  int32 ByteShift{};
  int32 ZigZagShift{};
  int32 ContinuationMask{};
  int32 ContinuationShift{};
  int32 ContinuationFlag{};
  int32 SignatureTypeLength{};
  int32 TagNumberLength{};
};

struct FEncodingData {
  FString Plus;
  FString Minus;
  FString Slash;
  FString Underscore;
  FString Padding;
  int32 Base64BlockSize{};
  FString PaddingPatternSuffix;
  FString CaseInsensitiveRegexFlag;
};

struct FCatalogFieldData {
  FString Version;
  FString Souls;
};

struct FCatalogEntryFieldData {
  FString TransactionId;
  FString Name;
  FString NpcId;
  FString ExportedAt;
  FString StorageUrl;
  FString Provider;
  FString Digest;
  FString Signature;
};

struct FCatalogData {
  FCatalogFieldData Fields;
  FCatalogEntryFieldData EntryFields;
};

struct FSoulFieldData {
  FString Id;
  FString Version;
  FString Name;
  FString StructuredPersona;
  FString Memories;
  FString State;
};

struct FSoulData {
  FString Version;
  FString DefaultName;
  FSoulFieldData Fields;
};

struct FLifecycleData {
  FString Idle;
  FString Exporting;
  FString Importing;
  FString Succeeded;
  FString Failed;
};

struct FProviderResponseFieldData {
  FString Id;
  FString Owner;
  FString DataCaches;
  FString FastFinalityIndexes;
};

struct FProviderData {
  FString Name;
  FString UploadUrl;
  FString UploadMethod;
  FString UploadContentType;
  FString Accept;
  FString GatewayScheme;
  FString GatewayPathSeparator;
  TArray<FString> RetrievalGateways;
  int32 RequestTimeoutMs{};
  int32 SuccessMinimum{};
  int32 SuccessMaximumExclusive{};
  FString ContentTypeHeader;
  FString AcceptHeader;
  FString GatewayTrailingPattern;
  FString AbsoluteUrlPattern;
  FProviderResponseFieldData ResponseFields;
};

struct FRetrievalRetryData {
  int32 MaximumCycles{};
  int32 InitialAttempt{};
  int32 AttemptStep{};
  int32 DelayMs{};
};

struct FUploadRetryData {
  int32 MaximumAttempts{};
  int32 InitialAttempt{};
  int32 AttemptStep{};
  int32 DelayMs{};
};

struct FWalletFieldData {
  FString KeyType;
  FString PublicExponent;
  FString Modulus;
  FString PrivateExponent;
  FString FirstPrimeFactor;
  FString SecondPrimeFactor;
  FString FirstFactorCrtExponent;
  FString SecondFactorCrtExponent;
  FString FirstCrtCoefficient;
};

struct FWalletData {
  FString Algorithm;
  FString Hash;
  int32 ModulusLength{};
  TArray<int32> PublicExponent;
  int32 SaltLength{};
  FString KeyType;
  FString KeyFormat;
  FWalletFieldData Fields;
  TArray<FString> GenerateUsages;
  TArray<FString> SignUsages;
};

struct FEncryptionFieldData {
  FString Version;
  FString Algorithm;
  FString KeyDerivation;
  FString Salt;
  FString InitializationVector;
  FString Ciphertext;
};

struct FEncryptionData {
  FString Version;
  FString Algorithm;
  FString AlgorithmLabel;
  FString KeyDerivation;
  FString KeyDerivationLabel;
  FString Hash;
  int32 KeyLength{};
  int32 SaltLength{};
  int32 InitializationVectorLength{};
  FString AdditionalData;
  FString KeyFormat;
  TArray<FString> DeriveUsages;
  TArray<FString> CipherUsages;
  FEncryptionFieldData Fields;
};

struct FDataItemTagData {
  FString Name;
  FString Value;
};

struct FDataItemData {
  int32 SignatureType{};
  int32 SignatureLength{};
  int32 OwnerLength{};
  int32 TargetPresent{};
  int32 AnchorPresent{};
  FString DeepHashAlgorithm;
  FString DeepHashList;
  FString DeepHashBlob;
  FString Domain;
  FString Version;
  TArray<FDataItemTagData> Tags;
};

struct FDigestData {
  FString Algorithm;
};

struct FPersistenceData {
  FString Directory;
  FString WalletFile;
  FString CatalogFile;
  FString TemporarySuffix;
  int32 FileMode{};
  int32 DirectoryMode{};
  int32 JsonIndent{};
  FString BrowserWalletKey;
  FString BrowserCatalogKey;
  FString TextEncoding;
  FString ErrorCodeField;
  FString MissingFileCode;
};

struct FTextData {
  FString MissingClient;
  FString MissingPrivateKey;
  FString InvalidOwnerLength;
  FString InvalidSignatureLength;
  FString UploadFailed;
  FString UploadNetworkFailed;
  FString UploadResponseInvalid;
  FString TransactionMismatch;
  FString PendingUploadMissing;
  FString RetrievalFailed;
  FString RetrievalNetworkFailed;
  FString RetrievalUrlSeparator;
  FString RetrievalMismatch;
  FString ProviderStatusMissing;
  FString InvalidEnvelope;
  FString DecryptFailed;
  FString CatalogEntryMissingPrefix;
  FString CatalogEntryMissingSuffix;
  FString InvalidCatalog;
  FString InvalidWallet;
  FString InvalidSoul;
  FString UnsupportedSoulVersion;
  FString BrowserStorageUnavailable;
  FString ConfirmationMismatch;
  FString VerificationFailed;
  FString ExportFailed;
  FString ImportFailed;
  FString ListFailed;
  FString VerifyFailed;
  FString InvalidTextEncoding;
  FString CleanupFailedSeparator;
  FString ErrorSeparator;
  FString CauseSeparator;
  FString StatusSeparator;
  FString CatalogVersion;
  FString Empty;
};

struct FSoulStorageConfigurationData {
  FRegistryData Registry;
  FRuntimeTypeData RuntimeTypes;
  FNumberData Numbers;
  FEncodingData Encoding;
  FCatalogData Catalog;
  FSoulData Soul;
  FLifecycleData Lifecycle;
  FProviderData Provider;
  FRetrievalRetryData Retrieval;
  FUploadRetryData Upload;
  FWalletData Wallet;
  FEncryptionData Encryption;
  FDataItemData DataItem;
  FDigestData Digest;
  FPersistenceData Persistence;
  FTextData Text;
};

} // namespace SoulStorage::Configuration
