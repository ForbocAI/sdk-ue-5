#pragma once

#include "Async/Async.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "Features/Memory/Local/LocalAdapters.h"
#include "Features/Memory/MemorySlice.h"
#include "HAL/FileManager.h"

namespace rtk {

inline ThunkAction<FMemoryItem, FRuntimeState>
nodeMemoryStoreThunk(const FMemoryItem &Item);

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
nodeMemoryRecallThunk(const FMemoryRecallRequest &Request);

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

} // namespace rtk
