#pragma once

#include "Core/RTK/Action/Action.hpp"

namespace rtk {
/**
 * Phase 7: RTK Query
 * User Story: As a maintainer, I need this implementation note so I can understand which milestone behavior the surrounding code is preserving.
 */

enum class QueryStatus {
  uninitialized,
  pending,
  fulfilled,
  rejected,
};

enum class DefinitionType {
  query,
  mutation,
  infinitequery,
};

struct _NEVER {};

struct NamedSchemaError {
  FString name;
  FString message;

  NamedSchemaError() : name(TEXT("NamedSchemaError")) {}
  explicit NamedSchemaError(const FString &Message)
      : name(TEXT("NamedSchemaError")), message(Message) {}
};

struct SkipToken {};
inline const SkipToken skipToken = SkipToken();

typedef SkipToken SkipTokenType;
typedef FString QueryCacheKey;
typedef FString ResponseHandler;

struct SubscriptionOptions {
  int32 pollingInterval;
  bool skipPollingIfUnfocused;
  bool refetchOnReconnect;
  bool refetchOnFocus;

  SubscriptionOptions()
      : pollingInterval(0), skipPollingIfUnfocused(false),
        refetchOnReconnect(false), refetchOnFocus(false) {}
};

struct PrefetchOptions {
  bool force;
  int32 ifOlderThan;

  PrefetchOptions() : force(false), ifOlderThan(0) {}
};

struct StartQueryActionCreatorOptions {
  bool subscribe;
  bool forceRefetch;
  SubscriptionOptions subscriptionOptions;

  StartQueryActionCreatorOptions() : subscribe(true), forceRefetch(false) {}
};

struct RetryOptions {
  int32 maxRetries;

  RetryOptions() : maxRetries(5) {}
};

struct Module {
  FString name;
};

typedef Module CoreModule;
typedef TArray<Module> ApiModules;

inline const TCHAR *coreModuleName = TEXT("coreModule");

inline Module coreModule() {
  Module Value;
  Value.name = coreModuleName;
  return Value;
}

struct FetchArgs {
  FString url;
  FString method;
  FString body;
  ResponseHandler responseHandler;
  TMap<FString, FString> headers;
  TMap<FString, FString> params;
  int32 timeout;

  FetchArgs() : method(TEXT("GET")), timeout(0) {}
};

struct FetchBaseQueryArgs {
  FString baseUrl;
  TMap<FString, FString> headers;
  ResponseHandler responseHandler;
  int32 timeout;

  FetchBaseQueryArgs() : responseHandler(TEXT("json")), timeout(0) {}
};

struct FetchBaseQueryRequest {
  FString url;
  FString method;
  FString body;
  TMap<FString, FString> headers;
};

struct FetchBaseQueryResponse {
  int32 status;
  FString data;
  TMap<FString, FString> headers;

  FetchBaseQueryResponse() : status(0) {}
};

struct FetchBaseQueryMeta {
  FetchBaseQueryRequest request;
  func::Maybe<FetchBaseQueryResponse> response;

  FetchBaseQueryMeta()
      : response(func::nothing<FetchBaseQueryResponse>()) {}
};

struct FetchBaseQueryError {
  FString status;
  int32 statusCode;
  int32 originalStatus;
  FString data;
  FString error;

  FetchBaseQueryError() : statusCode(0), originalStatus(0) {}

  static FetchBaseQueryError httpError(int32 StatusCode, const FString &Data) {
    FetchBaseQueryError Value;
    Value.status = FString::FromInt(StatusCode);
    Value.statusCode = StatusCode;
    Value.data = Data;
    return Value;
  }

  static FetchBaseQueryError fetchError(const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT("FETCH_ERROR");
    Value.error = Error;
    return Value;
  }

  static FetchBaseQueryError parsingError(int32 OriginalStatus,
                                          const FString &Data,
                                          const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT("PARSING_ERROR");
    Value.originalStatus = OriginalStatus;
    Value.data = Data;
    Value.error = Error;
    return Value;
  }

  static FetchBaseQueryError timeoutError(const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT("TIMEOUT_ERROR");
    Value.error = Error;
    return Value;
  }

