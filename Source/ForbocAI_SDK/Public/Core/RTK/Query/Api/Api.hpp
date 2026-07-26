#pragma once

#include "Core/FP/Map/Map.hpp"
#include "Core/RTK/Query/Endpoint/Endpoint.hpp"
#include "Core/RTK/Thunk/Thunk.hpp"

namespace rtk {
/**
 * @fn template <typename State> Api<State> createApi(const FString &ReducerPath, const TArray<FString> &TagTypes)
 * @brief Creates an API slice registry with a defined path and tag types.
 * @param ReducerPath The state path where the API slice mounts.
 * @param TagTypes The list of cache tag types supported by this API.
 * @return Api<State> The initialized API definition.
 *
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
template <typename State>
Api<State> createApi(const FString &ReducerPath,
                               const TArray<FString> &TagTypes) {
  Api<State> Slice;
  Slice.ReducerPath = ReducerPath;
  Slice.TagTypes = TagTypes;
  return Slice;
}

/** User Story: As a rtk query api consumer, I need to invoke create api through a stable signature so the rtk query api workflow remains explicit and composable. @fn template <typename State> Api<State> createApi(const CreateApiOptions<State> &Options) */
template <typename State>
Api<State> createApi(const CreateApiOptions<State> &Options) {
  EndpointBuilder<State> Builder;
  Api<State> Slice = createApi<State>(Options.reducerPath, Options.tagTypes);
  Slice.Modules = Options.modules;
  Options.endpoints ? Options.endpoints(Builder) : void();
  return Slice;
}

/** User Story: As a rtk query api consumer, I need to invoke build create api through a stable signature so the rtk query api workflow remains explicit and composable. @fn template <typename State> BuildCreateApi<State> buildCreateApi(const ApiModules &Modules = ApiModules{coreModule()}) */
template <typename State>
BuildCreateApi<State>
buildCreateApi(const ApiModules &Modules = ApiModules{coreModule()}) {
  return [Modules](const CreateApiOptions<State> &Options) {
    CreateApiOptions<State> NextOptions = Options;
    NextOptions.modules = Modules;
    return createApi<State>(NextOptions);
  };
}

/** User Story: As a rtk query api consumer, I need to invoke setup listeners through a stable signature so the rtk query api workflow remains explicit and composable. @fn template <typename State> std::function<void()> setupListeners(Dispatcher Dispatch, const SubscriptionOptions &Options = SubscriptionOptions()) */
template <typename State>
std::function<void()> setupListeners(Dispatcher Dispatch,
                                     const SubscriptionOptions &Options =
                                         SubscriptionOptions()) {
  (void)Dispatch;
  (void)Options;
  return []() {};
}

/**
 * @fn template <typename Result> func::AsyncResult<Result> unwrapEndpointResult(QueryReturnValue<Result> QueryResult)
 * @brief Unwraps an RTK Query return value into an AsyncResult.
 * @param QueryResult The query return value containing data or error.
 * @return func::AsyncResult<Result> The async result that resolves or rejects based on success.
 *
 * User Story: As a developer writing endpoint queries, I need standard HTTP results translated directly into chainable AsyncResults.
 */
template <typename Result>
func::AsyncResult<Result>
unwrapEndpointResult(QueryReturnValue<Result> QueryResult) {
  return QueryResult.data.hasValue
             ? func::createAsyncResult<Result>(
                   [QueryResult](std::function<void(Result)> Resolve,
                                 std::function<void(std::string)> Reject) {
                     Resolve(QueryResult.data.value);
                   })
             : func::createAsyncResult<Result>(
                   [QueryResult](std::function<void(Result)> Resolve,
                                 std::function<void(std::string)> Reject) {
                     const FString Error =
                         QueryResult.error.hasValue
                             ? (!QueryResult.error.value.error.IsEmpty()
                                    ? QueryResult.error.value.error
                                    : (!QueryResult.error.value.data.IsEmpty()
                                           ? QueryResult.error.value.data
                                           : QueryResult.error.value.status))
                             : TEXT("RTK Query endpoint returned no data");
                     Reject(std::string(TCHAR_TO_UTF8(*Error)));
                   });
}

/**
 * @fn template <typename State, typename Arg, typename Result> Api<State> &injectEndpoints(Api<State> &Slice, const ApiEndpoint<Arg, Result, State> &EndpointDesc, bool OverrideExisting = false)
 * @brief Registers a typed endpoint on an existing API slice and returns that same slice.
 * @param Slice The API slice registry to extend.
 * @param EndpointDesc The endpoint descriptor to register.
 * @param OverrideExisting Whether a same-name definition may replace its metadata.
 * @return Api<State> & The same API slice instance with the endpoint registered.
 *
 * User Story: As an API developer, I need injectEndpoints to extend and return
 * the same API object so registration remains separate from request initiation.
 */
template <typename State, typename Arg, typename Result>
Api<State> &injectEndpoints(Api<State> &Slice,
                            const ApiEndpoint<Arg, Result, State> &EndpointDesc,
                            bool OverrideExisting = false) {
  check(!EndpointDesc.EndpointName.IsEmpty());
  const auto IsDeclaredTag = [&Slice](const FApiEndpointTag &Tag) {
    return Tag.Type.IsEmpty() || Slice.TagTypes.Contains(Tag.Type);
  };
  check(func::all_array<FApiEndpointTag>(EndpointDesc.providesTags,
                                         IsDeclaredTag) &&
        func::all_array<FApiEndpointTag>(EndpointDesc.invalidatesTags,
                                         IsDeclaredTag));
  const FApiEndpointMetadata Metadata{
      EndpointDesc.EndpointName, EndpointDesc.Type, EndpointDesc.providesTags,
      EndpointDesc.invalidatesTags};
  const bool ShouldRegister =
      OverrideExisting || !Slice.Endpoints.Contains(EndpointDesc.EndpointName);
  Slice.Endpoints =
      ShouldRegister
          ? func::upsert_map_value<FString, FApiEndpointMetadata>(
                Slice.Endpoints, EndpointDesc.EndpointName, Metadata,
                [Metadata](const FApiEndpointMetadata &) { return Metadata; })
          : Slice.Endpoints;
  return Slice;
}

/**
 * @fn template <typename State, typename Arg, typename Result> AsyncThunkConfig<Result, Arg, State> initiate(const Api<State> &Slice, const ApiEndpoint<Arg, Result, State> &EndpointDesc)
 * @brief Creates the executable lifecycle thunk for a registered endpoint.
 * @param Slice The API slice containing the endpoint registration.
 * @param EndpointDesc The typed endpoint request executor.
 * @return AsyncThunkConfig<Result, Arg, State> The endpoint initiation thunk.
 *
 * User Story: As an RTK Query consumer, I need endpoint initiation separate
 * from injection so registration and execution have the same roles as RTK Query.
 */
template <typename State, typename Arg, typename Result>
AsyncThunkConfig<Result, Arg, State>
initiate(const Api<State> &Slice,
         const ApiEndpoint<Arg, Result, State> &EndpointDesc) {
  check(Slice.Endpoints.Contains(EndpointDesc.EndpointName));
  const FString ThunkPrefix = Slice.ReducerPath + TEXT("/") + EndpointDesc.EndpointName;
  return createAsyncThunk<Result, Arg, State>(
      ThunkPrefix,
      [EndpointDesc](const Arg &arg, const ThunkApi<State> &api)
          -> func::AsyncResult<Result> {
        const ApiContext<State> Context{api.getState, api.dispatch};
        return func::AsyncChain::then<QueryReturnValue<Result>, Result>(
            EndpointDesc.RequestBuilder(arg, Context),
            unwrapEndpointResult<Result>);
      });
}

} // namespace rtk
