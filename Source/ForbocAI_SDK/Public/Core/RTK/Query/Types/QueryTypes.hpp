#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/RTK/Action/Action.hpp"
#include "Core/RTK/Query/Body/BodyTypes.hpp"

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

  /** User Story: As a rtk query types consumer, I need to invoke named schema error through a stable signature so the rtk query types workflow remains explicit and composable. @fn NamedSchemaError() */
  NamedSchemaError() : name(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA1D17E255B84)) {}
  /** User Story: As a rtk query types consumer, I need to invoke named schema error through a stable signature so the rtk query types workflow remains explicit and composable. @fn explicit NamedSchemaError(const FString &Message) */
  explicit NamedSchemaError(const FString &Message)
      : name(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA1D17E255B84)), message(Message) {}
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

  /** User Story: As a rtk query types consumer, I need to invoke subscription options through a stable signature so the rtk query types workflow remains explicit and composable. @fn SubscriptionOptions() */
  SubscriptionOptions()
      : pollingInterval(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), skipPollingIfUnfocused(false),
        refetchOnReconnect(false), refetchOnFocus(false) {}
};

struct PrefetchOptions {
  bool force;
  int32 ifOlderThan;

  /** User Story: As a rtk query types consumer, I need to invoke prefetch options through a stable signature so the rtk query types workflow remains explicit and composable. @fn PrefetchOptions() */
  PrefetchOptions() : force(false), ifOlderThan(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

struct StartQueryActionCreatorOptions {
  bool subscribe;
  bool forceRefetch;
  SubscriptionOptions subscriptionOptions;

  /** User Story: As a rtk query types consumer, I need to invoke start query action creator options through a stable signature so the rtk query types workflow remains explicit and composable. @fn StartQueryActionCreatorOptions() */
  StartQueryActionCreatorOptions() : subscribe(true), forceRefetch(false) {}
};

struct RetryOptions {
  int32 maxRetries;

  /** User Story: As a rtk query types consumer, I need to invoke retry options through a stable signature so the rtk query types workflow remains explicit and composable. @fn RetryOptions() */
  RetryOptions() : maxRetries(FORBOCAI_SDK_AUTHORED_NUMBERV2B61CCD40B6E) {}
};

struct Module {
  FString name;
};

typedef Module CoreModule;
typedef TArray<Module> ApiModules;

inline const TCHAR *coreModuleName = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV27131D726108);

/** User Story: As a rtk query types consumer, I need to invoke core module through a stable signature so the rtk query types workflow remains explicit and composable. @fn inline Module coreModule() */
inline Module coreModule() {
  Module Value;
  Value.name = coreModuleName;
  return Value;
}

struct FetchArgs {
  FString url;
  FString method;
  FetchBody body;
  ResponseHandler responseHandler;
  TMap<FString, FString> headers;
  TMap<FString, FString> params;
  int32 timeout;

