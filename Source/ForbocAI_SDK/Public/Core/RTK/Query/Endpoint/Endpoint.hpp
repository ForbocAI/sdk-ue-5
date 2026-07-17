#pragma once

#include "Core/RTK/Query/BaseQuery/BaseQuery.hpp"
#include "Core/RTK/Middleware/Middleware.hpp"

namespace rtk {
struct FApiEndpointTag {
  FString Type;
  FString Id;
};

typedef FApiEndpointTag TagDescription;

template <typename State> struct ApiContext {
  std::function<const State &()> getState;
  Dispatcher dispatch;
};

template <typename Arg, typename Result, typename State = FEmptyPayload>
struct ApiEndpoint {
  FString EndpointName;
  DefinitionType Type;
  TArray<FApiEndpointTag> providesTags;
  TArray<FApiEndpointTag> invalidatesTags;

  /**
   * Abstract request builder/executor
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  std::function<func::AsyncResult<QueryReturnValue<Result>>(
      const Arg &, const ApiContext<State> &)> RequestBuilder;

  /** User Story: As a rtk query endpoint consumer, I need to invoke api endpoint through a stable signature so the rtk query endpoint workflow remains explicit and composable. @fn ApiEndpoint() */
  ApiEndpoint() : Type(DefinitionType::query) {}
};

/**
 * Stores type-erased endpoint registration metadata on an API slice.
 * User Story: As an RTK Query consumer, I need injected endpoint names, kinds,
 * and cache tags inspectable without erasing their typed request executors.
 */
struct FApiEndpointMetadata {
  FString EndpointName;
  DefinitionType Type = DefinitionType::query;
  TArray<FApiEndpointTag> providesTags;
  TArray<FApiEndpointTag> invalidatesTags;
};

template <typename Arg, typename Result, typename State = FEmptyPayload>
using BaseEndpointDefinition = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using EndpointDefinition = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using QueryDefinition = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using MutationDefinition = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using InfiniteQueryDefinition = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using EndpointDefinitions = TArray<ApiEndpoint<Arg, Result, State>>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using ApiEndpointQuery = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using ApiEndpointMutation = ApiEndpoint<Arg, Result, State>;

template <typename Arg, typename Result, typename State = FEmptyPayload>
using ApiEndpointInfiniteQuery = ApiEndpoint<Arg, Result, State>;

template <typename Arg> using QueryArgFrom = Arg;
template <typename Result> using ResultTypeFrom = Result;
template <typename PageParam> using PageParamFrom = PageParam;
template <typename Arg> using InfiniteQueryArgFrom = Arg;

template <typename Result, typename Arg>
using QueryActionCreatorResult = func::AsyncResult<Result>;

template <typename Result, typename Arg>
using MutationActionCreatorResult = func::AsyncResult<Result>;

template <typename Result, typename Arg>
using InfiniteQueryActionCreatorResult = func::AsyncResult<Result>;

template <typename Result> struct QuerySubState {
  QueryStatus status;
  func::Maybe<Result> data;
  func::Maybe<FetchBaseQueryError> error;

  /** User Story: As a rtk query endpoint consumer, I need to invoke query sub state through a stable signature so the rtk query endpoint workflow remains explicit and composable. @fn QuerySubState() */
  QuerySubState() : status(QueryStatus::uninitialized) {}
};

template <typename Result> using MutationResultSelectorResult = QuerySubState<Result>;
template <typename Result> using QueryResultSelectorResult = QuerySubState<Result>;
template <typename Result>
using InfiniteQueryResultSelectorResult = QuerySubState<Result>;
template <typename Result> using InfiniteQuerySubState = QuerySubState<Result>;

template <typename Result, typename PageParam> struct InfiniteData {
  TArray<Result> pages;
  TArray<PageParam> pageParams;
};

template <typename Arg, typename Result, typename State = FEmptyPayload>
using QueryExtraOptions = ApiEndpoint<Arg, Result, State>;
template <typename Arg, typename Result, typename State = FEmptyPayload>
using MutationExtraOptions = ApiEndpoint<Arg, Result, State>;
template <typename Arg, typename Result, typename State = FEmptyPayload>
using InfiniteQueryExtraOptions = ApiEndpoint<Arg, Result, State>;
template <typename Arg, typename Result, typename State = FEmptyPayload>
using InfiniteQueryConfigOptions = ApiEndpoint<Arg, Result, State>;

template <typename State> using RootState = State;
template <typename State, typename QueryState, typename MutationState>
struct CombinedState {
  State root;
  QueryState queries;
  MutationState mutations;
};
template <typename ApiT> using DefinitionsFromApi = ApiT;
template <typename ApiT> using TagTypesFromApi = ApiT;
template <typename DefinitionT, typename Result>
using OverrideResultType = DefinitionT;
template <typename DefinitionsT, typename Result>
using UpdateDefinitions = DefinitionsT;
template <typename T> using TSHelpersId = T;
template <typename T> using TSHelpersNoInfer = T;
template <typename T, typename U> using TSHelpersOverride = U;
template <typename T> using QueryKeys = TArray<QueryCacheKey>;
template <typename Schema> using SchemaType = Schema;
template <typename Error> using SchemaFailureInfo = Error;
template <typename Error> using SchemaFailureHandler = std::function<void(const Error &)>;
template <typename Error>
using SchemaFailureConverter = std::function<NamedSchemaError(const Error &)>;
template <typename Result>
using ResultDescription = TArray<FApiEndpointTag>;
template <typename Arg, typename Result>
using TypedQueryOnQueryStarted =
    std::function<void(const Arg &, const QueryActionCreatorResult<Result, Arg> &)>;
template <typename Arg, typename Result>
using TypedMutationOnQueryStarted =
    std::function<void(const Arg &,
                       const MutationActionCreatorResult<Result, Arg> &)>;

template <typename State> struct EndpointBuilder {
  /** User Story: As a rtk query endpoint consumer, I need to invoke query through a stable signature so the rtk query endpoint workflow remains explicit and composable. @fn template <typename Arg, typename Result> QueryDefinition<Arg, Result, State> query(const QueryDefinition<Arg, Result, State> &Definition) const */
  template <typename Arg, typename Result>
  QueryDefinition<Arg, Result, State>
  query(const QueryDefinition<Arg, Result, State> &Definition) const {
    QueryDefinition<Arg, Result, State> Copy = Definition;
    Copy.Type = DefinitionType::query;
    return Copy;
  }

  /** User Story: As a rtk query endpoint consumer, I need to invoke mutation through a stable signature so the rtk query endpoint workflow remains explicit and composable. @fn template <typename Arg, typename Result> MutationDefinition<Arg, Result, State> mutation(const MutationDefinition<Arg, Result, State> &Definition) const */
  template <typename Arg, typename Result>
  MutationDefinition<Arg, Result, State>
  mutation(const MutationDefinition<Arg, Result, State> &Definition) const {
    MutationDefinition<Arg, Result, State> Copy = Definition;
    Copy.Type = DefinitionType::mutation;
    return Copy;
  }

  /** User Story: As a rtk query endpoint consumer, I need to invoke infinite query through a stable signature so the rtk query endpoint workflow remains explicit and composable. @fn template <typename Arg, typename Result> InfiniteQueryDefinition<Arg, Result, State> infiniteQuery( const InfiniteQueryDefinition<Arg, Result, State> &Definition) const */
  template <typename Arg, typename Result>
  InfiniteQueryDefinition<Arg, Result, State>
  infiniteQuery(
      const InfiniteQueryDefinition<Arg, Result, State> &Definition) const {
    InfiniteQueryDefinition<Arg, Result, State> Copy = Definition;
    Copy.Type = DefinitionType::infinitequery;
    return Copy;
  }
};

/**
 * Simplified dynamic slice registry mapped by string path
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename State> struct Api {
  FString ReducerPath;
  TArray<FString> TagTypes;
  ApiModules Modules;
  TMap<FString, FApiEndpointMetadata> Endpoints;
};

template <typename State> using CreateApi = Api<State>;

template <typename State> struct CreateApiOptions {
  FString reducerPath;
  TArray<FString> tagTypes;
  std::function<void(EndpointBuilder<State> &)> endpoints;
  ApiModules modules;
};

template <typename State>
using BuildCreateApi = std::function<Api<State>(const CreateApiOptions<State> &)>;

} // namespace rtk
