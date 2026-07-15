#pragma once

#include "Core/RTK/Query/Endpoint/Endpoint.hpp"
#include "Core/RTK/Thunk/Thunk.hpp"

namespace rtk {
/**
 * @brief Creates an API slice registry with a defined path and tag types.
 * @signature template <typename State> Api<State> createApi(const FString &ReducerPath, const TArray<FString> &TagTypes)
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

template <typename State>
Api<State> createApi(const CreateApiOptions<State> &Options) {
  EndpointBuilder<State> Builder;
  Api<State> Slice = createApi<State>(Options.reducerPath, Options.tagTypes);
  Slice.Modules = Options.modules;
  Options.endpoints ? Options.endpoints(Builder) : void();
  return Slice;
}

template <typename State>
BuildCreateApi<State>
buildCreateApi(const ApiModules &Modules = ApiModules{coreModule()}) {
  return [Modules](const CreateApiOptions<State> &Options) {
    CreateApiOptions<State> NextOptions = Options;
    NextOptions.modules = Modules;
    return createApi<State>(NextOptions);
  };
}

template <typename State>
std::function<void()> setupListeners(Dispatcher Dispatch,
                                     const SubscriptionOptions &Options =
                                         SubscriptionOptions()) {
  (void)Dispatch;
  (void)Options;
  return []() {};
}

/**
 * @brief Unwraps an RTK Query return value into an AsyncResult.
 * @signature template <typename Result> func::AsyncResult<Result> unwrapEndpointResult(QueryReturnValue<Result> QueryResult)
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
                                    : QueryResult.error.value.status)
                             : TEXT("RTK Query endpoint returned no data");
                     Reject(std::string(TCHAR_TO_UTF8(*Error)));
                   });
}

/**
 * @brief Injects a specific endpoint into an existing API slice definition.
 * @signature template <typename State, typename Arg, typename Result> AsyncThunkConfig<Result, Arg, State> injectEndpoints(const Api<State> &Slice, const ApiEndpoint<Arg, Result> &EndpointDesc)
 * @param Slice The base API slice.
 * @param EndpointDesc The endpoint descriptor.
 * @return AsyncThunkConfig<Result, Arg, State> A configured thunk managing the endpoint request lifecycle.
 *
 * User Story: As an API developer, I need to dynamically inject endpoints into a base API slice without altering the slice core.
 */
template <typename State, typename Arg, typename Result>
AsyncThunkConfig<Result, Arg, State>
injectEndpoints(const Api<State> &Slice,
               const ApiEndpoint<Arg, Result> &EndpointDesc) {
  const FString ThunkPrefix = Slice.ReducerPath + TEXT("/") + EndpointDesc.EndpointName;
  return createAsyncThunk<Result, Arg, State>(
      ThunkPrefix,
      [EndpointDesc](const Arg &arg, const ThunkApi<State> &api)
          -> func::AsyncResult<Result> {
        return func::AsyncChain::then<QueryReturnValue<Result>, Result>(
            EndpointDesc.RequestBuilder(arg), unwrapEndpointResult<Result>);
      });
}

} // namespace rtk