  /** User Story: As a rtk query types consumer, I need to invoke fetch args through a stable signature so the rtk query types workflow remains explicit and composable. @fn FetchArgs() */
  FetchArgs() : method(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9FB15D43C73B)), timeout(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

struct FetchBaseQueryArgs {
  FString baseUrl;
  TMap<FString, FString> headers;
  ResponseHandler responseHandler;
  int32 timeout;

  /** User Story: As a rtk query types consumer, I need to invoke fetch base query args through a stable signature so the rtk query types workflow remains explicit and composable. @fn FetchBaseQueryArgs() */
  FetchBaseQueryArgs() : responseHandler(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7ED033B4CF87)), timeout(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

struct FetchBaseQueryRequest {
  FString url;
  FString method;
  FetchBody body;
  TMap<FString, FString> headers;
};

struct FetchBaseQueryResponse {
  int32 status;
  FString data;
  TMap<FString, FString> headers;

  /** User Story: As a rtk query types consumer, I need to invoke fetch base query response through a stable signature so the rtk query types workflow remains explicit and composable. @fn FetchBaseQueryResponse() */
  FetchBaseQueryResponse() : status(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}
};

struct FetchBaseQueryMeta {
  FetchBaseQueryRequest request;
  func::Maybe<FetchBaseQueryResponse> response;

  /** User Story: As a rtk query types consumer, I need to invoke fetch base query meta through a stable signature so the rtk query types workflow remains explicit and composable. @fn FetchBaseQueryMeta() */
  FetchBaseQueryMeta()
      : response(func::nothing<FetchBaseQueryResponse>()) {}
};

struct FetchBaseQueryError {
  FString status;
  int32 statusCode;
  int32 originalStatus;
  FString data;
  FString error;

  /** User Story: As a rtk query types consumer, I need to invoke fetch base query error through a stable signature so the rtk query types workflow remains explicit and composable. @fn FetchBaseQueryError() */
  FetchBaseQueryError() : statusCode(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA), originalStatus(FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA) {}

  /** User Story: As a rtk query types consumer, I need to invoke http error through a stable signature so the rtk query types workflow remains explicit and composable. @fn static FetchBaseQueryError httpError(int32 StatusCode, const FString &Data) */
  static FetchBaseQueryError httpError(int32 StatusCode, const FString &Data) {
    FetchBaseQueryError Value;
    Value.status = FString::FromInt(StatusCode);
    Value.statusCode = StatusCode;
    Value.data = Data;
    return Value;
  }

  /** User Story: As a rtk query types consumer, I need to invoke fetch error through a stable signature so the rtk query types workflow remains explicit and composable. @fn static FetchBaseQueryError fetchError(const FString &Error) */
  static FetchBaseQueryError fetchError(const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV99D7AD1D0B62);
    Value.error = Error;
    return Value;
  }

  /** User Story: As a rtk query types consumer, I need to invoke parsing error through a stable signature so the rtk query types workflow remains explicit and composable. @fn static FetchBaseQueryError parsingError(int32 OriginalStatus, const FString &Data, const FString &Error) */
  static FetchBaseQueryError parsingError(int32 OriginalStatus,
                                          const FString &Data,
                                          const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6CD4A181637C);
    Value.originalStatus = OriginalStatus;
    Value.data = Data;
    Value.error = Error;
    return Value;
  }

  /** User Story: As a rtk query types consumer, I need to invoke timeout error through a stable signature so the rtk query types workflow remains explicit and composable. @fn static FetchBaseQueryError timeoutError(const FString &Error) */
  static FetchBaseQueryError timeoutError(const FString &Error) {
    FetchBaseQueryError Value;
    Value.status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV91EFC04D6057);
    Value.error = Error;
    return Value;
  }

  /** User Story: As a rtk query types consumer, I need to invoke custom error through a stable signature so the rtk query types workflow remains explicit and composable. @fn static FetchBaseQueryError customError(const FString &Error, const FString &Data = TEXT("")) */
  static FetchBaseQueryError customError(const FString &Error,
                                         const FString &Data = TEXT("")) {
    FetchBaseQueryError Value;
    Value.status = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV00A275DB8CB9);
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

  /** User Story: As a rtk query types consumer, I need to invoke base query api through a stable signature so the rtk query types workflow remains explicit and composable. @fn BaseQueryApi() */
  BaseQueryApi() : type(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV32006DA77DEF)), forced(false) {}
};

template <typename T = FString, typename E = FetchBaseQueryError,
          typename M = FetchBaseQueryMeta>
struct QueryReturnValue {
  func::Maybe<E> error;
  func::Maybe<T> data;
  func::Maybe<M> meta;

  /** User Story: As a rtk query types consumer, I need to invoke query return value through a stable signature so the rtk query types workflow remains explicit and composable. @fn QueryReturnValue() */
  QueryReturnValue()
      : error(func::nothing<E>()), data(func::nothing<T>()),
        meta(func::nothing<M>()) {}

  /** User Story: As a rtk query types consumer, I need to invoke success through a stable signature so the rtk query types workflow remains explicit and composable. @fn static QueryReturnValue<T, E, M> success( const T &Data, const func::Maybe<M> &Meta = func::nothing<M>()) */
  static QueryReturnValue<T, E, M> success(
      const T &Data, const func::Maybe<M> &Meta = func::nothing<M>()) {
    QueryReturnValue<T, E, M> Value;
    Value.data = func::just(Data);
    Value.meta = Meta;
    return Value;
  }

  /** User Story: As a rtk query types consumer, I need to invoke failure through a stable signature so the rtk query types workflow remains explicit and composable. @fn static QueryReturnValue<T, E, M> failure( const E &Error, const func::Maybe<M> &Meta = func::nothing<M>()) */
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

} // namespace rtk
