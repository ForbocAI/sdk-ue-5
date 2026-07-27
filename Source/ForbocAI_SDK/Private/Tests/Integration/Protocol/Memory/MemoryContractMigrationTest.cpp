#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/Memory/Local/Storage/Sqlite/Connection/ConnectionAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Mutation/MutationAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Query/SqliteQueryAdapters.h"
#include "Systems/Memory/Vector/MemoryVectorAdapters.h"
#include "Systems/Testing/Memory/TestingMemoryAdapters.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite-vec.h"
#include "sqlite3.h"
#endif

using namespace Testing::Memory;

namespace {

#if WITH_FORBOC_SQLITE_VEC
/** User Story: As a native migration verifier, I need authored SQLite control statements executed against a real handle so fixture construction cannot bypass sqlite-vec behavior. @fn func::Either<FString, bool> executeContractSqlAdapter(sqlite3 *Database, const FString &Sql) */
func::Either<FString, bool>
executeContractSqlAdapter(sqlite3 *Database, const FString &Sql) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  char *NativeError = nullptr;
  const FTCHARToUTF8 SqlUtf8(*Sql);
  const int32 Result =
      sqlite3_exec(Database, SqlUtf8.Get(), nullptr, nullptr, &NativeError);
  const FString Error =
      Result == SQLITE_OK
          ? Data.Text.Empty
          : Data.Errors.MemoryContractInvalid +
                (NativeError ? UTF8_TO_TCHAR(NativeError)
                             : UTF8_TO_TCHAR(sqlite3_errmsg(Database)));
  NativeError ? sqlite3_free(NativeError) : void();
  return Result == SQLITE_OK
             ? func::make_right<FString, bool>(true)
             : func::make_left<FString, bool>(Error);
}

/**
 * User Story: As a migration verifier, I need a deterministic non-current vector so successful re-embedding is observable through the production search boundary.
 * @fn TArray<float> legacyContractVectorAdapter( const FMemoryContractTestFixtures &Fixtures)
 */
TArray<float> legacyContractVectorAdapter(
    const FMemoryContractTestFixtures &Fixtures) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  TArray<float> Vector;
  Vector.Init(Fixtures.Numbers.LegacyVectorValue, Data.Vector.Dimension);
  Vector[Data.Iteration.InitialIndex] =
      Fixtures.Numbers.LegacyFirstVectorValue;
  return Vector;
}

/**
 * User Story: As a migration verifier, I need a real sqlite-vec database authored before SDK initialization so legacy and newer contract paths are exercised without mocks.
 * @fn func::Either<FString, bool> createContractDatabaseAdapter( const FString &Path, const FMemoryContractTestFixtures &Fixtures, int32 StoredVersion)
 */
func::Either<FString, bool> createContractDatabaseAdapter(
    const FString &Path, const FMemoryContractTestFixtures &Fixtures,
    int32 StoredVersion) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  sqlite3 *Database = nullptr;
  const FTCHARToUTF8 PathUtf8(*Path);
  const bool bOpened =
      sqlite3_open(PathUtf8.Get(), &Database) == SQLITE_OK && Database;
  if (!bOpened) {
    const FString Error = Data.Errors.SqliteOpenFailed + Path;
    Database ? (sqlite3_close(Database), void()) : void();
    return func::make_left<FString, bool>(Error);
  }

  char *ExtensionError = nullptr;
  const bool bExtension =
      sqlite3_vec_init(Database, &ExtensionError, nullptr) == SQLITE_OK;
  if (!bExtension) {
    const FString Error =
        Data.Errors.SqliteExtensionFailed +
        (ExtensionError ? UTF8_TO_TCHAR(ExtensionError)
                        : UTF8_TO_TCHAR(sqlite3_errmsg(Database)));
    ExtensionError ? sqlite3_free(ExtensionError) : void();
    sqlite3_close(Database);
    return func::make_left<FString, bool>(Error);
  }

  FMemoryItem Item = TypeFactory::MemoryItem(
      Fixtures.Record.Id, Fixtures.Record.Text, Fixtures.Record.Type,
      Fixtures.Record.Importance, Fixtures.Record.Timestamp);
  Item.Embedding = legacyContractVectorAdapter(Fixtures);
  const func::Either<FString, bool> Created = func::ebind(
      executeContractSqlAdapter(Database,
                                Data.Storage.Sqlite.CreateVectorTable),
      [Database, &Data, &Item, StoredVersion](bool) {
        return func::ebind(
            Native::Sqlite::upsert(reinterpret_cast<Native::Sqlite::DB>(Database),
                                   Item),
            [Database, &Data, StoredVersion](const FMemoryItem &) {
              return executeContractSqlAdapter(
                  Database,
                  FString::Format(*Data.Storage.Sqlite.WriteUserVersion,
                                  {StoredVersion}));
            });
      });
  sqlite3_close(Database);
  return Created;
}

/** User Story: As a migration verifier, I need the persisted SQLite contract version read directly so successful opening cannot be mistaken for successful version advancement. @fn func::Either<FString, int32> readContractVersionAdapter(Native::Sqlite::DB Database) */
func::Either<FString, int32>
readContractVersionAdapter(Native::Sqlite::DB Database) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  sqlite3_stmt *Statement = nullptr;
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  const FTCHARToUTF8 SqlUtf8(*Data.Storage.Sqlite.ReadUserVersion);
  const int32 Prepared = sqlite3_prepare_v2(
      Handle, SqlUtf8.Get(), Data.Iteration.SqliteVariableLength, &Statement,
      nullptr);
  if (Prepared != SQLITE_OK || !Statement) {
    Statement ? sqlite3_finalize(Statement) : SQLITE_OK;
    return func::make_left<FString, int32>(
        Data.Errors.SqlitePrepareFailed);
  }
  const int32 Step = sqlite3_step(Statement);
  const int32 Version =
      Step == SQLITE_ROW
          ? sqlite3_column_int(Statement, Data.Iteration.InitialIndex)
          : Data.Contract.LegacyVectorVersion;
  sqlite3_finalize(Statement);
  return Step == SQLITE_ROW
             ? func::make_right<FString, int32>(Version)
             : func::make_left<FString, int32>(Data.Errors.SqliteStepFailed);
}
#endif