  static FetchBaseQueryError customError(const FString &Error,
                                         const FString &Data = TEXT("")) {
    FetchBaseQueryError Value;
    Value.status = TEXT("CUSTOM_ERROR");
    Value.data = Data;
    Value.error = Error;
    return Value;
  }
};

struct BaseQueryApi {
  FString endpoint;
  FString type;
  bool forced;
  FString queryCacheKey;

  BaseQueryApi() : type(TEXT("query")), forced(false) {}
};

template <typename T = FString, typename E = FetchBaseQueryError,
          typename M = FetchBaseQueryMeta>
struct QueryReturnValue {
  func::Maybe<E> error;
  func::Maybe<T> data;
  func::Maybe<M> meta;

  QueryReturnValue()
      : error(func::nothing<E>()), data(func::nothing<T>()),
        meta(func::nothing<M>()) {}

  static QueryReturnValue<T, E, M> success(
      const T &Data, const func::Maybe<M> &Meta = func::nothing<M>()) {
    QueryReturnValue<T, E, M> Value;
    Value.data = func::just(Data);
    Value.meta = Meta;
    return Value;
  }

  static QueryReturnValue<T, E, M> failure(
      const E &Error, const func::Maybe<M> &Meta = func::nothing<M>()) {
    QueryReturnValue<T, E, M> Value;
    Value.error = func::just(Error);
    Value.meta = Meta;
    return Value;
  }
};

template <typename Args = FetchArgs, typename Result = FString,
          typename Error = FetchBaseQueryError,
          typename DefinitionExtraOptions = FEmptyPayload,
          typename Meta = FetchBaseQueryMeta>
using BaseQueryFn =
    std::function<func::AsyncResult<QueryReturnValue<Result, Error, Meta>>(
        const Args &, const BaseQueryApi &, const DefinitionExtraOptions &)>;

template <typename BaseQuery> struct BaseQueryTraits {};

template <typename Args, typename Result, typename Error,
          typename DefinitionExtraOptions, typename Meta>
struct BaseQueryTraits<std::function<
    func::AsyncResult<QueryReturnValue<Result, Error, Meta>>(
        const Args &, const BaseQueryApi &, const DefinitionExtraOptions &)>> {
  typedef Args Arg;
  typedef Result ResultType;
  typedef Error ErrorType;
  typedef DefinitionExtraOptions ExtraOptionsType;
  typedef Meta MetaType;
};

template <typename BaseQuery>
using BaseQueryArg = typename BaseQueryTraits<BaseQuery>::Arg;

template <typename BaseQuery>
using BaseQueryResult = typename BaseQueryTraits<BaseQuery>::ResultType;

template <typename BaseQuery>
using BaseQueryError = typename BaseQueryTraits<BaseQuery>::ErrorType;

template <typename BaseQuery>
using BaseQueryExtraOptions =
    typename BaseQueryTraits<BaseQuery>::ExtraOptionsType;

template <typename BaseQuery>
using BaseQueryMeta = typename BaseQueryTraits<BaseQuery>::MetaType;

template <typename AdditionalArgs = FEmptyPayload,
          typename AdditionalDefinitionExtraOptions = FEmptyPayload,
          typename Config = FEmptyPayload>
using BaseQueryEnhancer = std::function<BaseQueryFn<FetchArgs, FString>(
    const BaseQueryFn<FetchArgs, FString> &, const Config &)>;

struct SerializeQueryArgsOptions {
  FString endpointName;
  FString queryArgs;
};

typedef std::function<QueryCacheKey(const SerializeQueryArgsOptions &)>
    SerializeQueryArgs;

inline QueryCacheKey
defaultSerializeQueryArgs(const SerializeQueryArgsOptions &Options) {
  return Options.endpointName + TEXT("(") + Options.queryArgs + TEXT(")");
}

template <typename Arg>
QueryCacheKey defaultSerializeQueryArgs(const FString &EndpointName,
                                        const Arg &QueryArgs) {
  SerializeQueryArgsOptions Options;
  Options.endpointName = EndpointName;
  Options.queryArgs = payload_debug::DebugPayloadString(QueryArgs);
  return defaultSerializeQueryArgs(Options);
}

} // namespace rtk
