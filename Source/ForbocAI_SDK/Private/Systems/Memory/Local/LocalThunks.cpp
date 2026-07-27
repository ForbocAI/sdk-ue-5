#include "Systems/Memory/Local/LocalThunks.h"

#include "Async/Async.h"
#include "Core/fp.hpp"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Memory/Local/LocalAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Connection/ConnectionAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Mutation/MutationAdapters.h"
#include "Systems/Memory/Local/Storage/Sqlite/Query/SqliteQueryAdapters.h"
#include "Entities/Memory/MemorySlice.h"

namespace {

/** User Story: As native memory effects, I need one scoped connection combinator so every operation closes SQLite on success and failure. @fn template <typename Result, typename Effect> func::Either<FString, Result> withDatabase(const FString &DatabaseName, Effect RunEffect) */
template <typename Result, typename Effect>
func::Either<FString, Result> withDatabase(const FString &DatabaseName,
                                           Effect RunEffect) {
  const auto Paths =
      MemoryLocalAdapters::resolveMemoryDatabasePathsAdapter(DatabaseName);
  return func::ematch(
      Paths,
      [](const FString &Error) {
        return func::make_left<FString, Result>(Error);
      },
      [&RunEffect](const MemoryLocalTypes::FMemoryDatabasePaths &Resolved) {
        return func::ematch(
            Native::Sqlite::open(Resolved.DatabasePath),
            [](const FString &Error) {
              return func::make_left<FString, Result>(Error);
            },
            [&RunEffect](Native::Sqlite::DB Database) {
              const func::Either<FString, Result> ResultValue =
                  RunEffect(Database);
              Native::Sqlite::close(Database);
              return ResultValue;
            });
      });
}

/** User Story: As RTK thunk rejection, I need FString errors converted once at the async boundary so native details survive promise propagation. @fn std::string toErrorString(const FString &Error) */
std::string toErrorString(const FString &Error) {
  return std::string(TCHAR_TO_UTF8(*Error));
}

/** User Story: As native memory persistence, I need embedding and SQLite upsert shared by new writes and imports so those workflows cannot drift. @fn func::Either<FString, FMemoryItem> persistMemoryItem(const FString &DatabaseName, const FMemoryItem &Item) */
func::Either<FString, FMemoryItem>
persistMemoryItem(const FString &DatabaseName, const FMemoryItem &Item) {
  const FMemoryItem Record =
      MemoryLocalAdapters::createMemoryVectorRecordAdapter(Item);
  return withDatabase<FMemoryItem>(
      DatabaseName, [&Record](Native::Sqlite::DB Database) {
        return Native::Sqlite::upsert(Database, Record);
      });
}

} // namespace

namespace rtk {

/** User Story: As native memory setup, I need an NPC-scoped database verified before use so callers do not manage SQLite handles. @fn ThunkAction<MemoryLocalTypes::FMemoryDatabasePaths, FRuntimeState> initNodeMemoryThunk(const FString &DatabaseName) */
ThunkAction<MemoryLocalTypes::FMemoryDatabasePaths, FRuntimeState>
initNodeMemoryThunk(const FString &DatabaseName) {
  return [DatabaseName](std::function<AnyAction(const AnyAction &)>,
                        std::function<const FRuntimeState &()>) {
    return func::AsyncResult<MemoryLocalTypes::FMemoryDatabasePaths>::create(
        [DatabaseName](
            std::function<void(MemoryLocalTypes::FMemoryDatabasePaths)> Resolve,
            std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread, [DatabaseName, Resolve, Reject]() {
            const auto Paths = MemoryLocalAdapters::
                resolveMemoryDatabasePathsAdapter(DatabaseName);
            const auto Opened = Paths.isLeft
                                    ? func::make_left<FString,
                                                      Native::Sqlite::DB>(
                                          Paths.left)
                                    : Native::Sqlite::open(
                                          Paths.right.DatabasePath);
            Opened.isLeft ? void() : Native::Sqlite::close(Opened.right);
            AsyncTask(ENamedThreads::GameThread,
                      [Paths, Opened, Resolve, Reject]() {
                        func::ematch(
                            Opened,
                            [Reject](const FString &Error) {
                              Reject(toErrorString(Error));
                            },
                            [Paths, Resolve](Native::Sqlite::DB) {
                              Resolve(Paths.right);
                            });
                      });
          });
        });
  };
}

