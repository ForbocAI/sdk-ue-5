#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

#include "Async/Async.h"
#include "Core/SdkVectorizer.h"
#include "Features/API/APISlice.h"
#include "Features/Memory/MemorySlice.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "NativeStorage.h"

namespace rtk {
namespace detail {

/**
 * Returns the singleton handle backing the local sqlite memory database.
 * User Story: As node-memory thunks, I need a shared database handle so local
 * memory operations reuse one opened connection.
 */
inline Native::Sqlite::DB &NodeMemoryHandle() {
  static Native::Sqlite::DB Handle = nullptr;
  return Handle;
}

/**
 * Returns the project-local infrastructure root used by native runtimes.
 * User Story: As local runtime setup, I need one canonical infrastructure root
 * so vector assets resolve from one place.
 */
inline FString GetLocalInfrastructureDir() {
  return FPaths::ProjectDir() + TEXT("local_infrastructure/");
}

/**
 * Returns the default sqlite database path for node-backed memory.
 * User Story: As node-memory setup, I need a canonical database location so
 * local memory storage initializes predictably.
 */
inline FString DefaultNodeMemoryPath() {
  return GetLocalInfrastructureDir() + TEXT("vectors/forbocai_vectors.db");
}

/**
 * Returns the mutable storage holding the active node-memory database path.
 * User Story: As node-memory setup, I need one mutable path slot so config and
 * initialization code share the active database target.
 */
inline FString &NodeMemoryPathStorage() {
  static FString Path = DefaultNodeMemoryPath();
  return Path;
}

/**
 * Returns the current node-memory database handle.
 * User Story: As node-memory helpers, I need one place to read the active
 * database handle so storage and recall use the same connection.
 */
inline Native::Sqlite::DB EnsureNodeMemoryDatabase() {
  return NodeMemoryHandle();
}

/**
 * Builds a persisted memory item from a memory-store instruction.
 * User Story: As node-memory store thunks, I need instructions converted into
 * persisted memory items so local storage uses a complete record shape.
 */
inline FMemoryItem MakeMemoryItem(const FMemoryStoreInstruction &Instruction) {
  FMemoryItem Item;
  Item.Id = FGuid::NewGuid().ToString();
  Item.Text = Instruction.Text;
  Item.Type = Instruction.Type.IsEmpty() ? TEXT("observation")
                                         : Instruction.Type;
  Item.Importance = Instruction.Importance;
  Item.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
  Item.Embedding = ForbocAI::SDK::Vectorizer::Embed(Instruction.Text);
  return Item;
}

} // namespace detail

/**
 * Forward declarations
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
inline ThunkAction<FMemoryItem, FRuntimeState>
nodeMemoryStoreThunk(const FMemoryItem &Item);

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
nodeMemoryRecallThunk(const FMemoryRecallRequest &Request);

/**
 * User Story: As memory persistence setup, I need validated DB/table paths so
 * vector storage cannot escape the intended infrastructure directory. (From TS)
 */

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
initNodeMemoryThunk(const FString &DatabasePath = TEXT("")) {
  return [DatabasePath](std::function<AnyAction(const AnyAction &)> Dispatch,
                        std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return func::AsyncResult<rtk::FEmptyPayload>::create(
        [DatabasePath](std::function<void(rtk::FEmptyPayload)> Resolve,
                       std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread, [DatabasePath, Resolve, Reject]() {
            Native::Sqlite::DB &Handle = detail::NodeMemoryHandle();
            Handle
                ? (Native::Sqlite::Close(Handle), (void)(Handle = nullptr))
                : (void)0;

            const FString Path = DatabasePath.IsEmpty()
                                     ? detail::DefaultNodeMemoryPath()
                                     : DatabasePath;
            detail::NodeMemoryPathStorage() = Path;
            Handle = Native::Sqlite::Open(Path);

            AsyncTask(ENamedThreads::GameThread, [Handle, Resolve, Reject]() {
              Handle
                  ? (Resolve(rtk::FEmptyPayload{}), void())
                  : (Reject("Failed to initialize node memory database"),
                     void());
            });
          });
        });
  };
}

