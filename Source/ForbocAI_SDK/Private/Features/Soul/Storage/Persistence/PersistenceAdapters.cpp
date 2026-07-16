#include "Features/Soul/Storage/Persistence/PersistenceAdapters.h"

#include "Features/Soul/Storage/Configuration/ConfigurationAdapters.h"
#include "Features/Soul/Storage/Serialization/SerializationAdapters.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SoulStorage::Persistence {
namespace {

struct FSoulPersistencePaths {
  FString Directory;
  FString Wallet;
  FString Catalog;
};

/** User Story: As UE Soul persistence, I need wallet and catalog paths resolved from the current user profile so CLI and game processes share one identity portably. @fn FSoulPersistencePaths soulPersistencePathsAdapter() */
FSoulPersistencePaths soulPersistencePathsAdapter() {
  const Configuration::FPersistenceData &Data =
      Configuration::soulStorageData().Persistence;
  const FString Directory =
      FPaths::Combine(FPlatformProcess::UserSettingsDir(), Data.Directory);
  return {Directory, FPaths::Combine(Directory, Data.WalletFile),
          FPaths::Combine(Directory, Data.CatalogFile)};
}

/** User Story: As a Soul persistence reader, I need absent files distinguished from malformed or inaccessible documents. @fn bool readSoulDocumentAdapter(const FString &Path, FString &Json, bool &bExists, FString &Error) */
bool readSoulDocumentAdapter(const FString &Path, FString &Json,
                             bool &bExists, FString &Error) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  bExists = PlatformFile.FileExists(*Path);
  const bool bRead = !bExists || FFileHelper::LoadFileToString(Json, *Path);
  Error = bRead ? FString()
                : FString::Printf(TEXT("Unable to read Soul persistence: %s"),
                                  *Path);
  return bRead;
}

/** User Story: As a Soul persistence writer, I need same-directory atomic replacement so interrupted writes cannot publish partial key or catalog JSON. @fn bool writeSoulDocumentAdapter(const FString &Directory, const FString &Path, const FString &Json, FString &Error) */
bool writeSoulDocumentAdapter(const FString &Directory, const FString &Path,
                              const FString &Json, FString &Error) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const Configuration::FPersistenceData &Data =
      Configuration::soulStorageData().Persistence;
  const FString Temporary =
      Path + Data.TemporarySuffix +
      FString::FromInt(FPlatformProcess::GetCurrentProcessId());
  const bool bDirectory = PlatformFile.CreateDirectoryTree(*Directory);
  const bool bWritten = bDirectory &&
      FFileHelper::SaveStringToFile(Json, *Temporary,
                                    FFileHelper::EEncodingOptions::ForceUTF8);
  const bool bMoved = bWritten &&
      IFileManager::Get().Move(*Path, *Temporary, true, true);
  !bMoved && PlatformFile.FileExists(*Temporary)
      ? PlatformFile.DeleteFile(*Temporary)
      : false;
  Error = bMoved
              ? FString()
              : FString::Printf(TEXT("Unable to write Soul persistence: %s"),
                                *Path);
  return bMoved;
}

} // namespace

/** User Story: As a soul storage persistence consumer, I need to invoke read soul wallet adapter through a stable signature so the soul storage persistence workflow remains explicit and composable. @fn bool readSoulWalletAdapter(FSoulWallet &Wallet, bool &bExists, FString &Error) */
bool readSoulWalletAdapter(FSoulWallet &Wallet, bool &bExists,
                           FString &Error) {
  const FSoulPersistencePaths Paths = soulPersistencePathsAdapter();
  FString Json;
  return readSoulDocumentAdapter(Paths.Wallet, Json, bExists, Error) &&
         (!bExists ||
          Serialization::normalizeSoulWalletAdapter(Json, Wallet, Error));
}

/** User Story: As a soul storage persistence consumer, I need to invoke write soul wallet adapter through a stable signature so the soul storage persistence workflow remains explicit and composable. @fn bool writeSoulWalletAdapter(const FSoulWallet &Wallet, FString &Error) */
bool writeSoulWalletAdapter(const FSoulWallet &Wallet, FString &Error) {
  const FSoulPersistencePaths Paths = soulPersistencePathsAdapter();
  return writeSoulDocumentAdapter(
      Paths.Directory, Paths.Wallet,
      Serialization::encodeSoulWalletAdapter(Wallet), Error);
}

/** User Story: As a soul storage persistence consumer, I need to invoke read soul catalog adapter through a stable signature so the soul storage persistence workflow remains explicit and composable. @fn bool readSoulCatalogAdapter(FSoulCatalog &Catalog, FString &Error) */
bool readSoulCatalogAdapter(FSoulCatalog &Catalog, FString &Error) {
  const FSoulPersistencePaths Paths = soulPersistencePathsAdapter();
  FString Json;
  bool bExists = false;
  const bool bRead = readSoulDocumentAdapter(Paths.Catalog, Json, bExists,
                                             Error);
  return !bRead
             ? false
             : !bExists
                   ? (Catalog.Version =
                          Configuration::soulStorageData().Text.CatalogVersion,
                      Catalog.Souls.Reset(), true)
                   : Serialization::normalizeSoulCatalogAdapter(Json, Catalog,
                                                                Error);
}

/** User Story: As a soul storage persistence consumer, I need to invoke write soul catalog adapter through a stable signature so the soul storage persistence workflow remains explicit and composable. @fn bool writeSoulCatalogAdapter(const FSoulCatalog &Catalog, FString &Error) */
bool writeSoulCatalogAdapter(const FSoulCatalog &Catalog, FString &Error) {
  const FSoulPersistencePaths Paths = soulPersistencePathsAdapter();
  return writeSoulDocumentAdapter(
      Paths.Directory, Paths.Catalog,
      Serialization::encodeSoulCatalogAdapter(Catalog), Error);
}

} // namespace SoulStorage::Persistence