/** User Story: As native memory persistence, I need new records created and stored through one RTK effect boundary. @fn ThunkAction<FMemoryItem, FRuntimeState> storeNodeMemoryThunk(const FString &Text, const FString &Type, float Importance, const FString &DatabaseName) */
ThunkAction<FMemoryItem, FRuntimeState>
storeNodeMemoryThunk(const FString &Text, const FString &Type,
                     float Importance, const FString &DatabaseName) {
  return upsertNodeMemoryThunk(
      MemoryLocalAdapters::createMemoryItemAdapter(Text, Type, Importance),
      DatabaseName);
}

/** User Story: As native memory import, I need existing records upserted without changing identity or chronology. @fn ThunkAction<FMemoryItem, FRuntimeState> upsertNodeMemoryThunk(const FMemoryItem &Item, const FString &DatabaseName) */
ThunkAction<FMemoryItem, FRuntimeState>
upsertNodeMemoryThunk(const FMemoryItem &Item, const FString &DatabaseName) {
  return [Item, DatabaseName](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const FRuntimeState &()>) {
    Dispatch(MemorySlice::Actions::memoryStoreStart());
    return func::AsyncResult<FMemoryItem>::create(
        [Item, DatabaseName, Dispatch](std::function<void(FMemoryItem)> Resolve,
                                       std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread,
                [Item, DatabaseName, Dispatch, Resolve, Reject]() {
                  const auto Stored = persistMemoryItem(DatabaseName, Item);
                  AsyncTask(ENamedThreads::GameThread,
                            [Stored, Dispatch, Resolve, Reject]() {
                              func::ematch(
                                  Stored,
                                  [Dispatch, Reject](const FString &Error) {
                                    Dispatch(MemorySlice::Actions::
                                                 memoryStoreFailed(Error));
                                    Reject(toErrorString(Error));
                                  },
                                  [Dispatch, Resolve](
                                      const FMemoryItem &StoredItem) {
                                    Dispatch(MemorySlice::Actions::
                                                 memoryStoreSuccess(
                                                     StoredItem));
                                    Resolve(StoredItem);
                                  });
                            });
                });
        });
  };
}

/** User Story: As native memory recall, I need semantic search scoped to one NPC database and validated before Redux receives results. @fn ThunkAction<TArray<FMemoryItem>, FRuntimeState> recallNodeMemoryThunk(const FString &Query, int32 Limit, float Threshold, const FString &DatabaseName) */
ThunkAction<TArray<FMemoryItem>, FRuntimeState>
recallNodeMemoryThunk(const FString &Query, int32 Limit, float Threshold,
                      const FString &DatabaseName) {
  return [Query, Limit, Threshold, DatabaseName](
             std::function<AnyAction(const AnyAction &)> Dispatch,
             std::function<const FRuntimeState &()>) {
    Dispatch(MemorySlice::Actions::memoryRecallStart());
    return func::AsyncResult<TArray<FMemoryItem>>::create(
        [Query, Limit, Threshold, DatabaseName, Dispatch](
            std::function<void(TArray<FMemoryItem>)> Resolve,
            std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread,
                [Query, Limit, Threshold, DatabaseName, Dispatch, Resolve,
                 Reject]() {
                  const auto &Data = MemoryConfiguration::memoryData();
                  const auto Results = Query.TrimStartAndEnd().IsEmpty()
                                           ? func::make_left<
                                                 FString,
                                                 TArray<FMemoryItem>>(
                                                 Data.Errors.EmptyQuery)
                                           : withDatabase<TArray<FMemoryItem>>(
                                                 DatabaseName,
                                                 [&Query, Limit](
                                                     Native::Sqlite::DB Db) {
                                                   return Native::Sqlite::
                                                       search(
                                                           Db,
                                                           MemoryVectorAdapters::
                                                               embed(Query),
                                                           Limit);
                                                 });
                  const auto Filtered = Results.isLeft
                                            ? Results
                                            : func::make_right<
                                                  FString,
                                                  TArray<FMemoryItem>>(
                                                  func::filter_array<
                                                      FMemoryItem>(
                                                      Results.right,
                                                      [Threshold](
                                                          const FMemoryItem
                                                              &Item) {
                                                        return Item.Similarity >=
                                                               Threshold;
                                                      }));
                  AsyncTask(ENamedThreads::GameThread,
                            [Filtered, Dispatch, Resolve, Reject]() {
                              func::ematch(
                                  Filtered,
                                  [Dispatch, Reject](const FString &Error) {
                                    Dispatch(MemorySlice::Actions::
                                                 memoryRecallFailed(Error));
                                    Reject(toErrorString(Error));
                                  },
                                  [Dispatch, Resolve](
                                      const TArray<FMemoryItem> &Items) {
                                    Dispatch(MemorySlice::Actions::
                                                 memoryRecallSuccess(Items));
                                    Resolve(Items);
                                  });
                            });
                });
        });
  };
}