/**
 * User Story: As a persistent-memory test, I need an isolated database path per run so native artifacts cannot share state across automation processes.
 * @fn FString contractDatabasePathAdapter( const FMemoryContractTestFixtures &Fixtures)
 */
FString contractDatabasePathAdapter(
    const FMemoryContractTestFixtures &Fixtures) {
  return FPaths::ConvertRelativePathToFull(FPaths::Combine(
      FPaths::ProjectSavedDir(),
      Fixtures.DatabasePrefix +
          FGuid::NewGuid().ToString(EGuidFormats::Digits) +
          MemoryConfiguration::memoryData().Storage.Sqlite.DatabaseExtension));
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMemoryContractMigrationTest, TestingMemoryContractFixtures().Suite,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As an SDK integrator, I need real legacy memories migrated and newer contracts rejected so upgrades preserve data without silently corrupting vectors. @fn bool FMemoryContractMigrationTest::RunTest(const FString &Parameters) */
bool FMemoryContractMigrationTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const FMemoryContractTestFixtures &Fixtures =
      TestingMemoryContractFixtures();
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  const FString LegacyPath = contractDatabasePathAdapter(Fixtures);
  Native::Sqlite::clearPath(LegacyPath);
  const func::Either<FString, bool> LegacyCreated =
      createContractDatabaseAdapter(LegacyPath, Fixtures,
                                    Data.Contract.LegacyVectorVersion);
  TestTrue(Fixtures.Assertions.LegacyCreated, !LegacyCreated.isLeft);

  const func::Either<FString, Native::Sqlite::DB> LegacyOpened =
      Native::Sqlite::open(LegacyPath);
  TestTrue(Fixtures.Assertions.LegacyOpened, !LegacyOpened.isLeft);
  LegacyOpened.isLeft ? AddError(LegacyOpened.left) : void();
  if (!LegacyOpened.isLeft) {
    const Native::Sqlite::DB Database = LegacyOpened.right;
    const func::Either<FString, TArray<FMemoryItem>> Items =
        Native::Sqlite::list(Database, Data.Defaults.ListLimit,
                             Data.Defaults.ListOffset);
    const func::Either<FString, TArray<FMemoryItem>> Search =
        Native::Sqlite::search(
            Database, MemoryVectorAdapters::embed(Fixtures.Record.Text),
            Fixtures.Numbers.ExpectedCount);
    const func::Either<FString, int32> Version =
        readContractVersionAdapter(Database);
    const bool bExpectedItem =
        !Items.isLeft &&
        Items.right.Num() == Fixtures.Numbers.ExpectedCount;
    TestTrue(Fixtures.Assertions.MigratedCount, bExpectedItem);
    TestTrue(Fixtures.Assertions.MigratedText,
             bExpectedItem &&
                 Items.right[Fixtures.Numbers.FirstIndex].Text ==
                     Fixtures.Record.Text);
    const float SimilarityTolerance = FMath::Pow(
        FORBOCAI_SDK_AUTHORED_NUMBERV3B3FBD161C0D, -static_cast<float>(Fixtures.Numbers.SimilarityPrecision));
    TestTrue(
        Fixtures.Assertions.MigratedSimilarity,
        !Search.isLeft &&
            Search.right.Num() == Fixtures.Numbers.ExpectedCount &&
            FMath::IsNearlyEqual(
                Search.right[Fixtures.Numbers.FirstIndex].Similarity,
                Data.Storage.Sqlite.DistanceOrigin, SimilarityTolerance));
    TestTrue(Fixtures.Assertions.MigratedVersion,
             !Version.isLeft &&
                 Version.right == Data.Contract.VectorVersion);
    Native::Sqlite::close(Database);
  }
  Native::Sqlite::clearPath(LegacyPath);

  const FString NewerPath = contractDatabasePathAdapter(Fixtures);
  Native::Sqlite::clearPath(NewerPath);
  const func::Either<FString, bool> NewerCreated =
      createContractDatabaseAdapter(
          NewerPath, Fixtures,
          Data.Contract.VectorVersion + Data.Iteration.Step);
  TestTrue(Fixtures.Assertions.NewerCreated, !NewerCreated.isLeft);
  const func::Either<FString, Native::Sqlite::DB> NewerOpened =
      Native::Sqlite::open(NewerPath);
  const bool bNewerRejected =
      NewerOpened.isLeft &&
      NewerOpened.left.Contains(Data.Errors.MemoryContractNewer);
  TestTrue(Fixtures.Assertions.NewerRejected, bNewerRejected);
  !bNewerRejected && NewerOpened.isLeft ? AddError(NewerOpened.left) : void();
  !NewerOpened.isLeft ? Native::Sqlite::close(NewerOpened.right) : void();
  Native::Sqlite::clearPath(NewerPath);
  return true;
#else
  AddError(Data.Errors.SqliteUnavailable);
  return false;
#endif
}