inline ThunkAction<FMemoryItem, FRuntimeState>
nodeMemoryStoreThunk(const FMemoryItem &Item) {
  return [Item](std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<FMemoryItem> {
    Dispatch(MemorySlice::Actions::memoryStoreStart());

    return func::AsyncResult<FMemoryItem>::create(
        [Item, Dispatch](std::function<void(FMemoryItem)> Resolve,
                         std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread, [Item, Dispatch, Resolve, Reject]() {
            Native::Sqlite::DB Db = detail::EnsureNodeMemoryDatabase();
            !Db
                ? [&]() {
                    const FString Error =
                        TEXT("Local memory is not initialized");
                    AsyncTask(ENamedThreads::GameThread,
                              [Dispatch, Reject, Error]() {
                                Dispatch(MemorySlice::Actions::
                                             memoryStoreFailed(Error));
                                Reject(TCHAR_TO_UTF8(*Error));
                              });
                  }()
	                : [&]() {
	                    FMemoryItem Stored = Item;
	                    Stored.Embedding =
	                        Stored.Embedding.Num() > 0
	                            ? Stored.Embedding
	                            : ForbocAI::SDK::Vectorizer::Embed(Stored.Text);
	                    const bool bStored =
	                        Native::Sqlite::Upsert(Db, Stored, Stored.Embedding);

                    AsyncTask(
                        ENamedThreads::GameThread,
                        [Dispatch, Resolve, Reject, Stored, bStored]() {
                          bStored
                              ? (Dispatch(MemorySlice::Actions::
                                              memoryStoreSuccess(Stored)),
                                 Resolve(Stored), void())
                              : [&]() {
                                  const FString Error =
                                      TEXT("Failed to store local memory");
                                  Dispatch(MemorySlice::Actions::
                                               memoryStoreFailed(Error));
                                  Reject(TCHAR_TO_UTF8(*Error));
                                }();
                        });
                  }();
          });
        });
  };
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
nodeMemoryRecallThunk(const FMemoryRecallRequest &Request) {
  return [Request](std::function<AnyAction(const AnyAction &)> Dispatch,
                   std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FMemoryItem>> {
    Dispatch(MemorySlice::Actions::memoryRecallStart());

    return func::AsyncResult<TArray<FMemoryItem>>::create(
        [Request, Dispatch](std::function<void(TArray<FMemoryItem>)> Resolve,
                            std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread, [Request, Dispatch, Resolve,
                                          Reject]() {
            Native::Sqlite::DB Db = detail::EnsureNodeMemoryDatabase();
            !Db
                ? [&]() {
                    const FString Error =
                        TEXT("Local memory is not initialized");
                    AsyncTask(ENamedThreads::GameThread,
                              [Dispatch, Reject, Error]() {
                                Dispatch(MemorySlice::Actions::
                                             memoryRecallFailed(Error));
                                Reject(TCHAR_TO_UTF8(*Error));
                              });
                  }()
	                : [&]() {
	                    const TArray<float> QueryEmbedding =
	                        ForbocAI::SDK::Vectorizer::Embed(Request.Query);
	                    TArray<FMemoryItem> Results =
                        Native::Sqlite::Search(Db, QueryEmbedding,
                                               Request.Limit);

                    Request.Threshold > 0.0f
                        ? (void)Results.RemoveAll(
                              [Request](const FMemoryItem &Item) {
                                return Item.Similarity < Request.Threshold;
                              })
                        : (void)0;

                    AsyncTask(ENamedThreads::GameThread,
                              [Dispatch, Resolve, Results]() {
                                Dispatch(MemorySlice::Actions::
                                             memoryRecallSuccess(Results));
                                Resolve(Results);
                              });
                  }();
          });
        });
  };
}

/**
 * Convenience wrappers
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
inline ThunkAction<FMemoryItem, FRuntimeState>
storeNodeMemoryThunk(const FString &Text,
                     const FString &Type = TEXT("observation"),
                     float Importance = 0.5f) {
  FMemoryStoreInstruction Instruction;
  Instruction.Text = Text;
  Instruction.Type = Type;
  Instruction.Importance = Importance;
  return nodeMemoryStoreThunk(detail::MakeMemoryItem(Instruction));
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
recallNodeMemoryThunk(const FString &Query, int32 Limit = 10,
                      float Threshold = 0.7f) {
  FMemoryRecallRequest Request;
  Request.Query = Query;
  Request.Limit = Limit;
  Request.Threshold = Threshold;
  return nodeMemoryRecallThunk(Request);
}

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState> clearNodeMemoryThunk() {
  return [](std::function<AnyAction(const AnyAction &)> Dispatch,
            std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return func::AsyncResult<rtk::FEmptyPayload>::create(
        [Dispatch](std::function<void(rtk::FEmptyPayload)> Resolve,
                   std::function<void(std::string)> Reject) {
          Async(EAsyncExecution::Thread, [Dispatch, Resolve]() {
            Native::Sqlite::DB &Handle = detail::NodeMemoryHandle();
            const FString Path = detail::NodeMemoryPathStorage();
            Handle
                ? (Native::Sqlite::Clear(Handle),
                   Native::Sqlite::Close(Handle),
                   (void)(Handle = nullptr))
                : (void)Native::Sqlite::ClearPath(Path);

            IFileManager::Get().Delete(*Path, false, true, true);
            detail::NodeMemoryPathStorage() = detail::DefaultNodeMemoryPath();

            AsyncTask(ENamedThreads::GameThread, [Dispatch, Resolve]() {
              Dispatch(MemorySlice::Actions::memoryClear());
              Resolve(rtk::FEmptyPayload{});
            });
          });
        });
  };
}

/**
 * Remote memory thunks (mirrors TS core thunks.ts)
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
storeMemoryRemoteThunk(const FString &NpcId, const FString &Observation,
                       float Importance = 0.8f) {
  return [NpcId, Observation,
          Importance](std::function<AnyAction(const AnyAction &)> Dispatch,
                      std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return APISlice::Endpoints::postMemoryStore(
        NpcId, TypeFactory::RemoteMemoryStoreRequest(Observation, Importance))(
        Dispatch, GetState);
  };
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
listMemoryRemoteThunk(const FString &NpcId) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FMemoryItem>> {
    return func::AsyncChain::then<TArray<FMemoryItem>, TArray<FMemoryItem>>(
        APISlice::Endpoints::getMemoryList(NpcId)(Dispatch, GetState),
        [Dispatch](const TArray<FMemoryItem> &Items) {
          Dispatch(MemorySlice::Actions::memoryRecallSuccess(Items));
          return detail::ResolveAsync(Items);
        });
  };
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
recallMemoryRemoteThunk(const FString &NpcId, const FString &Query,
                        float Similarity = 0.0f) {
  return [NpcId, Query,
          Similarity](std::function<AnyAction(const AnyAction &)> Dispatch,
                      std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FMemoryItem>> {
    Dispatch(MemorySlice::Actions::memoryRecallStart());
    return func::AsyncChain::then<TArray<FMemoryItem>, TArray<FMemoryItem>>(
               APISlice::Endpoints::postMemoryRecall(
                   NpcId, TypeFactory::RemoteMemoryRecallRequest(
                              Query, Similarity))(Dispatch, GetState),
               [Dispatch](const TArray<FMemoryItem> &Items) {
                 Dispatch(MemorySlice::Actions::memoryRecallSuccess(Items));
                 return detail::ResolveAsync(Items);
               })
        .catch_([Dispatch](std::string Error) {
          Dispatch(MemorySlice::Actions::memoryRecallFailed(
              FString(UTF8_TO_TCHAR(Error.c_str()))));
        });
  };
}

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
clearMemoryRemoteThunk(const FString &NpcId) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return func::AsyncChain::then<rtk::FEmptyPayload, rtk::FEmptyPayload>(
        APISlice::Endpoints::deleteMemoryClear(NpcId)(Dispatch, GetState),
        [Dispatch](const rtk::FEmptyPayload &Payload) {
          Dispatch(MemorySlice::Actions::memoryClear());
          return detail::ResolveAsync(Payload);
        });
  };
}

} // namespace rtk