/** User Story: As native memory inspection, I need persisted pages read directly from the NPC database so diagnostics do not depend on cache history. @fn ThunkAction<TArray<FMemoryItem>, FRuntimeState> listNodeMemoryThunk(int32 Limit, int32 Offset, const FString &DatabaseName) */
ThunkAction<TArray<FMemoryItem>, FRuntimeState>
listNodeMemoryThunk(int32 Limit, int32 Offset, const FString &DatabaseName) {
  return [Limit, Offset, DatabaseName](
             std::function<AnyAction(const AnyAction &)>,
             std::function<const FRuntimeState &()>) {
    return func::AsyncResult<TArray<FMemoryItem>>::create(
        [Limit, Offset, DatabaseName](
            std::function<void(TArray<FMemoryItem>)> Resolve,
            std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread,
                [Limit, Offset, DatabaseName, Resolve, Reject]() {
                  const auto Results = withDatabase<TArray<FMemoryItem>>(
                      DatabaseName, [Limit, Offset](Native::Sqlite::DB Db) {
                        return Native::Sqlite::list(Db, Limit, Offset);
                      });
                  AsyncTask(ENamedThreads::GameThread,
                            [Results, Resolve, Reject]() {
                              Results.isLeft
                                  ? Reject(toErrorString(Results.left))
                                  : Resolve(Results.right);
                            });
                });
        });
  };
}

/** User Story: As native memory lifecycle, I need all database artifacts removed before Redux clears its entities. @fn ThunkAction<FEmptyPayload, FRuntimeState> clearNodeMemoryThunk(const FString &DatabaseName) */
ThunkAction<FEmptyPayload, FRuntimeState>
clearNodeMemoryThunk(const FString &DatabaseName) {
  return [DatabaseName](std::function<AnyAction(const AnyAction &)> Dispatch,
                        std::function<const FRuntimeState &()>) {
    return func::AsyncResult<FEmptyPayload>::create(
        [DatabaseName, Dispatch](std::function<void(FEmptyPayload)> Resolve,
                                 std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread,
                [DatabaseName, Dispatch, Resolve, Reject]() {
                  const auto Paths = MemoryLocalAdapters::
                      resolveMemoryDatabasePathsAdapter(DatabaseName);
                  const auto Cleared = Paths.isLeft
                                           ? func::make_left<FString, bool>(
                                                 Paths.left)
                                           : Native::Sqlite::clearPath(
                                                 Paths.right.DatabasePath);
                  AsyncTask(ENamedThreads::GameThread,
                            [Cleared, Dispatch, Resolve, Reject]() {
                              func::ematch(
                                  Cleared,
                                  [Reject](const FString &Error) {
                                    Reject(toErrorString(Error));
                                  },
                                  [Dispatch, Resolve](bool) {
                                    Dispatch(
                                        MemorySlice::Actions::memoryClear());
                                    Resolve(FEmptyPayload{});
                                  });
                            });
                });
        });
  };
}

} // namespace rtk
